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
  rowStart = this->findValueInSettings("ROWstart");
  rowStop  = this->findValueInSettings("ROWstop");
  colStart = this->findValueInSettings("COLstart");
  colStop  = this->findValueInSettings("COLstop");


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

  for (const auto group : *theChnGroupHandler)
    for (const auto cBoard : *fDetectorContainer)
      for (const auto cModule : *cBoard)
        for (const auto cChip : *cModule)
          fReadoutChipInterface->maskChannelsAndSetInjectionSchema(static_cast<ReadoutChip*>(cChip), group, true, false);


  // ###########################################
  // # Initialize directory and data container #
  // ###########################################
  this->CreateResultDirectory(RESULTDIR,false,false);
  this->fChannelGroupHandler = theChnGroupHandler.get();
  ContainerFactory::copyAndInitChannel<OccupancyAndPh>(*fDetectorContainer, theOccContainer);
}

void Physics::Start (int currentRun)
{
  SystemController::Start(currentRun);

  keepRunning = true;
  std::thread thrRun_(&Physics::run, this);
  thrRun.swap(thrRun_);
}

void Physics::sendData (BoardContainer* const& cBoard)
{
  ChannelContainerStream<OccupancyAndPh> theOccStream = prepareChannelContainerStreamer<OccupancyAndPh>();

  if (fStreamerEnabled == true)
    theOccStream.streamAndSendBoard(theOccContainer.at(cBoard->getIndex()), fNetworkStreamer);
}

void Physics::Stop ()
{
  keepRunning = false;
  thrRun.join();


  // ################
  // # Error report #
  // ################
  Physics::chipErrorReport();


  this->Destroy();
}

void Physics::run ()
{
  // uint8_t status;
  unsigned int dataSize = 0;

  while (keepRunning == true)
    {
      for (const auto cBoard : *fDetectorContainer)
        {
          RD53decodedEvents.clear();
          dataSize = SystemController::ReadData(static_cast<BeBoard*>(cBoard), false);


          // #####################
          // # ReadData for RD53 #
          // #####################
          // Data theData;
          // std::vector<uint32_t> data;
          // BeBoard* theBoard = static_cast<BeBoard*>(cBoard);
          // dataSize = fBeBoardInterface->ReadData(theBoard, false, data, false);


          if (dataSize != 0)
            {
              // RD53decodedEvents.clear();
              // RD53FWInterface::DecodeEvents(data, status, RD53decodedEvents);
              // theData.DecodeData(theBoard, data, dataSize, fBeBoardInterface->getBoardType(theBoard));

              Physics::fillDataContainer(cBoard);
              Physics::sendData(cBoard);
            }
        }

      usleep(READOUTSLEEP);
    }

  if (dataSize == 0) LOG (WARNING) << BOLDBLUE << "No data collected" << RESET;
}

void Physics::fillDataContainer (BoardContainer* const& cBoard)
{
  // ###################
  // # Clear container #
  // ###################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        for (auto row = 0u; row < RD53::nRows; row++)
          for (auto col = 0u; col < RD53::nCols; col++)
            {
              theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy   = 0;
              theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh          = 0;
              theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPhError     = 0;
              theOccContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).readoutError = false;
            }


  // ##################
  // # Fill container #
  // ##################
  const std::vector<Event*>& events = SystemController::GetEvents(static_cast<BeBoard*>(cBoard));
  for (const auto& event : events)
    event->fillDataContainer(theOccContainer.at(cBoard->getIndex()), this->fChannelGroupHandler->allChannelGroup());
}

void Physics::chipErrorReport ()
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
          LOG (INFO) << BOLDBLUE << "TRIG_CNT        = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "TRIG_CNT")        << std::setfill(' ') << std::setw(8) << "" << RESET;
        }
}
