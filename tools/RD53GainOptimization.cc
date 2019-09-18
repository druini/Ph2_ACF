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
  Gain::doDisplay = false;
  Gain::doSave    = false;


  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart      = this->findValueInSettings("ROWstart");
  rowStop       = this->findValueInSettings("ROWstop");
  colStart      = this->findValueInSettings("COLstart");
  colStop       = this->findValueInSettings("COLstop");
  nEvents       = this->findValueInSettings("nEvents");
  startValue    = this->findValueInSettings("VCalHstart");
  stopValue     = this->findValueInSettings("VCalHstop");
  targetCharge  = RD53chargeConverter::Charge2VCal(this->findValueInSettings("TargetCharge"));
  KrumCurrStart = this->findValueInSettings("KrumCurrStart");
  KrumCurrStop  = this->findValueInSettings("KrumCurrStop");;
  doFast        = this->findValueInSettings("DoFast");
  doDisplay     = this->findValueInSettings("DisplayHisto");
  doSave        = this->findValueInSettings("Save");
}

void GainOptimization::Start (int currentRun)
{
  GainOptimization::run();
  GainOptimization::analyze();


  // #############
  // # Send data #
  // #############
  auto theKrumStream = prepareChannelContainerStreamer<RegisterValue>();

  if (fStreamerEnabled == true)
    for (const auto cBoard : theKrumCurrContainer) theKrumStream.streamAndSendBoard(cBoard, fNetworkStreamer);
}

void GainOptimization::Stop ()
{
  this->Destroy();
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
  ContainerFactory::copyAndInitStructure<RegisterValue>(*fDetectorContainer, theKrumCurrContainer);
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        theKrumCurrContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<RegisterValue>().fRegisterValue = static_cast<RD53*>(cChip)->getReg("KRUM_CURR_LIN");


  // ################
  // # Error report #
  // ################
  GainOptimization::chipErrorReport();
}

void GainOptimization::draw ()
{
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (doSave    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  Gain::draw();

  GainOptimization::initHisto();
  GainOptimization::fillHisto();
  GainOptimization::display();

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

void GainOptimization::analyze ()
{
  for (const auto cBoard : theKrumCurrContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        LOG(INFO) << BOLDGREEN << "\t--> Krummenacher Current for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
                  << cChip->getSummary<RegisterValue>().fRegisterValue << RESET;
}

void GainOptimization::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void GainOptimization::fillHisto () { histos.fill(theKrumCurrContainer);                           }
void GainOptimization::display   () { histos.process();                                            }

void GainOptimization::bitWiseScan (const std::string& regName, uint32_t nEvents, const float& target, uint16_t startValue, uint16_t stopValue)
{
  // #################################
  // # Number of standard deviations #
  // #################################
  float nStDev = 2;

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
              size_t ToTpoint = RD53::setBits(RD53EvtEncoder::NBIT_TOT/NPIX_REGION) - 2;
              float newValue  = (ToTpoint - cChip->getSummary<GainAndIntercept>().fIntercept) / (cChip->getSummary<GainAndIntercept>().fGain + nStDev*stdDev);


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

              if ((newValue > target) || (newValue < 0))

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
          LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
          LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
          LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
        }
}
