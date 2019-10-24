/*!
  \file                  RD53Interface.h
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53Interface_H
#define RD53Interface_H

#include "RD53FWInterface.h"
#include "BeBoardFWInterface.h"
#include "ReadoutChipInterface.h"


// #############
// # CONSTANTS #
// #############
#define VCALSLEEP 50000 // [microseconds]
#define NPIXCMD     100 // Number of possible pixel commands to stack


namespace Ph2_HwInterface
{
  class RD53Interface: public ReadoutChipInterface
  {
  public:
    RD53Interface (const BeBoardFWMap& pBoardMap);

    bool     ConfigureChip                     (Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                      override;
    bool     WriteChipReg                      (Chip* pChip, const std::string& pRegNode, uint16_t data, bool pVerifLoop = true)                     override;
    bool     WriteChipAllLocalReg              (ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop = true)       override;
    void     ReadChipAllLocalReg               (ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue)                               override;
    uint16_t ReadChipReg                       (Chip* pChip, const std::string& pRegNode)                                                            override;
    bool     ConfigureChipOriginalMask         (ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                               override;
    bool     MaskAllChannels                   (ReadoutChip* pChip, bool mask, bool pVerifLoop = true)                                               override;
    bool     maskChannelsAndSetInjectionSchema (ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop = false)  override;

    bool     WriteChipMultReg                  (Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pVecReg, bool pVerifLoop = true) override
    { std::cout << __PRETTY_FUNCTION__ << std::endl; exit(EXIT_FAILURE); }; // @TMP@
    bool     setInjectionSchema                (ReadoutChip* pChip, const ChannelGroupBase* group, bool pVerifLoop = true)                           override
    { std::cout << __PRETTY_FUNCTION__ << std::endl; exit(EXIT_FAILURE); }; // @TMP@
    bool     maskChannelsGroup                 (ReadoutChip* pChip, const ChannelGroupBase* group, bool pVerifLoop = true)                           override
    { std::cout << __PRETTY_FUNCTION__ << std::endl; exit(EXIT_FAILURE); }; // @TMP@

  private:
    void WriteRD53Mask  (RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop = false);
    std::vector<std::pair<uint16_t,uint16_t>> ReadRD53Reg (Chip* pChip, const std::string& pRegNode);
    void InitRD53Aurora (Chip* pChip);
    void SyncRD53       (Chip* pChip);
    void ResetRD53      (Chip* pChip);

    template <class Cmd>
      void sendCommand (Chip* pChip, const Cmd& cmd) { static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(cmd.getFrames(), pChip->getFeId()); }

    template <typename T, size_t N>
      static size_t arraySize (const T(&)[N]) { return N; }
  };
}

#endif
