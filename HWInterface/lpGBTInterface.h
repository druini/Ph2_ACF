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
  using ParameterVect = std::vector<std::pair<std::string, uint8_t>>;

  class lpGBTInterface : public ChipInterface
  {
  public:
    lpGBTInterface  (const BeBoardFWMap& pBoardMap);
    ~lpGBTInterface ();


    bool ConfigureChip   (Ph2_HwDescription::Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override
    {
      LOG (INFO) << GREEN << "Configuring: " << BOLDYELLOW << "lpGBT" << RESET;
      return false;
    }
    bool WriteChipReg    (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop = true) override { return false; }
    uint16_t ReadChipReg (Ph2_HwDescription::Chip* pChip, const std::string& pRegNode ) override                                         { return 0;     }


    // lpGBT Calibration Data Configuration
    void lpgbtConfigureCalibData (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters);

    // lpGBT Clock Generator Block configuration
    void lpgbtConfigureClkGenBlock (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters);

    // lpGBT CHIP Config configuration
    void lpgbtConfigureChipConfig (Ph2_HwDescription::lpGBT* plpGBT, const ParameterVect& pParameters);

    // lpGBT Line Driver configuration
    void lpgbtConfigureLineDriver (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters);

    // lpGBT Power Good configuration
    void lpgbtConfigurePowerGood (Ph2_HwDescription::lpGBT* plpGBT, const ParameterVect& pParameters);

    // lpGBT E-Links Rx, Tx, Clocks configuration
    void lpgbtConfigureClocks (Ph2_HwDescription::lpGBT* plpGBT, std::vector<uint8_t>& pClocks, const std::string& pRegister, const ParameterVect& pParameters);
    void lpgbtConfigureTx     (Ph2_HwDescription::lpGBT* plpGBT, std::vector<uint8_t>& pGroups, std::vector<uint8_t>& pChannels, const std::string& pRegister, const ParameterVect& pParameters);
    void lpgbtConfigureRx     (Ph2_HwDescription::lpGBT* plpGBT, std::vector<uint8_t>& pGroups, std::vector<uint8_t>& pChannels, const std::string& pRegister, const ParameterVect& pParameters);

    // lpGBT Power Up State Machine configuration
    void lpgbtConfigurePowerUpSM (Ph2_HwDescription::lpGBT* plpGBT, const ParameterVect& pParameters);

    // lpGBT Testing configuration
    void lpgbtConfigureTesting (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters);

    // lpGBT Debug configuration
    void lpgbtConfigureDebug (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters);

    // lpGBT ePort Rx read-only registers
    uint8_t lpgbtGetRx (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister);

    // lpGBT BERT Tester read-only registers
    uint8_t lpgbtGetBERTTester (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister);

    // lpGBT Power Up State Machine read-only registers
    uint8_t lpgbtGetPowerUpSM (Ph2_HwDescription::lpGBT* plpGBT, const std::string& pRegister);

    // GBT-SCA - enable I2C master interfaces, GPIO, ADC
    uint8_t scaEnable        (Ph2_HwDescription::lpGBT* plpGBT, uint16_t cI2Cmaster = 0x00);
    void    scaConfigure     (Ph2_HwDescription::lpGBT* plpGBT);
    bool    scaSetGPIO       (Ph2_HwDescription::lpGBT* plpGBT, uint8_t cChannel , uint8_t cLevel);
    void    scaConfigureGPIO (Ph2_HwDescription::lpGBT* plpGBT);

    // Multi-register I2C Write
    void i2cWrite (Ph2_HwDescription::lpGBT* plpGBT, const std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies);


  private:
    // lpGBT External Registers (EC) (read/write/reset)
    void     ecReset (Ph2_HwDescription::lpGBT* plpGBT);
    uint32_t ecWrite (Ph2_HwDescription::lpGBT* plpGBT, uint16_t pI2Cmaster, uint32_t pCommand , uint32_t pData = 0x00);
    uint32_t ecWrite (Ph2_HwDescription::lpGBT* plpGBT, uint16_t pI2Cmaster, const std::vector<std::pair<uint32_t,uint32_t>>& pCommands);
    uint32_t ecRead  (Ph2_HwDescription::lpGBT* plpGBT, uint16_t pI2Cmaster, uint32_t pCommand , uint32_t pData = 0x00);

    // lpGBT Internal Registers (IC) (read/write/reset)
    void     icReset (Ph2_HwDescription::lpGBT* plpGBT);
    void     icWrite (Ph2_HwDescription::lpGBT* plpGBT, uint32_t pAddress, uint32_t pData);
    uint32_t icRead  (Ph2_HwDescription::lpGBT* plpGBT, uint32_t pAddress, uint32_t pNwords);

    // General Slow Control (config/read/write)
    uint8_t  configI2C (Ph2_HwDescription::lpGBT* plpGBT, uint16_t pMaster, const ParameterVect& pParameters);
    uint32_t readI2C   (Ph2_HwDescription::lpGBT* plpGBT, uint16_t pMaster, uint8_t pSlave , uint8_t pNBytes);
    uint8_t  writeI2C  (Ph2_HwDescription::lpGBT* plpGBT, uint16_t pMaster, uint8_t pSlave , uint32_t pData, uint8_t pNBytes);


  protected:
    const uint8_t flpGBTAddress = 0x01;
  };
}

#endif
