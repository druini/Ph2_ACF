/*!
  \file                  RD53ChannelGroupHandler.cc
  \brief                 Channel container handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ChannelGroupHandler.h"

void RD53ChannelGroupHandler::RD53ChannelGroupAll::makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster) const
{
  static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();

  for (auto row = 0u; row < Ph2_HwDescription::RD53::nRows; row++)
    for (auto col = 0u; col < Ph2_HwDescription::RD53::nCols; col++)
      if (isChannelEnabled(row,col)) static_cast<RD53ChannelGroupAll*>(currentChannelGroup)->enableChannel(row, col);
}

void RD53ChannelGroupHandler::RD53ChannelGroupPattern::makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster) const
{
  static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();

  for (int core_row = 0; core_row < NROW_CORE; core_row++)
    {
      for (int core_col = 0; core_col < 3; core_col++)
	{
	  int row_offset = (groupNumber % 64) % NROW_CORE;
	  int col_offset = (groupNumber % 64) / NROW_CORE;
	  int col_in_core = (core_row + col_offset) % NROW_CORE;

	  for (int i = 0; i < 6; i++)
	    {
	      int row_in_core = (core_col + row_offset + (i % 2 == 0 ? 0 : 3)) % NROW_CORE;
	      int row = (64 * (groupNumber / 64) + 32 * i + core_row * 8 + row_in_core) % Ph2_HwDescription::RD53::nRows;
	      int col = 128 + 24 * i + core_col * NROW_CORE + col_in_core;

	      if (isChannelEnabled(row,col)) static_cast<RD53ChannelGroupPattern*>(currentChannelGroup)->enableChannel(row, col);
	    }
	}
    }
}

RD53ChannelGroupHandler::RD53ChannelGroupHandler(bool doAll)
{
  if (doAll == true)
    {
      allChannelGroup_     = new RD53ChannelGroupAll();
      currentChannelGroup_ = new RD53ChannelGroupAll();
      numberOfGroups_      = 1;
    }
  else
    {
      allChannelGroup_     = new RD53ChannelGroupPattern();
      currentChannelGroup_ = new RD53ChannelGroupPattern();
      numberOfGroups_      = Ph2_HwDescription::RD53::nRows;
    }
}

RD53ChannelGroupHandler::~RD53ChannelGroupHandler()
{
  delete allChannelGroup_;
  delete currentChannelGroup_;
}
