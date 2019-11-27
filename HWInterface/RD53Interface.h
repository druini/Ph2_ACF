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

    bool     ConfigureChip                     (Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                     override;
    bool     WriteChipReg                      (Chip* pChip, const std::string& pRegNode, uint16_t data, bool pVerifLoop = true)                    override;
    void     WriteBoardBroadcastChipReg        (const BeBoard* pBoard, const std::string& pRegNode, uint16_t data)                                  override;
    bool     WriteChipAllLocalReg              (ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop = true)      override;
    void     ReadChipAllLocalReg               (ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue)                              override;
    uint16_t ReadChipReg                       (Chip* pChip, const std::string& pRegNode)                                                           override;
    bool     ConfigureChipOriginalMask         (ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                              override;
    bool     MaskAllChannels                   (ReadoutChip* pChip, bool mask, bool pVerifLoop = true)                                              override;
    bool     maskChannelsAndSetInjectionSchema (ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop = false) override;

  private:
    std::vector<std::pair<uint16_t,uint16_t>> ReadRD53Reg (Chip* pChip, const std::string& pRegNode);
    void WriteRD53Mask  (RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop = false);
    void InitRD53Aurora (Chip* pChip);

    template <typename T>
      void sendCommand (Chip* pChip, const T& cmd) { static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(cmd.getFrames(), pChip->getFeId()); }

    template <typename T, size_t N>
      static size_t arraySize (const T(&)[N]) { return N; }
  };
}

#endif
