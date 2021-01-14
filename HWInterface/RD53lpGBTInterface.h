/*!
  \file                  RD53lpGBTInterface.h
  \brief                 Interface to access and control the Low-power Gigabit Transceiver chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  03/03/20
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53lpGBTInterface_H
#define RD53lpGBTInterface_H

#include "../Utils/RD53Shared.h"
#include "lpGBTInterface.h"

// ##########################
// # LpGBT useful constants #
// ##########################
namespace RD53lpGBTconstants
{
const uint8_t MAXATTEMPTS  = 10;   // Maximum number of attempts
const uint8_t LPGBTADDRESS = 0x70; // LpGBT chip address
} // namespace RD53lpGBTconstants

namespace Ph2_HwInterface
{
class RD53lpGBTInterface : public lpGBTInterface
{
  public:
    RD53lpGBTInterface(const BeBoardFWMap& pBoardMap) : lpGBTInterface(pBoardMap) {}

    // ###################################
    // # LpGBT register access functions #
    // ###################################
    bool     ConfigureChip(Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    bool     WriteChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true) override;
    bool     WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& RegVec, bool pVerifLoop = true) override;
    uint16_t ReadChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode) override;

    bool     WriteReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop = true);
    uint16_t ReadReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress);

    // #######################################
    // # LpGBT block configuration functions #
    // #######################################
    // Configures the LpGBT Rx Groups
    void ConfigureRxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate, uint8_t pTrackMode);
    // Configure LpGBT Rx Channels
    void ConfigureRxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pEqual,
                             uint8_t                     pTerm,
                             uint8_t                     pAcBias,
                             uint8_t                     pInvert,
                             uint8_t                     pPhase);
    // Configure LpGBT Tx Groups
    void ConfigureTxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate);
    // Configure LpGBT Tx Channels
    void ConfigureTxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pDriveStr,
                             uint8_t                     pPreEmphMode,
                             uint8_t                     pPreEmphStr,
                             uint8_t                     pPreEmphWidth,
                             uint8_t                     pInvert);
    // Configure LpGBT Clocks
    void ConfigureClocks(Ph2_HwDescription::Chip*    pChip,
                         const std::vector<uint8_t>& pClock,
                         uint8_t                     pFreq,
                         uint8_t                     pDriveStr,
                         uint8_t                     pInvert,
                         uint8_t                     pPreEmphWidth,
                         uint8_t                     pPreEmphMode,
                         uint8_t                     pPreEmphStr);
    // Configure LpGBT High Speed Link Tx and Rx polarity
    void ConfigureHighSpeedPolarity(Ph2_HwDescription::Chip* pChip, uint8_t pOutPolarity, uint8_t pInPolarity);
    // Configure LpGBT Data Player pattern
    void ConfigureDPPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern);
    // Configure LpGBT Rx Pseudo-Random Binary Sequence
    void ConfigureRxPRBS(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable);
    // Configure LpGBT Rx Groups data source
    void ConfigureRxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource);
    // Configure LpGBT Tx Groups data source
    void ConfigureTxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource);
    // Configure LpGBT Rx channels phase
    void ConfigureRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase);
    // Configure LpGBT Phase Shifter
    void ConfigurePhShifter(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq, uint8_t pDriveStr, uint8_t pEnFTune, uint16_t pDelay);

    // ####################################
    // # LpGBT specific routine functions #
    // ####################################
    // LpGBT Rx Groups(Channels) phase training
    void PhaseTrainRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups);
    // LpGBT Rx Groups(Channels) phase alignment
    void PhaseAlignRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pRate);

    // ################################
    // # LpGBT Block Status functions #
    // ################################
    // Print out LpGBT chip mode (data rate, FEC, transmission mode)
    void PrintChipMode(Ph2_HwDescription::Chip* pChip);
    // Get LpGBT Power Up State Machine status
    uint8_t GetPUSMStatus(Ph2_HwDescription::Chip* pChip);
    // Get LpGBT Rx Channel phase
    uint8_t GetRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel);
    // Get LpGBT Rx locking status
    bool IsRxLocked(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, const std::vector<uint8_t>& pChannels);
    // Get LpGBT Rx Group Delay-Locked-Loop state machine
    uint8_t GetRxDllStatus(Ph2_HwDescription::Chip* pChip, uint8_t pGroup);

    // ########################
    // # LpGBT GPIO functions #
    // ########################
    void ConfigureGPIO(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGPIOs, uint8_t pInOut, uint8_t pHighLow, uint8_t pDriveStr, uint8_t pPullEn, uint8_t pPullUpDown);

    // ###########################
    // # LpGBT ADC-DAC functions #
    // ###########################
    // configure ADC
    void ConfigureADC(Ph2_HwDescription::Chip* pChip, uint8_t pGainSelect, uint8_t pADCCoreDiffEnable);
    // brief Read single ended LpGBT ADC
    uint16_t ReadADC(Ph2_HwDescription::Chip* pChip, const std::string& pADCInput);
    // Read LpGBT differential ADC
    uint16_t ReadADCDiff(Ph2_HwDescription::Chip* pChip, const std::string& pADCInputP, const std::string& pADCInputN);

    // #####################
    // # LpGBT BERT Tester #
    // #####################
    // configure BER tester
    void ConfigureBERT(Ph2_HwDescription::Chip* pChip, uint8_t pCoarseSource, uint8_t pFineSource, uint8_t pMeasTime, uint8_t pSkipDisable, bool pStart);
    // Configure BER pattern
    void ConfigureBERTPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern);
    // Get BER status
    uint8_t GetBERTStatus(Ph2_HwDescription::Chip* pChip);
    // Get BERT errors
    uint64_t GetBERTErrors(Ph2_HwDescription::Chip* pChip);
    // Perform BER test
    float PerformBERTest(Ph2_HwDescription::Chip* pChip, uint8_t pCoarseSource, uint8_t pFineSource, uint8_t pMeasTime, uint8_t pSkipDisable, uint32_t pPattern);

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

    std::map<std::string, uint8_t> revertedPUSMStatusMap;
};
} // namespace Ph2_HwInterface
#endif
