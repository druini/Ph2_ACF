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
  rowStart       = this->findValueInSettings("ROWstart");
  rowStop        = this->findValueInSettings("ROWstop");
  colStart       = this->findValueInSettings("COLstart");
  colStop        = this->findValueInSettings("COLstop");
  nEvents        = this->findValueInSettings("nEvents");
  startValue     = this->findValueInSettings("LatencyStart");
  stopValue      = this->findValueInSettings("LatencyStop");
  doDisplay      = this->findValueInSettings("DisplayHisto");
  doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData = this->findValueInSettings("SaveBinaryData");


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  const size_t nSteps = (stopValue - startValue + 1 <= RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1 ? stopValue - startValue + 1 : RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1);
  const float  step   = (stopValue - startValue + 1) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);


  // #######################
  // # Initialize progress #
  // #######################
  RD53RunProgress::total() += Latency::getNumberIterations();
}

void Latency::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[Latency::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      std::string dir(RESULTDIR);
      this->addFileHandler(dir + "/LatencyRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  Latency::run();
  Latency::analyze();
  Latency::sendData();
}

void Latency::sendData ()
{
  const size_t LatencySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  auto theStream        = prepareChipContainerStreamer<EmptyContainer,GenericDataArray<LatencySize>>("Occ"); // @TMP@
  auto theLatencyStream = prepareChipContainerStreamer<EmptyContainer,uint16_t>                     ("Latency"); // @TMP@

  if (fStreamerEnabled == true)
    {
      for (const auto cBoard : theOccContainer)     theStream       .streamAndSendBoard(cBoard, fNetworkStreamer);
      for (const auto cBoard : theLatencyContainer) theLatencyStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void Latency::Stop ()
{
  LOG (INFO) << GREEN << "[Latency::Stop] Stopping" << RESET;

  this->closeFileHandler();
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
  const size_t LatencySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  ContainerFactory::copyAndInitChip<GenericDataArray<LatencySize>>(*fDetectorContainer, theOccContainer);
  Latency::scanDac("LATENCY_CONFIG", dacList, nEvents, &theOccContainer);


  // ################
  // # Error report #
  // ################
  Latency::chipErrorReport();
}

void Latency::draw ()
{
#ifdef __USE_ROOT__
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->CreateResultDirectory(RESULTDIR,false,false);
  this->InitResultFile(fileRes);

  Latency::initHisto();
  Latency::fillHisto();
  Latency::display();
#endif

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
          LOG (INFO) << BOLDBLUE << "\t--> Latency saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
        }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  this->WriteRootFile();
  this->CloseResultFile();
#endif
}

void Latency::analyze ()
{
  const size_t LatencySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theLatencyContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto best   = 0.;
          auto regVal = 0;

          for (auto i = 0u; i < dacList.size(); i++)
            {
              auto current = theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<LatencySize>>().data[i];
              if (current > best)
                {
                  regVal = dacList[i];
                  best   = current;
                }
            }

          LOG (INFO) << GREEN << "Best latency for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is "
                     << BOLDYELLOW << regVal << GREEN << " (n.bx)" << RESET;


          // ######################################################
          // # Fill latency container and download new DAC values #
          // ######################################################
          theLatencyContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", regVal, true);
        }
}

void Latency::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}

void Latency::fillHisto ()
{
#ifdef __USE_ROOT__
  histos.fillOccupancy(theOccContainer);
  histos.fillLatency  (theLatencyContainer);
#endif
}

void Latency::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void Latency::scanDac (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  const size_t LatencySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  for (auto i = 0u; i < dacList.size(); i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      LOG (INFO) << BOLDMAGENTA << ">>> Register value = " << BOLDYELLOW << dacList[i] << BOLDMAGENTA << " <<<" << RESET;
      for (const auto cBoard : *fDetectorContainer)
        this->fReadoutChipInterface->WriteBoardBroadcastChipReg(static_cast<BeBoard*>(cBoard), regName, dacList[i]);


      // ################
      // # Run analysis #
      // ################
      PixelAlive::run();
      auto output = PixelAlive::analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, this->fChannelGroupHandler->allChannelGroup(), 1);


      // ###############
      // # Save output #
      // ###############
      for (const auto cBoard : *output)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              float occ = cChip->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy;
              theContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<LatencySize>>().data[i] = occ;
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
          LOG (INFO) << GREEN << "Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "TRIG_CNT        = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "TRIG_CNT")        << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
