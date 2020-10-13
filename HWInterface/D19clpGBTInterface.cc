/*!
  \file                  D19clpGBTInterface.cc
  \brief                 Interface to access and control the low-power Gigabit Transceiver chip
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
*/

#include "D19clpGBTInterface.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
bool D19clpGBTInterface::ConfigureChip(Ph2_HwDescription::Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    LOG(INFO) << BOLDBLUE << "Configuring lpGBT" << RESET;
    this->setBoard(pChip->getBeBoardId());
/*
     //Load register map from configuration file
     ChipRegMap clpGBTRegMap = pChip->getRegMap();
     for(const auto& cRegItem: clpGBTRegMap)
     {
         if(cRegItem.second.fAddress < 0x13c)
         {
             LOG(INFO) << BOLDBLUE << "Writing 0x" << std::hex << +cRegItem.second.fValue << std::dec << " to " << cRegItem.first << " [0x" << std::hex << +cRegItem.second.fAddress << std::dec << "]"
                       << RESET;
             this->WriteReg(pChip, cRegItem.second.fAddress, cRegItem.second.fValue);
         }
     }
    // Configure High Speed Link Tx Rx Polarity
    this->ConfigureHighSpeedPolarity(pChip, 1, 0);
*/
    //#FIXME YOUNES BLOCK PLEASE DON'T MODIFY
    // Clocks
    std::vector<uint8_t> cClocks         = {1, 6, 11, 26};
    uint8_t              cClkFreq = 4, cClkDriveStr = 7, cClkInvert = 1;
    uint8_t              cClkPreEmphWidth = 0, cClkPreEmphMode = 0, cClkPreEmphStr = 0;
    this->ConfigureClocks(pChip, cClocks, cClkFreq, cClkDriveStr, cClkInvert, cClkPreEmphWidth, cClkPreEmphMode, cClkPreEmphStr);
    //Tx Groups and Channels
    std::vector<uint8_t> cTxGroups = {0, 1, 2, 3}, cTxChannels = {0};
    uint8_t              cTxDataRate = 3, cTxDriveStr = 7, cTxPreEmphMode = 1, cTxPreEmphStr = 4, cTxPreEmphWidth = 0, cTxInvert = 1;
    this->ConfigureTxSource(pChip, cTxGroups, 0); // 0 --> link data, 3 --> constant pattern
    this->ConfigureTxGroups(pChip, cTxGroups, cTxChannels, cTxDataRate);
    for(const auto& cGroup: cTxGroups)
    {
      cTxInvert = (cGroup % 2 == 0) ? 1 : 0;
      for(const auto& cChannel: cTxChannels)
        this->ConfigureTxChannels(pChip, {cGroup}, {cChannel}, cTxDriveStr, cTxPreEmphMode, cTxPreEmphStr, cTxPreEmphWidth, cTxInvert);
    }
    // Rx configuration and Phase Align 
    // Configure Rx Groups
    std::vector<uint8_t> cRxGroups = {0, 1, 2, 3, 4, 5, 6}, cRxChannels = {0, 2};
    uint8_t cRxDataRate = 2, cRxTrackMode = 0;
    this->ConfigureRxGroups(pChip, cRxGroups, cRxChannels, cRxDataRate, cRxTrackMode);
    // Configure Rx Channels
    uint8_t cRxEqual = 0, cRxTerm = 1, cRxAcBias = 1, cRxInvert = 0, cRxPhase = 12;
    for(const auto& cGroup: cRxGroups)
    {
        for(const auto cChannel: cRxChannels)
        {
            if(cGroup == 0 && cChannel == 0)
                cRxInvert = 1;
            else if(cGroup == 1 && cChannel == 0)
                cRxInvert = 1;
            else if(cGroup == 3 && cChannel == 2)
                cRxInvert = 1;
            else if(cGroup == 2)
                cRxInvert = 0;
            this->ConfigureRxChannels(pChip, {cGroup}, {cChannel}, cRxEqual, cRxTerm, cRxAcBias, cRxInvert, cRxPhase);
        }
    }
    /*
    this->ConfigureRxPRBS(pChip, cRxGroups, cRxChannels, true);
    this->PhaseAlignRx(pChip, cRxGroups, cRxChannels);
    this->ConfigureRxGroups(pChip, cRxGroups, cRxChannels, cRxDataRate, 0);
    */
    // Reset I2C Masters
    this->ResetI2C(pChip, {0, 1, 2});
    // setting GPIO levels Uncomment this for Skeleton test
    this->WriteChipReg(pChip, "PIODirH", 0x12);
    this->WriteChipReg(pChip, "PIODirL", 0x4B);
    this->WriteChipReg(pChip, "PIOOutH", 0x12);
    this->WriteChipReg(pChip, "PIOOutL", 0x4B);
    
//    this->WriteChipReg(pChip, "POWERUP2", 0x06);
/*
    // LET'S  USE BERT 
    this->WriteChipReg(pChip, "BERTSource", ( 1 << 4 ) | 5); //channel 2, data rate == 2 [640] 
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint8_t cDuration = 3; // 2**(5+2*cDuration) clock cycles
    uint8_t cSkipDisable = 1; 
    this->WriteChipReg(pChip,"BERTConfig", ( cDuration << 4) | (cSkipDisable << 1) | (1 << 0) ); 
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint8_t cBERTStatus = this->ReadChipReg( pChip, "BERTStatus"); 
    uint8_t cAllZeros  = (cBERTStatus & (0x1 << 2)) >> 2;
    if ( cAllZeros ) 
    {
	LOG (INFO) << BOLDBLUE << "All zeros at input..." <<RESET;
	exit(0);
    }
    else
    {
	while( (cBERTStatus & 0x1) != 0x1)  
    	{
	   LOG (INFO) << BOLDBLUE << "BERT not done ... status is :  " << std::bitset<3>(cBERTStatus) << RESET;
	   std::this_thread::sleep_for(std::chrono::milliseconds(100));
           cBERTStatus = this->ReadChipReg( pChip, "BERTStatus"); 
    	}  
    }
    LOG (INFO) << BOLDBLUE << "Reading BERT counters ...." << RESET;
    uint64_t cReg0 = this->ReadChipReg( pChip, "BERTResult0");
    uint64_t cReg1 = this->ReadChipReg( pChip, "BERTResult1");
    uint64_t cReg2 = this->ReadChipReg( pChip, "BERTResult2");
    uint64_t cReg3 = this->ReadChipReg( pChip, "BERTResult3");
    uint64_t cReg4 = this->ReadChipReg( pChip, "BERTResult4");
    uint64_t cErrors = (cReg4 << 32) | (cReg3 << 24) | (cReg2 << 16) | (cReg1 << 8) | cReg0; 
    uint32_t cBitsRxd = std::pow(2,5 + cDuration*2)*16; //640 Mbps is 16 bits per 40 MHz clock cycle 
    LOG (INFO) << BOLDBLUE << "BERT counter0: " << +cReg0 << RESET;
    LOG (INFO) << BOLDBLUE << "BERT counter1: " << +cReg1 << RESET;
    LOG (INFO) << BOLDBLUE << "BERT counter2: " << +cReg2 << RESET;
    LOG (INFO) << BOLDBLUE << "BERT counter3: " << +cReg3 << RESET;
    LOG (INFO) << BOLDBLUE << "BERT counter4: " << +cReg4 << RESET;
    LOG (INFO) << BOLDBLUE << "Test received : " << +cBitsRxd << " bits." << RESET;
    LOG (INFO) << BOLDBLUE << "Of which : " << +cErrors << " bits were in error." << RESET;
    // stop BERT
    this->WriteChipReg( pChip,"BERTConfig",( cDuration << 4) | (cSkipDisable << 1) | (0 << 0) ); 
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    if( cErrors != 0 ) exit(0);

    //LET'S USE BERT WITH CONSTANT PATTERN
    this->ConfigureRxSource(pChip, cRxGroups, 4);
    this->WriteChipReg(pChip, "DPDataPattern0" , 0xCC);
    this->WriteChipReg(pChip, "DPDataPattern1" , 0xCC);
    this->WriteChipReg(pChip, "DPDataPattern2" , 0xCC);
    this->WriteChipReg(pChip, "DPDataPattern3" , 0xCC);

    this->WriteChipReg(pChip, "BERTDataPattern0" , 0xCC);
    this->WriteChipReg(pChip, "BERTDataPattern1" , 0xCC);
    this->WriteChipReg(pChip, "BERTDataPattern2" , 0xCC);
    this->WriteChipReg(pChip, "BERTDataPattern3" , 0xCC);
    uint8_t cFineSource = 4; 
    //pattern checker 
    //hard coded for now 
    for( auto cTestGroup : cRxGroups) 
    {
        LOG (INFO) << BOLDBLUE << "Rx Group " << +cTestGroup << RESET;
        uint8_t cCoarseSource = cTestGroup+1; 
        this->WriteChipReg( pChip,"BERTSource",( cCoarseSource << 4 ) |  cFineSource ); //data against constant pattern 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        this->WriteChipReg( pChip,"BERTConfig",( cDuration << 4) | (cSkipDisable << 1) | (1 << 0) ); 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        uint8_t cBERTStatus = this->ReadChipReg( pChip, "BERTStatus"); 
        uint8_t cAllZeros  = (cBERTStatus & (0x1 << 2)) >> 2;
        if ( cAllZeros ) 
        {
        	LOG (INFO) << BOLDBLUE << "All zeros at input..." <<RESET;
        	continue;
        }
        else
        {
    	   while( (cBERTStatus & 0x1) != 0x1)  
           {
        	   LOG (INFO) << BOLDBLUE << "BERT not done ... status is :  " << std::bitset<3>(cBERTStatus) << RESET;
        	   std::this_thread::sleep_for(std::chrono::milliseconds(100));
               cBERTStatus = this->ReadChipReg( pChip, "BERTStatus"); 
           }  
        }
        LOG (INFO) << BOLDBLUE << "Reading BERT counters ...." << RESET;
        uint64_t cReg0 = this->ReadChipReg( pChip, "BERTResult0");
        uint64_t cReg1 = this->ReadChipReg( pChip, "BERTResult1");
        uint64_t cReg2 = this->ReadChipReg( pChip, "BERTResult2");
        uint64_t cReg3 = this->ReadChipReg( pChip, "BERTResult3");
        uint64_t cReg4 = this->ReadChipReg( pChip, "BERTResult4");
        uint64_t cErrors = (cReg4 << 32) | (cReg3 << 24) | (cReg2 << 16) | (cReg1 << 8) | cReg0; 
        uint32_t cBitsRxd = std::pow(2,5 + cDuration*2)*16; //640 Mbps is 16 bits per 40 MHz clock cycle 
        LOG (INFO) << BOLDBLUE << "BERT counter0: " << +cReg0 << RESET;
        LOG (INFO) << BOLDBLUE << "BERT counter1: " << +cReg1 << RESET;
        LOG (INFO) << BOLDBLUE << "BERT counter2: " << +cReg2 << RESET;
        LOG (INFO) << BOLDBLUE << "BERT counter3: " << +cReg3 << RESET;
        LOG (INFO) << BOLDBLUE << "BERT counter4: " << +cReg4 << RESET;
        LOG (INFO) << BOLDBLUE << "Test received : " << +cBitsRxd << " bits." << RESET;
        LOG (INFO) << BOLDBLUE << "Of which : " << +cErrors << " bits were in error." << RESET;
        // stop BERT
        this->WriteChipReg( pChip,"BERTConfig",( cDuration << 4) | (cSkipDisable << 1) | (0 << 0) ); 
    }

    //this->ConfigureRxPRBS(pChip, cRxGroups, cRxChannels, false);
    //this->WriteChipReg(pChip, "DataPath", 0x0);//0x7
    //this->WriteChipReg(pChip, "ULDataSource0", 12);
    //this->ConfigureRxSource(pChip, cRxGroups, 4);
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //this->ConfigureDPPattern(pChip, 0xAAAAAAAA);
*/
    //#FIXME END OF YOUNES BLOCK PLEASE DON'T MODIFY

   return true;
}

/*-------------------------------------------------------------------------*/
/* Read/Write LpGBT chip registers                                         */
/*-------------------------------------------------------------------------*/

bool D19clpGBTInterface::WriteChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
{
    LOG(INFO) << BOLDBLUE << "Writing 0x" << std::hex << +pValue << std::dec << " to " << pRegNode << " [0x" << std::hex << +pChip->getRegItem(pRegNode).fAddress << std::dec << "]" << RESET;
    return this->WriteReg(pChip, pChip->getRegItem(pRegNode).fAddress, pValue, pVerifLoop);
}

uint16_t D19clpGBTInterface::ReadChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode)
{
    return this->ReadReg(pChip, pChip->getRegItem(pRegNode).fAddress);
}

bool D19clpGBTInterface::WriteReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop)
{
    this->setBoard(pChip->getBeBoardId());
    // Make sure the value is not > 8 bits
    if(pValue > 0xFF)
    {
        LOG(ERROR) << "LpGBT registers are 8 bits, impossible to write " << pValue << " to address " << pAddress;
        return false;
    }
    if(pAddress >= 0x13c)
    {
        LOG(ERROR) << "LpGBT read-write registers end at 0x13c ... impossible to write to " << +pAddress;
        return false;
    }
    // Now pick one configuration mode
    if(fUseOpticalLink)
      fBoardFW->WriteOptoLinkRegister(pChip, pAddress, pValue, pVerifLoop);
    else
    {
        // use PS-ROH test card USB interface
#ifdef __TCUSB__
        fTC_PSROH.write_i2c(pAddress, static_cast<char>(pValue));
#endif
    }
    if(!pVerifLoop) return true;
    // Verify success of Write
    uint16_t cReadBack = this->ReadReg(pChip, pAddress);
    uint8_t cIter = 0, cMaxIter = 10;
    while(cReadBack != pValue && cIter < cMaxIter)
    {
      cReadBack = this->ReadReg(pChip, pAddress);
      LOG(INFO) << BOLDRED << "REGISTER WRITE MISMATCH" << RESET;
      cIter++;
    }
    if(cReadBack != pValue)
      exit(0);
    return true;
}

uint16_t D19clpGBTInterface::ReadReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress)
{
    this->setBoard(pChip->getBeBoardId());
    uint16_t cReadBack = 0;
    if(fUseOpticalLink) { 
        cReadBack = fBoardFW->ReadOptoLinkRegister(pChip, pAddress); 
    }
    else
    {
// use PS-ROH test card USB interface
#ifdef __TCUSB__
        cReadBack = fTC_PSROH.read_i2c(pAddress);
#endif
    }
    return cReadBack;
}

bool D19clpGBTInterface::WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& pRegVec, bool pVerifLoop)
{
    bool writeGood = true;
    for(const auto& cReg: pRegVec) writeGood = this->WriteChipReg(pChip, cReg.first, cReg.second);
    return writeGood;
}

/*-------------------------------------------------------------------------*/
/* lpGBT configuration functions                                           */
/*-------------------------------------------------------------------------*/

void D19clpGBTInterface::ConfigureRxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate, uint8_t pTrackMode)
{
    for(const auto& cGroup: pGroups)
    {
        // Enable Rx Groups Channels and set Data Rate and Phase Tracking mode
        uint8_t cValueEnableRx = 0;
        for(const auto cChannel: pChannels) cValueEnableRx += (1 << cChannel);
        char cBuffer[12];
        sprintf(cBuffer, "EPRX%iControl", cGroup);
        std::string cRXCntrlReg(cBuffer, sizeof(cBuffer));
        this->WriteChipReg(pChip, cRXCntrlReg, (cValueEnableRx << 4) | (pDataRate << 2) | (pTrackMode << 0));
    }
}

void D19clpGBTInterface::ConfigureRxChannels(Ph2_HwDescription::Chip*    pChip,
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
            char cBuffer[13];
            sprintf(cBuffer, "EPRX%i%iChnCntr", cGroup, cChannel);
            std::string cRXChnCntrReg(cBuffer, sizeof(cBuffer));
            this->WriteChipReg(pChip, cRXChnCntrReg, (pPhase << 4) | (pInvert << 3) | (pAcBias << 2) | (pTerm << 1) | (pEqual << 0));
        }
    }
}

void D19clpGBTInterface::ConfigureTxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate)
{
    for(const auto& cGroup: pGroups)
    {
        // Configure Tx Group Data Rate value for specified group
        uint8_t cValueDataRate = this->ReadChipReg(pChip, "EPTXDataRate");
        this->WriteChipReg(pChip, "EPTXDataRate", (cValueDataRate & ~(0x03 << 2 * cGroup)) | (pDataRate << 2 * cGroup));
        // Enable given channels for specified group
        std::string cEnableTxReg;
        if(cGroup == 0 || cGroup == 1)
            cEnableTxReg = "EPTX10Enable";
        else if(cGroup == 2 || cGroup == 3)
            cEnableTxReg = "EPTX32Enable";
        uint8_t cValueEnableTx = this->ReadChipReg(pChip, cEnableTxReg);
        for(const auto cChannel: pChannels) cValueEnableTx += (1 << (cChannel + 4 * (cGroup % 2)));
        this->WriteChipReg(pChip, cEnableTxReg, cValueEnableTx);
    }
}

void D19clpGBTInterface::ConfigureTxChannels(Ph2_HwDescription::Chip*    pChip,
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
            char cBuffer1[13];
            sprintf(cBuffer1, "EPTX%i%iChnCntr", cGroup, cChannel);
            std::string cTXChnCntrl(cBuffer1, sizeof(cBuffer1));
            this->WriteChipReg(pChip, cTXChnCntrl, (pPreEmphStr << 5) | (pPreEmphMode << 3) | (pDriveStr << 0));

            // Configure Tx Channel PreEmphasisWidth, Inversion
            char cBuffer2[16];
            if(cChannel == 0 || cChannel == 1)
                sprintf(cBuffer2, "EPTX%i1_%i0ChnCntr", cGroup, cGroup);
            else if(cChannel == 2 || cChannel == 3)
                sprintf(cBuffer2, "EPTX%i3_%i2ChnCntr", cGroup, cGroup);
            std::string cTXChn_Cntr(cBuffer2, sizeof(cBuffer2));
            uint8_t     cValue_ChnCntr = this->ReadChipReg(pChip, cTXChn_Cntr);
            this->WriteChipReg(pChip, cTXChn_Cntr, (cValue_ChnCntr & ~(0x0F << 4 * (cChannel % 2))) | ((pInvert << 3 | pPreEmphWidth << 0) << 4 * (cChannel % 2)));
        }
    }
}

void D19clpGBTInterface::ConfigureClocks(Ph2_HwDescription::Chip*    pChip,
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
        char cBuffer1[15], cBuffer2[15];
        sprintf(cBuffer1, "EPCLK%iChnCntrH", cClock);
        sprintf(cBuffer2, "EPCLK%iChnCntrL", cClock);
        std::string cClkHReg(cBuffer1, sizeof(cBuffer1));
        std::string cClkLReg(cBuffer2, sizeof(cBuffer2));
        this->WriteChipReg(pChip, cBuffer1, pInvert << 6 | pDriveStr << 3 | pFreq);
        this->WriteChipReg(pChip, cBuffer2, pPreEmphStr << 5 | pPreEmphMode << 3 | pPreEmphWidth);
    }
}

void D19clpGBTInterface::ConfigureHighSpeedPolarity(Ph2_HwDescription::Chip* pChip, uint8_t pOutPolarity, uint8_t pInPolarity)
{
    // Configure Highr Speed Link Rx and Tx polarity
    uint8_t cPolarity = (pOutPolarity << 7 | pInPolarity << 6);
    this->WriteChipReg(pChip, "ChipConfig", cPolarity);
}

void D19clpGBTInterface::ConfigureDPPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern)
{
    // Configure Constant Pattern
    LOG(INFO) << BOLDBLUE << "Loading pattern " << std::bitset<32>(pPattern) << " to lpGBT." << RESET;
    this->WriteChipReg(pChip, "DPDataPattern0", (pPattern & 0xFF));
    this->WriteChipReg(pChip, "DPDataPattern1", ((pPattern & 0xFF00) >> 8));
    this->WriteChipReg(pChip, "DPDataPattern2", ((pPattern & 0xFF0000) >> 16));
    this->WriteChipReg(pChip, "DPDataPattern3", ((pPattern & 0xFF000000) >> 24));
}

void D19clpGBTInterface::ConfigureRxPRBS(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable)
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

        uint8_t cValueEnablePRBS = this->ReadChipReg(pChip, cPRBSReg);
        uint8_t cEnabledCh       = 0;
        for(const auto cChannel: pChannels) cEnabledCh |= pEnable << cChannel;
        this->WriteChipReg(pChip, cPRBSReg, (cValueEnablePRBS & ~(0xF << 4 * (cGroup % 2))) | (cEnabledCh << (4 * (cGroup % 2))));
    }
}

void D19clpGBTInterface::ConfigureRxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource)
{
    // Configure Rx Data Source
    for(const auto& cGroup: pGroups)
    {
        if(pSource == 0)
            LOG(INFO) << BOLDBLUE << "Configuring Rx Group " << +cGroup << " Source to NORMAL " << RESET;
        else if(pSource == 1)
            LOG(INFO) << BOLDBLUE << "Configuring Rx Group " << +cGroup << " Source to PRBS7 " << RESET;
        else if(pSource == 4 || pSource == 5)
            LOG(INFO) << BOLDBLUE << "Configuring Rx Group " << +cGroup << " Source to Constant Pattern" << RESET;
        std::string cRxSourceReg;
        if(cGroup == 0 || cGroup == 1)
            cRxSourceReg = "ULDataSource1";
        else if(cGroup == 2 || cGroup == 3)
            cRxSourceReg = "ULDataSource2";
        else if(cGroup == 4 || cGroup == 5)
            cRxSourceReg = "ULDataSource3";
        else if(cGroup == 6)
            cRxSourceReg = "ULDataSource4";

        uint8_t cValueRxSource = this->ReadChipReg(pChip, cRxSourceReg);
        this->WriteChipReg(pChip, cRxSourceReg, (cValueRxSource & ~(0x7 << 3 * (cGroup % 2))) | (pSource << 3 * (cGroup % 2)));
    }
}

void D19clpGBTInterface::ConfigureTxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource)
{
    // Configure Tx Data Source
    for(const auto& cGroup: pGroups)
    {
        uint8_t cULDataSrcValue = this->ReadChipReg(pChip, "ULDataSource5");
        cULDataSrcValue         = (cULDataSrcValue & ~(0x3 << (2 * cGroup))) | (pSource << (2 * cGroup));
        this->WriteChipReg(pChip, "ULDataSource5", cULDataSrcValue);
    }
}

void D19clpGBTInterface::ConfigureRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase)
{
    // Configure Rx Channel Phase
    char cBuffer[13];
    sprintf(cBuffer, "EPRX%i%iChnCntr", pGroup, pChannel);
    std::string cRegName(cBuffer, sizeof(cBuffer));
    uint8_t     cValueChnCntr = this->ReadChipReg(pChip, cRegName);
    cValueChnCntr             = (cValueChnCntr & ~(0xF << 4)) | (pPhase << 4);
    this->WriteChipReg(pChip, cRegName, cValueChnCntr);
}

void D19clpGBTInterface::ConfigurePhShifter(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq, uint8_t pDriveStr, uint8_t pEnFTune, uint16_t pDelay)
{
    // Configure Rx Phase Shifter
    for(const auto& cClock: pClocks)
    {
        char cBuffer1[8], cBuffer2[9];
        sprintf(cBuffer1, "PS%iDelay", cClock);
        sprintf(cBuffer2, "PS%iConfig", cClock);
        std::string cDelayReg(cBuffer1, sizeof(cBuffer1));
        std::string cConfigReg(cBuffer2, sizeof(cBuffer2));

        this->WriteChipReg(pChip, cConfigReg, (((pDelay & 0x100) >> 8) << 7) | pEnFTune << 6 | pDriveStr << 3 | pFreq);
        this->WriteChipReg(pChip, cDelayReg, pDelay);
    }
}

/*-------------------------------------------------------------------------*/
/* lpGBT specific routine functions                                        */
/*-------------------------------------------------------------------------*/

void D19clpGBTInterface::PhaseTrainRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups)
{
    // Launch Rx Phase Training
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

        this->WriteChipReg(pChip, cTrainRxReg, 0x0F << 4 * (cGroup % 2));
        this->WriteChipReg(pChip, cTrainRxReg, 0x00 << 4 * (cGroup % 2));
    }
}

void D19clpGBTInterface::PhaseAlignRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels)
{
    // Phase Align Rx Channels
    // Turn ON PRBS for channels 0,2
    this->ConfigureRxPRBS(pChip, pGroups, pChannels, true);
    // Find Phase
    // Configure Rx Phase Shifter
    uint16_t cDelay = 0x00;
    uint8_t  cFreq = 5, cEnFTune = 0, cDriveStr = 0; // 4 --> 320 MHz || 5 --> 640 MHz
    this->ConfigurePhShifter(pChip, {0, 1, 2, 3}, cFreq, cDriveStr, cEnFTune, cDelay);
    // Phase Train channels 0,2
    this->PhaseTrainRx(pChip, pGroups);
    for(const auto& cGroup: pGroups)
    {
        // Wait until channels lock
        LOG(INFO) << BOLDMAGENTA << "Phase Aligning Rx Group " << +cGroup << RESET;
        do
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } while(!this->IsRxLocked(pChip, cGroup, pChannels));
        LOG(INFO) << BOLDBLUE << "    Group " << +cGroup << BOLDGREEN << " LOCKED" << RESET;
        // Set new phase to channels 0,2
        for(const auto cChannel: pChannels)
        {
            uint8_t cCurrPhase = this->GetRxPhase(pChip, cGroup, cChannel);
            LOG(INFO) << BOLDBLUE << "    Channel " << +cChannel << " phase is " << +cCurrPhase << RESET;
            this->ConfigureRxPhase(pChip, cGroup, cChannel, cCurrPhase);
        }
    }
    this->ConfigureRxGroups(pChip, pGroups, pChannels, 2, 0);
}

/*-------------------------------------------------------------------------*/
/* lpGBT status functions                                                  */
/*-------------------------------------------------------------------------*/

void D19clpGBTInterface::PrintChipMode(Ph2_HwDescription::Chip* pChip)
{
    switch((this->ReadChipReg(pChip, "ConfigPins") & 0xF0) >> 4)
    {
      case 0: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Off" << RESET; break;
      case 1: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex TX" << RESET; break;
      case 2: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex RX" << RESET; break;
      case 3: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Transceiver" << RESET; break;
      case 4: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Off" << RESET; break;
      case 5: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Simplex TX" << RESET; break;
      case 6: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Simplex RX" << RESET; break;
      case 7: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Transceiver" << RESET; break;
      case 8: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Off" << RESET; break;
      case 9: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex TX" << RESET; break;
      case 10: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex RX" << RESET; break;
      case 11: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Transceiver" << RESET; break;
      case 12: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Off" << RESET; break;
      case 13: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Simplex TX" << RESET; break;
      case 14: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Simplex RX" << RESET; break;
      case 15: LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Transceiver" << RESET; break;
    }
}

bool D19clpGBTInterface::IslpGBTReady(Ph2_HwDescription::Chip* pChip)
{
    // Gets success/failure of power-up FSM
    return (this->ReadChipReg(pChip, "PUSMStatus") == 0x12);
}

uint8_t D19clpGBTInterface::GetRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel)
{
    // Get Rx Channel Phase
    char cBuffer[19];
    if(pChannel == 0 || pChannel == 1)
        sprintf(cBuffer, "EPRX%iCurrentPhase10", pGroup);
    else if(pChannel == 3 || pChannel == 2)
        sprintf(cBuffer, "EPRX%iCurrentPhase32", pGroup);
    std::string cRxPhaseReg(cBuffer, sizeof(cBuffer));
    uint8_t     cRxPhaseRegValue = this->ReadChipReg(pChip, cRxPhaseReg);
    return ((cRxPhaseRegValue & (0x0F << 4 * (pChannel % 2))) >> 4 * (pChannel % 2));
}

bool D19clpGBTInterface::IsRxLocked(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, const std::vector<uint8_t>& pChannels)
{
    // Cheks if Rx channels are locked #FIXME needs to check depending on the
    // enabled channels not on all (0x0F)
    char cBuffer[11];
    sprintf(cBuffer, "EPRX%iLocked", pGroup);
    std::string cRXLockedReg(cBuffer, sizeof(cBuffer));
    uint8_t cChannelMask = 0x00; 
    for(auto cChannel : pChannels)
        cChannelMask += (1 << cChannel); 
    return (((this->ReadChipReg(pChip, cRXLockedReg) & (cChannelMask << 4)) >> 4) == cChannelMask);
}

uint8_t D19clpGBTInterface::GetRxDllStatus(Ph2_HwDescription::Chip* pChip, uint8_t pGroup)
{
    // Gets Rx Group Delay-Locked-Loop status
    char cBuffer[14];
    sprintf(cBuffer, "EPRX%iDllStatus", pGroup);
    std::string cRXDllStatReg(cBuffer, sizeof(cBuffer));
    return this->ReadChipReg(pChip, cRXDllStatReg);
}

uint8_t D19clpGBTInterface::GetI2CStatus(Ph2_HwDescription::Chip* pChip, uint8_t pMaster)
{
    // Gets I2C Master status
    char cBuffer[11];
    sprintf(cBuffer, "I2CM%iStatus", pMaster);
    std::string cI2CStatReg(cBuffer, sizeof(cBuffer));
    return this->ReadChipReg(pChip, cI2CStatReg);
}

/*-------------------------------------------------------------------------*/
/* lpGBT I2C Master functions                                              */
/*-------------------------------------------------------------------------*/

void D19clpGBTInterface::ResetI2C(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pMasters)
{
    std::vector<uint8_t> cBitPosition = {2,1,0};
    for(const auto& cMaster: pMasters)
    {
        // generating reset pulse on dedicated register bit
        this->WriteChipReg(pChip, "RST0", 0 << cBitPosition.at(cMaster));
        this->WriteChipReg(pChip, "RST0", 1 << cBitPosition.at(cMaster));
        this->WriteChipReg(pChip, "RST0", 0 << cBitPosition.at(cMaster));
    }
}

void D19clpGBTInterface::ConfigureI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pFreq, uint8_t pNBytes, uint8_t pSCLDriveMode)
{
    // Configures I2C Masters
    // First let's write configuration data into the I2C Master Data register
    char cBuffer1[10];
    sprintf(cBuffer1, "I2CM%iData0", pMaster);
    std::string cI2CCntrlReg(cBuffer1, sizeof(cBuffer1));
    uint8_t     cValueCntrl = (pFreq << 0) | (pNBytes << 2) | (pSCLDriveMode << 7);
    this->WriteChipReg(pChip, cI2CCntrlReg, cValueCntrl);

    // Now let's write Command (0x00) to the Command register to tranfer Configuration to the I2C Master Control register
    char cBuffer2[8];
    sprintf(cBuffer2, "I2CM%iCmd", pMaster);
    std::string cI2CCmdReg(cBuffer2, sizeof(cBuffer2));
    this->WriteChipReg(pChip, cI2CCmdReg, 0x00);
}

bool D19clpGBTInterface::WriteI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint32_t pData, uint8_t pNBytes)
{
    // Write Data to Slave Address using I2C Master
    this->ConfigureI2C(pChip, pMaster, 0, ( pNBytes > 1 ) ? pNBytes : 0, 0);

    // Prepare Address Register
    char cBuffer2[12];
    sprintf(cBuffer2, "I2CM%iAddress", pMaster);
    std::string cI2CAddressReg(cBuffer2, sizeof(cBuffer2));
    // Write Slave Address
    this->WriteChipReg(pChip, cI2CAddressReg, pSlaveAddress);

    // Write Data to Data Register
    for(uint8_t cByte = 0; cByte < pNBytes; cByte++)
    {
        char cBuffer1[10];
        sprintf(cBuffer1, "I2CM%iData%i", pMaster, cByte);
        std::string cI2CDataReg(cBuffer1, sizeof(cBuffer1));
        this->WriteChipReg(pChip, cI2CDataReg, (pData >> 8*cByte) & 0xFF);
    }

    // Prepare Command Register
    char cBuffer3[8];
    sprintf(cBuffer3, "I2CM%iCmd", pMaster);
    std::string cI2CCmdReg(cBuffer3, sizeof(cBuffer3));
    // If Multi-Byte, write command to save data locally before transfer to slave
    // FIXME for now this only provides a maximum of 32 bits (4 Bytes) write
    // Write Command to launch I2C transaction
    if(pNBytes == 1)
        this->WriteChipReg(pChip, cI2CCmdReg, 0x2);
    else
    {
        this->WriteChipReg(pChip, cI2CCmdReg, 0x8);
        this->WriteChipReg(pChip, cI2CCmdReg, 0xC);
    }

    // wait until the transaction is done
    uint8_t cMaxIter = 10, cIter = 0;
    bool    cSuccess = false;
    do
    {
        LOG(DEBUG) << BOLDBLUE << "Waiting for I2C transaction to finisih" << RESET;
        uint8_t cStatus = this->GetI2CStatus(pChip, pMaster);
        LOG(INFO) << BOLDBLUE << "I2C Master " << +pMaster << " -- Status : " << fI2CStatusMap[cStatus] << RESET;
        cSuccess = (cStatus == 4);
        cIter++;
    } while(cIter < cMaxIter && !cSuccess);
    return cSuccess;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Verify success of write #FIXME can be removed or put under condition
    if(this->ReadI2C(pChip, pMaster, pSlaveAddress, pNBytes) != pData)
    {
        LOG(INFO) << BOLDRED << "I2C Master transaction MISMATCH in writing 0x" << std::hex << +pData << std::dec << " to address 0x" << std::hex << +pSlaveAddress << std::dec << RESET;
        return false;
    }
    else
        return true;
}

uint32_t D19clpGBTInterface::ReadI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint8_t pNBytes)
{
    // Read Data from Slave Address using I2C Master
    this->ConfigureI2C(pChip, pMaster, 0, pNBytes, 0);
    // Prepare Address Register
    char cBuffer1[12];
    sprintf(cBuffer1, "I2CM%iAddress", pMaster);
    std::string cI2CAddressReg(cBuffer1, sizeof(cBuffer1));

    // Prepare Command Register
    char cBuffer2[8];
    sprintf(cBuffer2, "I2CM%iCmd", pMaster);
    std::string cI2CCmdReg(cBuffer2, sizeof(cBuffer2));
    // Write Slave Address
    this->WriteChipReg(pChip, cI2CAddressReg, pSlaveAddress);

    // Write Read Command and then Read from Read Data Register
    // Procedure and registers depend on number on Bytes
    if(pNBytes == 1)
    {
        this->WriteChipReg(pChip, cI2CCmdReg, 0x3);
        char cBuffer1[13];
        sprintf(cBuffer1, "I2CM%iReadByte", pMaster);
        std::string cI2CDataReg(cBuffer1, sizeof(cBuffer1));
        return this->ReadChipReg(pChip, cI2CDataReg);
    }
    else
    {
        this->WriteChipReg(pChip, cI2CCmdReg, 0xD);
        uint32_t cReadData = 0;
        for(uint8_t cByte = 0; cByte < pNBytes; cByte++)
        {
            char cBuffer1[10];
            sprintf(cBuffer1, "I2CM%dRead%i", pMaster, 15 - cByte);
            std::string cI2CDataReg(cBuffer1, sizeof(cBuffer1));
            cReadData |= ((uint32_t)this->ReadChipReg(pChip, cI2CDataReg) << cByte);
        }
        return cReadData;
    }
}

/*-------------------------------------------------------------------------*/
/* lpGBT ADC-DAC functions                                                 */
/*-------------------------------------------------------------------------*/

void D19clpGBTInterface::ConfigureADC(Ph2_HwDescription::Chip* pChip, uint8_t pGainSelect, uint8_t pADCEnable)
{
    this->WriteChipReg(pChip, "ADCConfig", pADCEnable << 2 | pGainSelect);
}

uint16_t D19clpGBTInterface::ReadADC(Ph2_HwDescription::Chip* pChip, const std::string& pADCInput)
{
    // Read (converted) data from ADC Input with VREF/2 as negative Input
    uint8_t cADCInput = fADCInputMap[pADCInput];
    uint8_t cVREF     = fADCInputMap["VREF/2"];
    LOG(INFO) << BOLDBLUE << "Reading ADC value from " << pADCInput << RESET;
    // Select ADC Input
    this->WriteChipReg(pChip, "ADCSelect", cADCInput << 4 | cVREF << 0);
    // Enable ADC Input
    this->WriteChipReg(pChip, "ADCConfig", 1 << 2);
    // Enable Internal VREF
    this->WriteChipReg(pChip, "VREFCNTR", 1 << 7);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Start ADC conversion
    this->WriteChipReg(pChip, "ADCConfig", 1 << 7 | 1 << 2);
    // Check conversion status
    uint8_t cMaxIter = 100, cIter = 0;
    bool    cSuccess = false;
    do
    {
        LOG(INFO) << BOLDBLUE << "Waiting for ADC conversion to end" << RESET;
        uint8_t cStatus = this->ReadChipReg(pChip, "ADCStatusH");
        cSuccess        = (((cStatus & 0x40) >> 6) == 1);
        cIter++;
    } while(cIter < cMaxIter && !cSuccess);
    // Read ADC value
    uint8_t cADCvalue1 = this->ReadChipReg(pChip, "ADCStatusH") & 0x3;
    uint8_t cADCvalue2 = this->ReadChipReg(pChip, "ADCStatusL");
    // Clear ADC conversion bit (#FIXME disable ADC Input as well ??)
    this->WriteChipReg(pChip, "ADCConfig", 0 << 7 | 1 << 2);
    return (cADCvalue1 << 8 | cADCvalue2);
}

uint16_t D19clpGBTInterface::ReadADCDiff(Ph2_HwDescription::Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN)
{
    // Read differential (converted) data on two ADC inputs
    uint8_t cADCInputP = fADCInputMap[pADCInputP];
    uint8_t cADCInputN = fADCInputMap[pADCInputN];
    LOG(INFO) << BOLDBLUE << "Reading ADC value from " << pADCInputP << RESET;
    // Select ADC Input
    this->WriteChipReg(pChip, "ADCSelect", cADCInputP << 4 | cADCInputN << 0);
    // Enable ADC Input
    this->WriteChipReg(pChip, "ADCConfig", 1 << 2);
    // Start ADC conversion
    this->WriteChipReg(pChip, "ADCConfig", 1 << 7 | 1 << 2);
    // Check conversion status
    uint8_t cMaxIter = 10, cIter = 0;
    bool    cSuccess = false;
    do
    {
        LOG(INFO) << BOLDBLUE << "Waiting for ADC conversion to end" << RESET;
        uint8_t cStatus = this->ReadChipReg(pChip, "ADCStatusH");
        cSuccess        = (((cStatus & 0x40) >> 6) == 1);
        cIter++;
    } while(cIter < cMaxIter && !cSuccess);
    // Read ADC value
    uint8_t cADCvalue1 = this->ReadChipReg(pChip, "ADCStatusH") & 0x3;
    uint8_t cADCvalue2 = this->ReadChipReg(pChip, "ADCStatusL");
    // Clear ADC conversion bit (#FIXME disable ADC Input as well ??)
    this->WriteChipReg(pChip, "ADCConfig", 0 << 7 | 1 << 2);
    return (cADCvalue1 << 8 | cADCvalue2);
}



/*-------------------------------------------------------------------------*/
/* OT specific functions                                                   */
/*-------------------------------------------------------------------------*/

void D19clpGBTInterface::SetConfigMode(Ph2_HwDescription::Chip* pChip, const std::string& pMode, bool pToggle)
{
    if(pMode == "serial")
    {
#ifdef __TCUSB__
        LOG(INFO) << BOLDBLUE << "serial before toggle" << RESET;
        if(pToggle)
          fTC_PSROH.toggle_SCI2C();
#endif
        LOG(INFO) << BOLDGREEN << "Switched software flag to Serial Interface configuration mode" << RESET;
        fUseOpticalLink = true;
    }
    else if(pMode == "i2c")
    {
        LOG(INFO) << BOLDGREEN << "Switched software flag to I2C Slave Interface configuration mode" << RESET;
        fUseOpticalLink = false;
    }
    else
    {
        LOG(INFO) << BOLDRED <<  "Wrong configuration mode : choose [serial] or [i2c]" << RESET;
        exit(0);
    }
}

bool D19clpGBTInterface::cicWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pRegisterAddress, uint8_t pRegisterValue, bool pReadBack)
{
    bool cWriteStatus = this->WriteI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x60, (pRegisterValue << 16) | (pRegisterAddress << 8), 3);
    if(pReadBack && cWriteStatus)
    {
        bool cWriteSuccess = ( this->cicRead(pChip, pFeId, pRegisterAddress) == pRegisterValue );
        if(!cWriteSuccess)
        {
            LOG(INFO) << BOLDRED << "CIC I2C ReadBack Mismatch for hybrid " << +pFeId << " register 0x" << std::hex << + pRegisterAddress << std::dec << RESET;
            throw std::runtime_error(std::string("I2C ReadBack Mismatch"));
        }
        return cWriteSuccess;
    }
    else
        return cWriteStatus;
}

uint32_t D19clpGBTInterface::cicRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pRegisterAddress)
{
    this->WriteI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x60, pRegisterAddress << 8, 2);
    uint32_t cReadBack  = this->ReadI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x60, 1);
    LOG(INFO) << BOLDBLUE << "Readback 0x" << std::hex << +cReadBack << std::dec << " from register 0x" << std::hex << +pRegisterAddress << RESET;
    return cReadBack;
}

bool D19clpGBTInterface::ssaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pReadBack)
{
    bool cWriteStatus = this->WriteI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x20 | (1 + pChipId), pRegisterValue << 16 | pRegisterAddress, 3);
    if(pReadBack && cWriteStatus)
    { 
        bool cWriteSuccess = ( this->ssaRead(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x20 | (1 + pChipId), pRegisterAddress) == pRegisterValue );
        if(!cWriteSuccess)
        {
            LOG(INFO) << BOLDRED << "SSA I2C ReadBack Mismatch in hybrid " << +pFeId << " Chip " << +pChipId << " register 0x" << std::hex << +pRegisterAddress << std::dec << RESET;
            throw std::runtime_error(std::string("I2C readback mismatch"));
        }
        return cWriteSuccess;
    }
    return cWriteStatus;
}

uint32_t D19clpGBTInterface::ssaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress)
{
    this->WriteI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x20 | (1 + pChipId), pRegisterAddress, 2);
    uint32_t cReadBack = this->ReadI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x20 | (1 + pChipId), 1);
    LOG(INFO) << BOLDBLUE << "Readback 0x" << std::hex << +cReadBack << std::dec << " from register 0x" << std::hex << +pRegisterAddress << RESET;
    return cReadBack;
}

bool D19clpGBTInterface::mpaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pReadBack)
{
    bool cWriteStatus = this->WriteI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x00 | (1 + pChipId), pRegisterValue << 16 | pRegisterAddress, 3);
    if(pReadBack && cWriteStatus)
    { 
        bool cWriteSuccess = ( this->ssaRead(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x00 | (1 + pChipId), pRegisterAddress) == pRegisterValue );
        if(!cWriteSuccess)
        {
            LOG(INFO) << BOLDRED << "SSA I2C ReadBack Mismatch in hybrid " << +pFeId << " Chip " << +pChipId << " register 0x" << std::hex << +pRegisterAddress << std::dec << RESET;
            throw std::runtime_error(std::string("I2C readback mismatch"));
        }
        return cWriteSuccess;
    }
    return cWriteStatus;
}

uint32_t D19clpGBTInterface::mpaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress)
{
    this->WriteI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x00 | (1 + pChipId), pRegisterAddress, 2);
    uint32_t cReadBack = this->ReadI2C(pChip, ((pFeId%2) == 1) ? 2 : 0, 0x00 | (1 + pChipId), 1);
    LOG(INFO) << BOLDBLUE << "Readback 0x" << std::hex << +cReadBack << std::dec << " from register 0x" << std::hex << +pRegisterAddress << RESET;
    return cReadBack;
}
} // namespace Ph2_HwInterface // namespace Ph2_HwInterface
