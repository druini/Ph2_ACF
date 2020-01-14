/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

void PixelAlive::ConfigureCalibration ()
{
  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart       = this->findValueInSettings("ROWstart");
  rowStop        = this->findValueInSettings("ROWstop");
  colStart       = this->findValueInSettings("COLstart");
  colStop        = this->findValueInSettings("COLstop");
  nEvents        = this->findValueInSettings("nEvents");
  nEvtsBurst     = this->findValueInSettings("nEvtsBurst");
  nTRIGxEvent    = this->findValueInSettings("nTRIGxEvent");
  injType        = this->findValueInSettings("INJtype");
  nHITxCol       = this->findValueInSettings("nHITxCol");
  doFast         = this->findValueInSettings("DoFast");
  thrOccupancy   = this->findValueInSettings("TargetOcc");
  doDisplay      = this->findValueInSettings("DisplayHisto");
  doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData = this->findValueInSettings("SaveBinaryData");

  if (injType != INJtype::None) nTRIGxEvent = 1;
  else                          doFast      = false;


  // ################################
  // # Custom channel group handler #
  // ################################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>(customChannelGroup, injType != INJtype::None ? (doFast == true ? RD53GroupType::OneGroup : RD53GroupType::AllGroups) : RD53GroupType::AllPixels, nHITxCol);
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);


  // ######################
  // # Set injection type #
  // ######################
  size_t inj = 0;
  if (injType == INJtype::Digital) inj = 1 << static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY");
  size_t maxDelay = RD53::setBits(static_cast<RD53*>(fDetectorContainer->at(0)->at(0)->at(0))->getNumberOfBits("INJECTION_SELECT_DELAY"));

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          auto val = this->fReadoutChipInterface->ReadChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT");
          this->fReadoutChipInterface->WriteChipReg(static_cast<RD53*>(cChip), "INJECTION_SELECT", inj | (val & maxDelay), true);
        }


  // #######################
  // # Initialize progress #
  // #######################
  RD53RunProgress::total() += PixelAlive::getNumberIterations();
}

void PixelAlive::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[PixelAlive::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      this->addFileHandler(std::string(RESULTDIR) + "/PixelAliveRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  PixelAlive::run();
  PixelAlive::analyze();
  PixelAlive::sendData();
}

void PixelAlive::sendData ()
{
  const size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  const size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

  auto theOccStream   = prepareChannelContainerStreamer<OccupancyAndPh>                         ("Occ");
  auto theBCIDStream  = prepareChipContainerStreamer<EmptyContainer,GenericDataArray<BCIDsize>> ("BCID"); // @TMP@
  auto theTrgIDStream = prepareChipContainerStreamer<EmptyContainer,GenericDataArray<TrgIDsize>>("TrgID"); // @TMP@

  if (fStreamerEnabled == true)
    {
      for (const auto cBoard : *theOccContainer.get()) theOccStream  .streamAndSendBoard(cBoard, fNetworkStreamer);
      for (const auto cBoard :  theBCIDContainer)      theBCIDStream .streamAndSendBoard(cBoard, fNetworkStreamer);
      for (const auto cBoard :  theTrgIDContainer)     theTrgIDStream.streamAndSendBoard(cBoard, fNetworkStreamer);
    }
}

void PixelAlive::Stop ()
{
  LOG (INFO) << GREEN << "[PixelAlive::Stop] Stopping" << RESET;

  this->closeFileHandler();
}

void PixelAlive::initialize (const std::string fileRes_, const std::string fileReg_)
{
  fileRes = fileRes_;
  fileReg = fileReg_;

  PixelAlive::ConfigureCalibration();
}

void PixelAlive::run ()
{
  theOccContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  this->fDetectorDataContainer = theOccContainer.get();
  ContainerFactory::copyAndInitStructure<OccupancyAndPh,GenericDataVector>(*fDetectorContainer, *this->fDetectorDataContainer);

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(injType);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents, nEvtsBurst, nTRIGxEvent);


  // ################
  // # Error report #
  // ################
  PixelAlive::chipErrorReport();
}

void PixelAlive::draw (bool doSave)
{
#ifdef __USE_ROOT__
  TApplication* myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (doSave    == true)
    {
      this->CreateResultDirectory(RESULTDIR,false,false);
      this->InitResultFile(fileRes);
    }

  PixelAlive::initHisto();
  PixelAlive::fillHisto();
  PixelAlive::display();
#endif

  // #######################################
  // # Save and Update register new values #
  // #######################################
  if (doSave == true)
    {
      for (const auto cBoard : *fDetectorContainer)
        for (const auto cModule : *cBoard)
          for (const auto cChip : *cModule)
            {
              if (doUpdateChip == true) static_cast<RD53*>(cChip)->saveRegMap("");
              static_cast<RD53*>(cChip)->saveRegMap(fileReg);
              std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
              system(command.c_str());
              LOG (INFO) << BOLDBLUE << "\t--> PixelAlive saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
            }
    }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  if (doSave    == true)
    {
      this->WriteRootFile();
      this->CloseResultFile();
    }
#endif
}

std::shared_ptr<DetectorDataContainer> PixelAlive::analyze ()
{
  const size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  const size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

  theBCIDContainer.reset();
  theTrgIDContainer.reset();
  ContainerFactory::copyAndInitChip<GenericDataArray<BCIDsize>> (*fDetectorContainer, theBCIDContainer);
  ContainerFactory::copyAndInitChip<GenericDataArray<TrgIDsize>>(*fDetectorContainer, theTrgIDContainer);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          size_t nMaskedPixelsPerCalib = 0;

          LOG (INFO) << GREEN << "Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << GREEN << "] is " << BOLDYELLOW
                     << std::setprecision(-1) << theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy << RESET;

          static_cast<RD53*>(cChip)->copyMaskFromDefault();

          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
                {
                  float occupancy = theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy;
                  static_cast<RD53*>(cChip)->enablePixel(row,col,injType == INJtype::None ? occupancy < thrOccupancy : occupancy != 0);
                  if (((injType == INJtype::None) && (occupancy >= thrOccupancy)) || ((injType != INJtype::None) && (occupancy == 0))) nMaskedPixelsPerCalib++;
                }

          LOG (INFO) << BOLDBLUE << "\t--> Number of potentially masked pixels in this iteration: " << BOLDYELLOW << nMaskedPixelsPerCalib << RESET;
          LOG (INFO) << BOLDBLUE << "\t--> Total number of potentially masked pixels: " << BOLDYELLOW << static_cast<RD53*>(cChip)->getNbMaskedPixels() << RESET;


          // ######################################
          // # Copy register values for streaming #
          // ######################################
          for (auto i = 0u; i < BCIDsize; i++)  theBCIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<BCIDsize>>().data[i]   = 0;
          for (auto i = 0u; i < TrgIDsize; i++) theTrgIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TrgIDsize>>().data[i] = 0;

          for (auto i = 1u; i < theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1.size(); i++)
            {
              int deltaBCID = theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1[i] -
                theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1[i-1];
              deltaBCID += (deltaBCID >= 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1);
              if (deltaBCID >= int(BCIDsize)) LOG (ERROR) << BOLDBLUE <<"[PixelAlive::analyze] " << BOLDRED << "deltaBCID out of range: " << deltaBCID << RESET;
              else theBCIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<BCIDsize>>().data[deltaBCID]++;
            }

          for (auto i = 1u; i < theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2.size(); i++)
            {
              int deltaTrgID = theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2[i] -
                theOccContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2[i-1];
              deltaTrgID += (deltaTrgID >= 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1);
              if (deltaTrgID >= int(TrgIDsize)) LOG (ERROR) << BOLDBLUE << "[PixelAlive::analyze] " << BOLDRED << "deltaTrgID out of range: " << deltaTrgID << RESET;
              else theTrgIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TrgIDsize>>().data[deltaTrgID]++;
            }
        }

  return theOccContainer;
}

void PixelAlive::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}

void PixelAlive::fillHisto ()
{
#ifdef __USE_ROOT__
  histos.fill     (*theOccContainer.get());
  histos.fillBCID (theBCIDContainer);
  histos.fillTrgID(theTrgIDContainer);
#endif
}

void PixelAlive::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void PixelAlive::chipErrorReport ()
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
