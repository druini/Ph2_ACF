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
  RD53ChannelGroupHandler  (size_t numberIterations, bool doAll = false);
  ~RD53ChannelGroupHandler ();

  class RD53ChannelGroupIterator : public std::iterator<std::output_iterator_tag,uint32_t>
    {
  public:
    RD53ChannelGroupIterator (RD53ChannelGroupHandler& channelGroupHandler, uint32_t groupNumber)
      : channelGroupHandler_ (channelGroupHandler)
      , groupNumber_         (groupNumber)
    {}

    RD53ChannelGroupIterator& operator++ ()
      {
	return ++(*this);
      }

    bool operator!= (const RD53ChannelGroupIterator& rhs) const
    {
      return groupNumber_ != rhs.groupNumber_;
    }

    const ChannelGroupBase* operator* () const
    {
      // #####################
      // # Printout progress #
      // #####################
      std::cout << "Progress: " << 1. * channelGroupHandler_.currentProgress / channelGroupHandler_.totalProgress << "%" << std::endl;
      channelGroupHandler_.currentProgress++;

      return channelGroupHandler_.getTestGroup(groupNumber_);      
    }

  protected:
    RD53ChannelGroupHandler& channelGroupHandler_;
    uint32_t groupNumber_ ;
  };
  friend class RD53ChannelGroupIterator;

  RD53ChannelGroupIterator begin ();
  RD53ChannelGroupIterator end   ();

 private:
  size_t totalProgress;
  size_t currentProgress = 0;

  class RD53ChannelGroupAll : public ChannelGroup<Ph2_HwDescription::RD53::nRows, Ph2_HwDescription::RD53::nCols>
   {
     void makeTestGroup (ChannelGroupBase* currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster = 1) const override;
   };
 
  class RD53ChannelGroupPattern : public ChannelGroup<Ph2_HwDescription::RD53::nRows, Ph2_HwDescription::RD53::nCols>
   {
     void makeTestGroup (ChannelGroupBase* currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster = 1) const override;
   };
};

#endif
