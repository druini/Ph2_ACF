/*!
  \file                  RD53Latency.cc
  \brief                 Implementaion of Latency scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Latency.h"

void Latency::ConfigureCalibration ()
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::ConfigureCalibration();


  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart     = this->findValueInSettings("ROWstart");
  rowStop      = this->findValueInSettings("ROWstop");
  colStart     = this->findValueInSettings("COLstart");
  colStop      = this->findValueInSettings("COLstop");
  nEvents      = this->findValueInSettings("nEvents");
  startValue   = this->findValueInSettings("LatencyStart");
  stopValue    = this->findValueInSettings("LatencyStop");
  doDisplay    = this->findValueInSettings("DisplayHisto");
  doUpdateChip = this->findValueInSettings("UpdateChipCfg");


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  size_t nSteps = stopValue - startValue + 1;
  float step    = (stopValue - startValue + 1) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);
}

void Latency::Start (int currentRun)
{
  Latency::run();
  Latency::analyze();
  Latency::sendData();
}

void Latency::sendData ()
{
  auto theStream        = prepareChannelContainerStreamer<GenericDataVector>("Occ");
  auto theLatencyStream = prepareChannelContainerStreamer<uint16_t>         ("Latency");

  if (fStreamerEnabled == true)
    {
      for (const auto cBoard : theOccContainer)     theStream.streamAndSendBoard(cBoard, fNetworkStreamer);
      for (const auto cBoard : theLatencyContainer) theLatencyStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void Latency::Stop ()
{
  this->Destroy();
}

void Latency::initialize (const std::string fileRes_, const std::string fileReg_)
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::doFast = 1;


  fileRes = fileRes_;
  fileReg = fileReg_;

  Latency::ConfigureCalibration();
}

void Latency::run ()
{
  ContainerFactory::copyAndInitChip<GenericDataVector>(*fDetectorContainer, theOccContainer);
  Latency::scanDac("LATENCY_CONFIG", dacList, nEvents, &theOccContainer);


  // ################
  // # Error report #
  // ################
  Latency::chipErrorReport();
}

void Latency::draw ()
{
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->CreateResultDirectory(RESULTDIR,false,false);
  this->InitResultFile(fileRes);

  Latency::initHisto();
  Latency::fillHisto();
  Latency::display();

  // ######################################
  // # Save or Update register new values #
  // ######################################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          static_cast<RD53*>(cChip)->copyMaskFromDefault();
          if (doUpdateChip == true) static_cast<RD53*>(cChip)->saveRegMap("");
          static_cast<RD53*>(cChip)->saveRegMap(fileReg);
          std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
          system(command.c_str());
          LOG (INFO) << BOLDGREEN << "\t--> Latency saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
        }

  if (doDisplay == true) myApp->Run(true);
  this->WriteRootFile();
  this->CloseResultFile();
}

void Latency::analyze ()
{
  ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theLatencyContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto best   = 0.;
          auto regVal = 0;

          for (auto dac : dacList)
            {
              auto current = theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector>().data1[dac-startValue];
              if (current > best)
                {
                  regVal = dac;
                  best   = current;
                }
            }

          LOG (INFO) << BOLDGREEN << "\t--> Best latency for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is "
                     << BOLDYELLOW << regVal << BOLDGREEN << " (n.bx)" << RESET;


          // ######################################################
          // # Fill latency container and download new DAC values #
          // ######################################################
          theLatencyContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", regVal, true);
        }
}

void Latency::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void Latency::fillHisto ()
{
  histos.fillOccupancy(theOccContainer);
  histos.fillLatency  (theLatencyContainer);
}
void Latency::display   () { histos.process(); }

void Latency::scanDac (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  for (auto dac : dacList)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      LOG (INFO) << BOLDMAGENTA << ">>> Register value = " << BOLDYELLOW << dac << BOLDMAGENTA << " <<<" << RESET;
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, dac, true);


      // ################
      // # Run analysis #
      // ################
      PixelAlive::run();
      auto output = PixelAlive::analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);


      // ###############
      // # Save output #
      // ###############
      for (const auto cBoard : *output)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              float occ = cChip->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy;
              theContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector>().data1.push_back(occ);
            }
    }
}

void Latency::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
