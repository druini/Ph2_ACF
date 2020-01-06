/*!
  \file                  RD53Physics.cc
  \brief                 Implementaion of Physics data taking
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Physics.h"

void Physics::ConfigureCalibration ()
{
  // #######################
  // # Retrieve parameters #
  // #######################
  rowStart       = this->findValueInSettings("ROWstart");
  rowStop        = this->findValueInSettings("ROWstop");
  colStart       = this->findValueInSettings("COLstart");
  colStop        = this->findValueInSettings("COLstop");
  doDisplay      = this->findValueInSettings("DisplayHisto");
  doUpdateChip   = this->findValueInSettings("UpdateChipCfg");
  saveBinaryData = this->findValueInSettings("SaveBinaryData");
  doLocal        = false;
  keepRunning    = true;


  // ################################
  // # Custom channel group handler #
  // ################################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>(customChannelGroup, RD53GroupType::AllPixels);
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);


  // ###########################################
  // # Initialize directory and data container #
  // ###########################################
  const size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  const size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

  this->CreateResultDirectory(RESULTDIR,false,false);
  ContainerFactory::copyAndInitStructure<OccupancyAndPh,GenericDataVector>(*fDetectorContainer, theOccContainer);
  ContainerFactory::copyAndInitChip<GenericDataArray<BCIDsize>> (*fDetectorContainer, theBCIDContainer);
  ContainerFactory::copyAndInitChip<GenericDataArray<TrgIDsize>>(*fDetectorContainer, theTrgIDContainer);
}

void Physics::Start (int currentRun)
{
  LOG (INFO) << GREEN << "[Physics::Start] Starting" << RESET;

  if (saveBinaryData == true)
    {
      this->addFileHandler(std::string(RESULTDIR) + "/PhysicsRun_" + fromInt2Str(currentRun) + ".raw", 'w');
      this->initializeFileHandler();
    }

  for (const auto cBoard : *fDetectorContainer)
    static_cast<RD53FWInterface*>(this->fBeBoardFWMap[static_cast<BeBoard*>(cBoard)->getBeBoardId()])->ChipReSync();
  SystemController::Start(currentRun);

  keepRunning = true;
  thrRun = std::thread(&Physics::run, this);
}

void Physics::sendData (BoardContainer* const& cBoard)
{
  const size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  const size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

  auto theOccStream   = prepareChannelContainerStreamer<OccupancyAndPh>                         ("Occ");
  auto theBCIDStream  = prepareChipContainerStreamer<EmptyContainer,GenericDataArray<BCIDsize>> ("BCID");
  auto theTrgIDStream = prepareChipContainerStreamer<EmptyContainer,GenericDataArray<TrgIDsize>>("TrgID");

  if (fStreamerEnabled == true)
    {
      theOccStream  .streamAndSendBoard(theOccContainer  .at(cBoard->getIndex()), fNetworkStreamer);
      theBCIDStream .streamAndSendBoard(theBCIDContainer .at(cBoard->getIndex()), fNetworkStreamer);
      theTrgIDStream.streamAndSendBoard(theTrgIDContainer.at(cBoard->getIndex()), fNetworkStreamer);
    }
}

void Physics::Stop ()
{
  LOG (INFO) << GREEN << "[Physics::Stop] Stopping" << RESET;

  SystemController::Stop();
  keepRunning = false;
  if (thrRun.joinable() == true) thrRun.join();


  // ################
  // # Error report #
  // ################
  Physics::chipErrorReport();


  this->closeFileHandler();
}

void Physics::initialize (const std::string fileRes_, const std::string fileReg_)
{
  fileRes = fileRes_;
  fileReg = fileReg_;

  Physics::ConfigureCalibration();

#ifdef __USE_ROOT__
  myApp = nullptr;

  if (doDisplay == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->InitResultFile(fileRes);

  Physics::initHisto();
#endif

  doLocal = true;
}

void Physics::run ()
{
  // ##############################
  // # Download mask to the chips #
  // ##############################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        fReadoutChipInterface->maskChannelsAndSetInjectionSchema(static_cast<ReadoutChip*>(cChip), theChnGroupHandler->allChannelGroup(), true, false);


  // #############
  // # Take data #
  // #############
  while (keepRunning == true)
    {
      RD53decodedEvents.clear();
      Physics::analyze(true);
      std::this_thread::sleep_for(std::chrono::microseconds(READOUTSLEEP));
    }
}

void Physics::draw ()
{
#ifdef __USE_ROOT__
  Physics::fillHisto();
  Physics::display();
#endif

  // #######################################
  // # Save and Update register new values #
  // #######################################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          if (doUpdateChip == true) static_cast<RD53*>(cChip)->saveRegMap("");
          static_cast<RD53*>(cChip)->saveRegMap(fileReg);
          std::string command("mv " + static_cast<RD53*>(cChip)->getFileName(fileReg) + " " + RESULTDIR);
          system(command.c_str());
          LOG (INFO) << BOLDBLUE << "\t--> Physics saved the configuration file for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << RESET << BOLDBLUE << "]" << RESET;
        }

#ifdef __USE_ROOT__
  if (doDisplay == true) myApp->Run(true);
  this->WriteRootFile();
  this->CloseResultFile();
#endif
}

void Physics::analyze (bool doReadBinary)
{
  for (const auto cBoard : *fDetectorContainer)
    {
      size_t dataSize = 0;

      if (doReadBinary == true) dataSize = SystemController::ReadData(static_cast<BeBoard*>(cBoard), true);
      else
        {
          dataSize = 1;
          std::vector<uint32_t> data;
          SystemController::setData(static_cast<BeBoard*>(cBoard), data, dataSize);
        }

      if (dataSize != 0)
        {
          Physics::fillDataContainer(cBoard);
          Physics::sendData(cBoard);
        }
    }
}

void Physics::initHisto ()
{
#ifdef __USE_ROOT__
  histos.book(fResultFile, *fDetectorContainer, fSettingsMap);
#endif
}

void Physics::fillHisto ()
{
#ifdef __USE_ROOT__
  histos.fill     (theOccContainer);
  histos.fillBCID (theBCIDContainer);
  histos.fillTrgID(theTrgIDContainer);
#endif
}

void Physics::display ()
{
#ifdef __USE_ROOT__
  histos.process();
#endif
}

void Physics::fillDataContainer (BoardContainer* const& cBoard)
{
  const size_t BCIDsize  = RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1;
  const size_t TrgIDsize = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;


  // ###################
  // # Clear container #
  // ###################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          for (auto row = 0u; row < RD53::nRows; row++)
            for (auto col = 0u; col < RD53::nCols; col++)
              {
                theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy   = 0;
                theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh          = 0;
                theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPhError     = 0;
                theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).readoutError = false;
              }

          for (auto i = 0u; i < BCIDsize; i++)  theBCIDContainer .at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<BCIDsize>>().data[i]  = 0;
          for (auto i = 0u; i < TrgIDsize; i++) theTrgIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TrgIDsize>>().data[i] = 0;
        }


  // ###################
  // # Fill containers #
  // ###################
  const std::vector<Event*>& events = SystemController::GetEvents(static_cast<BeBoard*>(cBoard));
  for (const auto& event : events)
    event->fillDataContainer(theOccContainer.at(cBoard->getIndex()), theChnGroupHandler->allChannelGroup());


  // ######################################
  // # Copy register values for streaming #
  // ######################################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
          for (auto i = 1u; i < theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1.size(); i++)
            {
              int deltaBCID = theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1[i] -
                theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1[i-1];
              deltaBCID += (deltaBCID >= 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_BCID) + 1);
              if (deltaBCID >= int(BCIDsize)) LOG (ERROR) << BOLDBLUE <<"[Physics::fillDataContainer] " << BOLDRED << "deltaBCID out of range: " << deltaBCID << RESET;
              else theBCIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<BCIDsize>>().data[deltaBCID]++;
            }
          theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data1.clear();

          for (auto i = 1u; i < theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2.size(); i++)
            {
              int deltaTrgID = theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2[i] -
                theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2[i-1];
              deltaTrgID += (deltaTrgID >= 0 ? 0 : RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1);
              if (deltaTrgID >= int(TrgIDsize)) LOG (ERROR) << BOLDBLUE << "[Physics::fillDataContainer] " << BOLDRED << "deltaTrgID out of range: " << deltaTrgID << RESET;
              else theTrgIDContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataArray<TrgIDsize>>().data[deltaTrgID]++;
            }
          theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().data2.clear();
        }


  // #######################
  // # Normalize container #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        for (auto row = 0u; row < RD53::nRows; row++)
          for (auto col = 0u; col < RD53::nCols; col++)
            theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).normalize(events.size(), true);
}

void Physics::chipErrorReport ()
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
