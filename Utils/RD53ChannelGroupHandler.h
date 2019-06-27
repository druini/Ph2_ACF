/*!
  \file                  RD53ChannelGroupHandler.h
  \brief                 Channel container handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53ChannelGroupHandler_h_
#define _RD53ChannelGroupHandler_h_

#include "ChannelGroupHandler.h"
#include "../HWDescription/RD53.h"


using namespace Ph2_HwDescription;

class RD53ChannelGroupHandler : public ChannelGroupHandler
{
 public:
  RD53ChannelGroupHandler();
  ~RD53ChannelGroupHandler();
};

#endif
