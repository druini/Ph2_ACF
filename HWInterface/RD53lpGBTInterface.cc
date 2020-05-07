/*!
  \file                  RD53lpGBTInterface.cc
  \brief                 The implementation follows the skeleton of the register map section of the lpGBT Manual
  \author                Mauro DINARDO
  \version               1.0
  \date                  03/03/20
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53lpGBTInterface.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  bool RD53lpGBTInterface::ConfigureChip (Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    this->setBoard(pChip->getBeBoardId());

    bool writeGood = true;
    ChipRegMap lpGBTRegMap = pChip->getRegMap();

    for (auto& cRegItem : lpGBTRegMap)
      writeGood = RD53lpGBTInterface::WriteChipReg(pChip, cRegItem.first, cRegItem.second.fValue, true);

    RD53lpGBTInterface::WriteChipReg(pChip, "EPRXLOCKFILTER", 0x55);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGConfig0",    0xC8);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGFLLIntCur",  0x0F);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGFFCAP",      0x00);
    RD53lpGBTInterface::WriteChipReg(pChip, "CLKGWaitTime",   0x88);

    return writeGood;
  }


  /*-------------------------------------------------------------------------*/
  /* Read/Write lpGBT chip registers                                         */
  /*-------------------------------------------------------------------------*/
  bool RD53lpGBTInterface::WriteChipReg(Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
  {
    if (pValue > 0xFF)
      {
        LOG (ERROR) << BOLDRED << "lpGBT registers are 8 bits, impossible to write " << BOLDYELLOW << pValue << BOLDRED << " on registed " << BOLDYELLOW << pRegNode << RESET;
        return false;
      }

    this->setBoard(pChip->getBeBoardId());

    if (pChip->getRegItem(pRegNode).fAddress > 316)
      {
        LOG (ERROR) << "[RD53lpGBTInterface::WriteChipReg] Writing to a read-only register " << BOLDYELLOW << pRegNode << RESET;
        return false;
      }

    return fBoardFW->WriteOptoLinkRegister(pChip, pChip->getRegItem(pRegNode).fAddress, pValue, pVerifLoop);
  }

  bool RD53lpGBTInterface::WriteChipMultReg(Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pRegVec, bool pVerifLoop)
  {
    bool writeGood = true;

    for (const auto& cReg : pRegVec)
      writeGood = RD53lpGBTInterface::WriteChipReg(pChip, cReg.first, cReg.second);

    return writeGood;
  }

  uint16_t RD53lpGBTInterface::ReadChipReg(Chip* pChip, const std::string& pRegNode)
  {
    this->setBoard(pChip->getBeBoardId());
    return fBoardFW->ReadOptoLinkRegister(pChip, pChip->getRegItem(pRegNode).fAddress);
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT configuration functions                                           */
  /*-------------------------------------------------------------------------*/
  void RD53lpGBTInterface::InitialiseLinks(std::vector<uint8_t>& pULGroups, std::vector<uint8_t>& pULChannels, std::vector<uint8_t>& pDLGroups, std::vector<uint8_t>& pDLChannels, std::vector<uint8_t>& pBERTGroups)
  {
    fULGroups   = std::move(pULGroups);
    fULChannels = std::move(pULChannels);

    fDLGroups   = std::move(pDLGroups);
    fDLChannels = std::move(pDLChannels);

    fBERTGroups = std::move(pBERTGroups);
  }

  void RD53lpGBTInterface::ConfigureDownLinks(Chip* pChip, uint8_t pCurrent, uint8_t pPreEmphasis, bool pInvert)
  {
    this->setBoard(pChip->getBeBoardId());

    // Configure EPTXDataRate
    uint32_t cValueDataRate = 0;
    for (const auto& cGroup : fDLGroups)
      cValueDataRate = (cValueDataRate & ~(0x03 << 2*cGroup)) | (2 << 2*cGroup);
    fBoardFW->WriteOptoLinkRegister(pChip,  0x0A7, cValueDataRate);

    // Configure EPTXEnable
    for (const auto& cGroup : fDLGroups)
      {
        uint32_t cValueEnableTx = fBoardFW->ReadOptoLinkRegister(pChip, 0x0A9 + (cGroup/2));
        LOG (INFO) << BOLDBLUE << "Enabling ePort Tx channels for group " << +cGroup << RESET;
        for (const auto& cChannel : fDLChannels)
          {
            LOG (INFO) << BOLDBLUE << "Enabling ePort Tx channel " << +cChannel << RESET;
            cValueEnableTx += (1 << (cChannel + 4*(cGroup % 2)));
          }
        fBoardFW->WriteOptoLinkRegister(pChip, 0x0A9 + (cGroup/2), cValueEnableTx);
      }

    // Configure EPTXChnCntr
    for (const auto& cGroup : fDLGroups)
      for (const auto& cChannel : fDLChannels)
        {
          uint32_t cValueChnCntr = 0;
          cValueChnCntr |= (pPreEmphasis << 5) | (3 << 3) | (pCurrent << 0);
          fBoardFW->WriteOptoLinkRegister(pChip, 0x0AC + (4*cGroup) + cChannel, cValueChnCntr);
        }

    // Configure EPTXChn_Cntr
    for (const auto& cGroup : fDLGroups)
      for (const auto& cChannel : fDLChannels)
        {
          uint32_t cValue_ChnCntr = fBoardFW->ReadOptoLinkRegister(pChip, 0x0BC + (2*cGroup) + (cChannel/2));
          cValue_ChnCntr |= (pInvert << (3 + 4*(cChannel%2))) | (0 << (0 + 4*(cChannel%2)));
          fBoardFW->WriteOptoLinkRegister(pChip, 0x0BC + (2*cGroup) + (cChannel/2), cValue_ChnCntr);
        }
  }

  void RD53lpGBTInterface::DisableDownLinks(Chip* pChip, const std::vector<uint8_t>& pGroups)
  {
    this->setBoard(pChip->getBeBoardId());

    // Configure EPTXDataRate
    uint32_t cValueDataRate = 0;
    for (const auto& cGroup : pGroups)
      cValueDataRate = (cValueDataRate & ~(0x03 << 2*cGroup)) | (0 << 2*cGroup);
    fBoardFW->WriteOptoLinkRegister(pChip, 0x0A7, cValueDataRate);
  }

  void RD53lpGBTInterface::ConfigureUpLinks(Chip* pChip, uint8_t pDataRate, uint8_t pPhaseMode, uint8_t pEqual, uint8_t pPhase, bool pEnableTerm, bool pEnableBias, bool pInvert)
  {
    // Configure EPRXControl
    for (const auto& cGroup : fULGroups)
      {
        uint32_t cValueEnableRx = 0;
        for (const auto& cChannel : fULChannels)
          cValueEnableRx += (1 << (cChannel + 4));
      uint32_t cValueEPRxControl = (cValueEnableRx << 4) | (pDataRate << 2) | (pPhaseMode << 0);
      char cBuffer[12];
      sprintf(cBuffer,"EPRX%iControl",cGroup);
      std::string cRegName(cBuffer,sizeof(cBuffer));
      RD53lpGBTInterface::WriteChipReg(pChip, cRegName, cValueEPRxControl);
    }

    // Configure EPRXChnCntr
    for (const auto& cGroup : fULGroups)
      for (const auto& cChannel : fULChannels)
        {
          uint32_t cValueChnCntr = (pPhase << 4) | (pInvert << 3) | (pEnableBias << 2) | (pEnableTerm << 1) | (pEqual << 0);
          char cBuffer[12];
          sprintf(cBuffer,"EPRX%i%iChnCntr",cGroup, cChannel);
          std::string cRegName(cBuffer,sizeof(cBuffer));
          RD53lpGBTInterface::WriteChipReg(pChip, cRegName, cValueChnCntr);
        }
  }

  void RD53lpGBTInterface::DisableUpLinks(Chip* pChip, const std::vector<uint8_t>& pGroups)
  {
    // Configure EPRXControl
    for (const auto& cGroup : fULGroups)
      {
        uint32_t cValueEPRxControl = (0 << 2) | (0 << 0);
        char cBuffer[12];
        sprintf(cBuffer,"EPRX%iControl",cGroup);
        std::string cRegName(cBuffer,sizeof(cBuffer));
        RD53lpGBTInterface::WriteChipReg(pChip, cRegName, cValueEPRxControl);
      }
  }


  /*-------------------------------------------------------------------------*/
  /* BERT configuration                                                      */
  /*-------------------------------------------------------------------------*/
  void RD53lpGBTInterface::ConfigureBERT(Chip* pChip)
  {
    this->setBoard(pChip->getBeBoardId());

    uint8_t cBERTSrcValue = 0;
    for (auto cGroup : fBERTGroups)
      cBERTSrcValue |= (1+cGroup) << 4;
    cBERTSrcValue |= 6;
    fBoardFW->WriteOptoLinkRegister(pChip, 0x126, cBERTSrcValue);
  }

  uint64_t RD53lpGBTInterface::RunBERT(Chip* pChip, uint8_t pTestTime)
  {
    this->setBoard(pChip->getBeBoardId());

    fBoardFW->WriteOptoLinkRegister(pChip, 0x127, pTestTime << 4 | 1);
    uint64_t cBitsChecked = pow(2,(5 + 2*pTestTime))*32;
    uint8_t cStatus = 2;
    while (cStatus & 2)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cStatus = fBoardFW->ReadOptoLinkRegister(pChip, 0x1BF);
        uint8_t cFSM = fBoardFW->ReadOptoLinkRegister(pChip, 0x1C7);
        if (cFSM != 18)
          LOG (DEBUG) << BOLDYELLOW << "Warning : lost FSM --> Status = " << +cFSM << RESET;
      }

    if (cStatus & (1 << 2))
      {
        RD53lpGBTInterface::ResetBERT(pChip);
        LOG (DEBUG) << BOLDRED << "BERT error flag (there was not data on the input)" << RESET;
        exit(EXIT_FAILURE);
      }

    uint64_t cBERTResult = (uint64_t)fBoardFW->ReadOptoLinkRegister(pChip, 0x1C4) |
      (uint64_t)fBoardFW->ReadOptoLinkRegister(pChip, 0x1C3) <<  8 |
      (uint64_t)fBoardFW->ReadOptoLinkRegister(pChip, 0x1C2) << 16 |
      (uint64_t)fBoardFW->ReadOptoLinkRegister(pChip, 0x1C1) << 24 |
      (uint64_t)fBoardFW->ReadOptoLinkRegister(pChip, 0x1C0) << 32;
    RD53lpGBTInterface::ResetBERT(pChip);

    uint64_t cBitErrorRate = (cBERTResult + 1) / cBitsChecked;

    return cBitErrorRate;
  }

  void RD53lpGBTInterface::ResetBERT(Chip* pChip)
  {
    fBoardFW->WriteOptoLinkRegister(pChip, 0x127, 0);
  }

  void RD53lpGBTInterface::SetBERTPattern(Chip* pChip, uint32_t pPattern)
  {
    this->setBoard(pChip->getBeBoardId());

    for (uint8_t cByte = 0; cByte < 4; cByte++)
      {
        uint cShift = (8*(3 - cByte));
        uint cPattern = (pPattern & (0xFF << cShift)) >> cShift;
        fBoardFW->WriteOptoLinkRegister(pChip, 0x128 + cByte, cPattern);
      }
  }

  void RD53lpGBTInterface::SetDPPattern(Chip* pChip, uint32_t pPattern)
  {
    LOG (INFO) << BOLDBLUE << "Loading pattern " << std::bitset<32>(pPattern) << " to lpGBT" << RESET;
    RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern0", (pPattern & 0xFF));
    RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern1", ((pPattern & 0xFF00) >> 8));
    RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern2", ((pPattern & 0xFF0000) >> 16));
    RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern3", ((pPattern & 0xFF000000) >> 24));
  }


  /*-------------------------------------------------------------------------*/
  /* More on Up/Down links                                                   */
  /*-------------------------------------------------------------------------*/
  void RD53lpGBTInterface::SetModeUpLink(Chip* pChip, uint8_t pSource, uint32_t pPattern)
  {
    if (pSource == 4 or pSource == 5) RD53lpGBTInterface::SetDPPattern(pChip, pPattern);

    RD53lpGBTInterface::WriteChipReg(pChip, "ULDataSource0", 0);
    RD53lpGBTInterface::WriteChipReg(pChip, "ULDataSource1", (0x0 << 6) | (pSource << 3) | (pSource << 0));
    RD53lpGBTInterface::WriteChipReg(pChip, "ULDataSource2", (pSource << 3) | (pSource << 0));
    RD53lpGBTInterface::WriteChipReg(pChip, "ULDataSource3", (pSource << 3) | (pSource << 0));
    RD53lpGBTInterface::WriteChipReg(pChip, "ULDataSource4", (pSource << 0));
  }

  void RD53lpGBTInterface::SetModeDownLink(Chip* pChip, uint8_t pSource, uint32_t pPattern)
  {
    this->setBoard(pChip->getBeBoardId());

    if (pSource == 3) RD53lpGBTInterface::SetDPPattern(pChip, pPattern);

    uint32_t cULDataSrcValue = 0;
    for (const auto& cGroup : fDLGroups)
      {
        uint8_t cShift = 2*cGroup;
        cULDataSrcValue = (cULDataSrcValue & ~(0x3 << cShift)) | (pSource << cShift);
      }
    fBoardFW->WriteOptoLinkRegister(pChip, 0x11D, cULDataSrcValue);
  }

  void RD53lpGBTInterface::FindPhase(Chip* pChip, uint8_t pTime, uint8_t pMaxPhase)
  {
    this->setBoard(pChip->getBeBoardId());

    for (auto cGroup : fBERTGroups)
      {
        uint64_t cBitsChecked = pow(2,(5 + 2*pTime))*32;
        std::vector<uint64_t> cBERVec;
        std::vector<uint8_t> cPhases(pMaxPhase+1);
        std::iota(cPhases.begin(), cPhases.end(), 0);

      LOG (DEBUG) << BOLDBLUE << "Finding best phase for group " << +cGroup << RESET;
      for (auto cPhase : cPhases)
      {
        RD53lpGBTInterface::ChangeUpLinksPhase(pChip, cPhase);
        cBERVec.push_back(RunBERT(pChip, pTime));
      }

      uint64_t cBestBER = *std::min_element(cBERVec.begin(), cBERVec.end());
      if (cBestBER > (2./cBitsChecked))
        LOG (DEBUG) << BOLDYELLOW << "Warning : best BER is " << +cBestBER << " - something is probably wrong" << RESET;

      size_t cMinPhaseIdxLow = std::distance(cBERVec.begin(), std::find(cBERVec.begin(), cBERVec.end(), cBestBER));
      size_t cMaxPhaseIdxLow = cMinPhaseIdxLow;
      for (size_t cBERIdx = cMinPhaseIdxLow; cBERIdx < cBERVec.size(); cBERIdx++)
        {
          if(cBERVec.at(cBERIdx+1) == cBestBER) cMaxPhaseIdxLow += 1;
          else break;
        }

      std::reverse(cBERVec.begin(), cBERVec.end());
      size_t cMinPhaseIdxHigh = std::distance(cBERVec.begin(), std::find(cBERVec.begin(), cBERVec.end(), cBestBER));
      // size_t cMaxPhaseIdxHigh = cMinPhaseIdxHigh;
      for (size_t cBERIdx = cMinPhaseIdxHigh; cBERIdx < cBERVec.size(); cBERIdx++)
        {
          if(cBERVec.at(cBERIdx+1) == cBestBER) cMaxPhaseIdxLow += 1;
          else break;
        }

      std::reverse(cBERVec.begin(), cBERVec.end());
      // uint8_t cBestPhase;
      // size_t cBestPhaseIdx;
      // if ((cMaxPhaseIdxLow - cMinPhaseIdxLow) > (cMaxPhaseIdxHigh - cMinPhaseIdxHigh))
      //   {
      //     cBestPhaseIdx = (cMinPhaseIdxLow + cMaxPhaseIdxLow)/2;
      //     cBestPhase    = cPhases.at(cBestPhaseIdx);
      //   }
      // else
      //   {
      //     cBestPhaseIdx = (cMinPhaseIdxHigh + cMaxPhaseIdxHigh)/2;
      //     cBestPhase    = cPhases.at(cBERVec.size() - cBestPhaseIdx - 1);
      //   }
      }
  }

  void RD53lpGBTInterface::ChangeUpLinksPhase(Chip* pChip, uint8_t pPhase)
  {
    this->setBoard(pChip->getBeBoardId());

    for (const auto& cGroup : fULGroups)
      for(const auto& cChannel : fULChannels)
        {
          uint32_t cValueChnCntr = fBoardFW->ReadOptoLinkRegister(pChip, 0x0CC + (4*cGroup) + cChannel);
          cValueChnCntr = (cValueChnCntr & ~(0xF << 4)) | (pPhase << 4);
          fBoardFW->WriteOptoLinkRegister(pChip, 0x0CC + 4*cGroup, cValueChnCntr);
      }
  }

  bool RD53lpGBTInterface::IslpGBTReady(Chip* pChip)
  {
    return (RD53lpGBTInterface::ReadChipReg(pChip, "PUSMStatus") == 0x12);
  }

  std::vector<std::pair<uint8_t, uint8_t>> RD53lpGBTInterface::GetRxStatus(Chip* pChip)
  {
    this->setBoard(pChip->getBeBoardId());

    std::vector<std::pair<uint8_t, uint8_t>> cRxStatusVec;
    for (const auto& cGroup : fULGroups)
      cRxStatusVec.push_back(std::make_pair(cGroup, fBoardFW->ReadOptoLinkRegister(pChip, 0x158 + cGroup)));

    return cRxStatusVec;
  }
}
