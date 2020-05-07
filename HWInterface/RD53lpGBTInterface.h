/*!
  \file                  RD53lpGBTInterface.h
  \brief                 The implementation follows the skeleton of the register map section of the lpGBT Manual
  \author                Mauro DINARDO
  \version               1.0
  \date                  03/03/20
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53lpGBTInterface_H
#define RD53lpGBTInterface_H

#include "lpGBTInterface.h"


namespace Ph2_HwInterface
{
  class RD53lpGBTInterface : public lpGBTInterface
  {
  public:
    RD53lpGBTInterface (const BeBoardFWMap& pBoardMap) : lpGBTInterface(pBoardMap) {}

    bool     ConfigureChip    (Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                     override;
    bool     WriteChipReg     (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true)                  override;
    bool     WriteChipMultReg (Ph2_HwDescription::Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& RegVec, bool pVerifLoop = true) override;
    uint16_t ReadChipReg      (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode)                                                           override;


    void     InitialiseLinks    (std::vector<uint8_t>& pULGroups, std::vector<uint8_t>& pULChannels, std::vector<uint8_t>& pDLGroups, std::vector<uint8_t>& pDLChannels, std::vector<uint8_t>& pBERTGroups)     override;
    void     ConfigureDownLinks (Ph2_HwDescription::Chip* pChip, uint8_t pCurrent, uint8_t pPreEmphasis, bool pInvert = false)                                                                                  override;
    void     DisableDownLinks   (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups)                                                                                                           override;
    void     ConfigureUpLinks   (Ph2_HwDescription::Chip* pChip, uint8_t pDataRate, uint8_t pPhaseMode, uint8_t pEqual, uint8_t pPhase, bool pEnableTerm = true, bool pEnableBias = true, bool pInvert = false) override;
    void     DisableUpLinks     (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups)                                                                                                           override;

    void     ConfigureBERT      (Ph2_HwDescription::Chip* pChip)                        override;
    uint64_t RunBERT            (Ph2_HwDescription::Chip* pChip, uint8_t pTestTime = 0) override;
    void     ResetBERT          (Ph2_HwDescription::Chip* pChip)                        override;
    void     SetDPPattern       (Ph2_HwDescription::Chip* pChip, uint32_t pPattern)     override;
    void     SetBERTPattern     (Ph2_HwDescription::Chip* pChip, uint32_t pPattern)     override;

    void     SetModeUpLink      (Ph2_HwDescription::Chip* pChip, uint8_t pSource = 0, uint32_t pPattern = 0x77778888) override;
    void     SetModeDownLink    (Ph2_HwDescription::Chip* pChip, uint8_t pSource = 0, uint32_t pPattern = 0x77778888) override;
    void     FindPhase          (Ph2_HwDescription::Chip* pChip, uint8_t pTime = 8, uint8_t pMaxPhase = 7)            override;
    void     ChangeUpLinksPhase (Ph2_HwDescription::Chip* pChip, uint8_t pPhase)                                      override;
    bool     IslpGBTReady       (Ph2_HwDescription::Chip* pChip)                                                      override;
    std::vector<std::pair<uint8_t, uint8_t>> GetRxStatus (Ph2_HwDescription::Chip* pChip)                             override;


  private:
    std::vector<uint8_t> fULGroups, fDLGroups, fBERTGroups;
    std::vector<uint8_t> fULChannels, fDLChannels;
  };
}

#endif
