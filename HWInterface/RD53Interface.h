/*!
  \file                  RD53Interface.h
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53INTERFACE_H_
#define _RD53INTERFACE_H_

#include "BeBoardFWInterface.h"
#include "ChipInterface.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/RegisterValue.h"

#include <vector>


// ################################
// # CONSTANTS AND BIT DEFINITION #
// ################################
#define DEEPSLEEP   500000 // [microseconds]
#define NWRITE_ATTEMPTS 10 // Number of write attempts to program the front-end chip


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  class RD53Interface: public ChipInterface
  {
  public:
    RD53Interface (const BeBoardFWMap& pBoardMap);
    ~RD53Interface();

    bool     ConfigureChip             (const Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                     override;
    bool     WriteChipReg              (Chip* pChip, const std::string& pRegNode, const uint16_t data, bool pVerifLoop = true)                    override;
    bool     WriteChipMultReg          (Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pVecReg, bool pVerifLoop = true)      override;
    bool     WriteChipAllLocalReg      (Chip* pChip, const std::string& dacName, ChipContainer& pValue, bool pVerifLoop = true)                   override;
    uint16_t ReadChipReg               (Chip* pChip, const std::string& pRegNode)                                                                 override;
    bool     ConfigureChipOriginalMask (Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                           override;
    bool     MaskAllChannels           (Chip* pChip, bool mask, bool pVerifLoop = true)                                                           override;
    bool     setInjectionSchema        (Chip* pChip, const ChannelGroupBase* group, bool pVerifLoop = true)                                       override;
    bool     maskChannelsGroup         (Chip* pChip, const ChannelGroupBase* group, bool pVerifLoop = true)                                       override;

    bool     WriteRD53Mask             (RD53* pRD53, bool pVerifLoop, bool defaultT_currentF);
    bool     WriteRD53Reg              (RD53* pRD53, const std::string& pRegNode, const std::vector<uint16_t>* dataVec);
    void     InitRD53Aurora            (RD53* pRD53);
    void     SyncRD53                  (RD53* pRD53, unsigned int nSyncWords = 1);
    void     ResetRD53                 (RD53* pRD53);

    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > ReadRD53Reg (RD53* pRD53, const std::string& pRegNode);
   };
}

#endif
