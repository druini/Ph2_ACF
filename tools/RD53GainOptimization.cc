/*!
  \file                  RD53GainOptimization.cc
  \brief                 Implementaion of gain optimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53GainOptimization.h"

void GainOptimization::ConfigureCalibration ()
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  Gain::ConfigureCalibration();
  Gain::doDisplay    = false;
  Gain::doUpdateChip = false;


  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart       = this->findValueInSettings("ROWstart");
  rowStop        = this->findValueInSettings("ROWstop");
  colStart       = this->findValueInSettings("COLstart");
  colStop        = this->findValueInSettings("COLstop");
  nEvents        = this->findValueInSettings("nEvents");
  startValue     = this->findValueInSettings("VCalHstart");
  stopValue      = this->findValueInSettings("VCalHstop");
  targetCharge   = RD53chargeConverter::Charge2VCal(this->findValueInSettings("TargetCharge"));
  KrumCurrStart  = this->findValueInSettings("KrumCurrStart");
  KrumCurrStop   = this->findValueInSettings("KrumCurrStop");;
  doFast         = this->findValueInSettings("DoFast");
  doDisplay      = this->findValueInSettings("DisplayHisto");
  doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData = this->findValueInSettings("SaveBinaryData");


  // #######################
  // # Initialize progress #
  // #######################
  RD53RunProgress::total() += GainOptimization::getNumberIterations();
}

void GainOptimization::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[GainOptimization::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      this->addFileHandler(std::string(RESULTDIR) + "/GainOptimizationRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  GainOptimization::run();
  GainOptimization::analyze();
  GainOptimization::sendData();
}

void GainOptimization::sendData ()
{
  auto theKrumStream = prepareChipContainerStreamer<EmptyContainer,uint16_t>(); // @TMP@

  if (fStreamerEnabled == true)
    for (const auto cBoard : theKrumCurrContainer) theKrumStream.streamAndSendBoard(cBoard, fNetworkStreamer);
}

void GainOptimization::Stop ()
{
  LOG (INFO) << GREEN << "[GainOptimization::Stop] Stopping" << RESET;

  this->closeFileHandler();
}

void GainOptimization::initialize (const std::string fileRes_, const std::string fileReg_)
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  Gain::fileRes = fileRes_;
  Gain::fileReg = fileReg_;


  fileRes = fileRes_;
  fileReg = fileReg_;

  GainOptimization::ConfigureCalibration();
}

void GainOptimization::run ()
{
  GainOptimization::bitWiseScan("KRUM_CURR_LIN", nEvents, targetCharge, KrumCurrStart, KrumCurrStop);


  // #######################################
  // # Fill Krummenacher Current container #
  // #######################################
  ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theKrumCurrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<RD53*>(cChip)->getReg("KRUM_CURR_LIN");


  // ################
  // # Error report #
  // ################
  GainOptimization::chipErrorReport();
}

void GainOptimization::draw ()
{
#ifdef __USE_ROOT__
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->CreateResultDirectory(RESULTDIR,false,false);
  this->InitResultFile(fileRes);

  Gain::draw(false);

  GainOptimization::initHisto();
  GainOptimization::fillHisto();
  GainOptimization::display();
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
          LOG (INFO) << BOLDBLUE << "\t--> GainOptimization saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
        }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  this->WriteRootFile();
  this->CloseResultFile();
#endif
}

void GainOptimization::analyze ()
{
  for (const auto cBoard : theKrumCurrContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        LOG(INFO) << GREEN << "Krummenacher Current for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is "
                  << BOLDYELLOW << cChip->getSummary<uint16_t>() << RESET;
}

void GainOptimization::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}

void GainOptimization::fillHisto ()
{
#ifdef __USE_ROOT__
  histos.fill(theKrumCurrContainer);
#endif
}

void GainOptimization::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void GainOptimization::bitWiseScan (const std::string& regName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
{
  uint16_t init;
  uint16_t numberOfBits = log2(stopValue - startValue + 1) + 1;

  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  DetectorDataContainer bestDACcontainer;
  DetectorDataContainer bestContainer;

  ContainerFactory::copyAndInitChip<uint16_t> (*fDetectorContainer, minDACcontainer, init = startValue);
  ContainerFactory::copyAndInitChip<uint16_t> (*fDetectorContainer, midDACcontainer);
  ContainerFactory::copyAndInitChip<uint16_t> (*fDetectorContainer, maxDACcontainer, init = (stopValue + 1));

  ContainerFactory::copyAndInitChip<uint16_t> (*fDetectorContainer, bestDACcontainer);
  ContainerFactory::copyAndInitChip<OccupancyAndPh>(*fDetectorContainer, bestContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh = 0;


  for (auto i = 0u; i <= numberOfBits; i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() =
                (minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() +
                 maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>()) / 2;

              this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>(), true);
            }


      // ################
      // # Run analysis #
      // ################
      Gain::run();
      auto output = Gain::analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, this->fChannelGroupHandler->allChannelGroup(), 1);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : *output)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              // ##############################################
              // # Search for maximum and build discriminator #
              // ##############################################
              float stdDev = 0;
              size_t cnt   = 0;
              for (auto row = 0u; row < RD53::nRows; row++)
                for (auto col = 0u; col < RD53::nCols; col++)
                  if (cChip->getChannel<GainAndIntercept>(row,col).fGain != 0)
                    {
                      stdDev += cChip->getChannel<GainAndIntercept>(row,col).fGain * cChip->getChannel<GainAndIntercept>(row,col).fGain;
                      cnt++;
                    }
              stdDev = (cnt != 0 ? stdDev/cnt : 0) - cChip->getSummary<GainAndIntercept>().fGain * cChip->getSummary<GainAndIntercept>().fGain;
              stdDev = (stdDev > 0 ? sqrt(stdDev) : 0);
              size_t ToTpoint = RD53::setBits(RD53EvtEncoder::NBIT_TOT/RD53Constants::NPIX_REGION) - 2;
              float newValue  = (ToTpoint - cChip->getSummary<GainAndIntercept>().fIntercept) / (cChip->getSummary<GainAndIntercept>().fGain + NSTDEV*stdDev);


              // ########################
              // # Save best DAC values #
              // ########################
              float oldValue = bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh;
              if (fabs(newValue - target) < fabs(oldValue - target))
                {
                  bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh = newValue;
                  bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() =
                    midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>();
                }

              if ((newValue > target) || (newValue < 0))

                maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() =
                  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>();

              else

                minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() =
                  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>();
            }
    }


  // ###########################
  // # Download new DAC values #
  // ###########################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>(), true);


  // ################
  // # Run analysis #
  // ################
  Gain::run();
  Gain::analyze();
}

void GainOptimization::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          LOG (INFO) << GREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << GREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << std::setfill(' ') << std::setw(8) << "" << RESET;
          LOG (INFO) << BOLDBLUE << "TRIG_CNT        = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "TRIG_CNT")        << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
