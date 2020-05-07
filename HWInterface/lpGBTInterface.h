/*!
  \file                  lpGBTInterface.h
  \brief                 The implementation follows the skeleton of the register map section of the lpGBT Manual
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef lpGBTInterface_H
#define lpGBTInterface_H

#include "ChipInterface.h"
#include "../HWDescription/lpGBT.h"


namespace Ph2_HwInterface
{
  class lpGBTInterface : public ChipInterface
  {
  public:
    lpGBTInterface (const BeBoardFWMap& pBoardMap) : ChipInterface(pBoardMap) {}
    virtual ~lpGBTInterface () {}

    virtual void     InitialiseLinks    (std::vector<uint8_t>& pULGroups, std::vector<uint8_t>& pULChannels, std::vector<uint8_t>& pDLGroups, std::vector<uint8_t>& pDLChannels, std::vector<uint8_t>& pBERTGroups) = 0;
    virtual void     ConfigureDownLinks (Ph2_HwDescription::Chip* pChip, uint8_t pCurrent, uint8_t pPreEmphasis, bool pInvert = false) = 0;
    virtual void     DisableDownLinks   (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups) = 0;
    virtual void     ConfigureUpLinks   (Ph2_HwDescription::Chip* pChip, uint8_t pDataRate, uint8_t pPhaseMode, uint8_t pEqual, uint8_t pPhase, bool pEnableTerm = true, bool pEnableBias = true, bool pInvert = false) = 0;
    virtual void     DisableUpLinks     (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups) = 0;

    virtual void     ConfigureBERT      (Ph2_HwDescription::Chip* pChip) = 0;
    virtual uint64_t RunBERT            (Ph2_HwDescription::Chip* pChip, uint8_t pTestTime = 0) = 0;
    virtual void     ResetBERT          (Ph2_HwDescription::Chip* pChip) = 0;
    virtual void     SetBERTPattern     (Ph2_HwDescription::Chip* pChip, uint32_t pPattern) = 0;
    virtual void     SetDPPattern       (Ph2_HwDescription::Chip* pChip, uint32_t pPattern) = 0;

    virtual void     SetModeUpLink      (Ph2_HwDescription::Chip* pChip, uint8_t pSource = 0, uint32_t pPattern = 0x77778888) = 0;
    virtual void     SetModeDownLink    (Ph2_HwDescription::Chip* pChip, uint8_t pSource = 0, uint32_t pPattern = 0x77778888) = 0;
    virtual void     FindPhase          (Ph2_HwDescription::Chip* pChip, uint8_t pTime = 8, uint8_t pMaxPhase = 7) = 0;
    virtual void     ChangeUpLinksPhase (Ph2_HwDescription::Chip* pChip, uint8_t pPhase) = 0;
    virtual bool     IslpGBTReady       (Ph2_HwDescription::Chip* pChip) = 0;
    virtual std::vector<std::pair<uint8_t, uint8_t>> GetRxStatus (Ph2_HwDescription::Chip* pChip) = 0;
  };
}

#endif
