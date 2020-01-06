/*!
  \file                  RD53ThrMinimization.cc
  \brief                 Implementaion of threshold minimization
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ThrMinimization.h"

void ThrMinimization::ConfigureCalibration ()
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::ConfigureCalibration();
  PixelAlive::doDisplay    = false;
  PixelAlive::doUpdateChip = false;


  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart        = this->findValueInSettings("ROWstart");
  rowStop         = this->findValueInSettings("ROWstop");
  colStart        = this->findValueInSettings("COLstart");
  colStop         = this->findValueInSettings("COLstop");
  nEvents         = this->findValueInSettings("nEvents");
  targetOccupancy = this->findValueInSettings("TargetOcc");
  ThrStart        = this->findValueInSettings("ThrStart");
  ThrStop         = this->findValueInSettings("ThrStop");
  doDisplay       = this->findValueInSettings("DisplayHisto");
  doUpdateChip    = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData  = this->findValueInSettings("SaveBinaryData");


  // #######################
  // # Initialize progress #
  // #######################
  RD53RunProgress::total() += ThrMinimization::getNumberIterations();
}

void ThrMinimization::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[ThrMinimization::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      this->addFileHandler(std::string(RESULTDIR) + "/ThrMinimizationRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  ThrMinimization::run();
  ThrMinimization::analyze();
  ThrMinimization::sendData();
}

void ThrMinimization::sendData ()
{
  auto theThrStream = prepareChipContainerStreamer<EmptyContainer,uint16_t>(); // @TMP@

  if (fStreamerEnabled == true)
    for (const auto cBoard : theThrContainer) theThrStream.streamAndSendBoard(cBoard, fNetworkStreamer);
}

void ThrMinimization::Stop ()
{
  LOG (INFO) << GREEN << "[ThrMinimization::Stop] Stopping" << RESET;

  this->closeFileHandler();
}

void ThrMinimization::initialize (const std::string fileRes_, const std::string fileReg_)
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::fileRes = fileRes_;
  PixelAlive::fileReg = fileReg_;


  fileRes = fileRes_;
  fileReg = fileReg_;

  ThrMinimization::ConfigureCalibration();
}

void ThrMinimization::run ()
{
  ThrMinimization::bitWiseScan("Vthreshold_LIN", nEvents, targetOccupancy, ThrStart, ThrStop);


  // ############################
  // # Fill threshold container #
  // ############################
  ContainerFactory::copyAndInitChip<uint16_t>(*fDetectorContainer, theThrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        theThrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<uint16_t>() = static_cast<RD53*>(cChip)->getReg("Vthreshold_LIN");


  // ################
  // # Error report #
  // ################
  ThrMinimization::chipErrorReport();
}

void ThrMinimization::draw ()
{
#ifdef __USE_ROOT__
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp", nullptr, nullptr);

  this->CreateResultDirectory(RESULTDIR,false,false);
  this->InitResultFile(fileRes);

  PixelAlive::draw(false);

  ThrMinimization::initHisto();
  ThrMinimization::fillHisto();
  ThrMinimization::display();
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
          LOG (INFO) << BOLDBLUE << "\t--> ThrMinimization saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
        }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  this->WriteRootFile();
  this->CloseResultFile();
#endif
}

void ThrMinimization::analyze ()
{
  for (const auto cBoard : theThrContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        LOG(INFO) << GREEN << "Global threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is "
                  << BOLDYELLOW << cChip->getSummary<uint16_t>() << RESET;
}

void ThrMinimization::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}
void ThrMinimization::fillHisto ()
{
#ifdef __USE_ROOT__
  histos.fill(theThrContainer);
#endif
}

void ThrMinimization::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void ThrMinimization::bitWiseScan (const std::string& regName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
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
      PixelAlive::run();
      auto output = PixelAlive::analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, this->fChannelGroupHandler->allChannelGroup(), 1);


      // #####################
      // # Compute next step #
      // #####################
      for (const auto cBoard : *output)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              // #######################
              // # Build discriminator #
              // #######################
              float newValue = cChip->getSummary<GenericDataVector, OccupancyAndPh>().fOccupancy;


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

              if (newValue < target)

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
  PixelAlive::run();
  PixelAlive::analyze();
}

void ThrMinimization::chipErrorReport()
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
