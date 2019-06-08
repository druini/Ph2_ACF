/*!
  \file                  RD53ChannelGroupHandler.cc
  \brief                 Channel container handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ChannelGroupHandler.h"

RD53ChannelGroupHandler::RD53ChannelGroupHandler()
{
  allChannelGroup_     = new ChannelGroup<RD53::nRows,RD53::nCols>();
  currentChannelGroup_ = new ChannelGroup<RD53::nRows,RD53::nCols>();
}


RD53ChannelGroupHandler::~RD53ChannelGroupHandler()
{
  delete allChannelGroup_;
  delete currentChannelGroup_;
}
