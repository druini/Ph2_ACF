/*!
  \file                  RD53ChannelGroupHandler.h
  \brief                 Channel container handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53ChannelGroupHandler_H
#define RD53ChannelGroupHandler_H

#include "ChannelGroupHandler.h"
#include "../HWDescription/RD53.h"


class RD53ChannelGroupHandler : public ChannelGroupHandler
{
 public:
  RD53ChannelGroupHandler(bool doAll = false);
  ~RD53ChannelGroupHandler();

 private:
  class RD53ChannelGroupAll : public ChannelGroup<Ph2_HwDescription::RD53::nRows, Ph2_HwDescription::RD53::nCols>
   {
     void makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster=1) const override;
   };
 
  class RD53ChannelGroupPattern : public ChannelGroup<Ph2_HwDescription::RD53::nRows, Ph2_HwDescription::RD53::nCols>
   {
     void makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster=1) const override;
   };
};

#endif
