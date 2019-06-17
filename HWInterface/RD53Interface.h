/*!
  \file                  RD53Interface.h
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53Interface_h_
#define _RD53Interface_h_

#include "BeBoardFWInterface.h"
#include "ChipInterface.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/RegisterValue.h"

#include <vector>


// #############
// # CONSTANTS #
// #############
#define DEEPSLEEP 50000 // [microseconds]
#define NPIXCMD       1 // Number of pixel commands to pack


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  class RD53Interface: public ChipInterface
  {
  public:
    RD53Interface (const BeBoardFWMap& pBoardMap);
    ~RD53Interface();

    bool     ConfigureChip             (const Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                 override;
    bool     WriteChipReg              (Chip* pChip, const std::string& pRegNode, uint16_t data, bool pVerifLoop = false)                     override;
    bool     WriteChipMultReg          (Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pVecReg, bool pVerifLoop = false) override;
    bool     WriteChipAllLocalReg      (Chip* pChip, const std::string& dacName, ChipContainer& pValue, bool pVerifLoop = false)              override;
    uint16_t ReadChipReg               (Chip* pChip, const std::string& pRegNode)                                                             override;
    bool     ConfigureChipOriginalMask (Chip* pChip, bool pVerifLoop = false, uint32_t pBlockSize = 310)                                      override;
    bool     MaskAllChannels           (Chip* pChip, bool mask, bool pVerifLoop = false)                                                      override;
    bool     setInjectionSchema        (Chip* pChip, const ChannelGroupBase* group, bool pVerifLoop = false)                                  override;
    bool     maskChannelsGroup         (Chip* pChip, const ChannelGroupBase* group, bool pVerifLoop = false)                                  override;

    void     WriteRD53Mask             (RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop = false);
    void     WriteRD53RegLong          (RD53* pRD53, const std::string& pRegNode, const std::vector<uint32_t>& dataVec, size_t nCmd = 1);
    void     WriteRD53RegShort         (RD53* pRD53, const std::string& pRegNode, uint16_t data, std::vector<uint32_t>& serialSymbols, size_t nCmd, bool download);
    void     InitRD53Aurora            (RD53* pRD53);
    void     SyncRD53                  (RD53* pRD53, unsigned int nSyncWords = 1);
    void     ResetRD53                 (RD53* pRD53);

    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > ReadRD53Reg (RD53* pRD53, const std::string& pRegNode);
   };
}

#endif
