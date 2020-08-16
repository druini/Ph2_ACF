/*!
  \file                  D19clpGBTInterface.h
  \brief                 Interface to access and control the low-power Gigabit Transceiver chip 
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
*/

#ifndef D19clpGBTInterface_H
#define D19clpGBTInterface_H

#include "lpGBTInterface.h"
#ifdef __TCUSB__
  #include "USB_a.h"
#endif

namespace Ph2_HwInterface
{
  class D19clpGBTInterface : public lpGBTInterface
  {
  public:
    D19clpGBTInterface (const BeBoardFWMap& pBoardMap) : lpGBTInterface(pBoardMap) {  }

    #ifdef __TCUSB__
      TC_PSROH fTC_PSROH;
    #endif

    //LpGBT register access functions 
    bool     ConfigureChip   (Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310)                                      override;
    bool     WriteChipReg    (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true)                   override;
    uint16_t ReadChipReg     (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode)                                                            override;
    bool     WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& RegVec, bool pVerifLoop = true)  override;
    bool     WriteReg        (Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop = true);
    uint16_t ReadReg         (Ph2_HwDescription::Chip* pChip, uint16_t pAddress);


    //LpGBT block configuration functions
    void     ConfigureRxGroups     (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate=0, uint8_t pTrackMode=0); 
    void     ConfigureRxChannels   (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pEqual=0, uint8_t pTerm=1, uint8_t pAcBias=0, uint8_t pInvert=0, uint8_t pPhase=12);
    void     ConfigureTxGroups     (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate=0);
    void     ConfigureTxChannels   (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDriveStr=0, uint8_t pPreEmphMode=0, uint8_t pPreEmphStr=0, uint8_t pPreEmphWidth=0, uint8_t pInvert=0);
    void     ConfigureClocks       (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClock, uint8_t pFreq=0, uint8_t pDriveStr=0, uint8_t pInvert=0, uint8_t pPreEmphWidth=0, uint8_t pPreEmphMode=0, uint8_t pPreEmphStr=0);
    void     ConfigureTxRxPolarity (Ph2_HwDescription::Chip* pChip, uint8_t pTxPolarity=1, uint8_t pRxPolarity=0);
    void     ConfigureDPPattern    (Ph2_HwDescription::Chip* pChip, uint32_t pPattern);
    void     ConfigureRxPRBS       (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable=false);
    void     ConfigureRxSource     (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource=0);
    void     ConfigureTxSource     (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource=0);
    void     ConfigureRxPhase      (Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase=0);
    void     ConfigurePhShifter    (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq=0, uint8_t pDriveStr=0, uint8_t pEnFTune=0, uint16_t pDelay=0);

    //LpGBT specific routine functions
    void     PhaseTrainRx          (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups);
    void     PhaseAlignRx          (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels);

    //LpGBT block status functions
    void     PrintChipMode         (Ph2_HwDescription::Chip* pChip);           
    uint8_t  GetRxDllStatus        (Ph2_HwDescription::Chip* pChip, uint8_t pGroup);
    uint8_t  GetRxPhase            (Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel);
    bool     IsRxLocked            (Ph2_HwDescription::Chip* pChip, uint8_t pGroup);
    bool     IslpGBTReady          (Ph2_HwDescription::Chip* pChip);

    //LpGBT I2C Masters functions (Slow Control)
    void     ResetI2C              (Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pMasters);
    void     ConfigureI2C          (Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pFreq=0, uint8_t pNBytes=1, uint8_t pSCLDriveMode=0);  
    bool     WriteI2C              (Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint32_t pData, uint8_t pNBytes=1);
    uint32_t ReadI2C               (Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint8_t pNBytes=1);
    uint8_t  GetI2CStatus          (Ph2_HwDescription::Chip* pChip, uint8_t pMaster);

    //LpGBT ADC-DAC functions
    uint16_t ReadADC               (Ph2_HwDescription::Chip* pChip, const std::string& pADCInput);
    uint16_t ReadADCDiff           (Ph2_HwDescription::Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN);

    //OT Specific test functions #FIXME need to be moved to a dedicated Tool
    void     SetConfigMode         (const std::string& pMode);
 
  private:

    std::map<std::string, uint8_t> fADCInputMap = {{"ADC0", 0}, {"ADC1", 1}, {"ADC2", 2}, {"ADC3", 3},
                                                   {"ADC4", 4}, {"ADC5", 5}, {"ADC6", 6}, {"ADC7", 7},
                                                   {"EOM_DAC", 8}, {"VDDIO", 9}, {"VDDTX", 10}, {"VDDRX", 11},
                                                   {"VDD", 12}, {"VDDA", 13}, {"TEMP", 14}, {"VREF/2", 15}};

    std::map<uint8_t, std::string> fI2CStatusMap = {{4, "TransactionSucess"}, {8, "SDAPulledLow"}, {32, "InvalidCommand"}, {64, "NotACK"}};

    //OT specific objects
    bool fUseOpticalLink = false;
    #ifdef __TCUSB__
      std::map<std::string, TC_PSROH::measurement> fResetLines =
      {
        {"L_MPA", TC_PSROH::measurement::L_MPA_RST},
        {"L_CIC", TC_PSROH::measurement::L_CIC_RST},
        {"L_SSA", TC_PSROH::measurement::L_SSA_RST},
        {"R_MPA", TC_PSROH::measurement::R_MPA_RST},
        {"R_CIC", TC_PSROH::measurement::R_CIC_RST},
        {"R_SSA", TC_PSROH::measurement::R_SSA_RST}
      };
    #endif
    
  };
}
#endif
