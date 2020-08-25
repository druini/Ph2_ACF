/*!
  \file                  D19clpGBTInterface.cc
  \brief                 Interface to access and control the low-power Gigabit Transceiver chip 
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
*/

#include "D19clpGBTInterface.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{

  bool D19clpGBTInterface::ConfigureChip (Ph2_HwDescription::Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    LOG(INFO) << BOLDBLUE << "Configuring lpGBT" << RESET;
    //Load register map from configuration file
    ChipRegMap clpGBTRegMap = pChip->getRegMap();;
    for(const auto& cRegItem : clpGBTRegMap) 
    {
      if( cRegItem.second.fAddress < 0x13c ){ 
        LOG (DEBUG) << BOLDBLUE << "Writing 0x" << std::hex << +cRegItem.second.fValue << std::dec << " to " << cRegItem.first <<  " [0x" << std::hex << +cRegItem.second.fAddress << std::dec << "]"<< RESET;
        this->WriteReg(pChip, cRegItem.second.fAddress, cRegItem.second.fValue);
      }
    }
    //Set dllConfigDone and pllConfigDone to finalize powerup
    this->WriteReg(pChip, 0x0ef, 0x6);

    //Additional configurations (could eventually be moved to configuration file)
    std::vector<uint8_t> cClocks = {1,6,11,26};
    uint8_t cFreq=4, cDriveStr=7, cInvert=0;
    uint8_t cPreEmphWidth=0, cPreEmphMode=1, cPreEmphStr=3;
    this->ConfigureClocks(pChip, cClocks, cFreq, cDriveStr, cInvert, cPreEmphWidth, cPreEmphMode, cPreEmphStr);
    //Configure Tx Rx Polarity
    this->ConfigureTxRxPolarity(pChip, 1, 0);
    //Phase Align Rx
    this->PhaseAlignRx(pChip, {0,1,2,3,4,5,6}, {0,2});
    //Reset I2C Masters
    this->ResetI2C(pChip, {0,1,2});
   
    return true;
  }

  /*-------------------------------------------------------------------------*/
  /* Read/Write LpGBT chip registers                                         */
  /*-------------------------------------------------------------------------*/

  bool D19clpGBTInterface::WriteChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
  {
    LOG (DEBUG) << BOLDBLUE << "Writing 0x" << std::hex << +pValue << std::dec << " to " << pRegNode <<  " [0x" << std::hex << +pChip->getRegItem(pRegNode).fAddress << std::dec << "]" << RESET;
    this->setBoard( pChip->getBeBoardId() );
    return this->WriteReg(pChip, pChip->getRegItem(pRegNode).fAddress, pValue, pVerifLoop );
  }


  uint16_t D19clpGBTInterface::ReadChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode)
  {
    this->setBoard(pChip->getBeBoardId());
    return this->ReadReg(pChip, pChip->getRegItem(pRegNode).fAddress);
  }


  bool D19clpGBTInterface::WriteReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop)
  {
    //Make sure the value is not > 8 bits
    if( pValue > 0xFF ){
      LOG (ERROR) << "LpGBT registers are 8 bits, impossible to write " << pValue << " to address " << pAddress ;
      return false;
    }
    if(pAddress >= 0x13c)
    {
      LOG (ERROR) << "LpGBT read-write registers end at 0x13c ... impossible to write to " << +pAddress  ;
      return false;
    }
    if(fUseOpticalLink)
      return fBoardFW->WriteOptoLinkRegister(pChip, pAddress, pValue, pVerifLoop);
    else
    {
      //use PS-ROH test card USB interface
      uint32_t cReadBack = 0;
      #ifdef __TCUSB__
        cReadBack = fTC_PSROH.write_i2c(pAddress, static_cast<char>(pValue));
      #endif
      if(!pVerifLoop )
        return true;
      if(cReadBack != pValue)
      {
        LOG (INFO) << BOLDRED << "ConfigureChip : I2C WRITE MISMATCH" << RESET;
        return false;
      }
      else
      {
        LOG (DEBUG) << BOLDBLUE << "\t\t.. read back 0x" << std::hex << +cReadBack << std::dec << " from register address 0x" << std::hex << pAddress << std::dec << RESET;
        return true;
      }
      return false;
    } 
  }


  uint16_t D19clpGBTInterface::ReadReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress)
  {
    uint16_t cReadBack = 0;
    if(fUseOpticalLink)
    {
      cReadBack = fBoardFW->ReadOptoLinkRegister(pChip, pAddress);
    }
    else{
      //use PS-ROH test card USB interface
      #ifdef __TCUSB__
        cReadBack = fTC_PSROH.read_i2c( pAddress);
      #endif
    }
    return cReadBack; 
  }


  bool D19clpGBTInterface::WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pRegVec, bool pVerifLoop)
  {
    bool writeGood = true;
    for (const auto& cReg : pRegVec)
      writeGood = this->WriteChipReg(pChip, cReg.first, cReg.second);
    return writeGood;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT configuration functions                                           */
  /*-------------------------------------------------------------------------*/

  void D19clpGBTInterface::ConfigureRxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate, uint8_t pTrackMode)
  {
    for(const auto& cGroup : pGroups)
    {
      //Enable Rx Groups Channels and set Data Rate and Phase Tracking mode
      uint8_t cValueEnableRx=0;
      for(const auto cChannel : pChannels)
        cValueEnableRx += (1 << cChannel);
      char cBuffer[12];
      sprintf(cBuffer, "EPRX%iControl", cGroup);
      std::string cRXCntrlReg(cBuffer, sizeof(cBuffer));
      this->WriteChipReg(pChip, cRXCntrlReg, (cValueEnableRx << 4) | (pDataRate << 2) | (pTrackMode << 0));
    }
  }


  void D19clpGBTInterface::ConfigureRxChannels(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pEqual, uint8_t pTerm, uint8_t pAcBias, uint8_t pInvert, uint8_t pPhase)
  {
    for(const auto& cGroup : pGroups)
    {
      for(const auto& cChannel : pChannels)
      {
        //Configure Rx Channel Phase, Inversion, AcBias enabling, Termination enabling, Equalization enabling
        char cBuffer[13];
        sprintf(cBuffer,"EPRX%i%iChnCntr", cGroup, cChannel);
        std::string cRXChnCntrReg(cBuffer, sizeof(cBuffer));
        this->WriteChipReg(pChip, cRXChnCntrReg, (pPhase << 4) | (pInvert << 3) | (pAcBias << 2) | (pTerm << 1) | (pEqual << 0));
      }
    }
  }


  void D19clpGBTInterface::ConfigureTxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate)
  {
    for(const auto& cGroup : pGroups)
    {
      //Configure Tx Group Data Rate value for specified group
      uint8_t cValueDataRate = this->ReadChipReg(pChip, "EPTXDataRate");
      this->WriteChipReg(pChip, "EPTXDataRate", (cValueDataRate & ~(0x03 << 2*cGroup)) | (pDataRate << 2*cGroup));
      //Enable given channels for specified group
      std::string cEnableTxReg;
      if(cGroup == 0 || cGroup == 1) 
        cEnableTxReg = "EPTX10Enable";
      else if(cGroup == 2 || cGroup == 3) 
        cEnableTxReg = "EPTX32Enable";
      uint8_t cValueEnableTx = this->ReadChipReg(pChip, cEnableTxReg);
      for(const auto cChannel : pChannels)
        cValueEnableTx += (1 << ( cChannel + 4*(cGroup % 2) ) );
      this->WriteChipReg(pChip, cEnableTxReg, cValueEnableTx);
    }
  }


  void D19clpGBTInterface::ConfigureTxChannels(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDriveStr, uint8_t pPreEmphMode, uint8_t pPreEmphStr, uint8_t pPreEmphWidth, uint8_t pInvert)
  {
    for(const auto& cGroup : pGroups)
    {
      for(const auto& cChannel : pChannels)
      {
        //Configure Tx Channel PreEmphasisStrenght, PreEmphasisMode, DriveStrength
        char cBuffer1[13];
        sprintf(cBuffer1, "EPTX%i%iChnCntr", cGroup, cChannel);
        std::string cTXChnCntrl(cBuffer1,sizeof(cBuffer1));
        this->WriteChipReg(pChip, cTXChnCntrl, (pPreEmphStr << 5) | (pPreEmphMode << 3) | (pDriveStr << 0));

        //Configure Tx Channel PreEmphasisWidth, Inversion
        char cBuffer2[16];
        if(cChannel == 0 || cChannel == 1) 
          sprintf(cBuffer2, "EPTX%i1_%i0ChnCntr", cGroup, cGroup);
        else if(cChannel == 2 || cChannel == 3) 
          sprintf(cBuffer2, "EPTX%i3_%i2ChnCntr", cGroup, cGroup);
        std::string cTXChn_Cntr(cBuffer2, sizeof(cBuffer2));
        uint8_t cValue_ChnCntr = this->ReadChipReg(pChip, cTXChn_Cntr);
        this->WriteChipReg(pChip, cTXChn_Cntr, (cValue_ChnCntr & ~(0x0F << 4*(cChannel%2))) | ((pInvert << 3 | pPreEmphWidth << 0) << 4*(cChannel%2)) );
      }
    }
  }

 
  void D19clpGBTInterface::ConfigureClocks(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq, uint8_t pDriveStr, uint8_t pInvert, uint8_t pPreEmphWidth, uint8_t pPreEmphMode, uint8_t pPreEmphStr)
  {
    for(const auto& cClock : pClocks)
    {
      //Configure Clocks Frequency, Drive Strength, Inversion, Pre-Emphasis Width, Pre-Emphasis Mode, Pre-Emphasis Strength
      char cBuffer1[15], cBuffer2[15];
      sprintf(cBuffer1, "EPCLK%iChnCntrH", cClock);
      sprintf(cBuffer2, "EPCLK%iChnCntrL", cClock);
      std::string cClkHReg(cBuffer1, sizeof(cBuffer1));
      std::string cClkLReg(cBuffer2, sizeof(cBuffer2));
      this->WriteChipReg(pChip, cBuffer1, pInvert << 6 | pDriveStr << 3 | pFreq);
      this->WriteChipReg(pChip, cBuffer2, pPreEmphStr << 5 | pPreEmphMode << 3 | pPreEmphWidth);
    }
  }


  void D19clpGBTInterface::ConfigureTxRxPolarity(Ph2_HwDescription::Chip* pChip, uint8_t pTxPolarity, uint8_t pRxPolarity)
  {
    //Configure Rx and Tx lines polarity
    uint8_t cPolarity = (pTxPolarity << 7 | pRxPolarity << 6);
    this->WriteChipReg(pChip, "ChipConfig", cPolarity);
  }

  
  void D19clpGBTInterface::ConfigureDPPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern)
  {
    //Configure Constant Pattern
    LOG (INFO) << BOLDBLUE << "Loading pattern " << std::bitset<32>(pPattern) << " to lpGBT." << RESET;
    this->WriteChipReg(pChip, "DPDataPattern0", (pPattern & 0xFF));
    this->WriteChipReg(pChip, "DPDataPattern1", ((pPattern & 0xFF00) >> 8));
    this->WriteChipReg(pChip, "DPDataPattern2", ((pPattern & 0xFF0000) >> 16));
    this->WriteChipReg(pChip, "DPDataPattern3", ((pPattern & 0xFF000000) >> 24));
  }


  void D19clpGBTInterface::ConfigureRxPRBS(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable)
  {
    //Configure build-in PRBS generators
    for(const auto& cGroup : pGroups)
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
      uint8_t cEnabledCh = 0;
      for(const auto cChannel : pChannels)
         cEnabledCh |= pEnable << cChannel;
      this->WriteChipReg(pChip, cPRBSReg, (cValueEnablePRBS & ~(0xF << 4*(cGroup%2))) | (cEnabledCh << (4*(cGroup%2))));
    }
  }


  void D19clpGBTInterface::ConfigureRxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource)
  {
    //Configure Rx Data Source  
    for(const auto& cGroup : pGroups)
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
      this->WriteChipReg(pChip, cRxSourceReg, ( cValueRxSource & ~(0x7 << 3*(cGroup%2)) ) | (pSource << 3*(cGroup%2)) );
    }
  }


  void D19clpGBTInterface::ConfigureTxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource)
  {
    //Configure Tx Data Source
    for(const auto& cGroup : pGroups)
    {
      uint8_t cULDataSrcValue = this->ReadChipReg(pChip, "ULDataSource5");
      cULDataSrcValue = (cULDataSrcValue & ~(0x3 << (2*cGroup))) | (pSource << (2*cGroup));
      this->WriteChipReg(pChip, "ULDataSource5", cULDataSrcValue);
    }
  }


  void D19clpGBTInterface::ConfigureRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase)
  {
    //Configure Rx Channel Phase
    char cBuffer[13];
    sprintf(cBuffer, "EPRX%i%iChnCntr", pGroup, pChannel);
    std::string cRegName(cBuffer,sizeof(cBuffer));
    uint8_t cValueChnCntr = this->ReadChipReg(pChip, cRegName);
    cValueChnCntr = (cValueChnCntr & ~(0xF << 4)) | (pPhase << 4);
    this->WriteChipReg(pChip , cRegName, cValueChnCntr);
  }


  void D19clpGBTInterface::ConfigurePhShifter(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq, uint8_t pDriveStr, uint8_t pEnFTune, uint16_t pDelay)
  {
    //Configure Rx Phase Shifter
    for(const auto& cClock : pClocks)
    {
      char cBuffer1[8], cBuffer2[9];
      sprintf(cBuffer1, "PS%iDelay", cClock);
      sprintf(cBuffer2, "PS%iConfig", cClock);
      std::string cDelayReg(cBuffer1, sizeof(cBuffer1));
      std::string cConfigReg(cBuffer2, sizeof(cBuffer2));

      this->WriteChipReg(pChip, cConfigReg, ( ((pDelay & 0x100) >> 8) << 7 ) | pEnFTune << 6 | pDriveStr << 3 | pFreq );
      this->WriteChipReg(pChip, cDelayReg, pDelay);
    }
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT specific routine functions                                        */
  /*-------------------------------------------------------------------------*/

  void D19clpGBTInterface::PhaseTrainRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups)
  {
    //Launch Rx Phase Training
    for(const auto& cGroup : pGroups)
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

      this->WriteChipReg(pChip, cTrainRxReg, 0x0F << 4*(cGroup%2));
      this->WriteChipReg(pChip, cTrainRxReg, 0x00 << 4*(cGroup%2));
    }
  }


  void D19clpGBTInterface::PhaseAlignRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels)
  {   
    //FIXME the Rx configuration before the training is OT specific. Needs to be moved out
    //Phase Align Rx Channels
    //Configure Rx Groups
    uint8_t cDataRate = 2, cTrackMode = 1;
    this->ConfigureRxGroups(pChip, pGroups, pChannels, cDataRate, cTrackMode);

    //Configure Rx Channels
    uint8_t cEqual=0, cTerm=1, cAcBias=0, cInvert=0, cPhase=12;
    for(const auto& cGroup : pGroups)
    {
      for(const auto cChannel : pChannels)
      {
        if( cGroup == 0  && cChannel == 0 )  cInvert = 1;
        else if( cGroup == 1 && cChannel == 0 )  cInvert = 1;
        else if( cGroup == 3 && cChannel == 2 )  cInvert = 1;
        else if( cGroup == 2  )  cInvert = 1;
        else  cInvert = 0;
        this->ConfigureRxChannels(pChip, {cGroup}, {cChannel}, cEqual, cTerm, cAcBias, cInvert, cPhase);
      }
    }

    //Configure Rx Phase Shifter
    uint16_t cDelay = 0x00;
    uint8_t cFreq = 4, cEnFTune=0, cDriveStr=0; //Freq = 4 --> 320 MHz
    this->ConfigurePhShifter(pChip, {0,1,2,3}, cFreq, cDriveStr, cEnFTune, cDelay);

    //Find Phase
    //Turn ON PRBS for channels 0,2
    this->ConfigureRxPRBS(pChip, pGroups, pChannels, true);
    //Phase Train channels 0,2
    this->PhaseTrainRx(pChip, pGroups);
    for(const auto& cGroup : pGroups)
    {
      //Wait until channels lock
      LOG(INFO) << BOLDMAGENTA << "Phase Aligning Rx Group " << +cGroup  << RESET;
      do
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }while(!this->IsRxLocked(pChip, cGroup));
      LOG(INFO) << BOLDBLUE << "    Group " << +cGroup << BOLDGREEN << " LOCKED" << RESET; 
      //Set new phase to channels 0,2
      for(const auto cChannel : pChannels)
      {
        uint8_t cCurrPhase = this->GetRxPhase(pChip, cGroup, cChannel);
        LOG(INFO) << BOLDBLUE << "    Channel " << +cChannel << " phase is " << +cCurrPhase << RESET;
        this->ConfigureRxPhase(pChip, cGroup, cChannel, cCurrPhase);
      }
    }
    //Set back track mode to fixed phase (TrackMode = 0)
    this->ConfigureRxGroups(pChip, pGroups, pChannels, cDataRate, 0);
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT status functions                                                  */
  /*-------------------------------------------------------------------------*/

  void D19clpGBTInterface::PrintChipMode(Ph2_HwDescription::Chip* pChip)
  {
    switch((this->ReadChipReg(pChip, "ConfigPins") & 0xF0) >> 4)
    {
      case 0:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Off" << RESET; break;
      case 1:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex TX" << RESET; break;
      case 2:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex RX" << RESET; break;
      case 3:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Transceiver" << RESET; break;
      case 4:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Off" << RESET; break;
      case 5:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Simplex TX" << RESET; break;
      case 6:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Simplex RX" << RESET; break;
      case 7:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 5Gbps __ TxEncoding : FEC12 __ lpGBT Mode : Transceiver" << RESET; break;
      case 8:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Off" << RESET; break;
      case 9:  LOG(INFO) << BOLDGREEN << "lpGBT CHIP INFO __ Tx Data Rate : 10Gbps __ TxEncoding : FEC5 __ lpGBT Mode : Simplex TX" << RESET; break;
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
    //Gets success/failure of power-up FSM
    return ((uint8_t)this->ReadChipReg(pChip, "PUSMStatus") == 0x12);
  }


  uint8_t D19clpGBTInterface::GetRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel)
  {
    //Get Rx Channel Phase
    char cBuffer[19];
    if(pChannel == 0 || pChannel == 1)
      sprintf(cBuffer, "EPRX%iCurrentPhase10", pGroup);
    else if(pChannel == 3 || pChannel == 2)
      sprintf(cBuffer, "EPRX%iCurrentPhase32", pGroup);
    std::string cRxPhaseReg(cBuffer, sizeof(cBuffer));
    uint8_t cRxPhaseRegValue = this->ReadChipReg(pChip, cRxPhaseReg);
    return ( ( cRxPhaseRegValue & (0x0F << 4*(pChannel%2))) >> 4*(pChannel%2) );
  }
 

  bool D19clpGBTInterface::IsRxLocked(Ph2_HwDescription::Chip* pChip, uint8_t pGroup)
  {
    //Cheks if Rx channels are locked #FIXME needs to check depending on the
    //enabled channels not on all (0x0F)
    char cBuffer[11];
    sprintf(cBuffer, "EPRX%iLocked", pGroup);
    std::string cRXLockedReg(cBuffer, sizeof(cBuffer));
    return ( ((this->ReadChipReg(pChip, cRXLockedReg) & 0xF0) >> 4) == 0x0F );
  }


  uint8_t D19clpGBTInterface::GetRxDllStatus(Ph2_HwDescription::Chip* pChip, uint8_t pGroup)
  {
    //Gets Rx Group Delay-Locked-Loop status
    char cBuffer[14];
    sprintf(cBuffer, "EPRX%iDllStatus", pGroup);
    std::string cRXDllStatReg(cBuffer, sizeof(cBuffer));
    return this->ReadChipReg(pChip, cRXDllStatReg);
  }


  uint8_t D19clpGBTInterface::GetI2CStatus(Ph2_HwDescription::Chip* pChip, uint8_t pMaster)
  {
    //Gets I2C Master status
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
    //Resets I2C Masters
    for(const auto& cMaster : pMasters)  
    {
      //generating reset pulse on dedicated register bit 
      this->WriteChipReg(pChip, "RST0", 0 << cMaster);
      this->WriteChipReg(pChip, "RST0", 1 << cMaster);
      this->WriteChipReg(pChip, "RST0", 0 << cMaster);
    }
  }

  void D19clpGBTInterface::ConfigureI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pFreq, uint8_t pNBytes, uint8_t pSCLDriveMode)
  {
    //Configures I2C Masters
    //First let's write configuration data into the I2C Master Data register
    char cBuffer1[10];
    sprintf(cBuffer1, "I2CM%iData0", pMaster);
    std::string cI2CCntrlReg(cBuffer1, sizeof(cBuffer1));
    uint8_t cValueCntrl = (pFreq << 0) | (pNBytes << 2) | (pSCLDriveMode << 7);
    this->WriteChipReg(pChip, cI2CCntrlReg, cValueCntrl);

    //Now let's write Command (0x00) to the Command register to tranfer Configuration to the I2C Master Control register
    char cBuffer2[8];
    sprintf(cBuffer2, "I2CM%iCmd", pMaster);
    std::string cI2CCmdReg(cBuffer2, sizeof(cBuffer2));
    this->WriteChipReg(pChip, cI2CCmdReg, 0x00);
  }


  bool D19clpGBTInterface::WriteI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint32_t pData, uint8_t pNBytes)
  { 
    //Write Data to Slave Address using I2C Master
    if(pNBytes > 1)
      this->ConfigureI2C(pChip, pMaster, 0, pNBytes, 0);
    else
      this->ConfigureI2C(pChip, pMaster, 0, 0, 0);
      
    //Prepare Address Register
    char cBuffer2[12];
    sprintf(cBuffer2, "I2CM%iAddress", pMaster);
    std::string cI2CAddressReg(cBuffer2, sizeof(cBuffer2));
    //Write Slave Address
    this->WriteChipReg(pChip, cI2CAddressReg, pSlaveAddress);

    //Write Data to Data Register
    for(uint8_t cNByte=0; cNByte<pNBytes; cNByte++)
    {
      char cBuffer1[10];
      sprintf(cBuffer1, "I2CM%iData%i", pMaster, cNByte);
      std::string cI2CDataReg(cBuffer1, sizeof(cBuffer1));
      this->WriteChipReg(pChip, cI2CDataReg, pData);
    }


    //Prepare Command Register
    char cBuffer3[8];
    sprintf(cBuffer3, "I2CM%iCmd", pMaster);
    std::string cI2CCmdReg(cBuffer3, sizeof(cBuffer3));
    //If Multi-Byte, write command to save data locally before transfer to slave
    //FIXME for now this only provides a maximum of 32 bits (4 Bytes) write
    if(pNBytes>1)
      this->WriteChipReg(pChip, cI2CCmdReg, 0x8);
    //Write Command to launch I2C transaction
    if(pNBytes == 1)
      this->WriteChipReg(pChip, cI2CCmdReg, 0x2);
    else
      this->WriteChipReg(pChip, cI2CCmdReg, 0xC);
    
    //wait until the transaction is done
    uint8_t cMaxIter = 10, cIter=0;
    bool cSuccess = false;
    do
    {
      LOG(DEBUG) << BOLDBLUE << "Waiting for I2C transaction to finisih" << RESET;
      uint8_t cStatus = this->GetI2CStatus(pChip, pMaster);
      LOG(INFO) << BOLDBLUE << "I2C Master " << +pMaster << " -- Status : " << fI2CStatusMap[cStatus] << RESET;
      cSuccess = (cStatus == 4); 
      cIter++;
    }while(cIter<cMaxIter && !cSuccess);
    return cSuccess;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    //Verify success of write #FIXME can be removed or put under condition
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
    //Read Data from Slave Address using I2C Master
    this->ConfigureI2C(pChip, pMaster, 0, pNBytes, 0);
    //Prepare Address Register
    char cBuffer1[12];
    sprintf(cBuffer1, "I2CM%iAddress", pMaster);
    std::string cI2CAddressReg(cBuffer1, sizeof(cBuffer1));

    //Prepare Command Register
    char cBuffer2[8];
    sprintf(cBuffer2, "I2CM%iCmd", pMaster);
    std::string cI2CCmdReg(cBuffer2, sizeof(cBuffer2));
    //Write Slave Address
    this->WriteChipReg(pChip, cI2CAddressReg, pSlaveAddress);

    //Write Read Command and then Read from Read Data Register
    //Procedure and registers depend on number on Bytes
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
      for(uint8_t cNByte=0; cNByte<pNBytes; cNByte++)
      {
        char cBuffer1[10];
        sprintf(cBuffer1, "I2CM%dRead%i", pMaster, cNByte);
        std::string cI2CDataReg(cBuffer1, sizeof(cBuffer1));
        cReadData |= ((uint32_t)this->ReadChipReg(pChip, cI2CDataReg) << cNByte); 
      }
      return cReadData;
    }
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT ADC-DAC functions                                                 */
  /*-------------------------------------------------------------------------*/

  uint16_t D19clpGBTInterface::ReadADC(Ph2_HwDescription::Chip* pChip, const std::string& pADCInput)
  {
    //Read (converted) data from ADC Input with VREF/2 as negative Input
    uint8_t cADCInput = fADCInputMap[pADCInput];
    uint8_t cVREF = fADCInputMap["VREF/2"];
    LOG(INFO) << BOLDBLUE << "Reading ADC value from " << pADCInput << RESET;
    //Select ADC Input
    this->WriteChipReg(pChip, "ADCSelect", cADCInput << 4 | cVREF << 0); 
    //Enable ADC Input
    this->WriteChipReg(pChip, "ADCConfig", 1 << 2); 
    //Enable Internal VREF
    this->WriteChipReg(pChip, "VREFCNTR", 1 << 7); 
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //Start ADC conversion
    this->WriteChipReg(pChip, "ADCConfig", 1 << 7 | 1 << 2); 
    //Check conversion status
    uint8_t cMaxIter = 100, cIter=0;
    bool cSuccess = false;
    do
    {
      LOG(INFO) << BOLDBLUE << "Waiting for ADC conversion to end" << RESET;
      uint8_t cStatus = this->ReadChipReg(pChip, "ADCStatusH");
      cSuccess = ( ((cStatus&0x40) >> 6) == 1); 
      cIter++;
    }while(cIter<cMaxIter && !cSuccess);
    //Read ADC value
    uint8_t cADCvalue1 = this->ReadChipReg(pChip, "ADCStatusH")&0x3; 
    uint8_t cADCvalue2 = this->ReadChipReg(pChip, "ADCStatusL"); 
    //Clear ADC conversion bit (#FIXME disable ADC Input as well ??)
    this->WriteChipReg(pChip, "ADCConfig", 0 << 7 | 1 << 2); 
    return (cADCvalue1 << 8 | cADCvalue2);
  }


  uint16_t D19clpGBTInterface::ReadADCDiff(Ph2_HwDescription::Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN)
  {
    //Read differential (converted) data on two ADC inputs 
    uint8_t cADCInputP = fADCInputMap[pADCInputP];
    uint8_t cADCInputN = fADCInputMap[pADCInputN];
    LOG(INFO) << BOLDBLUE << "Reading ADC value from " << pADCInputP << RESET;
    //Select ADC Input
    this->WriteChipReg(pChip, "ADCSelect", cADCInputP << 4 | cADCInputN << 0); 
    //Enable ADC Input
    this->WriteChipReg(pChip, "ADCConfig", 1 << 2); 
    //Start ADC conversion
    this->WriteChipReg(pChip, "ADCConfig", 1 << 7 | 1 << 2); 
    //Check conversion status
    uint8_t cMaxIter = 10, cIter=0;
    bool cSuccess = false;
    do
    {
      LOG(INFO) << BOLDBLUE << "Waiting for ADC conversion to end" << RESET;
      uint8_t cStatus = this->ReadChipReg(pChip, "ADCStatusH");
      cSuccess = ( ((cStatus&0x40) >> 6) == 1); 
      cIter++;
    }while(cIter<cMaxIter && !cSuccess);
    //Read ADC value
    uint8_t cADCvalue1 = this->ReadChipReg(pChip, "ADCStatusH")&0x3; 
    uint8_t cADCvalue2 = this->ReadChipReg(pChip, "ADCStatusL"); 
    //Clear ADC conversion bit (#FIXME disable ADC Input as well ??)
    this->WriteChipReg(pChip, "ADCConfig", 0 << 7 | 1 << 2); 
    return (cADCvalue1 << 8 | cADCvalue2);
  }

  /*-------------------------------------------------------------------------*/
  /* OT specific test functions                                              */
  /*-------------------------------------------------------------------------*/

  void D19clpGBTInterface::SetConfigMode(const std::string& pMode)
  {
    if(pMode == "optical"){
      LOG(INFO) << "LpGBT in optical configuration mode" << RESET;
      fUseOpticalLink = true;
    }
    else if(pMode == "i2c"){
      LOG(INFO) << "LpGBT in I2C configuration mode" << RESET;
      fUseOpticalLink = false;
    }
    else{
      LOG(ERROR) << "Wrong configuration mode : choose [optical] or [i2c]" << RESET;
    }
  }
}

