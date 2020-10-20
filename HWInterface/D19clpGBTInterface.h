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
    D19clpGBTInterface(const BeBoardFWMap& pBoardMap) : lpGBTInterface(pBoardMap) {}

#ifdef __TCUSB__
    TC_PSROH fTC_PSROH;
#endif

    // ##################################
    // # LpGBT register access functions#
    // ##################################
    //General configuration of the lpGBT chip from register file
    bool     ConfigureChip(Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    //R/W functions using register name
    bool     WriteChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true) override;
    uint16_t ReadChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode) override;
    //R/W functions using register address
    bool     WriteReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop = false);
    uint16_t ReadReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress);
    bool     WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& RegVec, bool pVerifLoop = true) override;

    // ######################################
    // # LpGBT block configuration functions#
    // ######################################
    //Sets the flag used to select which lpGBT configuration interface to use
    void SetConfigMode(Ph2_HwDescription::Chip* pChip, const std::string& pMode, bool pToggle);
    //Configures the lpGBT Rx Groups
    void ConfigureRxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate = 0, uint8_t pTrackMode = 0);
    //Configure lpGBT Rx Channels

    void ConfigureRxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pEqual  = 0,
                             uint8_t                     pTerm   = 1,
                             uint8_t                     pAcBias = 0,
                             uint8_t                     pInvert = 0,
                             uint8_t                     pPhase  = 12);
    //Configure lpGBT Tx Groups
    void ConfigureTxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate = 0);
    //Configure lpGBT Tx Channels
    void ConfigureTxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pDriveStr     = 0,
                             uint8_t                     pPreEmphMode  = 0,
                             uint8_t                     pPreEmphStr   = 0,
                             uint8_t                     pPreEmphWidth = 0,
                             uint8_t                     pInvert       = 0);
    //Configure lpGBT Clocks
    void ConfigureClocks(Ph2_HwDescription::Chip*    pChip,
                         const std::vector<uint8_t>& pClock,
                         uint8_t                     pFreq         = 0,
                         uint8_t                     pDriveStr     = 0,
                         uint8_t                     pInvert       = 0,
                         uint8_t                     pPreEmphWidth = 0,
                         uint8_t                     pPreEmphMode  = 0,
                         uint8_t                     pPreEmphStr   = 0);
    //Configure lpGBT High Speed Link Tx and Rx polarity
    void ConfigureHighSpeedPolarity(Ph2_HwDescription::Chip* pChip, uint8_t pOutPolarity = 1, uint8_t pInPolarity = 0);
    //Configure lpGBT Data Player pattern
    void ConfigureDPPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern);
    //Configure lpGBT Rx Pseudo-Random Binary Sequence
    void ConfigureRxPRBS(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable = false);
    //Configure lpGBT Rx Groups data source
    void ConfigureRxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource = 0);
    //Configure lpGBT Tx Groups data source
    void ConfigureTxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource = 0);
    //Configure lpGBT Rx channels phase
    void ConfigureRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase = 0);
    //Configure lpGBT Phase Shifter
    void ConfigurePhShifter(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq = 0, uint8_t pDriveStr = 0, uint8_t pEnFTune = 0, uint16_t pDelay = 0);

    // ####################################
    // # LpGBT specific routine functions #
    // ####################################
    //lpGBT Rx Groups(Channels) phase training
    void PhaseTrainRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups);
    //lpGBT Rx Groups(Channels) phase alignment
    void PhaseAlignRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels);

    // ##############################################
    // # LpGBT Block Status functions               #
    // ##############################################
    //Print out lpGBT chip mode (data rate, FEC, transmission mode)
    void PrintChipMode(Ph2_HwDescription::Chip* pChip);
    //Get lpGBT Rx Group Delay-Locked-Loop state machine
    uint8_t GetRxDllStatus(Ph2_HwDescription::Chip* pChip, uint8_t pGroup);
    //Get lpGBT Rx Channel phase
    uint8_t GetRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel);
    //Get lpGBT Rx locking status
    bool IsRxLocked(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, const std::vector<uint8_t>& pChannels);
    //Get lpGBT Power Up State Machine status
    bool IslpGBTReady(Ph2_HwDescription::Chip* pChip);

    // ##############################################
    // # LpGBT I2C Masters functions (Slow Control) #
    // ##############################################
    //Reset I2C Masters
    void ResetI2C(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pMasters);
    //Configure lpGBT I2C Master
    void ConfigureI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pFreq = 0, uint8_t pNBytes = 1, uint8_t pSCLDriveMode = 0);
    //I2C Write transaction using the lpGBT I2C Master
    bool WriteI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint32_t pData, uint8_t pNBytes = 1);
    //I2C Read transaction using the lpGBT I2C Master
    uint32_t ReadI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint8_t pNBytes = 1);
    //Get lpGBT I2C Master status
    uint8_t GetI2CStatus(Ph2_HwDescription::Chip* pChip, uint8_t pMaster);

    // ###########################
    // # LpGBT ADC-DAC functions #
    // ###########################
    //configure ADC
    void ConfigureADC(Ph2_HwDescription::Chip* pChip, uint8_t pGainSelect = 0, uint8_t pADCCoreDiffEnable = 0);
    //brief Read single ended lpGBT ADC
    uint16_t ReadADC(Ph2_HwDescription::Chip* pChip, const std::string& pADCInput);
    //Read lpGBT differential ADC
    uint16_t ReadADCDiff(Ph2_HwDescription::Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN);

    // ###############################
    // # Operation specific funtions #
    // ###############################
    // cbc read/write
    bool     cbcWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pPage, uint8_t pRegistergAddress, uint8_t pRegisterValue, bool pReadBack=true, bool pSetPage=false){ return true; }
    uint32_t cbcRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pPage, uint8_t pRegisterAddress){ return 0; }
    uint8_t  cbcSetPage(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pPage){ return 0; }
    uint8_t  cbcGetPageRegister(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t cChipId){ return 0; }
    // cic read/write
    bool     cicWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pRegisterAddress, uint8_t pRegisterValue, bool pRetry=true);
    uint32_t cicRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pRegisterAddress);
    // ssa read/write 
    bool     ssaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pRetry=false);
    uint32_t ssaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress);
    // mpa read/write
    bool     mpaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pRetry=false);
    uint32_t mpaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress);

  private:
    std::map<std::string, uint8_t> fADCInputMap = {{"ADC0", 0},
                                                   {"ADC1", 1},
                                                   {"ADC2", 2},
                                                   {"ADC3", 3},
                                                   {"ADC4", 4},
                                                   {"ADC5", 5},
                                                   {"ADC6", 6},
                                                   {"ADC7", 7},
                                                   {"EOM_DAC", 8},
                                                   {"VDDIO", 9},
                                                   {"VDDTX", 10},
                                                   {"VDDRX", 11},
                                                   {"VDD", 12},
                                                   {"VDDA", 13},
                                                   {"TEMP", 14},
                                                   {"VREF/2", 15}};

    std::map<uint8_t, std::string> fI2CStatusMap = {{4, "TransactionSucess"}, {8, "SDAPulledLow"}, {32, "InvalidCommand"}, {64, "NotACK"}};

    // OT specific objects
    bool fUseOpticalLink = false;
#ifdef __TCUSB__
    std::map<std::string, TC_PSROH::measurement> fResetLines = {{"L_MPA", TC_PSROH::measurement::L_MPA_RST},
                                                                {"L_CIC", TC_PSROH::measurement::L_CIC_RST},
                                                                {"L_SSA", TC_PSROH::measurement::L_SSA_RST},
                                                                {"R_MPA", TC_PSROH::measurement::R_MPA_RST},
                                                                {"R_CIC", TC_PSROH::measurement::R_CIC_RST},
                                                                {"R_SSA", TC_PSROH::measurement::R_SSA_RST}};
#endif
};
} // namespace Ph2_HwInterface
#endif
