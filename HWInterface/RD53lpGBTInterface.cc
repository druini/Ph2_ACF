/*!
  \file                  RD53lpGBTInterface.cc
  \brief                 Interface to access and control the Low-power Gigabit Transceiver chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  03/03/20
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53lpGBTInterface.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
// ###################################
// # LpGBT register access functions #
// ###################################
bool RD53lpGBTInterface::ConfigureChip(Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    this->setBoard(pChip->getBeBoardId());

    // #####################
    // # Make reverted map #
    // #####################
    for(auto& ele: fPUSMStatusMap) revertedPUSMStatusMap[ele.second] = ele.first;

    // ###############################
    // # In case of not efused LpGBT #
    // ###############################
    // @TMP@
    // ChipRegMap& lpGBTRegMap = pChip->getRegMap();
    // for(const auto& cRegItem: lpGBTRegMap)
    //     if((cRegItem.second.fAddress < 0x13C) && (cRegItem.second.fPrmptCfg == true)) RD53lpGBTInterface::WriteReg(pChip, cRegItem.second.fAddress, cRegItem.second.fValue);

    // #########################
    // # Configure PLL and DLL #
    // #########################
    RD53lpGBTInterface::WriteChipReg(pChip, "LDConfigH", 1 << 5, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "EPRXLOCKFILTER", 0x55, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "EPRXDllConfig", 1 << 6 | 1 << 4 | 1 << 2, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "PSDllConfig", 5 << 4 | 1 << 2 | 1, false);
    RD53lpGBTInterface::WriteChipReg(pChip, "POWERUP2", 1 << 2 | 1 << 1, false);

    // #####################
    // # Check PUSM status #
    // #####################
    uint8_t      PUSMStatus = RD53lpGBTInterface::GetPUSMStatus(pChip);
    unsigned int nAttempts  = 0;
    while((PUSMStatus != revertedPUSMStatusMap["READY"]) && (nAttempts < RD53lpGBTconstants::MAXATTEMPTS))
    {
        PUSMStatus = RD53lpGBTInterface::GetPUSMStatus(pChip);
        usleep(RD53Shared::DEEPSLEEP);
        nAttempts++;
    }

    if(PUSMStatus != revertedPUSMStatusMap["READY"])
    {
        LOG(ERROR) << BOLDRED << "LpGBT PUSM status: " << BOLDYELLOW << fPUSMStatusMap[PUSMStatus] << RESET;
        return false;
    }
    LOG(INFO) << GREEN << "LpGBT PUSM status: " << BOLDYELLOW << fPUSMStatusMap[PUSMStatus] << RESET;

    RD53lpGBTInterface::PrintChipMode(pChip);

    return true;
}

bool RD53lpGBTInterface::WriteChipReg(Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
{
    return RD53lpGBTInterface::WriteReg(pChip, pChip->getRegItem(pRegNode).fAddress, pValue, pVerifLoop);
}

uint16_t RD53lpGBTInterface::ReadChipReg(Chip* pChip, const std::string& pRegNode) { return RD53lpGBTInterface::ReadReg(pChip, pChip->getRegItem(pRegNode).fAddress); }

bool RD53lpGBTInterface::WriteReg(Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop)
{
    this->setBoard(pChip->getBeBoardId());

    if(pValue > 0xFF)
    {
        LOG(ERROR) << BOLDRED << "LpGBT registers are 8 bits, impossible to write " << BOLDYELLOW << pValue << BOLDRED << " to address " << BOLDYELLOW << pAddress << RESET;
        return false;
    }

    if(pAddress >= 0x13C)
    {
        LOG(ERROR) << "LpGBT read-write registers end at 0x13C ... impossible to write to address " << BOLDYELLOW << pAddress << RESET;
        return false;
    }

    int  nAttempts = 0;
    bool status;
    do
    {
        status = fBoardFW->WriteOptoLinkRegister(pAddress, pValue, pVerifLoop);
        nAttempts++;
    } while((pVerifLoop == true) && (status == false) && (nAttempts < RD53lpGBTconstants::MAXATTEMPTS));

    if((pVerifLoop == true) && (status == false)) throw std::runtime_error(std::string("LpGBT register writing issue"));

    return true;
}

 uint16_t RD53lpGBTInterface::ReadReg(Chip* pChip, uint16_t pAddress)
 {
     this->setBoard(pChip->getBeBoardId());
     return fBoardFW->ReadOptoLinkRegister(pAddress);
 }

 bool RD53lpGBTInterface::WriteChipMultReg(Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& pRegVec, bool pVerifLoop)
 {
     bool writeGood = true;
     for(const auto& cReg: pRegVec) writeGood &= RD53lpGBTInterface::WriteChipReg(pChip, cReg.first, cReg.second);
     return writeGood;
 }

 // #######################################
 // # LpGBT block configuration functions #
 // #######################################
 void RD53lpGBTInterface::ConfigureRxGroups(Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate, uint8_t pTrackMode)
 {
     for(const auto& cGroup: pGroups)
     {
         // Enable Rx Groups Channels and set Data Rate and Phase Tracking mode
         uint8_t cValueEnableRx = 0;
         for(const auto cChannel: pChannels) cValueEnableRx += (1 << cChannel);
         std::string cRXCntrlReg = "EPRX" + std::to_string(cGroup) + "Control";
         RD53lpGBTInterface::WriteChipReg(pChip, cRXCntrlReg, (cValueEnableRx << 4) | (pDataRate << 2) | (pTrackMode << 0));
     }
 }

 void RD53lpGBTInterface::ConfigureRxChannels(Chip*                       pChip,
                                              const std::vector<uint8_t>& pGroups,
                                              const std::vector<uint8_t>& pChannels,
                                              uint8_t                     pEqual,
                                              uint8_t                     pTerm,
                                              uint8_t                     pAcBias,
                                              uint8_t                     pInvert,
                                              uint8_t                     pPhase)
 {
     for(const auto& cGroup: pGroups)
     {
         for(const auto& cChannel: pChannels)
         {
             // Configure Rx Channel Phase, Inversion, AcBias enabling, Termination enabling, Equalization enabling
             std::string cRXChnCntrReg = "EPRX" + std::to_string(cGroup) + std::to_string(cChannel) + "ChnCntr";
             RD53lpGBTInterface::WriteChipReg(pChip, cRXChnCntrReg, (pPhase << 4) | (pInvert << 3) | (pAcBias << 2) | (pTerm << 1) | (pEqual << 0));
         }
     }
 }

 void RD53lpGBTInterface::ConfigureTxGroups(Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate)
 {
     for(const auto& cGroup: pGroups)
     {
         // Configure Tx Group Data Rate value for specified group
         uint8_t cValueDataRate = RD53lpGBTInterface::ReadChipReg(pChip, "EPTXDataRate");
         RD53lpGBTInterface::WriteChipReg(pChip, "EPTXDataRate", (cValueDataRate & ~(0x03 << 2 * cGroup)) | (pDataRate << 2 * cGroup));

         // Enable given channels for specified group
         std::string cEnableTxReg;
         if(cGroup == 0 || cGroup == 1)
             cEnableTxReg = "EPTX10Enable";
         else if(cGroup == 2 || cGroup == 3)
             cEnableTxReg = "EPTX32Enable";

         uint8_t cValueEnableTx = RD53lpGBTInterface::ReadChipReg(pChip, cEnableTxReg);
         for(const auto cChannel: pChannels) cValueEnableTx += (1 << (cChannel + 4 * (cGroup % 2)));
         RD53lpGBTInterface::WriteChipReg(pChip, cEnableTxReg, cValueEnableTx);
     }
 }

 void RD53lpGBTInterface::ConfigureTxChannels(Chip*                       pChip,
                                              const std::vector<uint8_t>& pGroups,
                                              const std::vector<uint8_t>& pChannels,
                                              uint8_t                     pDriveStr,
                                              uint8_t                     pPreEmphMode,
                                              uint8_t                     pPreEmphStr,
                                              uint8_t                     pPreEmphWidth,
                                              uint8_t                     pInvert)
 {
     for(const auto& cGroup: pGroups)
     {
         for(const auto& cChannel: pChannels)
         {
             // Configure Tx Channel PreEmphasisStrenght, PreEmphasisMode, DriveStrength
             std::string cTXChnCntrl = "EPTX" + std::to_string(cGroup) + std::to_string(cChannel) + "ChnCntr";
             RD53lpGBTInterface::WriteChipReg(pChip, cTXChnCntrl, (pPreEmphStr << 5) | (pPreEmphMode << 3) | (pDriveStr << 0));

             // Configure Tx Channel PreEmphasisWidth, Inversion
             std::string cTXChn_Cntr;
             if(cChannel == 0 || cChannel == 1)
                 cTXChn_Cntr = "EPTX" + std::to_string(cGroup) + "1_" + std::to_string(cGroup) + "0ChnCntr";
             else if(cChannel == 2 || cChannel == 3)
                 cTXChn_Cntr = "EPTX" + std::to_string(cGroup) + "3_" + std::to_string(cGroup) + "2ChnCntr";

             uint8_t cValue_ChnCntr = RD53lpGBTInterface::ReadChipReg(pChip, cTXChn_Cntr);
             RD53lpGBTInterface::WriteChipReg(pChip, cTXChn_Cntr, (cValue_ChnCntr & ~(0x0F << 4 * (cChannel % 2))) | ((pInvert << 3 | pPreEmphWidth << 0) << 4 * (cChannel % 2)));
         }
     }
 }

 void RD53lpGBTInterface::ConfigureClocks(Chip*                       pChip,
                                          const std::vector<uint8_t>& pClocks,
                                          uint8_t                     pFreq,
                                          uint8_t                     pDriveStr,
                                          uint8_t                     pInvert,
                                          uint8_t                     pPreEmphWidth,
                                          uint8_t                     pPreEmphMode,
                                          uint8_t                     pPreEmphStr)
 {
     for(const auto& cClock: pClocks)
     {
         // Configure Clocks Frequency, Drive Strength, Inversion, Pre-Emphasis Width, Pre-Emphasis Mode, Pre-Emphasis Strength
         std::string cClkHReg = "EPCLK" + std::to_string(cClock) + "ChnCntrH";
         std::string cClkLReg = "EPCLK" + std::to_string(cClock) + "ChnCntrL";
         RD53lpGBTInterface::WriteChipReg(pChip, cClkHReg, pInvert << 6 | pDriveStr << 3 | pFreq);
         RD53lpGBTInterface::WriteChipReg(pChip, cClkLReg, pPreEmphStr << 5 | pPreEmphMode << 3 | pPreEmphWidth);
     }
 }

 void RD53lpGBTInterface::ConfigureHighSpeedPolarity(Chip* pChip, uint8_t pOutPolarity, uint8_t pInPolarity)
 {
     // Configure High Speed Link Rx and Tx polarity
     uint8_t cPolarity = (pOutPolarity << 7 | pInPolarity << 6);
     RD53lpGBTInterface::WriteChipReg(pChip, "ChipConfig", cPolarity);
 }

 void RD53lpGBTInterface::ConfigureDPPattern(Chip* pChip, uint32_t pPattern)
 {
     // Configure Constant Pattern
     RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern0", (pPattern & 0xFF));
     RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern1", ((pPattern & 0xFF00) >> 8));
     RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern2", ((pPattern & 0xFF0000) >> 16));
     RD53lpGBTInterface::WriteChipReg(pChip, "DPDataPattern3", ((pPattern & 0xFF000000) >> 24));
 }

 void RD53lpGBTInterface::ConfigureRxPRBS(Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable)
 {
     // Configure build-in PRBS generators
     for(const auto& cGroup: pGroups)
     {
         std::string cPRBSReg;
         if(cGroup == 1 || cGroup == 0)
             cPRBSReg = "EPRXPRBS0";
         else if(cGroup == 3 || cGroup == 2)
             cPRBSReg = "EPRXPRBS1";
         else if(cGroup == 5 || cGroup == 4)
             cPRBSReg = "EPRXPRBS2";
         else if(cGroup == 6)
             cPRBSReg = "EPRXPRBS3";

         uint8_t cEnabledCh       = 0;
         uint8_t cValueEnablePRBS = RD53lpGBTInterface::ReadChipReg(pChip, cPRBSReg);
         for(const auto cChannel: pChannels) cEnabledCh |= pEnable << cChannel;
         RD53lpGBTInterface::WriteChipReg(pChip, cPRBSReg, (cValueEnablePRBS & ~(0xF << 4 * (cGroup % 2))) | (cEnabledCh << (4 * (cGroup % 2))));
     }
 }

 void RD53lpGBTInterface::ConfigureRxSource(Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource)
 {
     for(const auto& cGroup: pGroups)
     {
         if(pSource == 0)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to NORMAL " << RESET;
         else if(pSource == 1)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to PRBS7 " << RESET;
         else if(pSource == 4 || pSource == 5)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to Constant Pattern" << RESET;

         std::string cRxSourceReg;
         if(cGroup == 0 || cGroup == 1)
             cRxSourceReg = "ULDataSource1";
         else if(cGroup == 2 || cGroup == 3)
             cRxSourceReg = "ULDataSource2";
         else if(cGroup == 4 || cGroup == 5)
             cRxSourceReg = "ULDataSource3";
         else if(cGroup == 6)
             cRxSourceReg = "ULDataSource4";

         uint8_t cValueRxSource = RD53lpGBTInterface::ReadChipReg(pChip, cRxSourceReg);
         RD53lpGBTInterface::WriteChipReg(pChip, cRxSourceReg, (cValueRxSource & ~(0x7 << 3 * (cGroup % 2))) | (pSource << 3 * (cGroup % 2)));
     }
 }

 void RD53lpGBTInterface::ConfigureTxSource(Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource)
 {
     for(const auto& cGroup: pGroups)
     {
         if(pSource == 0)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to NORMAL " << RESET;
         else if(pSource == 1)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to PRBS7 " << RESET;
         else if(pSource == 2)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to Binary counter " << RESET;
         else if(pSource == 3)
             LOG(INFO) << GREEN << "Configuring Rx Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " Source to Constant Pattern" << RESET;

         uint8_t cULDataSrcValue = RD53lpGBTInterface::ReadChipReg(pChip, "ULDataSource5");
         cULDataSrcValue         = (cULDataSrcValue & ~(0x3 << (2 * cGroup))) | (pSource << (2 * cGroup));
         RD53lpGBTInterface::WriteChipReg(pChip, "ULDataSource5", cULDataSrcValue);
     }
 }

 void RD53lpGBTInterface::ConfigureRxPhase(Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase)
 {
     std::string cRegName      = "EPRX" + std::to_string(pGroup) + std::to_string(pChannel) + "ChnCntr";
     uint8_t     cValueChnCntr = RD53lpGBTInterface::ReadChipReg(pChip, cRegName);
     cValueChnCntr             = (cValueChnCntr & ~(0xF << 4)) | (pPhase << 4);
     RD53lpGBTInterface::WriteChipReg(pChip, cRegName, cValueChnCntr);
 }

 void RD53lpGBTInterface::ConfigurePhShifter(Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq, uint8_t pDriveStr, uint8_t pEnFTune, uint16_t pDelay)
 {
     for(const auto& cClock: pClocks)
     {
         std::string cDelayReg  = "PS" + std::to_string(cClock) + "Delay";
         std::string cConfigReg = "PS" + std::to_string(cClock) + "Config";
         RD53lpGBTInterface::WriteChipReg(pChip, cConfigReg, (((pDelay & 0x100) >> 8) << 7) | pEnFTune << 6 | pDriveStr << 3 | pFreq);
         RD53lpGBTInterface::WriteChipReg(pChip, cDelayReg, pDelay);
     }
 }

 // ####################################
 // # LpGBT specific routine functions #
 // ####################################
 void RD53lpGBTInterface::PhaseTrainRx(Chip* pChip, const std::vector<uint8_t>& pGroups)
 {
     for(const auto& cGroup: pGroups)
     {
         std::string cTrainRxReg;
         if(cGroup == 0 || cGroup == 1)
             cTrainRxReg = "EPRXTrain10";
         else if(cGroup == 2 || cGroup == 3)
             cTrainRxReg = "EPRXTrain32";
         else if(cGroup == 4 || cGroup == 5)
             cTrainRxReg = "EPRXTrain32";
         else if(cGroup == 6)
             cTrainRxReg = "EPRXTrain32";

         RD53lpGBTInterface::WriteChipReg(pChip, cTrainRxReg, 0x0F << 4 * (cGroup % 2));
         RD53lpGBTInterface::WriteChipReg(pChip, cTrainRxReg, 0x00 << 4 * (cGroup % 2));
     }
 }

 void RD53lpGBTInterface::PhaseAlignRx(Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pRate)
 {
     // Set data source for channels 0,2 to PRBS
     RD53lpGBTInterface::ConfigureRxSource(pChip, pGroups, 1);
     // Turn ON PRBS for channels 0,2
     RD53lpGBTInterface::ConfigureRxPRBS(pChip, pGroups, pChannels, true);

     // Configure Rx Phase Shifter
     uint16_t cDelay = 0x00;
     uint8_t  cFreq = (pRate = 5) ? 4 : 5, cEnFTune = 0, cDriveStr = 0; // 4 --> 320 MHz || 5 --> 640 MHz
     RD53lpGBTInterface::ConfigurePhShifter(pChip, {0, 1, 2, 3}, cFreq, cDriveStr, cEnFTune, cDelay);
     // Phase Train channels 0,2
     RD53lpGBTInterface::PhaseTrainRx(pChip, pGroups);
     for(const auto& cGroup: pGroups)
     {
         // Wait until channels lock
         LOG(INFO) << GREEN << "Phase Aligning Rx Group " << BOLDYELLOW << +cGroup << RESET;
         do
         {
             std::this_thread::sleep_for(std::chrono::milliseconds(10));
         } while(!IsRxLocked(pChip, cGroup, pChannels));
         LOG(INFO) << GREEN << "Group " << BOLDYELLOW << +cGroup << RESET << GREEN << " LOCKED" << RESET;

         // Set new phase to channels 0,2
         for(const auto cChannel: pChannels)
         {
             uint8_t cCurrPhase = RD53lpGBTInterface::GetRxPhase(pChip, cGroup, cChannel);
             LOG(INFO) << GREEN << "Channel " << BOLDYELLOW << +cChannel << RESET << GREEN << " phase is " << BOLDYELLOW << +cCurrPhase << RESET;
             RD53lpGBTInterface::ConfigureRxPhase(pChip, cGroup, cChannel, cCurrPhase);
         }
     }

     // Set back Rx source to Normal data
     RD53lpGBTInterface::ConfigureRxSource(pChip, pGroups, 0);
     // Turn off PRBS for channels 0,2
     RD53lpGBTInterface::ConfigureRxPRBS(pChip, pGroups, pChannels, false);
 }

 // ################################
 // # LpGBT Block Status functions #
 // ################################
 void RD53lpGBTInterface::PrintChipMode(Chip* pChip)
 {
     switch((ReadChipReg(pChip, "ConfigPins") & 0xF0) >> 4)
     {
     case 0: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC5; LpGBT Mode = Off" << RESET; break;
     case 1: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC5; LpGBT Mode = Simplex TX" << RESET; break;
     case 2: LOG(INFO) << GREEN << "LpGBT CHIP INFO; Tx Data Rate = 5Gbps; TxEncoding = FEC5; LpGBT Mode = Simplex RX" << RESET; break;
     case 3: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC5; LpGBT Mode = Transceiver" << RESET; break;
     case 4: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC12; LpGBT Mode = Off" << RESET; break;
     case 5: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC12; LpGBT Mode = Simplex TX" << RESET; break;
     case 6: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC12; LpGBT Mode = Simplex RX" << RESET; break;
     case 7: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 5Gbps; TxEncoding = FEC12; LpGBT Mode = Transceiver" << RESET; break;
     case 8: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC5; LpGBT Mode = Off" << RESET; break;
     case 9: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC5; LpGBT Mode = Simplex TX" << RESET; break;
     case 10: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC5; LpGBT Mode = Simplex RX" << RESET; break;
     case 11: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC5; LpGBT Mode = Transceiver" << RESET; break;
     case 12: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC12; LpGBT Mode = Off" << RESET; break;
     case 13: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC12; LpGBT Mode = Simplex TX" << RESET; break;
     case 14: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC12; LpGBT Mode = Simplex RX" << RESET; break;
     case 15: LOG(INFO) << GREEN << "LpGBT CHIP INFO: Tx Data Rate = 10Gbps; TxEncoding = FEC12; LpGBT Mode = Transceiver" << RESET; break;
    }
}

uint8_t RD53lpGBTInterface::GetPUSMStatus(Chip* pChip) { return RD53lpGBTInterface::ReadChipReg(pChip, "PUSMStatus"); }

uint8_t RD53lpGBTInterface::GetRxPhase(Chip* pChip, uint8_t pGroup, uint8_t pChannel)
{
    std::string cRxPhaseReg;
    if(pChannel == 0 || pChannel == 1)
        cRxPhaseReg = "EPRX" + std::to_string(pGroup) + "CurrentPhase10";
    else if(pChannel == 3 || pChannel == 2)
        cRxPhaseReg = "EPRX" + std::to_string(pGroup) + "CurrentPhase32";

    uint8_t cRxPhaseRegValue = RD53lpGBTInterface::ReadChipReg(pChip, cRxPhaseReg);
    return ((cRxPhaseRegValue & (0x0F << 4 * (pChannel % 2))) >> 4 * (pChannel % 2));
}

bool RD53lpGBTInterface::IsRxLocked(Chip* pChip, uint8_t pGroup, const std::vector<uint8_t>& pChannels)
{
    std::string cRXLockedReg = "EPRX" + std::to_string(pGroup) + "Locked";
    uint8_t     cChannelMask = 0x00;
    for(auto cChannel: pChannels) cChannelMask += (1 << cChannel);
    return (((RD53lpGBTInterface::ReadChipReg(pChip, cRXLockedReg) & (cChannelMask << 4)) >> 4) == cChannelMask);
}

uint8_t RD53lpGBTInterface::GetRxDllStatus(Chip* pChip, uint8_t pGroup)
{
    std::string cRXDllStatReg = "EPRX" + std::to_string(pGroup) + "DllStatus";
    return RD53lpGBTInterface::ReadChipReg(pChip, cRXDllStatReg);
}

// ########################
// # LpGBT GPIO functions #
// ########################
void RD53lpGBTInterface::ConfigureGPIO(Chip* pChip, const std::vector<uint8_t>& pGPIOs, uint8_t pDir, uint8_t pOut, uint8_t pDriveStr, uint8_t pPullEn, uint8_t pUpDown)
{
    uint8_t cDirH      = RD53lpGBTInterface::ReadChipReg(pChip, "PIODirH");
    uint8_t cDirL      = RD53lpGBTInterface::ReadChipReg(pChip, "PIODirL");
    uint8_t cOutH      = RD53lpGBTInterface::ReadChipReg(pChip, "PIOOutH");
    uint8_t cOutL      = RD53lpGBTInterface::ReadChipReg(pChip, "PIOOutL");
    uint8_t cDriveStrH = RD53lpGBTInterface::ReadChipReg(pChip, "PIODriveStrengthH");
    uint8_t cDriveStrL = RD53lpGBTInterface::ReadChipReg(pChip, "PIODriveStrengthL");
    uint8_t cPullEnH   = RD53lpGBTInterface::ReadChipReg(pChip, "PIOPullEnaH");
    uint8_t cPullEnL   = RD53lpGBTInterface::ReadChipReg(pChip, "PIOPullEnaL");
    uint8_t cUpDownH   = RD53lpGBTInterface::ReadChipReg(pChip, "PIOUpDownH");
    uint8_t cUpDownL   = RD53lpGBTInterface::ReadChipReg(pChip, "PIOUpDownL");

    for(auto cGPIO: pGPIOs)
    {
        if(cGPIO < 8)
        {
            cDirL |= (pDir << cGPIO);
            cOutL |= (pOut << cGPIO);
            cDriveStrL |= (pDriveStr << cGPIO);
            cPullEnL |= (pPullEn << cGPIO);
            cUpDownL |= (pUpDown << cGPIO);
        }
        else
        {
            cDirH |= (pDir << (cGPIO - 8));
            cOutH |= (pOut << (cGPIO - 8));
            cDriveStrH |= (pDriveStr << (cGPIO - 8));
            cPullEnH |= (pPullEn << (cGPIO - 8));
            cUpDownH |= (pUpDown << (cGPIO - 8));
        }
    }

    RD53lpGBTInterface::WriteChipReg(pChip, "PIODirH", cDirH);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIODirL", cDirL);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIOOutH", cOutH);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIOOutL", cOutL);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIODriveStrengthH", cDriveStrH);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIODriveStrengthL", cDriveStrL);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIOPullEnaH", cPullEnH);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIOPullEnaL", cPullEnL);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIOUpDownH", cUpDownH);
    RD53lpGBTInterface::WriteChipReg(pChip, "PIOUpDownL", cUpDownL);
}

// ###########################
// # LpGBT ADC-DAC functions #
// ###########################
void RD53lpGBTInterface::ConfigureADC(Chip* pChip, uint8_t pGainSelect, uint8_t pADCEnable) { RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", pADCEnable << 2 | pGainSelect); }

uint16_t RD53lpGBTInterface::ReadADC(Chip* pChip, const std::string& pADCInput)
{
    // Read (converted) data from ADC Input with VREF/2 as negative Input
    uint8_t cADCInput = fADCInputMap[pADCInput];
    uint8_t cVREF     = fADCInputMap["VREF/2"];

    LOG(INFO) << GREEN << "Reading ADC value from " << BOLDYELLOW << pADCInput << RESET;

    // Select ADC Input
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCSelect", cADCInput << 4 | cVREF << 0);
    // Enable ADC Input
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", 1 << 2);
    // Enable Internal VREF
    RD53lpGBTInterface::WriteChipReg(pChip, "VREFCNTR", 1 << 7);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Start ADC conversion
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", 1 << 7 | 1 << 2);

    // Check conversion status
    unsigned int nAttempts = 0;
    bool         cSuccess  = false;
    do
    {
        LOG(INFO) << GREEN << "Waiting for ADC conversion to end" << RESET;

        uint8_t cStatus = RD53lpGBTInterface::ReadChipReg(pChip, "ADCStatusH");
        cSuccess        = (((cStatus & 0x40) >> 6) == 1);
        nAttempts++;
    } while((nAttempts < RD53lpGBTconstants::MAXATTEMPTS) && (cSuccess == false));

    // Read ADC value
    uint8_t cADCvalue1 = RD53lpGBTInterface::ReadChipReg(pChip, "ADCStatusH") & 0x3;
    uint8_t cADCvalue2 = RD53lpGBTInterface::ReadChipReg(pChip, "ADCStatusL");
    // Clear ADC conversion bit (#FIXME disable ADC Input as well ??)
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", 0 << 7 | 1 << 2);

    return (cADCvalue1 << 8 | cADCvalue2);
}

uint16_t RD53lpGBTInterface::ReadADCDiff(Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN)
{
    // Read differential (converted) data on two ADC inputs
    uint8_t cADCInputP = fADCInputMap[pADCInputP];
    uint8_t cADCInputN = fADCInputMap[pADCInputN];

    LOG(INFO) << GREEN << "Reading ADC value from " << BOLDYELLOW << pADCInputP << RESET;

    // Select ADC Input
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCSelect", cADCInputP << 4 | cADCInputN << 0);
    // Enable ADC Input
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", 1 << 2);
    // Start ADC conversion
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", 1 << 7 | 1 << 2);

    // Check conversion status
    unsigned int nAttempts = 0;
    bool         cSuccess  = false;
    do
    {
        LOG(INFO) << GREEN << "Waiting for ADC conversion to end" << RESET;

        uint8_t cStatus = RD53lpGBTInterface::ReadChipReg(pChip, "ADCStatusH");
        cSuccess        = (((cStatus & 0x40) >> 6) == 1);
        nAttempts++;
    } while((nAttempts < RD53lpGBTconstants::MAXATTEMPTS) && (cSuccess == false));

    // Read ADC value
    uint8_t cADCvalue1 = RD53lpGBTInterface::ReadChipReg(pChip, "ADCStatusH") & 0x3;
    uint8_t cADCvalue2 = RD53lpGBTInterface::ReadChipReg(pChip, "ADCStatusL");
    // Clear ADC conversion bit (#FIXME disable ADC Input as well ??)
    RD53lpGBTInterface::WriteChipReg(pChip, "ADCConfig", 0 << 7 | 1 << 2);

    return (cADCvalue1 << 8 | cADCvalue2);
}

// #####################
// # LpGBT BERT Tester #
// #####################
void RD53lpGBTInterface::ConfigureBERT(Chip* pChip, uint8_t pCoarseSource, uint8_t pFineSource, uint8_t pMeasTime, uint8_t pSkipDisable, bool pStart)
{
    if(pStart == true)
        RD53lpGBTInterface::WriteChipReg(pChip, "BERTSource", (pCoarseSource << 4) | pFineSource);
    else
        RD53lpGBTInterface::WriteChipReg(pChip, "BERTConfig", (pMeasTime << 4) | (pSkipDisable << 1) | pStart);
}

void RD53lpGBTInterface::ConfigureBERTPattern(Chip* pChip, uint32_t pPattern)
{
    RD53lpGBTInterface::WriteChipReg(pChip, "BERTDataPattern0", (pPattern & (0xFF << 0)) >> 0);
    RD53lpGBTInterface::WriteChipReg(pChip, "BERTDataPattern1", (pPattern & (0xFF << 8)) >> 8);
    RD53lpGBTInterface::WriteChipReg(pChip, "BERTDataPattern2", (pPattern & (0xFF << 16)) >> 16);
    RD53lpGBTInterface::WriteChipReg(pChip, "BERTDataPattern3", (pPattern & (0xFF << 24)) >> 24);
}

uint8_t RD53lpGBTInterface::GetBERTStatus(Chip* pChip) { return RD53lpGBTInterface::ReadChipReg(pChip, "BERTStatus"); }

uint64_t RD53lpGBTInterface::GetBERTErrors(Chip* pChip)
{
    uint64_t cResult0 = RD53lpGBTInterface::ReadChipReg(pChip, "BERTResult0");
    uint64_t cResult1 = RD53lpGBTInterface::ReadChipReg(pChip, "BERTResult1");
    uint64_t cResult2 = RD53lpGBTInterface::ReadChipReg(pChip, "BERTResult2");
    uint64_t cResult3 = RD53lpGBTInterface::ReadChipReg(pChip, "BERTResult3");
    uint64_t cResult4 = RD53lpGBTInterface::ReadChipReg(pChip, "BERTResult4");
    return ((cResult4 << 32) | (cResult3 << 24) | (cResult2 << 16) | (cResult1 << 8) | cResult0);
}

float RD53lpGBTInterface::PerformBERTest(Chip* pChip, uint8_t pCoarseSource, uint8_t pFineSource, uint8_t pMeasTime, uint8_t pSkipDisable, uint32_t pPattern)
{
    if(pPattern == 0)
        LOG(INFO) << GREEN << "Performing BER test with PRBS" << RESET;
    else
    {
        LOG(INFO) << GREEN << "Performing BER test with Constant Pattern" << RESET;

        RD53lpGBTInterface::ConfigureDPPattern(pChip, pPattern);
        RD53lpGBTInterface::ConfigureBERTPattern(pChip, pPattern);
    }
    RD53lpGBTInterface::ConfigureBERT(pChip, pCoarseSource, pFineSource, pMeasTime, pSkipDisable, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    uint8_t cBERTStatus = RD53lpGBTInterface::GetBERTStatus(pChip);
    bool    cAllZeros   = ((cBERTStatus & (0x1 << 2)) >> 2) == 1;

    if(cAllZeros == true) throw std::runtime_error(std::string("BERT: All zeros at input"));

    while((cBERTStatus & 0x1) != 1)
    {
        LOG(INFO) << GREEN << "BERT still running ... status is: " << BOLDYELLOW << std::bitset<3>(cBERTStatus) << RESET;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cBERTStatus = RD53lpGBTInterface::GetBERTStatus(pChip);
    }

    LOG(INFO) << GREEN << "Reading BERT counter" << RESET;

    uint64_t cErrors      = RD53lpGBTInterface::GetBERTErrors(pChip);
    uint32_t cBitsChecked = std::pow(2, 5 + pMeasTime * 2) * 16; // # @TMP@ : currently hard coded for 640 MHz

    LOG(INFO) << GREEN << "Bits checked : " << BOLDYELLOW << +cBitsChecked << RESET << GREEN << " bits" << RESET;
    LOG(INFO) << GREEN << "Bits in error: " << BOLDYELLOW << +cErrors << RESET << GREEN << " bits" << RESET;

    RD53lpGBTInterface::ConfigureBERT(pChip, pCoarseSource, pFineSource, pMeasTime, pSkipDisable, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    LOG(INFO) << "BER test done" << RESET;

    return float(cErrors) / cBitsChecked;
}
} // namespace Ph2_HwInterface
