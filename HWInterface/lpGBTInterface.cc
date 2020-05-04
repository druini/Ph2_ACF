/*!
  \file                  lpGBTInterface.cc
  \brief                 The implementation follows the skeleton of the register map section of the lpGBT Manual
  \author                Younes Otarid
  \version               1.0
  \date                  03/03/20
  Support:               email to younes.otarid@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "lpGBTInterface.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
  lpGBTInterface::lpGBTInterface(const BeBoardFWMap& pBoardMap) : ChipInterface(pBoardMap)
  {}

  lpGBTInterface::~lpGBTInterface() {}

  bool lpGBTInterface::ConfigureChip (Ph2_HwDescription::Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    std::string cRegister;
    ParameterVect cParameters;
    lpGBT* plpGBT = static_cast<lpGBT*>(pChip);

    // Configure Calibration Data registers
    cRegister.assign("EPRXLOCKFILTER");
    cParameters = {{"LockThreshold", 5}, {"ReLockThreshold", 5}};
    lpGBTInterface::lpgbtConfigureCalibData(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGConfig0");
    cParameters = {{"CalibEndOfCount", 12}, {"BiasGenConfig", 8}};
    lpGBTInterface::lpgbtConfigureCalibData(plpGBT, cRegister, cParameters);
    //
    cRegister.assign("CLKGConfig1");
    cParameters = {{"CtrlOverrEnable", 0}, {"DisFrameAlignLockCtrl", 0}, {"CDRRes", 1}, {"VcoRailMode", 0}, {"VcoDAC", 4}};
    lpGBTInterface::lpgbtConfigureCalibData(plpGBT, cRegister, cParameters);

    // Configure Clock Generator Block registers
    cRegister.assign("CLKGPllRes");
    cParameters = {{"ResWhenLocked", 4}, {"Res", 4}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGPLLIntCur");
    cParameters = {{"CurWhenLocked", 5}, {"Cur", 5}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGPLLPropCur");
    cParameters = {{"CurWhenLocked", 5}, {"Cur", 5}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGCDRPropCur");
    cParameters = {{"CurWhenLocked", 5}, {"Cur", 5}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGCDRIntCur");
    cParameters = {{"CurWhenLocked", 5}, {"Cur", 5}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGCDRFFPropCur");
    cParameters = {{"CurWhenLocked", 5}, {"Cur", 5}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGFLLIntCur");
    cParameters = {{"CurWhenLocked", 0}, {"Cur", 15}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGFFCAP");
    cParameters = {{"ConnectCDR", 0}, {"OverrEnable", 0}, {"FFCapWhenLocked", 0}, {"FFCap", 0}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGCntOverride");
    cParameters = {{"OverrVc", 0}, {"RefClkSel", 0}, {"EnablePLL", 0}, {"EnableFD", 0}, {"EnableCDR", 0}, {"DisDataCntrRef", 0}, {"DisDESvbiasGen", 0}, {"ConnectPLL", 0}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGOverrideCapBank");
    cParameters = {{"CapBankSelect", 0}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGWaitTime");
    cParameters = {{"WaitCDRTime", 8}, {"WaitPLLTime", 8}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGLFConfig0");
    cParameters = {{"LockFilterEnable", 1}, {"CapBankSelect", 0}, {"LockThrCounter", 9}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("CLKGLFConfig1");
    cParameters = {{"ReLockThrCounter", 9}, {"UnLockThrCounter", 9}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    // Configure Debug register
    cRegister.assign("DataPath");
    cParameters = {{"BypassDeInterlevear", 0}, {"BypassFECDecoder", 0}, {"BypassDeScrambler", 0}, {"FECErrCntEna", 0}, {"BypassInterleaver", 0}, {"BypassScrambler", 0}, {"BypassFECCoder", 0}};
    lpGBTInterface::lpgbtConfigureDebug(plpGBT, cRegister, cParameters);

    // Configure ChipConfig register
    cParameters = {{"DataOutInvert", 0}, {"DataInInvert", 1}, {"ChipAddressBar", 0}};
    lpGBTInterface::lpgbtConfigureChipConfig(plpGBT, cParameters);

    // Configure Clock Generator Block registers
    cRegister.assign("EPRXDllConfig");
    cParameters = {{"Current", 1}, {"ConfirmCount", 1}, {"FSMClkAlwaysOn", 0}, {"CoarseLockDetection", 1}, {"EnableReInit", 0}, {"DataGatingEnable", 0}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("PSDllConfig") ;
    cParameters = {{"UnLockThreshold", 5}, {"ConfirmCount", 1}, {"CurrentSel", 1}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    // Configure Line Driver register
    cRegister.assign("LDConfigH") ;
    cParameters = {{"EmphasisEnable", 0}, {"ModulationCurrent", 32}};
    lpGBTInterface::lpgbtConfigureLineDriver(plpGBT, cRegister, cParameters);

    // Configure Power Good register
    cParameters = {{"PGEnable", 1}, {"PGLevel", 5}, {"PGDelay", 12}};
    lpGBTInterface::lpgbtConfigurePowerGood(plpGBT, cParameters);

    // Configure Clock Generator Block registers
    cRegister.assign("FAMaxHeaderFoundCount");
    cParameters = {{"FoundCount", 10}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("FAMaxHeaderFoundCountAfterNF");
    cParameters = {{"FoundCountAfterNF", 10}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("FAMaxHeaderNotFoundCount");
    cParameters = {{"NotFoundCount", 10}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    cRegister.assign("FAFAMaxSkipCycleCountAfterNF");
    cParameters = {{"SkipCycleCountAfterNF", 10}};
    lpGBTInterface::lpgbtConfigureClkGenBlock(plpGBT, cRegister, cParameters);

    // Configure PowerUp State Machine register
    cParameters = {{"dllConfigDone", 1}, {"pllConfigDone", 2}};
    lpGBTInterface::lpgbtConfigurePowerUpSM(plpGBT, cParameters);


    cRegister.assign("PUSMStatus");
    uint8_t cStatus = lpGBTInterface::lpgbtGetPowerUpSM(plpGBT, cRegister);
    if (cStatus != 18)
      {
        LOG (ERROR) << BOLDRED << "lpGBT status is not ready: " << +cStatus << RESET;
        exit(EXIT_FAILURE);
      }


    /*-------------------------------------------------------------------------*/
    /* Configuration of lpGBT down-links                                       */
    /*-------------------------------------------------------------------------*/
    std::vector<uint8_t> fDLGroups   = {0};
    std::vector<uint8_t> fDLChannels = {0};
    uint8_t pCurrent = 0;
    uint8_t pPreEmphasis = 0;
    bool pInvert = false;

    cRegister.assign("EPTXDataRate");
    cParameters = {{"DataRate", 2}};
    lpGBTInterface::lpgbtConfigureTx(plpGBT, fDLGroups, fDLChannels, cRegister, cParameters);

    cRegister.assign("EPTXEnable");
    lpGBTInterface::lpgbtConfigureTx(plpGBT, fDLGroups, fDLChannels, cRegister, cParameters);

    cRegister.assign("EPTXChnCntr");
    cParameters = {{"PreEmphasisMode", 3}, {"DriveStrength", pCurrent}};
    if (pPreEmphasis > 0)
      cParameters.push_back(std::make_pair("PreEmphasisStrength", pPreEmphasis));
    lpGBTInterface::lpgbtConfigureTx(plpGBT, fDLGroups, fDLChannels, cRegister ,cParameters);

    cRegister.assign("EPTX_ChnCntr");
    if (pInvert == true)
      {
        cParameters.push_back(std::make_pair("PreEmphasisStrength", 1));
      }
    else
      {
        cParameters.push_back(std::make_pair("PreEmphasisStrength", 0));
        cParameters.push_back(std::make_pair("PreEmphasisWidth", 0));
      }
    lpGBTInterface::lpgbtConfigureTx(plpGBT, fDLGroups, fDLChannels, cRegister, cParameters);


    /*-------------------------------------------------------------------------*/
    /* Configuration of lpGBT up-links                                         */
    /*-------------------------------------------------------------------------*/
    std::vector<uint8_t> fULGroups   = {0};
    std::vector<uint8_t> fULChannels = {0};
    uint8_t pPhaseMode = 0;
    uint8_t pEqual = 0;
    uint8_t pPhase = 0;
    bool pEnableTerm = false;
    bool pEnableBias = false;
    pInvert = false;

    cRegister.assign("EPRXControl");
    cParameters = {{"DataRate", 3}, {"TrackMode", pPhaseMode}};
    lpGBTInterface::lpgbtConfigureRx(plpGBT, fULGroups, fULChannels, cRegister, cParameters);

    cRegister.assign("EPRXChnCntr");
    cParameters = {{"PhaseSelect", pPhase}, {"Invert", uint8_t(pInvert)}, {"AcBias", uint8_t(pEnableBias)}, {"Term", uint8_t(pEnableTerm)}, {"Eq", pEqual}};
    lpGBTInterface::lpgbtConfigureRx(plpGBT, fULGroups, fULChannels, cRegister, cParameters);

    cRegister.assign("EPRXEqControl");
    cParameters = {{"Eq", pEqual}};
    lpGBTInterface::lpgbtConfigureRx(plpGBT, fULGroups, fULChannels, cRegister, cParameters);


    return cStatus;
  }


  /*-------------------------------------------------------------------------*/
  /* External Control (EC) ePort read/write/reset methods                    */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::ecReset(lpGBT* plpGBT)
  {
    this->setBoard(plpGBT->getBeBoardId());

    fBoardFW->WriteStackReg({
        {"fc7_daq_ctrl.optical_block.sca.start", 0x00},
        {"fc7_daq_cnfg.optical_block.sca",       0x00},
        {"fc7_daq_cnfg.optical_block.gbtx",      0x00}
      });
  }

  uint32_t lpGBTInterface::ecWrite(lpGBT* plpGBT, uint16_t pI2Cmaster, uint32_t pCommand, uint32_t pData)
  {
    this->setBoard(plpGBT->getBeBoardId());

    fBoardFW->WriteStackReg({
        {"fc7_daq_ctrl.optical_block.sca.start",   0x00},
        {"fc7_daq_cnfg.optical_block.sca",         0x00},
        {"fc7_daq_cnfg.optical_block.gbtx",        0x00},
        {"fc7_daq_cnfg.optical_block.sca.address", 0x01},
        {"fc7_daq_cnfg.optical_block.sca.id",      0x01},
        {"fc7_daq_cnfg.optical_block.sca.channel", pI2Cmaster},
        {"fc7_daq_cnfg.optical_block.sca.cmd",     pCommand},
        {"fc7_daq_cnfg.optical_block.sca.data",    pData}
      });
    LOG (DEBUG) << BOLDBLUE << "lpGBT EC write to I2C master " << +pI2Cmaster <<  " - data field : " << +pData << " [ command 0x" << std::hex << pCommand << std::dec << "]." << RESET;
    fBoardFW->WriteReg("fc7_daq_ctrl.optical_block.sca.start", 0x1);

    return fBoardFW->ReadReg("fc7_daq_stat.optical_block.sca.error");
  }

  uint32_t lpGBTInterface::ecWrite(lpGBT* plpGBT, uint16_t pI2Cmaster, const std::vector<std::pair<uint32_t,uint32_t>>& pCommands)
  {
    this->setBoard(plpGBT->getBeBoardId());

    for (const auto& pCommand : pCommands)
      {
        fBoardFW->WriteStackReg({
            {"fc7_daq_ctrl.optical_block.sca.start",   0x00},
            {"fc7_daq_cnfg.optical_block.sca",         0x00},
            {"fc7_daq_cnfg.optical_block.gbtx",        0x00},
            {"fc7_daq_cnfg.optical_block.sca.address", 0x01},
            {"fc7_daq_cnfg.optical_block.sca.id",      0x01},
            {"fc7_daq_cnfg.optical_block.sca.channel", pI2Cmaster},
            {"fc7_daq_cnfg.optical_block.sca.cmd",     pCommand.first},
            {"fc7_daq_cnfg.optical_block.sca.data",    pCommand.second}
          });
        LOG (DEBUG) << BOLDBLUE << "lpGBT EC write to I2C master " << +pI2Cmaster <<  " - data field : " << +pCommand.second << " [ command 0x" << std::hex << pCommand.first << std::dec << "]." << RESET;
      }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    fBoardFW->WriteReg("fc7_daq_ctrl.optical_block.sca.start", 0x1);
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    return fBoardFW->ReadReg("fc7_daq_stat.optical_block.sca.error");
  }

  uint32_t lpGBTInterface::ecRead(lpGBT* plpGBT, uint16_t pI2Cmaster, uint32_t pCommand , uint32_t pData)
  {
    this->setBoard(plpGBT->getBeBoardId());

    fBoardFW->WriteStackReg({
        {"fc7_daq_ctrl.optical_block.sca.start",   0x00},
        {"fc7_daq_cnfg.optical_block.sca",         0x00},
        {"fc7_daq_cnfg.optical_block.gbtx",        0x00},
        {"fc7_daq_cnfg.optical_block.sca.address", 0x01},
        {"fc7_daq_cnfg.optical_block.sca.id",      0x02},
        {"fc7_daq_cnfg.optical_block.sca.channel", pI2Cmaster},
        {"fc7_daq_cnfg.optical_block.sca.cmd",     pCommand},
        {"fc7_daq_cnfg.optical_block.sca.data",    pData}
      });
    fBoardFW->WriteReg("fc7_daq_ctrl.optical_block.sca.start", 0x1);

    uint32_t cRead = fBoardFW->ReadReg("fc7_daq_stat.optical_block.sca.data");
    LOG (DEBUG) << BOLDBLUE << "lpGBT EC read returns : " << std::bitset<32>(cRead) << RESET;
    return cRead;
  }


  /*-------------------------------------------------------------------------*/
  /* Internal Control (IC) ePort read/write/reset methods                    */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::icReset(lpGBT* plpGBT)
  {
    this->setBoard(plpGBT->getBeBoardId());

    fBoardFW->WriteStackReg({
        {"fc7_daq_ctrl.optical_block.ic",   0x00},
        {"fc7_daq_cnfg.optical_block.ic",   0x00},
        {"fc7_daq_cnfg.optical_block.gbtx", 0x00}
      });
  }

  void lpGBTInterface::icWrite(lpGBT* plpGBT, uint32_t pAddress, uint32_t pData)
  {
    this->setBoard(plpGBT->getBeBoardId());

    // Config
    fBoardFW->WriteStackReg({
        {"fc7_daq_cnfg.optical_block.gbtx.address", flpGBTAddress},
        {"fc7_daq_cnfg.optical_block.gbtx.data",    pData},
        {"fc7_daq_cnfg.optical_block.ic.register",  pAddress}
      });

    // Perform operation
    fBoardFW->WriteStackReg({
        {"fc7_daq_ctrl.optical_block.ic.write",       0x01},
        {"fc7_daq_ctrl.optical_block.ic.write",       0x00},
        {"fc7_daq_ctrl.optical_block.ic.start_write", 0x01},
        {"fc7_daq_ctrl.optical_block.ic.start_write", 0x00}
      });

    lpGBTInterface::icReset(plpGBT);
  }

  uint32_t lpGBTInterface::icRead(lpGBT* plpGBT, uint32_t pAddress, uint32_t pNwords)
  {
    this->setBoard(plpGBT->getBeBoardId());

    // Config
    fBoardFW->WriteStackReg({
        {"fc7_daq_cnfg.optical_block.gbtx.address", flpGBTAddress},
        {"fc7_daq_cnfg.optical_block.ic.register",  pAddress},
        {"fc7_daq_cnfg.optical_block.ic.nwords",    pNwords}
      });

    // Perform operation
    fBoardFW->WriteStackReg({
        {"fc7_daq_ctrl.optical_block.ic.start_read", 0x01},
        {"fc7_daq_ctrl.optical_block.ic.start_read", 0x00},
        {"fc7_daq_ctrl.optical_block.ic.read",       0x01},
        {"fc7_daq_ctrl.optical_block.ic.read",       0x00}
      });

    uint32_t cRead = fBoardFW->ReadReg("fc7_daq_stat.optical_block.ic.data");
    lpGBTInterface::icReset(plpGBT);
    return cRead;
  }


  // @TMP@ : to be checked
  /*-------------------------------------------------------------------------*/
  /* lpGBT SCA configuration                                                 */
  /*-------------------------------------------------------------------------*/
  uint8_t lpGBTInterface::scaEnable(lpGBT* plpGBT, uint16_t cI2Cmaster)
  {
    uint32_t cErrorCode = lpGBTInterface::ecWrite(plpGBT, cI2Cmaster, 0x02 , 0x04000000);
    if (cErrorCode != 0)
      {
        LOG (INFO) << BOLDRED << "SCA Error code : " << +cErrorCode << RESET;
        return 0;
      }

    cErrorCode = lpGBTInterface::ecWrite(plpGBT, cI2Cmaster, 0x04, 0x00000000);
    if (cErrorCode != 0)
      {
        LOG (INFO) << BOLDRED << "SCA Error code : " << +cErrorCode << RESET;
        return 0;
      }

    cErrorCode = lpGBTInterface::ecWrite(plpGBT, cI2Cmaster, 0x06, 0x16000000);
    if (cErrorCode != 0)
      {
        LOG (INFO) << BOLDRED << "SCA Error code : " << +cErrorCode << RESET;
        return 0;
      }

    return (cErrorCode == 0);
  }

  void lpGBTInterface::scaConfigure(lpGBT* plpGBT)
  {
    lpGBTInterface::icWrite(plpGBT, 231, 0x00dd);
    lpGBTInterface::icWrite(plpGBT, 232, 0x000d);
    lpGBTInterface::icWrite(plpGBT, 233, 0x0070);

    for (uint16_t cRegister = 237; cRegister < 246; cRegister += 4)
      lpGBTInterface::icWrite(plpGBT, cRegister, 0x0000);

    lpGBTInterface::icWrite(plpGBT, 248, 0x0007);
    lpGBTInterface::icWrite(plpGBT, 251, 0x0000);
    lpGBTInterface::icWrite(plpGBT, 254, 0x0070);
    lpGBTInterface::icWrite(plpGBT, 257, 0x0000);
    lpGBTInterface::icWrite(plpGBT, 273, 0x0020);
  }

  bool lpGBTInterface::scaSetGPIO(lpGBT* plpGBT, uint8_t cChannel, uint8_t cLevel)
  {
    uint32_t cMask = (1 << cChannel);
    cMask = (~cMask & 0xFFFFFFFF);
    uint8_t cSCAchannel = 0x02;

    if (cChannel < 31)
      {
        uint32_t cValue    = lpGBTInterface::ecRead(plpGBT, cSCAchannel , 0x11);
        uint8_t cErrorCode = lpGBTInterface::ecWrite(plpGBT, cSCAchannel, 0x10, (cLevel << cChannel) | (cValue & cMask));
        return (cErrorCode == 0);
      }

    return false;
  }

  void lpGBTInterface::scaConfigureGPIO(lpGBT* plpGBT)
  {
    uint32_t cMask = (1 << 31) | (1 << 30) | (1 << 3) | (1 << 2);
    cMask = (~cMask & 0xFFFFFFFF);
    uint8_t  cMaster = 0x02;
    uint32_t cData = (0 << 31) | (1 << 30) | (0 << 3) | (1 << 2);
    uint32_t cErrorCode = lpGBTInterface::ecWrite(plpGBT, cMaster, 0x10 , 0x40000004);
    if (cErrorCode != 0)
      {
        LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << RESET;
        exit(EXIT_FAILURE);
      }

    uint32_t cValue = lpGBTInterface::ecRead(plpGBT, cMaster, 0x21);
    cData = ((1 << 31) | (1 << 30) | (1 << 3) | (1 << 2));
    lpGBTInterface::ecWrite(plpGBT, cMaster, 0x20, cData | (cValue&cMask));

    cValue = lpGBTInterface::ecRead(plpGBT, cMaster, 0x31);
    cData = ((0 << 31) | (0 << 30) | (0 << 3) | (0 << 2));
    lpGBTInterface::ecWrite(plpGBT, cMaster, 0x30, cData | (cValue&cMask));

    cValue = lpGBTInterface::ecRead(plpGBT, cMaster, 0x11);
    cData = (1 << 31) | (0 << 30) | (1 << 3) | (0 << 2);
    lpGBTInterface::ecWrite(plpGBT, cMaster, 0x10, cData | (cValue&cMask));
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Calibration Data Configuration                                    */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigureCalibData(lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "EPRXLOCKFILTER")
      {
        uint32_t cValueEPRXLOCKFILTER = lpGBTInterface::icRead(plpGBT, 0x01f, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "LockThreshold")
              cValueEPRXLOCKFILTER = (cValueEPRXLOCKFILTER & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "ReLockThreshold")
              cValueEPRXLOCKFILTER = (cValueEPRXLOCKFILTER & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x01f, cValueEPRXLOCKFILTER);
      }
    else if (pRegister == "CLKGConfig0")
      {
        uint32_t cValueCLKGConfig0 = lpGBTInterface::icRead(plpGBT, 0x020, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CalibEndOfCount")
              cValueCLKGConfig0 = (cValueCLKGConfig0 & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "BiasGenConfig")
              cValueCLKGConfig0 = (cValueCLKGConfig0 & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x020, cValueCLKGConfig0);
      }
    else if (pRegister == "CLKGConfig1")
      {
        uint32_t cValueCLKGConfig1 = lpGBTInterface::icRead(plpGBT, 0x021, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CtrlOverrEnable")
              cValueCLKGConfig1 = (cValueCLKGConfig1 & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "DisFrameAlignLockCtrl")
              cValueCLKGConfig1 = (cValueCLKGConfig1 & ~0x40) | (cParameter.second << 6);
            else if (cParameter.first == "CDRRes")
              cValueCLKGConfig1 = (cValueCLKGConfig1 & ~0x20) | (cParameter.second << 5);
            else if (cParameter.first == "VcoRailMode")
              cValueCLKGConfig1 = (cValueCLKGConfig1 & ~0x10) | (cParameter.second << 4);
            else if (cParameter.first == "VcoDAC")
              cValueCLKGConfig1 = (cValueCLKGConfig1 & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x021, cValueCLKGConfig1);
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Clock Generator Block Configuration                               */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigureClkGenBlock(lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "CLKGPllRes")
      {
        uint32_t cValueCLKGPllRes = lpGBTInterface::icRead(plpGBT, 0x022, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "ResWhenLocked" )
              cValueCLKGPllRes = (cValueCLKGPllRes & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Res" )
              cValueCLKGPllRes = (cValueCLKGPllRes & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x022, cValueCLKGPllRes);
      }
    else if (pRegister == "CLKGPLLIntCur")
      {
        uint32_t cValueCLKGPLLIntCur = lpGBTInterface::icRead(plpGBT, 0x023, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CurWhenLocked" )
              cValueCLKGPLLIntCur = (cValueCLKGPLLIntCur & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Cur" )
              cValueCLKGPLLIntCur = (cValueCLKGPLLIntCur & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x023, cValueCLKGPLLIntCur);
      }
    else if (pRegister == "CLKGPLLPropCur")
      {
        uint32_t cValueCLKGPLLPropCur = lpGBTInterface::icRead(plpGBT, 0x024, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CurWhenLocked" )
              cValueCLKGPLLPropCur = (cValueCLKGPLLPropCur & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Cur" )
              cValueCLKGPLLPropCur = (cValueCLKGPLLPropCur & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x024, cValueCLKGPLLPropCur);
      }
    else if (pRegister == "CLKGCDRPropCur")
      {
        uint32_t cValueCLKGCDRPropCur = lpGBTInterface::icRead(plpGBT, 0x025, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CurWhenLocked" )
              cValueCLKGCDRPropCur = (cValueCLKGCDRPropCur & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Cur" )
              cValueCLKGCDRPropCur = (cValueCLKGCDRPropCur & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x025, cValueCLKGCDRPropCur);
      }
    else if (pRegister == "CLKGCDRIntCur")
      {
        uint32_t cValueCLKGCDRIntCur = lpGBTInterface::icRead(plpGBT, 0x026, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CurWhenLocked" )
              cValueCLKGCDRIntCur = (cValueCLKGCDRIntCur & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Cur" )
              cValueCLKGCDRIntCur = (cValueCLKGCDRIntCur & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x026, cValueCLKGCDRIntCur);
      }
    else if (pRegister == "CLKGCDRFFPropCur")
      {
        uint32_t cValueCLKGCDRFFPropCur = lpGBTInterface::icRead(plpGBT, 0x027, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CurWhenLocked" )
              cValueCLKGCDRFFPropCur = (cValueCLKGCDRFFPropCur & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Cur" )
              cValueCLKGCDRFFPropCur = (cValueCLKGCDRFFPropCur & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x027, cValueCLKGCDRFFPropCur);
      }
    else if (pRegister == "CLKGFLLIntCur")
      {
        uint32_t cValueCLKGFLLIntCur = lpGBTInterface::icRead(plpGBT, 0x028, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CurWhenLocked" )
              cValueCLKGFLLIntCur = (cValueCLKGFLLIntCur & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "Cur" )
              cValueCLKGFLLIntCur = (cValueCLKGFLLIntCur & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x028, cValueCLKGFLLIntCur);
      }
    else if (pRegister == "CLKGFFCAP")
      {
        uint32_t cValueCLKGFFCAP = lpGBTInterface::icRead(plpGBT, 0x029, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "ConnectCDR" )
              cValueCLKGFFCAP = (cValueCLKGFFCAP & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "OverrEnable" )
              cValueCLKGFFCAP = (cValueCLKGFFCAP & ~0x40) | (cParameter.second << 6);
            else if (cParameter.first == "FFCapWhenLocked" )
              cValueCLKGFFCAP = (cValueCLKGFFCAP & ~0x38) | (cParameter.second << 3);
            else if (cParameter.first == "FFCap" )
              cValueCLKGFFCAP = (cValueCLKGFFCAP & ~0x07) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x029, cValueCLKGFFCAP);
      }
    else if (pRegister == "CLKGCntOverride")
      {
        uint32_t cValueCLKGCntOverride = lpGBTInterface::icRead(plpGBT, 0x02a, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "OverrVc" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "RefClkSel" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x40) | (cParameter.second << 6);
            else if (cParameter.first == "EnablePLL" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x20) | (cParameter.second << 5);
            else if (cParameter.first == "EnableFD" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x10) | (cParameter.second << 4);
            else if (cParameter.first == "EnableCDR" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x08) | (cParameter.second << 3);
            else if (cParameter.first == "DisDataCntrRef" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x04) | (cParameter.second << 2);
            else if (cParameter.first == "DisDESvbiasGen" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x02) | (cParameter.second << 1);
            else if (cParameter.first == "ConnectPLL" )
              cValueCLKGCntOverride = (cValueCLKGCntOverride & ~0x01) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x02a, cValueCLKGCntOverride);
      }
    else if (pRegister == "CLKGOverrideCapBank")
      {
        uint32_t cValueCLKGOverrideCapBank = lpGBTInterface::icRead(plpGBT, 0x02b, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "CapBankSelect")
              cValueCLKGOverrideCapBank = (cValueCLKGOverrideCapBank & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x02b, cValueCLKGOverrideCapBank);
      }
    else if (pRegister == "CLKGWaitTime")
      {
        uint32_t cValueCLKGWaitTime = lpGBTInterface::icRead(plpGBT, 0x02c, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "WaitCDRTime" )
              cValueCLKGWaitTime = (cValueCLKGWaitTime & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "WaitPLLTime" )
              cValueCLKGWaitTime = (cValueCLKGWaitTime & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x02c, cValueCLKGWaitTime);
      }
    else if (pRegister == "CLKGLFConfig0")
      {
        uint32_t cValueCLKGLFConfig0 = lpGBTInterface::icRead(plpGBT, 0x02d, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "LockFilterEnable" )
              cValueCLKGLFConfig0 = (cValueCLKGLFConfig0 & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "CapBankSelect" )
              cValueCLKGLFConfig0 = (cValueCLKGLFConfig0 & ~0x10) | (cParameter.second << 4);
            else if (cParameter.first == "LockThrCounter" )
              cValueCLKGLFConfig0 = (cValueCLKGLFConfig0 & ~0x07) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x02d, cValueCLKGLFConfig0);
      }
    else if (pRegister == "CLKGLFConfig1")
      {
        uint32_t cValueCLKGLFConfig1 = lpGBTInterface::icRead(plpGBT, 0x02e, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "ReLockThrCounter" )
              cValueCLKGLFConfig1 = (cValueCLKGLFConfig1 & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "UnLockThrCounter" )
              cValueCLKGLFConfig1 = (cValueCLKGLFConfig1 & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x02e, cValueCLKGLFConfig1);
      }
    else if (pRegister == "FAMaxHeaderFoundCount" )
      {
        uint32_t cValueFAMaxHeaderFC = lpGBTInterface::icRead(plpGBT, 0x02f, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "FoundCount")
              cValueFAMaxHeaderFC = (cValueFAMaxHeaderFC & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x02f, cValueFAMaxHeaderFC);
      }
    else if (pRegister == "FAMaxHeaderFoundCountAfterNF")
      {
        uint32_t cValueFAMaxHeaderFCAF = lpGBTInterface::icRead(plpGBT, 0x030, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "FoundCountAfterNF")
              cValueFAMaxHeaderFCAF = (cValueFAMaxHeaderFCAF & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x030, cValueFAMaxHeaderFCAF);
      }
    else if (pRegister == "FAMaxHeaderNotFoundCount")
      {
        uint32_t cValueFAMaxHeaderNFC = lpGBTInterface::icRead(plpGBT, 0x031, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "NotFoundCount")
              cValueFAMaxHeaderNFC = (cValueFAMaxHeaderNFC & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x031, cValueFAMaxHeaderNFC);
      }
    else if (pRegister == "FAFAMaxSkipCycleCountAfterNF")
      {
        uint32_t cValueFAMaxSCCANF = lpGBTInterface::icRead(plpGBT, 0x032, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "SkipCycleCountAfterNF")
              cValueFAMaxSCCANF = (cValueFAMaxSCCANF & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x032, cValueFAMaxSCCANF);
      }
    else if (pRegister == "PSDllConfig")
      {
        uint32_t cValuePSDllConfig = lpGBTInterface::icRead(plpGBT, 0x033, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "UnLockThreshold")
              cValuePSDllConfig = (cValuePSDllConfig & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "ConfirmCount")
              cValuePSDllConfig = (cValuePSDllConfig & ~0x0C) | (cParameter.second << 2);
            else if (cParameter.first == "CurrentSel")
              cValuePSDllConfig = (cValuePSDllConfig & ~0x03) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x033, cValuePSDllConfig);
      }
    else if (pRegister == "EPRXDllConfig")
      {
        uint32_t cValueEPRXDllConfig = lpGBTInterface::icRead(plpGBT, 0x034, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "Current")
              cValueEPRXDllConfig = (cValueEPRXDllConfig & ~0xC0) | (cParameter.second << 6);
            else if (cParameter.first == "ConfirmCount")
              cValueEPRXDllConfig = (cValueEPRXDllConfig & ~0x30) | (cParameter.second << 4);
            else if (cParameter.first == "FSMClkAlwaysOn")
              cValueEPRXDllConfig = (cValueEPRXDllConfig & ~0x08) | (cParameter.second << 3);
            else if (cParameter.first == "CoarseLockDetect")
              cValueEPRXDllConfig = (cValueEPRXDllConfig & ~0x04) | (cParameter.second << 2);
            else if (cParameter.first == "dEnableReInit")
              cValueEPRXDllConfig = (cValueEPRXDllConfig & ~0x02) | (cParameter.second << 1);
            else if (cParameter.first == "DataGatingEnable")
              cValueEPRXDllConfig = (cValueEPRXDllConfig & ~0x01) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x034, cValueEPRXDllConfig);
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT CHIP Config Configuration                                         */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigureChipConfig(lpGBT* plpGBT, const ParameterVect& pParameters)
  {
    uint32_t cValueChipConfig = lpGBTInterface::icRead(plpGBT, 0x036, 1);
    for (const auto& cParameter : pParameters)
      {
        if (cParameter.first == "DataOutInvert")
          cValueChipConfig = (cValueChipConfig & ~0x80) | (cParameter.second << 7);
        else if (cParameter.first == "DataInInvert")
          cValueChipConfig = (cValueChipConfig & ~0x40) | (cParameter.second << 6);
        else if (cParameter.first == "ChipAddressBar")
          cValueChipConfig = (cValueChipConfig & ~0x7) | (cParameter.second << 0);
        else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
      }
    lpGBTInterface::icWrite(plpGBT, 0x036, cValueChipConfig);
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Line Driver Configuration                                         */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigureLineDriver(lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "LDConfigH")
      {
        uint32_t cValueLDConfigH = lpGBTInterface::icRead(plpGBT, 0x039, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "EmphasisEnable")
              cValueLDConfigH = (cValueLDConfigH & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "ModulationCurrent")
              cValueLDConfigH = (cValueLDConfigH & ~0x7F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x039, cValueLDConfigH);
      }
    else if (pRegister == "LDConfigL")
      {
        uint32_t cValueLDConfigL = lpGBTInterface::icRead(plpGBT, 0x03a, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "LDEmphasisShort")
              cValueLDConfigL = (cValueLDConfigL & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "LDEmphasisAmp")
              cValueLDConfigL = (cValueLDConfigL & ~0x7F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x03a, cValueLDConfigL);
      }
    else if (pRegister == "REFCLK")
      {
        uint32_t cValueREFCLK = lpGBTInterface::icRead(plpGBT, 0x03b, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "SetCM")
              cValueREFCLK = (cValueREFCLK & ~0x10) | (cParameter.second << 4);
            else if (cParameter.first == "ForceEnable")
              cValueREFCLK = (cValueREFCLK & ~0x4) | (cParameter.second << 2);
            else if (cParameter.first == "AcBias")
              cValueREFCLK = (cValueREFCLK & ~0x2) | (cParameter.second << 1);
            else if (cParameter.first == "Term")
              cValueREFCLK = (cValueREFCLK & ~0x1) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x03b, cValueREFCLK);
      }
    else if (pRegister == "SCCONFIG")
      {
        uint32_t cValueSCCONFIG = lpGBTInterface::icRead(plpGBT, 0x03c, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "ParityCheckDisable")
              cValueSCCONFIG = (cValueSCCONFIG & ~0x1) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x03c, cValueSCCONFIG);
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Power Good Configuration                                          */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigurePowerGood(lpGBT* plpGBT, const ParameterVect& pParameters)
  {
    uint32_t cValuePGConfig = lpGBTInterface::icRead(plpGBT, 0x03e, 1);
    for (const auto& cParameter : pParameters)
      {
        if (cParameter.first == "PGEnable")
          cValuePGConfig = (cValuePGConfig & ~0x80) | (cParameter.second << 7);
        else if (cParameter.first == "PGLevel")
          cValuePGConfig = (cValuePGConfig & ~0x70) | (cParameter.second << 4);
        else if (cParameter.first == "PGDelay")
          cValuePGConfig = (cValuePGConfig & ~0x0F) | (cParameter.second << 0);
        else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
      }
    lpGBTInterface::icWrite(plpGBT, 0x03e, cValuePGConfig);
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT E-Link Tx, Rx, Clocks Configuration                               */
  /*-------------------------------------------------------------------------*/
  // lpGBT ePort Clock Configurationx
  // pFrequency = 0 -- disable, 1 -- 40Mbps, 2 -- 80Mbps, 3 -- 160Mpbs, 4 -- 320Mpbs, 5 -- 640Mbps, 6 -- 1280Mpbs
  void lpGBTInterface::lpgbtConfigureClocks(lpGBT* plpGBT, std::vector<uint8_t>& pClocks, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "ClkChnCntrH")
      {
        for (const auto& cClock : pClocks)
          {
            uint32_t cValueClkChnCntrH = lpGBTInterface::icRead(plpGBT, 0x06c+(cClock * 2), 1);
            for (const auto& cParameter : pParameters)
              {
                if (cParameter.first == "Frequency")
                  cValueClkChnCntrH = (cValueClkChnCntrH & ~0x07) | (cParameter.second << 0);
                else if (cParameter.first == "DriveStrength")
                  cValueClkChnCntrH = (cValueClkChnCntrH & ~0x38) | (cParameter.second << 3);
                else if (cParameter.first == "Invert")
                  cValueClkChnCntrH = (cValueClkChnCntrH & ~0x40) | (cParameter.second << 6);
                else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
              }
            lpGBTInterface::icWrite(plpGBT, 0x06c+(cClock * 2), cValueClkChnCntrH);
            uint32_t cReadBack = lpGBTInterface::icRead(plpGBT, 0x06c+(cClock * 2), 1);
            LOG (DEBUG) << BOLDBLUE << "EPCLK " << +cClock << " ChnCntrH set to : " << std::bitset<8>(cReadBack) << RESET;
          }
      }
    else if (pRegister == "ClkChnCntrL")
      {
        for (const auto& cClock : pClocks)
          {
            uint32_t cValueClkChnCntrL = lpGBTInterface::icRead(plpGBT, 0x06d+(cClock * 2), 1);
            for (const auto& cParameter : pParameters)
              {
                if (cParameter.first == "PreEmphasisWidth")
                  cValueClkChnCntrL = (cValueClkChnCntrL & ~0x07) | (cParameter.second << 0);
                else if (cParameter.first == "PreEmphasisMode")
                  cValueClkChnCntrL = (cValueClkChnCntrL & ~0x18) | (cParameter.second << 3);
                else if (cParameter.first == "PreEmphasisStrength")
                  cValueClkChnCntrL = (cValueClkChnCntrL & ~0xE0) | (cParameter.second << 5);
                else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
              }
            lpGBTInterface::icWrite(plpGBT, 0x06d+(cClock * 2), cValueClkChnCntrL);
            uint32_t cReadBack = lpGBTInterface::icRead(plpGBT, 0x06d+(cClock * 2), 1);
            LOG (DEBUG) << BOLDBLUE << "EPCLK " << +cClock << " ChnCntrL set to : " << std::bitset<8>(cReadBack) << RESET;
          }
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }

  void lpGBTInterface::lpgbtConfigureTx(lpGBT* plpGBT, std::vector<uint8_t>& pGroups, std::vector<uint8_t>& pChannels, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "EPTXDataRate")
      {
        uint32_t cValueDataRate = lpGBTInterface::icRead(plpGBT, 0x0a7, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "DataRate")
              {
                for (const auto& cGroup : pGroups)
                  {
                    LOG (DEBUG) << BOLDBLUE << "Setting all output ePort Tx data rates for group " << +cGroup << " to 0x" << std::hex << +cParameter.second << std::dec << RESET;
                    cValueDataRate = (cValueDataRate & ~(0x03 << 2*cGroup)) | (cParameter.second << 2*cGroup);
                  }
                lpGBTInterface::icWrite(plpGBT, 0x0a7, cValueDataRate);
                uint32_t cReadBack = lpGBTInterface::icRead(plpGBT, 0x0a7, 1);
                LOG (DEBUG) << BOLDBLUE << "ePort Tx Data Rate register set to : " << std::bitset<8>(cReadBack) << RESET;
              }
          }
      }
    else if (pRegister == "EPTXEnable")
      {
        for (const auto& cGroup : pGroups)
          {
            LOG (DEBUG) << BOLDBLUE << "Enabling ePort Tx channels for group " << +cGroup << RESET;
            uint32_t cValueEnableTx = 0;
            for (const auto& cChannel : pChannels)
              {
                LOG(DEBUG) << BOLDBLUE << "... Enabling ePort Tx channel " << +cChannel << RESET;
                cValueEnableTx += (1 << (cChannel + 4*(cGroup % 2)));
              }
            lpGBTInterface::icWrite(plpGBT, 0x0a9+(cGroup/2), cValueEnableTx);
          }
      }
    else if (pRegister == "EPTXChnCntr")
      {
        for (const auto& cChannel : pChannels)
          {
            for (const auto& cGroup : pGroups)
              {
                uint32_t cValueChnCntr = lpGBTInterface::icRead(plpGBT, 0x0ac+(4*cChannel)+cGroup, 1);
                for (const auto& cParameter : pParameters)
                  {
                    if (cParameter.first == "PreEmphasisStrength")
                      cValueChnCntr = (cValueChnCntr & ~0xE0) | (cParameter.second << 5);
                    else if (cParameter.first == "PreEmphasisMode")
                      cValueChnCntr = (cValueChnCntr & ~0x18) | (cParameter.second << 3);
                    else if (cParameter.first == "DriveStrength")
                      cValueChnCntr = (cValueChnCntr & ~0x07) | (cParameter.second << 0);
                    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
                  }
                lpGBTInterface::icWrite(plpGBT,0x0ac+(4*cChannel)+cGroup, cValueChnCntr);
              }
          }
      }
    else if (pRegister == "EPTX_ChnCntr")
      {
        for (const auto& cGroup : pGroups)
          {
            for (const auto& cChannel : pChannels)
              {
                uint32_t cValue_ChnCntr = lpGBTInterface::icRead(plpGBT, 0x0bc+(2*cGroup)+(cChannel/2), 1);
                for (const auto& cParameter : pParameters)
                  {
                    if (cParameter.first == "Invert")
                      cValue_ChnCntr = (cValue_ChnCntr & ~(0x1 << (3+4*(cChannel%2)))) | (cParameter.second << (3+4*(cChannel%2)));
                    else if (cParameter.first == "PreEmphasisWidth")
                      cValue_ChnCntr = (cValue_ChnCntr & ~(0x1 << (0+4*(cChannel%2)))) | (cParameter.second << (0+4*(cChannel%2)));
                    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
                  }
                lpGBTInterface::icWrite(plpGBT, 0x0bc+(2*cGroup)+(cChannel/2), cValue_ChnCntr);
              }
          }
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }

  void lpGBTInterface::lpgbtConfigureRx(lpGBT* plpGBT, std::vector<uint8_t>& pGroups, std::vector<uint8_t>& pChannels, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "EPRXControl")
      {
        for (const auto& cGroup : pGroups)
          {
            LOG (DEBUG) << BOLDBLUE << "Configuring: ePort Rx channels for group " << +cGroup << RESET;
            uint32_t cValueEPRxControl = lpGBTInterface::icRead(plpGBT, 0x0c4+cGroup, 1);
            uint32_t cValueEnableRx = 0;
            for (const auto& cChannel : pChannels)
              {
                LOG(DEBUG) << BOLDBLUE << "... Enabling ePort Rx channel " << +cChannel << RESET;
                cValueEnableRx += (1 << ( cChannel + 4 ) );
              }
            cValueEPRxControl = (cValueEPRxControl & ~0xF0) | (cValueEnableRx << 4);
            for (const auto& cParameter : pParameters)
              {
                if (cParameter.first == "DataRate")
                  cValueEPRxControl = (cValueEPRxControl & ~0x0C) | (cParameter.second << 2);
                else if (cParameter.first == "TrackMode")
                  cValueEPRxControl = (cValueEPRxControl & ~0x03) | (cParameter.second << 0);
                else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
              }
            lpGBTInterface::icWrite(plpGBT, 0x0c4+cGroup, cValueEPRxControl);
          }
      }
    else if (pRegister == "EPRXChnCntr")
      {
        for (const auto& cGroup : pGroups)
          {
            for (const auto& cChannel : pChannels)
              {
                uint32_t cValueChnCntr = lpGBTInterface::icRead(plpGBT, 0x0cc+(4*cGroup)+cChannel, 1);
                for (const auto& cParameter : pParameters)
                  {
                    if (cParameter.first == "PhaseSelect")
                      cValueChnCntr = (cValueChnCntr & ~0xF0) | (cParameter.second << 4);
                    else if (cParameter.first == "Invert")
                      cValueChnCntr = (cValueChnCntr & ~0x08) | (cParameter.second << 3);
                    else if (cParameter.first == "AcBias")
                      cValueChnCntr = (cValueChnCntr & ~0x04) | (cParameter.second << 2);
                    else if (cParameter.first == "Term")
                      cValueChnCntr = (cValueChnCntr & ~0x02) | (cParameter.second << 1);
                    else if (cParameter.first == "Eq")
                      cValueChnCntr = (cValueChnCntr & ~0x01) | (cParameter.second << 0);
                    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
                  }
                lpGBTInterface::icWrite(plpGBT, 0x0cc+(4*cGroup)+cChannel, cValueChnCntr);
              }
          }
      }
    else if (pRegister == "EPRXEqControl")
      {
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "Eq")
              {
                for (const auto& cGroup : pGroups)
                  {
                    uint32_t cValueEqControl = lpGBTInterface::icRead(plpGBT, 0x0e9+(cGroup/2), 1);
                    for (const auto& cChannel : pChannels)
                      cValueEqControl = (cValueEqControl & ~(0x1 << (4*(cGroup%2)+cChannel))) | (cParameter.second << (4*(cGroup%2)+cChannel));
                    lpGBTInterface::icWrite(plpGBT, 0x0e9+(cGroup/2), cValueEqControl);
                  }
              }
          }
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Power Up State Machine Configuration                              */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigurePowerUpSM(lpGBT* plpGBT, const ParameterVect& pParameters)
  {
    uint32_t cValuePOWERUP2 = lpGBTInterface::icRead(plpGBT, 0x0ef, 1);
    for (const auto& cParameter : pParameters)
      {
        if (cParameter.first == "dllConfigDone")
          cValuePOWERUP2 = (cValuePOWERUP2 & ~0x4) | (cParameter.second << 2);
        else if (cParameter.first == "pllConfigDone")
          cValuePOWERUP2 = (cValuePOWERUP2 & ~0x2) | (cParameter.second << 1);
        else if (cParameter.first == "updateEnable")
          cValuePOWERUP2 = (cValuePOWERUP2 & ~0x1) | (cParameter.second << 0);
        else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
      }
    lpGBTInterface::icWrite(plpGBT, 0x0ef, cValuePOWERUP2);
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Testing Configuration                                             */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigureTesting(lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "ULDataSource0")
      {
        uint32_t cValueULDataSource0 = lpGBTInterface::icRead(plpGBT, 0x118, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "ECDataSource")
              cValueULDataSource0 = (cValueULDataSource0 & ~0xE0) | (cParameter.second << 5);
            else if (cParameter.first == "SerTestPattern")
              cValueULDataSource0 = (cValueULDataSource0 & ~0x0F) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x118, cValueULDataSource0);
      }
    else if (pRegister == "ULDataSource1")
      {
        uint32_t cValueULDataSource1 = lpGBTInterface::icRead(plpGBT, 0x119, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "LDDataSource")
              cValueULDataSource1 = (cValueULDataSource1 & ~0xC0) | (cParameter.second << 6);
            else if (cParameter.first == "G1DataSource")
              cValueULDataSource1 = (cValueULDataSource1 & ~0x38) | (cParameter.second << 3);
            else if (cParameter.first == "G0DataSource")
              cValueULDataSource1 = (cValueULDataSource1 & ~0x07) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x119, cValueULDataSource1);
      }
    else if (pRegister == "ULDataSource2")
      {
        uint32_t cValueULDataSource2 = lpGBTInterface::icRead(plpGBT, 0x11a, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "G3DataSource")
              cValueULDataSource2 = (cValueULDataSource2 & ~0x38) | (cParameter.second << 3);
            else if (cParameter.first == "G2DataSource")
              cValueULDataSource2 = (cValueULDataSource2 & ~0x38) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x11a, cValueULDataSource2);
      }
    else if (pRegister == "ULDataSource3")
      {
        uint32_t cValueULDataSource3 = lpGBTInterface::icRead(plpGBT, 0x11b, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "G5DataSource")
              cValueULDataSource3 = (cValueULDataSource3 & ~0x38) | (cParameter.second << 3);
            else if (cParameter.first == "G4DataSource")
              cValueULDataSource3 = (cValueULDataSource3 & ~0x38) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x11b, cValueULDataSource3);
      }
    else if (pRegister == "ULDataSource4")
      {
        uint32_t cValueULDataSource4 = lpGBTInterface::icRead(plpGBT, 0x11c, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "ECDataSource")
              cValueULDataSource4 = (cValueULDataSource4 & ~0xC0) | (cParameter.second << 6);
            else if (cParameter.first == "ICDataSource")
              cValueULDataSource4 = (cValueULDataSource4 & ~0x38) | (cParameter.second << 3);
            else if (cParameter.first == "G6DataSource")
              cValueULDataSource4 = (cValueULDataSource4 & ~0x07) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x11c, cValueULDataSource4);
      }
    else if (pRegister == "ULDataSource5")
      {
        uint32_t cValueULDataSource5 = lpGBTInterface::icRead(plpGBT, 0x11d, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "G3DataSource")
              cValueULDataSource5 = (cValueULDataSource5 & ~0xC0) | (cParameter.second << 6);
            else if (cParameter.first == "G2DataSource")
              cValueULDataSource5 = (cValueULDataSource5 & ~0x30) | (cParameter.second << 4);
            else if (cParameter.first == "G1DataSource")
              cValueULDataSource5 = (cValueULDataSource5 & ~0x0C) | (cParameter.second << 2);
            else if (cParameter.first == "G0DataSource")
              cValueULDataSource5 = (cValueULDataSource5 & ~0x03) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x11d, cValueULDataSource5);
      }
    else if (pRegister == "DPDataPattern3")
      {
        uint32_t cValueDPDP3 = lpGBTInterface::icRead(plpGBT, 0x11e, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "DPDataPattern3")
              cValueDPDP3 = (cValueDPDP3 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x11e, cValueDPDP3);
      }
    else if (pRegister == "DPDataPattern2")
      {
        uint32_t cValueDPDP2 = lpGBTInterface::icRead(plpGBT, 0x11f, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "DPDataPattern2")
              cValueDPDP2 = (cValueDPDP2 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x11f, cValueDPDP2);
      }
    else if (pRegister == "DPDataPattern1")
      {
        uint32_t cValueDPDP1 = lpGBTInterface::icRead(plpGBT, 0x120, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "DPDataPattern1")
              cValueDPDP1 = (cValueDPDP1 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x120, cValueDPDP1);
      }
    else if (pRegister == "DPDataPattern0")
      {
        uint32_t cValueDPDP0 = lpGBTInterface::icRead(plpGBT, 0x121, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "DPDataPattern0")
              cValueDPDP0 = (cValueDPDP0 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x121, cValueDPDP0);
      }
    else if (pRegister == "BERTSource")
      {
        uint32_t cValueBERTSource = lpGBTInterface::icRead(plpGBT, 0x126, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "BERTSource")
              cValueBERTSource = (cValueBERTSource & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x126, cValueBERTSource);
      }
    else if (pRegister == "BERTConfig")
      {
        uint32_t cValueBERTConfig = lpGBTInterface::icRead(plpGBT, 0x127, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "MeasTime")
              cValueBERTConfig = (cValueBERTConfig & ~0xF0) | (cParameter.second << 4);
            else if (cParameter.first == "SKIPDisable")
              cValueBERTConfig = (cValueBERTConfig & ~0x02) | (cParameter.second << 1);
            else if (cParameter.first == "Start")
              cValueBERTConfig = (cValueBERTConfig & ~0x01) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x127, cValueBERTConfig);
      }
    else if (pRegister == "BERTDataPattern3")
      {
        uint32_t cValueBERTDP3 = lpGBTInterface::icRead(plpGBT, 0x128, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "BERTDataPattern3")
              cValueBERTDP3 = (cValueBERTDP3 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x128, cValueBERTDP3);
      }
    else if (pRegister == "BERTDataPattern2")
      {
        uint32_t cValueBERTDP2 = lpGBTInterface::icRead(plpGBT, 0x129, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "BERTDataPattern2")
              cValueBERTDP2 = (cValueBERTDP2 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x129, cValueBERTDP2);
      }
    else if (pRegister == "BERTDataPattern1")
      {
        uint32_t cValueBERTDP1 = lpGBTInterface::icRead(plpGBT, 0x12a, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "BERTDataPattern1")
              cValueBERTDP1 = (cValueBERTDP1 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x12a, cValueBERTDP1);
      }
    else if (pRegister == "BERTDataPattern0")
      {
        uint32_t cValueBERTDP0 = lpGBTInterface::icRead(plpGBT, 0x12b, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "BERTDataPattern0")
              cValueBERTDP0 = (cValueBERTDP0 & ~0xFF) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x12b, cValueBERTDP0);
      }
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Debug Configuration                                               */
  /*-------------------------------------------------------------------------*/
  void lpGBTInterface::lpgbtConfigureDebug(lpGBT* plpGBT, const std::string& pRegister, const ParameterVect& pParameters)
  {
    if (pRegister == "DataPath")
      {
        uint32_t cValueDataPath = lpGBTInterface::icRead(plpGBT, 0x132, 1);
        for (const auto& cParameter : pParameters)
          {
            if (cParameter.first == "BypasDeInterlevear")
              cValueDataPath = (cValueDataPath & ~0x80) | (cParameter.second << 7);
            else if (cParameter.first == "BypasFECDecoder")
              cValueDataPath = (cValueDataPath & ~0x40) | (cParameter.second << 6);
            else if (cParameter.first == "BypassDeScrambler")
              cValueDataPath = (cValueDataPath & ~0x20) | (cParameter.second << 5);
            else if (cParameter.first == "FECErrCntEna")
              cValueDataPath = (cValueDataPath & ~0x10) | (cParameter.second << 4);
            else if (cParameter.first == "BypassInterleaver")
              cValueDataPath = (cValueDataPath & ~0x04) | (cParameter.second << 2);
            else if (cParameter.first == "BypassScrambler")
              cValueDataPath = (cValueDataPath & ~0x02) | (cParameter.second << 1);
            else if (cParameter.first == "BypassFECCoder")
              cValueDataPath = (cValueDataPath & ~0x01) | (cParameter.second << 0);
            else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
          }
        lpGBTInterface::icWrite(plpGBT, 0x132, cValueDataPath);
      }
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT ePort Rx read-only registers                                      */
  /*-------------------------------------------------------------------------*/
  uint8_t lpGBTInterface::lpgbtGetRx(lpGBT* plpGBT, const std::string& pRegister)
  {
    uint8_t cRegisterValue = 0;
    if (pRegister == "EPRX0DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x158, 1) & 0xFF;
    else if (pRegister == "EPRX1DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x159, 1) & 0xFF;
    else if (pRegister == "EPRX2DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x15a, 1) & 0xFF;
    else if (pRegister == "EPRX3DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x15b, 1) & 0xFF;
    else if (pRegister == "EPRX4DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x15c, 1) & 0xFF;
    else if (pRegister == "EPRX5DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x15d, 1) & 0xFF;
    else if (pRegister == "EPRX0DllStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x15e, 1) & 0xFF;
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
    return cRegisterValue;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT BERT Tester read-only registers                                   */
  /*-------------------------------------------------------------------------*/
  uint8_t lpGBTInterface::lpgbtGetBERTTester(lpGBT* plpGBT, const std::string& pRegister)
  {
    uint8_t cRegisterValue = 0;
    if (pRegister == "BERTStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1bf, 1) & 0xFF;
    else if (pRegister == "BERTResult4")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1c0, 1) & 0xFF;
    else if (pRegister == "BERTResult3")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1c1, 1) & 0xFF;
    else if (pRegister == "BERTResult2")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1c2, 1) & 0xFF;
    else if (pRegister == "BERTResult1")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1c3, 1) & 0xFF;
    else if (pRegister == "BERTResult0")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1c4, 1) & 0xFF;
    else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << pRegister << RESET;
    return cRegisterValue;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT Power Up State Machine read-only registers                        */
  /*-------------------------------------------------------------------------*/
  uint8_t lpGBTInterface::lpgbtGetPowerUpSM(lpGBT* plpGBT, const std::string& pRegister)
  {
    uint8_t cRegisterValue = 0;
    if (pRegister == "PUSMStatus")
      cRegisterValue = lpGBTInterface::icRead(plpGBT, 0x1c7, 1) & 0xFF;
    return cRegisterValue;
  }


  /*-------------------------------------------------------------------------*/
  /* lpGBT I2C configuration and communication                               */
  /*-------------------------------------------------------------------------*/
  uint8_t lpGBTInterface::configI2C(lpGBT* plpGBT, uint16_t  pMaster, const ParameterVect& pParameters)
  {
    uint8_t cValue = 0x00;
    for (const auto& cParameter : pParameters)
      {
        if (cParameter.first == "Frequency")
          cValue = (cValue & ~0x03) | (cParameter.second << 0);
        else if (cParameter.first == "NBytes")
          cValue = (cValue & ~0x7C) | (cParameter.second << 2);
        else if (cParameter.first == "SCLDriveMode")
          cValue = (cValue & ~0x80) | (cParameter.second << 7);
        else LOG (ERROR) << BOLDRED << "Wrong parameter name: " << cParameter.first << RESET;
      }

    uint32_t cErrorCode = lpGBTInterface::ecWrite(plpGBT, pMaster, 0x0, cValue << 8*3);
    if (cErrorCode != 0)
      {
        LOG (INFO) << BOLDRED << "lpGBT Error code : " << +cErrorCode << RESET;
        return cErrorCode;
      }

    return cErrorCode;
  }

  uint32_t lpGBTInterface::readI2C(lpGBT* plpGBT, uint16_t pMaster, uint8_t pSlave, uint8_t pNBytes)
  {
    lpGBTInterface::configI2C(plpGBT, pMaster, {{"Frequency", 2}, {"NBytes", pNBytes}, {"SCLDriveMode", 0}});
    uint32_t pData = lpGBTInterface::ecRead(plpGBT, pMaster, (pNBytes == 1) ? 0x3 : 0xD, (pSlave << 3*8));
    return ((pData & 0x00FFFF00) >> 8);
  }

  uint8_t lpGBTInterface::writeI2C(lpGBT* plpGBT, uint16_t pMaster, uint8_t pSlave , uint32_t pData, uint8_t pNBytes)
  {
    lpGBTInterface::configI2C(plpGBT , pMaster, {{"Frequency", 2}, {"NBytes", pNBytes}, {"SCLDriveMode", 0}});
    if (pNBytes == 1)
      {
        uint32_t cData = (pSlave << 3*8) | (pData << 2*8);
        return lpGBTInterface::ecWrite(plpGBT, pMaster, 0x2 , cData);
      }
    else
      {
        uint32_t cErrorCode = lpGBTInterface::ecWrite( plpGBT, pMaster , 0x8 , pData);
        if (cErrorCode != 0) return cErrorCode;
        return lpGBTInterface::ecWrite(plpGBT, pMaster , 0xC , (pSlave << 3*8));
      }
  }

  void lpGBTInterface::i2cWrite(lpGBT* plpGBT, const std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies)
  {
    this->setBoard(plpGBT->getBeBoardId());

    ParameterVect cParameters = {{"Frequency", 2}, {"SCLDriveMode", 0}};
    std::map<uint8_t, std::vector<uint32_t>> cI2CMasterWordsMap = {};

    for (auto cVecSendIter = pVecSend.begin(); cVecSendIter < pVecSend.end(); cVecSendIter++)
      {
        uint32_t cWord = *cVecSendIter;
        uint8_t cFeId = (cWord & (0xF << 23)) >> 23;
        cI2CMasterWordsMap[2*(cFeId % 2)].push_back(cWord); //FeId 0 : Master 0 -- FeId 1 : Master 2
      }

    for (auto cI2CMasterWordsMapIter = cI2CMasterWordsMap.begin(); cI2CMasterWordsMapIter != cI2CMasterWordsMap.end(); cI2CMasterWordsMapIter++)
      {
        auto cMaster = cI2CMasterWordsMapIter->first;
        std::map<uint8_t, std::vector<uint32_t>> cI2CWrite, cI2CRead;

        for (auto cWordsIter = cI2CMasterWordsMapIter->second.begin(); cWordsIter < cI2CMasterWordsMapIter->second.end(); cWordsIter++)
          {
            uint32_t cWord = *cWordsIter;
            uint8_t cWrite = !((cWord & (0x1 << 8*2)) >> 8*2);
            uint8_t cLinkId = (cWord & (0x3 << 29) ) >> 29;
            uint8_t cChipId = (cWord & (0xF << 18) ) >> 18;
            uint16_t cAddress = (cWord & 0xFFFF);
            uint8_t cValue = 0;
            if (cWrite == 1)
              {
                cWordsIter++;
                cWord = *cWordsIter;
                cValue = (cWord & 0xFF);
                cI2CWrite[cLinkId].push_back((cChipId << 8*3) | (cAddress << 8) | (cValue << 0));
              }
            else
              cI2CRead[cLinkId].push_back((cChipId << 8*3) | (cAddress << 8) | (cValue << 0));
          }

        for (auto cI2CWriteIter = cI2CWrite.begin(); cI2CWriteIter != cI2CWrite.end(); cI2CWriteIter++)
          {
            std::map<uint8_t, std::vector<uint32_t>> cI2CWriteChip;
            for (auto cWordsIter = cI2CWriteIter->second.begin(); cWordsIter < cI2CWriteIter->second.end(); cWordsIter++)
              {
                uint32_t cWord = *cWordsIter;
                uint8_t cChipId = (cWord & (0xF << 8*3)) >> 8*3;
                uint16_t cAddress = (cWord & (0xFFFF << 8)) >> 8;
                uint8_t cValue = (cWord & (0xFF << 0)) >> 0;
                cI2CWriteChip[cChipId].push_back((cAddress << 8) | (cValue << 0));
              }

            for (auto cI2CWriteChipIter = cI2CWriteChip.begin(); cI2CWriteChipIter != cI2CWriteChip.end(); cI2CWriteChipIter++)
              {
                auto cChipId = cI2CWriteChipIter->first;
                auto cChipRegisters = cI2CWriteChipIter->second;
                uint8_t cSlave = 0;
                if (cChipId < 8)
                  {
                    cParameters.push_back(std::make_pair(std::string("NBytes"), 3));
                    cSlave = 0x20;
                  }
                else
                  {
                    cParameters.push_back(std::make_pair(std::string("NBytes"), 3));
                    cSlave = 0x60;
                  }
                lpGBTInterface::configI2C(plpGBT, cMaster, cParameters);

                for (auto cChipRegistersIter = cChipRegisters.begin(); cChipRegistersIter < cChipRegisters.end(); cChipRegistersIter++)
                  {
                    uint32_t cWord = *cChipRegistersIter;
                    uint16_t cAddress = (cWord & (0xFFFF << 8)) >> 8;
                    uint8_t cValue = (cWord & (0xFF << 0)) >> 0;
                    uint32_t pData = ((cAddress << 8*2) | (cValue << 8));
                    lpGBTInterface::ecWrite(plpGBT, cMaster, 0x8, pData);
                    lpGBTInterface::ecWrite(plpGBT, cMaster, 0xC, (cSlave << 8*3));
                  }
              }
          }

        for (auto cI2CReadIter = cI2CRead.begin(); cI2CReadIter != cI2CRead.end(); cI2CReadIter++)
          fBoardFW->WriteReg("fc7_daq_cnfg.optical_block.mux", cI2CReadIter->first);
      }
  }
}
