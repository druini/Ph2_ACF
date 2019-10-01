/*!
  \file                  RD53InjectionDelay.cc
  \brief                 Implementaion of Injection Delay scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53InjectionDelay.h"

void InjectionDelay::ConfigureCalibration ()
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::ConfigureCalibration();


  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart   = this->findValueInSettings("ROWstart");
  rowStop    = this->findValueInSettings("ROWstop");
  colStart   = this->findValueInSettings("COLstart");
  colStop    = this->findValueInSettings("COLstop");
  nEvents    = this->findValueInSettings("nEvents");
  doFast     = this->findValueInSettings("DoFast");
  startValue = this->findValueInSettings("InjDelayStart");
  stopValue  = this->findValueInSettings("InjDelayStop");
  doDisplay  = this->findValueInSettings("DisplayHisto");
  doSave     = this->findValueInSettings("Save");


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  size_t nSteps = stopValue - startValue + 1;
  float step    = (stopValue - startValue + 1) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);


  // ######################
  // # Initialize Latency #
  // ######################
  std::string fileName = fileRes;
  fileName.replace(fileRes.find("_InjectionDelay"),15,"_Latency");
  la.Inherit(this);
  la.initialize(fileName, fileReg);
}

void InjectionDelay::Start (int currentRun)
{
  InjectionDelay::run();
  InjectionDelay::analyze();
  InjectionDelay::sendData();
  la.sendData();
}

void InjectionDelay::sendData ()
{
  auto theStream               = prepareChannelContainerStreamer<GenericDataVector>("Occ");
  auto theInjectionDelayStream = prepareChannelContainerStreamer<uint16_t>         ("InjDelay");

  if (fStreamerEnabled == true)
    {
      for (const auto cBoard : theOccContainer)            theStream.streamAndSendBoard(cBoard, fNetworkStreamer);
      for (const auto cBoard : theInjectionDelayContainer) theInjectionDelayStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void InjectionDelay::Stop ()
{
  this->Destroy();
}

void InjectionDelay::initialize (const std::string fileRes_, const std::string fileReg_)
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::fileRes = fileRes_;
  PixelAlive::fileReg = "";


  fileRes = fileRes_;
  fileReg = fileReg_;

  InjectionDelay::ConfigureCalibration();
}

void InjectionDelay::run ()
{
  // ###############
  // # Run Latency #
  // ###############
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", 0, true);
  la.run();
  la.analyze();
  la.draw();


  ContainerFactory::copyAndInitChip<GenericDataVector>(*fDetectorContainer, theOccContainer);


  // #######################
  // # Set Initial latency #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto latency = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG");
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", latency - 1, true);
        }


  // ###############################
  // # Scan two adjacent latencies #
  // ###############################
  for (auto i = 0; i < 2; i++)
    {
      std::vector<uint16_t> halfDacList(dacList.begin() + i*(dacList.end() - dacList.begin()) / 2, dacList.begin() + (i+1)*(dacList.end() - dacList.begin()) / 2);

      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              auto latency = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG");
              this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", latency + i, true);
            }

      InjectionDelay::scanDac("INJECTION_SELECT", halfDacList, nEvents, &theOccContainer);
    }


  // ################
  // # Error report #
  // ################
  InjectionDelay::chipErrorReport();
}

void InjectionDelay::draw ()
{
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (doSave    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  InjectionDelay::initHisto();
  InjectionDelay::fillHisto();
  InjectionDelay::display();

  if (doSave == true)
    {
      this->WriteRootFile();

      // ############################
      // # Save register new values #
      // ############################
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              static_cast<RD53*>(cChip)->copyMaskFromDefault();
              static_cast<RD53*>(cChip)->saveRegMap(fileReg);
              static_cast<RD53*>(cChip)->saveRegMap("");
              std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
              system(command.c_str());
            }
    }

  if (doDisplay == true) myApp->Run(true);
  if (doSave    == true) this->CloseResultFile();
}

void InjectionDelay::analyze ()
{
  size_t saveVal = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT")) -
    RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));
  size_t maxVal  = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));

  ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theInjectionDelayContainer);

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

          LOG (INFO) << BOLDGREEN << "\t--> Best delay for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is "
                     << BOLDYELLOW << regVal << BOLDGREEN << " (1.5625 ns) computed over two bx" << RESET;
          LOG (INFO) << BOLDGREEN << "\t--> New delay dac value for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is "
                     << BOLDYELLOW << (regVal & maxVal) << RESET;


          // ####################################################
          // # Fill delay container and download new DAC values #
          // ####################################################
          theInjectionDelayContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
          auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT");
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", (val & saveVal) | (regVal & maxVal), true);

          auto latency = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG");
          if (regVal / (maxVal+1) == 0) latency--;
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", latency, true);
          LOG (INFO) << BOLDGREEN << "\t--> New latency dac value for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is "
                     << BOLDYELLOW << latency << RESET;
        }
}

void InjectionDelay::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void InjectionDelay::fillHisto ()
{
  histos.fillOccupancy     (theOccContainer);
  histos.fillInjectionDelay(theInjectionDelayContainer);
}
void InjectionDelay::display   () { histos.process(); }

void InjectionDelay::scanDac (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  size_t saveVal = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits(regName)) -
    RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));
  size_t maxVal  = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));

  for (auto dac : dacList)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      LOG (INFO) << BOLDMAGENTA << ">>> Register value = " << BOLDYELLOW << dac << BOLDMAGENTA << " <<<" << RESET;
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), regName);
              this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, (val & saveVal) | (dac & maxVal), true);
            }


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

void InjectionDelay::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
        }
}