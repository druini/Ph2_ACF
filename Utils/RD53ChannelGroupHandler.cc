/*!
  \file                  RD53ChannelGroupHandler.cc
  \brief                 Channel container handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ChannelGroupHandler.h"

void RD53ChannelGroupHandler::RD53ChannelGroupAll::makeTestGroup (ChannelGroupBase* currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster) const
{
  static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();

  for (auto row = 0u; row < Ph2_HwDescription::RD53::nRows; row++)
    for (auto col = 0u; col < Ph2_HwDescription::RD53::nCols; col++)
      if (isChannelEnabled(row,col)) static_cast<RD53ChannelGroupAll*>(currentChannelGroup)->enableChannel(row, col);
}

void RD53ChannelGroupHandler::RD53ChannelGroupPattern::makeTestGroup (ChannelGroupBase* currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster) const
{
  static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();
  
  for (auto col = 0u; col < Ph2_HwDescription::RD53::nCols; col++)
    {
      auto row = NROW_CORE * col;
      row += (row/Ph2_HwDescription::RD53::nRows) % NROW_CORE + groupNumber;
      row %= Ph2_HwDescription::RD53::nRows;
      if (isChannelEnabled(row,col) == true) static_cast<RD53ChannelGroupPattern*>(currentChannelGroup)->enableChannel(row, col);
    }
}

RD53ChannelGroupHandler::RD53ChannelGroupHandler (bool doAll)
{
  if (doAll == true)
    {
      allChannelGroup_     = new RD53ChannelGroupAll();
      currentChannelGroup_ = new RD53ChannelGroupAll();
    }
  else
    {
      allChannelGroup_     = new RD53ChannelGroupPattern();
      currentChannelGroup_ = new RD53ChannelGroupPattern();
    }

  numberOfGroups_ = getNumberOfGroups(doAll);
}

RD53ChannelGroupHandler::~RD53ChannelGroupHandler()
{
  delete allChannelGroup_;
  delete currentChannelGroup_;
}
