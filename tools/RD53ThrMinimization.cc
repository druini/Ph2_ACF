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
  PixelAlive::doDisplay = false;
  PixelAlive::doSave    = false;


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
  doSave          = this->findValueInSettings("Save");
}

void ThrMinimization::Start (int currentRun)
{
  ThrMinimization::run();
  ThrMinimization::analyze();


  // #############
  // # Send data #
  // #############
  auto theThrStream = prepareChannelContainerStreamer<RegisterValue>();

  if (fStreamerEnabled == true)
    for (const auto cBoard : theThrContainer) theThrStream.streamAndSendBoard(cBoard, fNetworkStreamer);
}

void ThrMinimization::Stop ()
{
  this->Destroy();
}

void ThrMinimization::initialize (const std::string fileRes_, const std::string fileReg_)
{
  // ##############################
  // # Initialize sub-calibration #
  // ##############################
  PixelAlive::fileRes = fileRes_;
  PixelAlive::fileReg = "";


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
  ContainerFactory::copyAndInitStructure<RegisterValue>(*fDetectorContainer, theThrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        theThrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("Vthreshold_LIN");


  // ################
  // # Error report #
  // ################
  ThrMinimization::chipErrorReport();
}

void ThrMinimization::draw ()
{
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp", nullptr, nullptr);
  if (doSave    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  PixelAlive::draw();

  ThrMinimization::initHisto();
  ThrMinimization::fillHisto();
  ThrMinimization::display();

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

void ThrMinimization::analyze ()
{
  for (const auto cBoard : theThrContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        LOG(INFO) << BOLDGREEN << "\t--> Global threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
                  << cChip->getSummary<RegisterValue>().fRegisterValue << RESET;
}

void ThrMinimization::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void ThrMinimization::fillHisto () { histos.fill(theThrContainer);                                }
void ThrMinimization::display   () { histos.process();                                            }

void ThrMinimization::bitWiseScan (const std::string& regName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
{
  uint16_t numberOfBits = log2(stopValue - startValue + 1) + 1;

  DetectorDataContainer minDACcontainer;
  DetectorDataContainer midDACcontainer;
  DetectorDataContainer maxDACcontainer;

  DetectorDataContainer bestDACcontainer;
  DetectorDataContainer bestContainer;

  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, minDACcontainer);
  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, midDACcontainer);
  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, maxDACcontainer);

  ContainerFactory::copyAndInitStructure<RegisterValue> (*fDetectorContainer, bestDACcontainer);
  ContainerFactory::copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, bestContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = startValue;
          maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = stopValue + 1;

          bestContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<OccupancyAndPh>().fPh = 0;
        }


  for (auto i = 0u; i <= numberOfBits; i++)
    {
      // ###########################
      // # Download new DAC values #
      // ###########################
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
                (minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue +
                 maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue) / 2;

              this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue, true);
            }


      // ################
      // # Run analysis #
      // ################
      PixelAlive::run();
      auto output = PixelAlive::analyze();
      output->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);


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

                  bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
                    midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue;
                }

              if (newValue < target)

                maxDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
                  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue;

              else

                minDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue =
                  midDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue;
            }
    }


  // ###########################
  // # Download new DAC values #
  // ###########################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), regName, bestDACcontainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue, true);


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
          LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
        }
}
