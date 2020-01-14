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
  rowStart       = this->findValueInSettings("ROWstart");
  rowStop        = this->findValueInSettings("ROWstop");
  colStart       = this->findValueInSettings("COLstart");
  colStop        = this->findValueInSettings("COLstop");
  nEvents        = this->findValueInSettings("nEvents");
  doFast         = this->findValueInSettings("DoFast");
  startValue     = 0;
  stopValue      = RD53SharedConstants::NLATENCYBINS*(RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"))+1) - 1;
  doDisplay      = this->findValueInSettings("DisplayHisto");
  doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData = this->findValueInSettings("SaveBinaryData");


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  const size_t nSteps = (stopValue - startValue + 1 <= RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1 ? stopValue - startValue + 1 : RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1);
  const float  step   = (stopValue - startValue + 1) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);


  // ######################
  // # Initialize Latency #
  // ######################
  std::string fileName = fileRes;
  fileName.replace(fileRes.find("_InjectionDelay"),15,"_Latency");
  la.Inherit(this);
  la.initialize(fileName, fileReg);


  // ##############################
  // # Injection register masking #
  // ##############################
  saveInjection = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT")) -
    RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));
  maxDelay      = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));


  // #######################
  // # Initialize progress #
  // #######################
  RD53RunProgress::total() += InjectionDelay::getNumberIterations();
}

void InjectionDelay::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[InjectionDelay::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      this->addFileHandler(std::string(RESULTDIR) + "/InjectionDelayRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  InjectionDelay::run();
  InjectionDelay::analyze();
  InjectionDelay::sendData();
  la.sendData();
}

void InjectionDelay::sendData ()
{
  const size_t InjDelaySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  auto theStream               = prepareChipContainerStreamer<EmptyContainer,GenericDataArray<InjDelaySize>>("Occ"); // @TMP@
  auto theInjectionDelayStream = prepareChipContainerStreamer<EmptyContainer,uint16_t>                      ("InjDelay"); // @TMP@

  if (fStreamerEnabled == true)
    {
      for (const auto cBoard : theOccContainer)            theStream              .streamAndSendBoard(cBoard, fNetworkStreamer);
      for (const auto cBoard : theInjectionDelayContainer) theInjectionDelayStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void InjectionDelay::Stop ()
{
  LOG (INFO) << GREEN << "[InjectionDelay::Stop] Stopping" << RESET;

  this->closeFileHandler();
}

void InjectionDelay::initialize (const std::string fileRes_, const std::string fileReg_)
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::doFast = 1;


  fileRes = fileRes_;
  fileReg = fileReg_;

  InjectionDelay::ConfigureCalibration();
}

void InjectionDelay::run ()
{
  const size_t InjDelaySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;


  // ###############
  // # Run Latency #
  // ###############
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT");
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", val & saveInjection, true);
        }
  la.run();
  la.analyze();
  la.draw();


  ContainerFactory::copyAndInitChip<GenericDataArray<InjDelaySize>>(*fDetectorContainer, theOccContainer);


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
#ifdef __USE_ROOT__
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->CreateResultDirectory(RESULTDIR,false,false);
  this->InitResultFile(fileRes);

  InjectionDelay::initHisto();
  InjectionDelay::fillHisto();
  InjectionDelay::display();
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
          LOG (INFO) << BOLDBLUE << "\t--> InjectionDelay saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
        }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  this->WriteRootFile();
  this->CloseResultFile();
#endif
}

void InjectionDelay::analyze ()
{
  const size_t InjDelaySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theInjectionDelayContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto best   = 0.;
          auto regVal = 0;

          for (auto i = 0u; i < dacList.size(); i++)
            {
              auto current = theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<InjDelaySize>>().data[i];
              if (current > best)
                {
                  regVal = dacList[i];
                  best   = current;
                }
            }

          LOG (INFO) << GREEN << "Best delay for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is "
                     << BOLDYELLOW << regVal << RESET << GREEN << " (1.5625 ns) computed over two bx" << RESET;
          LOG (INFO) << GREEN << "New delay dac value for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is "
                     << BOLDYELLOW << (regVal & maxDelay) << RESET;


          // ####################################################
          // # Fill delay container and download new DAC values #
          // ####################################################
          theInjectionDelayContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = regVal;
          auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT");
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", (val & saveInjection) | (regVal & maxDelay), true);

          auto latency = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG");
          if (regVal / (maxDelay+1) == 0) latency--;
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "LATENCY_CONFIG", latency, true);
          LOG (INFO) << GREEN << "New latency dac value for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is "
                     << BOLDYELLOW << latency << RESET;
        }
}

void InjectionDelay::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}

void InjectionDelay::fillHisto ()
{
#ifdef __USE_ROOT__
  histos.fillOccupancy     (theOccContainer);
  histos.fillInjectionDelay(theInjectionDelayContainer);
#endif
}

void InjectionDelay::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void InjectionDelay::scanDac (const std::string& regName, const std::vector<uint16_t>& dacList, uint32_t nEvents, DetectorDataContainer* theContainer)
{
  const size_t InjDelaySize = RD53::setBits(RD53SharedConstants::MAXBITCHIPREG) + 1;

  for (auto i = 0u; i < dacList.size(); i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      LOG (INFO) << BOLDMAGENTA << ">>> Register value = " << BOLDYELLOW << dacList[i] << BOLDMAGENTA << " <<<" << RESET;
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), regName);
              this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, (val & saveInjection) | (dacList[i] & maxDelay), true);
            }


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
              theContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<InjDelaySize>>().data[i] = occ;
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
          LOG (INFO) << GREEN << "Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "TRIG_CNT        = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "TRIG_CNT")        << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
