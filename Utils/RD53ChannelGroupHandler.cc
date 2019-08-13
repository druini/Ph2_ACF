/*!
  \file                  RD53ChannelGroupHandler.cc
  \brief                 Channel container handler
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53ChannelGroupHandler.h"

void RD53ChannelGroup::makeTestGroup (ChannelGroupBase *currentChannelGroup, uint32_t groupNumber, uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster, uint16_t numberOfColsPerCluster) const {
    static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();
    for (int core_row = 0; core_row < 8; core_row++) {
        for (int core_col = 0; core_col < 3; core_col++) {
            int row_offset = (groupNumber % 64) % 8;
            int col_offset = (groupNumber % 64) / 8;
            int col_in_core = (core_row + col_offset) % 8;
            for (int i = 0; i < 6; i++) {
                int row_in_core = (core_col + row_offset + (i % 2 == 0 ? 0 : 3)) % 8;
                int row = (64 * (groupNumber / 64) + 32 * i + core_row * 8 + row_in_core) % 192;
                int col = 128 + 24 * i + core_col * 8 + col_in_core;
                if (isChannelEnabled(row,col)) {
                    // LOG (INFO) << "Enabled: " << groupNumber << ", " << row_in_core << ", " << row << ", " << col << "\n";
                    static_cast<RD53ChannelGroup*>(currentChannelGroup)->enableChannel(row, col);
                }
                // else {
                //     LOG (INFO) << "Disabled: " << groupNumber << ", " << row_in_core << ", " << row << ", " << col << "\n";
                // }
            }
        }
    }
}

RD53ChannelGroupHandler::RD53ChannelGroupHandler()
{
    allChannelGroup_ = new RD53ChannelGroup();
    currentChannelGroup_ = new RD53ChannelGroup();
}

RD53ChannelGroupHandler::~RD53ChannelGroupHandler()
{
    delete allChannelGroup_;
    delete currentChannelGroup_;
}

void RD53ChannelGroupHandler::setChannelGroupParameters(uint32_t numberOfClustersPerGroup, uint32_t numberOfRowsPerCluster, uint32_t numberOfColsPerCluster) {
    numberOfGroups_ = 192;
}

// void RD53ChannelGroupHandler::makeTestGroup(ChannelGroupBase* currentChannelGroup, uint32_t groupNumber,
//                                             uint32_t numberOfClustersPerGroup, uint16_t numberOfRowsPerCluster,
//                                             uint16_t numberOfColsPerCluster) const
// {
//     if (customPatternSet_ && (numberOfRowsPerCluster > 1 || numberOfColsPerCluster > 1))
//         std::cout << "Warning, automatic group creation may not work when a custom pattern is set\n";
//     if (numberOfClustersPerGroup * numberOfRowsPerCluster * numberOfColsPerCluster >= numberOfEnabledChannels_) {
//         static_cast<ChannelGroup<R, C>*>(currentChannelGroup)->setCustomPattern(*this);
//         return;
//     }
//     static_cast<ChannelGroup*>(currentChannelGroup)->disableAllChannels();

//     uint32_t numberOfClusterToSkip = numberOfEnabledChannels_ / (numberOfRowsPerCluster * numberOfColsPerCluster * numberOfClustersPerGroup) - 1;
//     if (numberOfEnabledChannels_ % (numberOfRowsPerCluster * numberOfColsPerCluster * numberOfClustersPerGroup) > 0)
//         ++numberOfClusterToSkip;

//     std::cout << "numberOfClustersPerGroup = " << numberOfClustersPerGroup << "\n";

//     std::cout << "numberOfClusterToSkip = " << numberOfClusterToSkip << "\n";

//     uint32_t clusterSkipped = numberOfClusterToSkip - groupNumber;
//     for (uint16_t col = 0; col < numberOfCols_; col += numberOfColsPerCluster) {
//         for (uint16_t row = 0; row < numberOfRows_; row += numberOfRowsPerCluster) {
//             if (clusterSkipped == numberOfClusterToSkip)
//                 clusterSkipped = 0;
//             else {
//                 ++clusterSkipped;
//                 continue;
//             }
//             if (isChannelEnabled(row, col)) {
//                 for (uint16_t clusterRow = 0; clusterRow < numberOfRowsPerCluster; ++clusterRow) {
//                     for (uint16_t clusterCol = 0; clusterCol < numberOfColsPerCluster; ++clusterCol) {
//                         static_cast<ChannelGroup<R, C>*>(currentChannelGroup)->enableChannel(row + clusterRow, col + clusterCol);
//                     }
//                 }
//             }
//         }
//     }
// }