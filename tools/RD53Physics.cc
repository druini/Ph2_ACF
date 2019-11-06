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
}

void Physics::Start (int currentRun)
{
  this->CreateResultDirectory(RESULTDIR,false,false);

  SystemController::Start(currentRun);

  keepRunning = true;
  std::thread thrRun_(&Physics::run, this);
  thrRun.swap(thrRun_);
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
  unsigned int dataSize = 0;

  while (keepRunning == true)
    {
      for (const auto cBoard : *fDetectorContainer)
        dataSize = SystemController::ReadData(static_cast<BeBoard*>(cBoard), false);

      usleep(READOUTSLEEP);
    }

  if (dataSize == 0) LOG (WARNING) << BOLDBLUE << "No data collected" << RESET;
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
