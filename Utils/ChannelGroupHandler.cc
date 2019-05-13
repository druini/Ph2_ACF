#include "../Utils/ChannelGroupHandler.h"


void ChannelGroupHandler::setChannelGroupParameters(uint32_t numberOfClustersPerGroup, uint32_t numberOfRowsPerCluster, uint32_t numberOfColsPerCluster)
{
    numberOfClustersPerGroup_ = numberOfClustersPerGroup;
    numberOfRowsPerCluster_   = numberOfRowsPerCluster  ;
    numberOfColsPerCluster_   = numberOfColsPerCluster  ;
    numberOfGroups_ = allChannelGroup_->getNumberOfEnabledChannels() / (numberOfClustersPerGroup*numberOfRowsPerCluster*numberOfColsPerCluster);
    if(allChannelGroup_->getNumberOfEnabledChannels() % (numberOfClustersPerGroup*numberOfRowsPerCluster*numberOfColsPerCluster) != 0) ++numberOfGroups_;
}

void ChannelGroupHandler::setCustomChannelGroup(ChannelGroupBase *customAllChannelGroup)
{
    delete allChannelGroup_;
    allChannelGroup_ = customAllChannelGroup;
}
