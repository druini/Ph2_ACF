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
    bool     ConfigureChip(Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    bool     WriteChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true) override;
    uint16_t ReadChipReg(Ph2_HwDescription::Chip* pChip, const std::string& pRegNode) override;
    bool     WriteChipMultReg(Ph2_HwDescription::Chip* pChip, const std::vector<std::pair<std::string, uint16_t>>& RegVec, bool pVerifLoop = true) override;
    bool     WriteReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress, uint16_t pValue, bool pVerifLoop = true);
    uint16_t ReadReg(Ph2_HwDescription::Chip* pChip, uint16_t pAddress);

    // ######################################
    // # LpGBT block configuration functions#
    // ######################################
    /*!
     * \brief Sets the flag used to select which lpGBT configuration interface to use
     * \param pChip : pointer to Chip object
     * \param pMode : configuration interface ["serial" / "i2c"]
     */
    void SetConfigMode(Ph2_HwDescription::Chip* pChip, const std::string& pMode, bool pToggle);

    /*!
     * \brief Configures the lpGBT Rx Groups
     * \param pChip      : pointer to Chip object
     * \param pGroups    : Rx Groups vector
     * \param pChannels  : Rx Channels vector
     * \param pDataRate  : Data Rate
     * \param pTrackMode : Phase Tracking Mode
     */
    void ConfigureRxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate = 0, uint8_t pTrackMode = 0);

    /*!
     * \brief Configure lpGBT Rx Channels
     * \param pChip     : pointer to Chip object
     * \param pGroups   : Rx Groups vector
     * \param pChannels : Rx Channels vector
     * \param pEqual    : Equalization control
     * \param pTerm     : 100 Ohm termination
     * \param pAcBias   : Common mode generation
     * \param pInvert   : Channel invertion
     * \param pPhase    : Channel phase selection
     */
    void ConfigureRxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pEqual  = 0,
                             uint8_t                     pTerm   = 1,
                             uint8_t                     pAcBias = 0,
                             uint8_t                     pInvert = 0,
                             uint8_t                     pPhase  = 12);

    /*!
     * \brief Configure lpGBT Tx Groups
     * \param pChip     : pointer to Chip object
     * \param pGroups   : Tx Groups vector
     * \param pChannels : Tx Channels vector
     * \param pDataRate : Data Rate
     */
    void ConfigureTxGroups(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, uint8_t pDataRate = 0);

    /*!
     * \brief Configure lpGBT Tx Channels
     * \param pChip         : pointer to Chip object
     * \param pGroups       : Tx Groups vector
     * \param pChannels     : Tx Channels vector
     * \param pDriveStr     : Driving strenght
     * \param pPreEmphMode  : Pre-Emphasis mode
     * \param pPreEmphStr   : Pre-Emphasis strength
     * \param pPreEmphWidth : Width of the pre-emphasis pulse
     * \param pInvert       : Channel inversion
     */
    void ConfigureTxChannels(Ph2_HwDescription::Chip*    pChip,
                             const std::vector<uint8_t>& pGroups,
                             const std::vector<uint8_t>& pChannels,
                             uint8_t                     pDriveStr     = 0,
                             uint8_t                     pPreEmphMode  = 0,
                             uint8_t                     pPreEmphStr   = 0,
                             uint8_t                     pPreEmphWidth = 0,
                             uint8_t                     pInvert       = 0);

    /*!
     * \brief Configure lpGBT Clocks
     * \param pChip         : pointer to Chip object
     * \param pClock        : Clocks vector
     * \param pFreq         : Frequency
     * \param pDriveStr     : Driving strength
     * \param pInvert       : Clock inversion
     * \param pPreEmphWidth : Width of the pre-emphasis pulse
     * \param pPreEmphMode  : Pre-Emphasis mode
     * \param pPreEmphStr   : Pre-Emphasis strength
     */
    void ConfigureClocks(Ph2_HwDescription::Chip*    pChip,
                         const std::vector<uint8_t>& pClock,
                         uint8_t                     pFreq         = 0,
                         uint8_t                     pDriveStr     = 0,
                         uint8_t                     pInvert       = 0,
                         uint8_t                     pPreEmphWidth = 0,
                         uint8_t                     pPreEmphMode  = 0,
                         uint8_t                     pPreEmphStr   = 0);

    /*!
     * \brief Configure lpGBT High Speed Link Tx and Rx polarity
     * \param pChip       : pointer to Chip object
     * \param pTxPolarity : Tx polarity
     * \param pRxPolarity : Rx polarity
     */
    void ConfigureHighSpeedPolarity(Ph2_HwDescription::Chip* pChip, uint8_t pOutPolarity = 1, uint8_t pInPolarity = 0);

    /*!
     * \brief Configure lpGBT Data Player pattern
     * \param pChip    : pointer to Chip object
     * \param pPattern : Data player pattern
     */
    void ConfigureDPPattern(Ph2_HwDescription::Chip* pChip, uint32_t pPattern);

    /*!
     * \brief Configure lpGBT Rx Pseudo-Random Binary Sequence
     * \param pChip     : pointer to Chip object
     * \param pGroups   : Rx Groups vector
     * \param pChannels : Rx Channels vector
     * \param pEnable   : Enable Pseudo-Random Binary Sequence
     */
    void ConfigureRxPRBS(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels, bool pEnable = false);

    /*!
     * \brief Configure lpGBT Rx Groups data source
     * \param pChip   : pointer to Chip object
     * \param pGroups : Rx Groups vector
     * \param pSource : Rx data source
     */
    void ConfigureRxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource = 0);

    /*!
     * \brief Configure lpGBT Tx Groups data source
     * \param pChip   : pointer to Chip object
     * \param pGroups : Tx Groups vector
     * \param pSource : Tx data source
     */
    void ConfigureTxSource(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, uint8_t pSource = 0);

    /*!
     * \brief Configure lpGBT Rx channels phase
     * \param pChip    : pointer to Chip object
     * \param pGroup   : Rx Groups vector
     * \param pChannel : Rx Channels vector
     * \param pPhase   : Rx phase selection
     */
    void ConfigureRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel, uint8_t pPhase = 0);

    /*!
     * \brief Configure lpGBT Phase Shifter
     * \param pChip     : pointer to Chip object
     * \param pClocks   : Clocks vector
     * \param pFreq     : Frequency
     * \param pDriveStr : Driving strength
     * \param pEnFTune  : Enable fine deskewing
     * \param pDelay    : Clock delay
     */
    void ConfigurePhShifter(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pClocks, uint8_t pFreq = 0, uint8_t pDriveStr = 0, uint8_t pEnFTune = 0, uint16_t pDelay = 0);

    // ####################################
    // # LpGBT specific routine functions #
    // ####################################

    /*!
     * \brief lpGBT Rx Groups(Channels) phase training
     * \param pChip   : pointer to Chip object
     * \param pGroups : Rx Groups vector
     */
    void PhaseTrainRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups);

    /*!
     * \brief lpGBT Rx Groups(Channels) phase alignment
     * \param pChip     : pointer to Chip object
     * \param pGroups   : Rx Groups vector
     * \param pChannels : Rx Channels vector
     */
    void PhaseAlignRx(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pGroups, const std::vector<uint8_t>& pChannels);

    // LpGBT block status functions

    /*!
     * \brief Print out lpGBT chip mode (data rate, FEC, transmission mode)
     * \param pChip : pointer to Chip object
     */
    void PrintChipMode(Ph2_HwDescription::Chip* pChip);

    /*!
     * \brief Get lpGBT Rx Group Delay-Locked-Loop state machine
     * \param pChip  : pointer to Chip object
     * \param pGroup : Rx Groups vector
     */
    uint8_t GetRxDllStatus(Ph2_HwDescription::Chip* pChip, uint8_t pGroup);

    /*!
     * \brief Get lpGBT Rx Channel phase
     * \param pChip    : pointer to Chip object
     * \param pGroup   : Rx Groups vector
     * \param pChannel : Rx Channels vector
     */
    uint8_t GetRxPhase(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, uint8_t pChannel);

    /*!
     * \brief Get lpGBT Rx locking status
     * \param pChip  : pointer to Chip object
     * \param pGroup : Rx Groups vector
     */
    bool IsRxLocked(Ph2_HwDescription::Chip* pChip, uint8_t pGroup, const std::vector<uint8_t>& pChannels);

    /*!
     * \brief Get lpGBT Power Up State Machine status
     * \param pChip : pointer to Chip object
     */
    bool IslpGBTReady(Ph2_HwDescription::Chip* pChip);

    // ##############################################
    // # LpGBT I2C Masters functions (Slow Control) #
    // ##############################################
    /*!
     * \brief Reset I2C Masters
     * \param pChip    : pointer to Chip object
     * \param pMasters : I2C Masters vector
     */
    void ResetI2C(Ph2_HwDescription::Chip* pChip, const std::vector<uint8_t>& pMasters);

    /*!
     * \brief Configure lpGBT I2C Master
     * \param pChip         : pointer to Chip object
     * \param pMaster       : I2C Master
     * \param pNBytes       : Number of bytes
     * \param pSCLDriveMode : SCL drive strength
     */
    void ConfigureI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pFreq = 0, uint8_t pNBytes = 1, uint8_t pSCLDriveMode = 0);

    /*!
     * \brief I2C Write transaction using the lpGBT I2C Master
     * \param pChip         : pointer to Chip object
     * \param pMaster       : I2C Master
     * \param pSlaveAddress : Slave address
     * \param pData         : Data
     * \param pNBytes       : Number of bytes
     */
    bool WriteI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint32_t pData, uint8_t pNBytes = 1);

    /*!
     * \brief I2C Read transaction using the lpGBT I2C Master
     * \param pChip         : pointer to Chip object
     * \param pMaster       : I2C Master
     * \param pSlaveAddress : Slave address
     * \param pNBytes       : Number of bytes
     */
    uint32_t ReadI2C(Ph2_HwDescription::Chip* pChip, uint8_t pMaster, uint8_t pSlaveAddress, uint8_t pNBytes = 1);

    /*!
     * \brief Get lpGBT I2C Master status
     * \param pChip   : pointer to Chip object
     * \param pMaster : I2C Master
     */
    uint8_t GetI2CStatus(Ph2_HwDescription::Chip* pChip, uint8_t pMaster);

    // ###########################
    // # LpGBT ADC-DAC functions #
    // ###########################
    void ConfigureADC(Ph2_HwDescription::Chip* pChip, uint8_t pGainSelect = 0, uint8_t pADCCoreDiffEnable = 0);
    /*!
     * \brief Read lpGBT ADC
     * \param pChip     : pointer to Chip object
     * \param pADCInput : ADC input
     */
    uint16_t ReadADC(Ph2_HwDescription::Chip* pChip, const std::string& pADCInput);

    /*!
     * \brief Read lpGBT differential ADC
     * \param pChip      : pointer to Chip object
     * \param pADCInputP : ADC input positive
     * \param pADCInputN : ADC input negative
     */
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
    bool     cicWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pRegisterAddress, uint8_t pRegisterValue, bool pReadBack=true);
    uint32_t cicRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pRegisterAddress);
    // ssa read/write 
    bool     ssaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pRegisterAddress, uint8_t pRegisterValue, bool pReadBack=true);
    uint32_t ssaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pRegisterAddress);
    // mpa read/write
    bool     mpaWrite(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pRegisterAddress, uint8_t pRegisterValue, bool pReadBack=true);
    uint32_t mpaRead(Ph2_HwDescription::Chip* pChip, uint8_t pFeId, uint8_t pChipId, uint8_t pRegisterAddress);
    bool i2cWrite(Ph2_HwDescription::Chip* pChip, const std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pReadBack);

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
