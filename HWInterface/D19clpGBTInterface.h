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
#include "USB_libusb.h"
#endif

namespace Ph2_HwInterface
{
class D19clpGBTInterface : public lpGBTInterface
{
  public:
    D19clpGBTInterface(const BeBoardFWMap& pBoardMap, bool pUseOpticalLink, bool pUseCPB) : lpGBTInterface(pBoardMap), fUseOpticalLink(pUseOpticalLink), fUseCPB(pUseCPB) {}
    ~D19clpGBTInterface()
    {
#ifdef __TCUSB__
        if(fTC_USB != nullptr) delete fTC_USB;
#endif
    }

    // ###################################
    // # LpGBT register access functions #
    // ###################################
    // General configuration of the lpGBT chip from register file
    bool ConfigureChip(Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    // R/W functions using register name
    bool     WriteChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true) override;
    uint16_t ReadChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode) override;
    // R/W functions using register address
    bool     WriteReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop = true);
    uint16_t ReadReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress);
    bool     WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& RegVec, bool pVerifLoop = true) override;
    void     StartPRBSpattern(Ph2_HwDescription::Chip* pChip) override{};
    void     StopPRBSpattern(Ph2_HwDescription::Chip* pChip) override{};

    // #######################################
    // # LpGBT block configuration functions #
    // #######################################
    void SetPUSMDone(Ph2_HwDescription::Chip* pChip, bool pPllConfigDone, bool pDllConfigDone);
    // Configures the lpGBT Rx Groups
    void ConfigureRxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate, uint8_t pTrackMode);
    // Configure lpGBT Rx Channels
    void ConfigureRxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pEqual,
                             uint8_t                     pTerm,
                             uint8_t                     pAcBias,
                             uint8_t                     pInvert,
                             uint8_t                     pPhase);
    // Configure lpGBT Tx Groups
    void ConfigureTxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate);
    // Configure lpGBT Tx Channels
    void ConfigureTxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pDriveStr,
                             uint8_t                     pPreEmphMode,
                             uint8_t                     pPreEmphStr,
                             uint8_t                     pPreEmphWidth,
                             uint8_t                     pInvert);
    // Configure lpGBT Clocks
    void ConfigureClocks(Ph2_HwDescription::Chip*    pChip,
                         const std::vector<uint8_t>& pClock,
                         uint8_t                     pFreq,
                         uint8_t                     pDriveStr,
                         uint8_t                     pInvert,
                         uint8_t                     pPreEmphWidth,
                         uint8_t                     pPreEmphMode,
                         uint8_t                     pPreEmphStr);
    // Configure lpGBT High Speed Link Tx and Rx polarity
    void ConfigureHighSpeedPolarity(Ph2_HwDescription::Chip* pChip, uint8_t pOutPolarity, uint8_t pInPolarity);
    // Configure lpGBT Data Player pattern
    void ConfigureDPPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern);
    // Configure lpGBT Rx Pseudo-Random Binary Sequence
    void ConfigureRxPRBS(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable);
    // Configure lpGBT Rx Groups data source
    void ConfigureRxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource);
    // Configure lpGBT Tx Groups data source
    void ConfigureTxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource);
    // Configure lpGBT Rx channels phase
    void ConfigureRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase);
    // Configure lpGBT Phase Shifter
    void ConfigurePhShifter(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq, uint16_t pDelay, uint8_t pDriveStr = 0, uint8_t pEnFTune = 0);

    // ####################################
    // # LpGBT specific routine functions #
    // ####################################
    // lpGBT Rx Groups(Channels) phase training
    void PhaseTrainRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, bool pTrain = false);
    // lpGBT Rx Groups(Channels) phase alignment
    void PhaseAlignRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels);

    // ################################
    // # LpGBT Block Status functions #
    // ################################
    // Print out lpGBT chip mode (data rate, FEC, transmission mode)
    void PrintChipMode(Ph2_HwDescription::Chip* pChip);
    // Get lpGBT Chip Rate
    uint8_t GetChipRate(Ph2_HwDescription::Chip* pChip);
    // Get lpGBT Rx Group Delay-Locked-Loop state machine
    uint8_t GetRxDllStatus(Ph2_HwDescription::Chip* pChip, uint8_t pGroup);
    // Get lpGBT Rx Channel phase
    uint8_t GetRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel);
    // Get lpGBT Rx locking status
    bool IsRxLocked(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, const std::vector<uint8_t>& pChannels);
    // Get lpGBT Power Up State Machine status
    uint8_t GetPUSMStatus(Ph2_HwDescription::Chip* pChip);
    bool    IsPUSMDone(Ph2_HwDescription::Chip* pChip);

    // ##############################################
    // # LpGBT I2C Masters functions (Slow Control) #
    // ##############################################
    // Reset I2C Masters
    void ResetI2C(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pMasters);
    // Configure lpGBT I2C Master
    void ConfigureI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pFreq, uint8_t pNBytes, uint8_t pSCLDriveMode);
    // I2C Write transaction using the lpGBT I2C Master
    bool WriteI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint32_t pData, uint8_t pNBytes);
    // I2C Read transaction using the lpGBT I2C Master
    uint32_t ReadI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint8_t pNBytes);
    // Get lpGBT I2C Master status
    uint8_t GetI2CStatus(Ph2_HwDescription::Chip* pChip, uint8_t pMaster);
    // Check if I2C Transaction is successful
    bool IsI2CSuccess(Ph2_HwDescription::Chip* pChip, uint8_t pMaster);

    // ###########################
    // # LpGBT ADC-DAC functions #
    // ###########################
    // configure ADC
    void ConfigureADC(Ph2_HwDescription::Chip* pChip, uint8_t pGainSelect, bool pADCEnable, bool pStartConversion);
    // configure current DAC
    void ConfigureCurrentDAC(Ph2_HwDescription::Chip* pChip, const std::vector<std::string>& pCurrentDACChannels, uint8_t pCurrentDACOutput);
    // Read lpGBT ADC
    uint16_t ReadADC(Ph2_HwDescription::Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN = "VREF/2", uint8_t pGain = 0);
    // Get ADC Read Status
    bool IsReadADCDone(Ph2_HwDescription::Chip* pChip);

    // ########################
    // # LpGBT GPIO functions #
    // ########################
    bool ReadGPIO(Ph2_HwDescription::Chip* pChip, const uint8_t& pGPIO);
    // Configure GPIO direction (In/Out)
    void ConfigureGPIODirection(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGPIOs, uint8_t pDir);
    // Configure GPIO Level (High/Low)
    void ConfigureGPIOLevel(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGPIOs, uint8_t pOut);
    // Configure FPIO Driver Strenght
    void ConfigureGPIODriverStrength(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGPIOs, uint8_t pDriveStr);
    // Configure GPIO Pull (Up/Down) -- (Enable/Disable)
    void ConfigureGPIOPull(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGPIOs, uint8_t pPullEn, uint8_t pPullUpDown);

    // ###############################
    // # LpGBT Bit Error Rate Tester #
    // ###############################
    // configure BER tester
    void ConfigureBERT(Ph2_HwDescription::Chip* pChip, uint8_t pCoarseSource, uint8_t pFineSource, uint8_t pMeasTime, bool pSkipDisable = false);
    // start BER tester
    void StartBERT(Ph2_HwDescription::Chip* pChip, bool pStartBERT = true);
    // configure BER pattern
    void ConfigureBERTPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern);
    // get BER status
    uint8_t GetBERTStatus(Ph2_HwDescription::Chip* pChip);
    // get if BER done
    bool IsBERTDone(Ph2_HwDescription::Chip* pChip);
    // get if BER tester received empty data
    bool IsBERTEmptyData(Ph2_HwDescription::Chip* pChip);
    // get BERT errors
    uint64_t GetBERTErrors(Ph2_HwDescription::Chip* pChip);
    // Run Bit Error Test
    float  GetBERTResult(Ph2_HwDescription::Chip* pChip);
    double RunBERtest(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, bool given_time, double frames_or_time, uint8_t frontendSpeed = 0) override { return 0; }

    // ####################################
    // # LpGBT Eye Opening Monitor Tester #
    // ####################################
    // Configure Eye Opening Monitor
    void ConfigureEOM(Ph2_HwDescription::Chip* pChip, uint8_t pEndOfCountSelect, bool pByPassPhaseInterpolator = false, bool pEnableEOM = true);
    // Start Eye Opening Monitor
    void StartEOM(Ph2_HwDescription::Chip* pChip, bool pStartEOM = true);
    // Select Eye Opening Monitor sampling phase
    void SelectEOMPhase(Ph2_HwDescription::Chip* pChip, uint8_t pPhase);
    // Select Eye Opening Monitor comparator voltage
    void SelectEOMVof(Ph2_HwDescription::Chip* pChip, uint8_t pVof);
    // Get Eye Opening Monitor status
    uint8_t GetEOMStatus(Ph2_HwDescription::Chip* pChip);
    // Get Eye Opening Monitor counter value
    uint16_t GetEOMCounter(Ph2_HwDescription::Chip* pChip);

    // ###################################
    // # Outer Tracker specific funtions #
    // ###################################
#ifdef __TCUSB__
    void InitialiseTCUSBHandler();
#ifdef __ROH_USB__
    void      SetTCUSBHandler(TC_PSROH* pTC_PSROH) { fTC_USB = pTC_PSROH; }
    TC_PSROH* GetTCUSBHandler() { return fTC_USB; }
#elif __SEH_USB__
    void                                              SetTCUSBHandler(TC_2SSEH* pTC_2SSEH) { fTC_USB = pTC_2SSEH; }
    TC_2SSEH*                                         GetTCUSBHandler() { return fTC_USB; }
#endif

#endif
    // Sets the flag used to select which lpGBT configuration interface to use
    void SetConfigMode(Ph2_HwDescription::Chip* pChip, bool pUseOpticalLink, bool pUseCPB, bool pToggleTC = false);
    // configure PS-ROH
    void ConfigurePSROH(Ph2_HwDescription::Chip* pChip);
    // configure 2S-SEH
    void Configure2SSEH(Ph2_HwDescription::Chip* pChip);
    // cbc read/write
    bool cbcWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pPage, uint8_t pRegistergAddress, uint8_t pRegisterValue, bool pReadBack = true, bool pSetPage = false)
    {
        return true;
    }
    uint32_t cbcRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pPage, uint8_t pRegisterAddress) { return 0; }
    uint8_t  cbcSetPage(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pPage) { return 0; }
    uint8_t  cbcGetPageRegister(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t cChipId) { return 0; }
    // cic read/write
    bool     cicWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pRetry = false);
    uint32_t cicRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint16_t pRegisterAddress);
    // ssa read/write
    bool     ssaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pRetry = false);
    uint32_t ssaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress);
    // mpa read/write
    bool     mpaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint16_t pRegisterAddress, uint8_t pRegisterValue, bool pRetry = false);
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

    std::map<uint8_t, std::string> fPUSMStatusMap = {{0, "ARESET"},
                                                     {1, "RESET"},
                                                     {2, "WAIT_VDD_STABLE"},
                                                     {3, "WAIT_VDD_HIGHER_THAN_0V90"},
                                                     {4, "FUSE_SAMPLING"},
                                                     {5, "UPDATE_FROM_FUSES"},
                                                     {6, "WAIT_FOR_PLL_CONFIG"},
                                                     {7, "WAIT_POWER_GOOD"},
                                                     {8, "RESETOUT"},
                                                     {9, "I2C_TRANS"},
                                                     {10, "RESET_PLL"},
                                                     {11, "WAIT_PLL_LOCK"},
                                                     {12, "INIT_SCRAM"},
                                                     {13, "PAUSE_FOR_DLL_CONFIG"},
                                                     {14, "RESET_DLLS"},
                                                     {15, "WAIT_DLL_LOCK"},
                                                     {16, "RESET_LOGIC_USING_DLL"},
                                                     {17, "WAIT_CHNS_LOCKED"},
                                                     {18, "READY"}};
    std::map<uint8_t, std::string> fI2CStatusMap  = {{4, "TransactionSucess"}, {8, "SDAPulledLow"}, {32, "InvalidCommand"}, {64, "NotACK"}};

    std::map<uint8_t, std::string> fEOMStatusMap = {{0, "smIdle"}, {1, "smResetCounters"}, {2, "smCount"}, {3, "smEndOfCount"}};

    // ###################################
    // # Outer Tracker specific objects  #
    // ###################################
    bool fUseOpticalLink = true;
    bool fUseCPB         = true;
#ifdef __TCUSB__
#ifdef __ROH_USB__
    TC_PSROH*                                    fTC_USB;
    std::map<std::string, TC_PSROH::measurement> fResetLines = {{"L_MPA", TC_PSROH::measurement::L_MPA_RST},
                                                                {"L_CIC", TC_PSROH::measurement::L_CIC_RST},
                                                                {"L_SSA", TC_PSROH::measurement::L_SSA_RST},
                                                                {"R_MPA", TC_PSROH::measurement::R_MPA_RST},
                                                                {"R_CIC", TC_PSROH::measurement::R_CIC_RST},
                                                                { "R_SSA",
                                                                  TC_PSROH::measurement::R_SSA_RST }};

#elif __SEH_USB__
    TC_2SSEH*                                         fTC_USB;
    std::map<std::string, TC_2SSEH::resetMeasurement> fSehResetLines = {{"RST_CBC_R", TC_2SSEH::resetMeasurement::RST_CBC_R},
                                                                        {"RST_CIC_R", TC_2SSEH::resetMeasurement::RST_CIC_R},
                                                                        {"RST_CBC_L", TC_2SSEH::resetMeasurement::RST_CBC_L},
                                                                        {"RST_CIC_L", TC_2SSEH::resetMeasurement::RST_CIC_L}};
#endif
#endif
};
} // namespace Ph2_HwInterface
#endif
