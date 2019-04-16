#include "../Utils/ChannelGroupHandler.h"


void ChannelGroupHandler::setChannelGroupParameters(uint32_t numberOfClustersPerGroup, uint32_t numberOfColsPerCluster, uint32_t numberOfRowsPerCluster)
{
    numberOfClustersPerGroup_ = numberOfClustersPerGroup;
    numberOfColsPerCluster_   = numberOfColsPerCluster  ;
    numberOfRowsPerCluster_   = numberOfRowsPerCluster  ;
    numberOfGroups_ = allChannelGroup_->getNumberOfEnabledChannels() / (numberOfClustersPerGroup*numberOfRowsPerCluster*numberOfColsPerCluster);
    if(allChannelGroup_->getNumberOfEnabledChannels() / (numberOfClustersPerGroup*numberOfRowsPerCluster*numberOfColsPerCluster) != 0) ++numberOfGroups_;
}