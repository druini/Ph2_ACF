/*!

        \file                                            SSAInterface.cc
        \brief                                           User Interface to the SSAs
        \author                                          Marc Osherson
        \version                                         1.0
        \date                        31/07/19
        Support :                    mail to : oshersonmarc@gmail.com

 */

#include "SSAInterface.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Container.h"
#include <bitset>

using namespace Ph2_HwDescription;

#define DEV_FLAG 0
namespace Ph2_HwInterface
{ // start namespace
SSAInterface::SSAInterface(const BeBoardFWMap& pBoardMap) : ReadoutChipInterface(pBoardMap) {}
SSAInterface::~SSAInterface() {}
//

//#FIXME temporary fix to use 1/2 PS skeleton
void SSAInterface::LinkLpGBT(D19clpGBTInterface* pLpGBTInterface, lpGBT* pLpGBT)
{
    flpGBTInterface = pLpGBTInterface;
    flpGBT          = pLpGBT;
}

bool SSAInterface::ConfigureChip(Chip* pSSA, bool pVerifLoop, uint32_t pBlockSize)
{
    setBoard(pSSA->getBeBoardId());
    std::vector<uint32_t> cVec;
    ChipRegMap            cSSARegMap = pSSA->getRegMap();
    // for some reason this makes block write work
    // otherwise need to configure one by one which
    // takes forever
    std::map<uint16_t, ChipRegItem> cMap;
    cMap.clear();
    for(auto& cRegInMap: cSSARegMap) { cMap[cRegInMap.second.fAddress] = cRegInMap.second; }
    std::vector<std::pair<uint16_t, uint16_t>> cRegs;
    for(auto& cRegItem: cMap)
    {
        LOG(DEBUG) << BOLDBLUE << "Register map for SSA contains a register with address " << std::hex << +cRegItem.second.fAddress << std::dec << RESET;
        std::pair<uint16_t, uint16_t> cReg;
        cReg.first  = cRegItem.second.fAddress;
        cReg.second = cRegItem.second.fValue;
        cRegs.push_back(cReg);
    } // loop over map
    return this->WriteRegs(pSSA, cRegs, pVerifLoop);
}

bool SSAInterface::enableInjection(ReadoutChip* pChip, bool inject, bool pVerifLoop)
{
    // for now always with asynchronous mode
    // uint8_t cValue=1;
    // uint8_t cRegValue       = (cValue << 4) | (cValue << 2) | (1 << 0);
    // bool    cEnableAnalogue = WriteChipSingleReg(pChip, "ENFLAGS", cRegValue, false);
    // bool    cEnableFECal    = WriteChipSingleReg(pChip, "FE_Calibration", 1, pVerifLoop);
    // cRegValue               = ReadChipReg(pChip, "ReadoutMode");
    // cRegValue               = (cRegValue & 0x4) | (1);
    // bool cReadoutMode       = WriteChipSingleReg(pChip, "ReadoutMode", cRegValue, pVerifLoop);
    // return cEnableAnalogue && cEnableFECal && cReadoutMode;
    return this->WriteChipReg(pChip, "AnalogueAsync", 1);
}
bool SSAInterface::setInjectionAmplitude(ReadoutChip* pChip, uint8_t injectionAmplitude, bool pVerifLoop) { return this->WriteChipReg(pChip, "InjectedCharge", injectionAmplitude, pVerifLoop); }

//
bool SSAInterface::setInjectionSchema(ReadoutChip* pSSA, const ChannelGroupBase* group, bool pVerifLoop) { return true; }
//
bool SSAInterface::maskChannelsGroup(ReadoutChip* pSSA, const ChannelGroupBase* group, bool pVerifLoop) { return true; }
//
bool SSAInterface::maskChannelsAndSetInjectionSchema(ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop) { return true; }
//
bool SSAInterface::ConfigureChipOriginalMask(ReadoutChip* pSSA, bool pVerifLoop, uint32_t pBlockSize) { return true; }
//

bool SSAInterface::MaskAllChannels(ReadoutChip* pSSA, bool mask, bool pVerifLoop) { return true; }
// I actually want this one!
bool SSAInterface::WriteChipReg(Chip* pSSA, const std::string& pRegName, uint16_t pValue, bool pVerifLoop)
{
    if(pRegName == "CountingMode")
    {
        uint8_t cRegValue = (pValue << 2) | (1 << 0);
        return WriteChipSingleReg(pSSA, "ENFLAGS", cRegValue, false);
    }
    else if(pRegName == "AmuxHigh")
    {
        return this->ConfigureAmux(pSSA, "HighZ");
    }
    else if(pRegName == "MonitorBandgap")
    {
        return this->ConfigureAmux(pSSA, "Bandgap");
    }
    else if(pRegName == "MonitorGround")
    {
        return this->ConfigureAmux(pSSA, "GND");
    }
    else if(pRegName == "AnalogueAsync")
    {
        uint8_t cRegValue       = (pValue << 4) | (pValue << 2) | (1 << 0);
        bool    cEnableAnalogue = WriteChipSingleReg(pSSA, "ENFLAGS", cRegValue, false);
        bool    cEnableFECal    = WriteChipSingleReg(pSSA, "FE_Calibration", 1, pVerifLoop);
        cRegValue               = ReadChipReg(pSSA, "ReadoutMode");
        cRegValue               = (cRegValue & 0x4) | (1);
        bool cReadoutMode       = WriteChipSingleReg(pSSA, "ReadoutMode", cRegValue, pVerifLoop);
        return cEnableAnalogue && cEnableFECal && cReadoutMode;
    }
    else if(pRegName == "Sync")
    {
        uint8_t pAnalogueCalib  = 1;
        uint8_t pDigitalCalib   = 1;
        uint8_t pHitCounter     = 0;
        uint8_t pSignalPolarity = 0;
        uint8_t pStripEnable    = 1;
        uint8_t cRegValue       = (pAnalogueCalib << 4) | (pDigitalCalib << 3) | (pHitCounter << 2) | (pSignalPolarity << 1);
        cRegValue               = cRegValue | (pStripEnable << 0);
        LOG(INFO) << BOLDRED << "Enable flag is 0x" << std::hex << +cRegValue << std::dec << RESET;
        bool cEnableAnalogue = WriteChipSingleReg(pSSA, "ENFLAGS", cRegValue, false);
        bool cEnableFECal    = WriteChipSingleReg(pSSA, "FE_Calibration", 1, pVerifLoop);
        cRegValue            = ReadChipReg(pSSA, "ReadoutMode");
        cRegValue            = (cRegValue & 0x4) | ((1 - pValue));
        bool cReadoutMode    = WriteChipSingleReg(pSSA, "ReadoutMode", cRegValue, pVerifLoop);
        LOG(INFO) << BOLDRED << "Readout mode is 0x" << std::hex << +cRegValue << std::dec << RESET;
        return cEnableAnalogue && cEnableFECal && cReadoutMode;
    }
    else if(pRegName == "DigitalAsync")
    {
        // digital injection, async , enable all strips
        uint8_t cRegValue = (pValue << 3) | (1 << 2) | (1 << 0);
        return WriteChipSingleReg(pSSA, "ENFLAGS", cRegValue, false);
    }
    else if(pRegName == "EnableSLVSTestOutput")
    {
        LOG(INFO) << BOLDBLUE << "Enabling SLVS test output on SSA#" << +pSSA->getId() << RESET;
        uint8_t cRegValue = ReadChipReg(pSSA, "ReadoutMode");
        cRegValue         = (cRegValue & 0x4) | (pValue << 1);
        return WriteChipSingleReg(pSSA, "ReadoutMode", cRegValue, pVerifLoop);
    }
    else if(pRegName == "CalibrationPattern")
    {
        uint8_t pAnalogueCalib  = 0;
        uint8_t pDigitalCalib   = 1;
        uint8_t pHitCounter     = 0;
        uint8_t pSignalPolarity = 0;
        uint8_t pStripEnable    = 1;
        uint8_t cRegValue       = (pAnalogueCalib << 4) | (pDigitalCalib << 3) | (pHitCounter << 2) | (pSignalPolarity << 1);
        cRegValue               = cRegValue | (pStripEnable << 0);
        bool cEnableAnalogue    = WriteChipSingleReg(pSSA, "ENFLAGS", cRegValue, false);
        if(cEnableAnalogue)
            return WriteChipSingleReg(pSSA, "DigCalibPattern_L", pValue, pVerifLoop);
        else
            return cEnableAnalogue;
    }
    else if(pRegName.find("CalibrationPattern") != std::string::npos)
    {
        int cChannel;
        std::sscanf(pRegName.c_str(), "CalibrationPatternS%d", &cChannel);
        uint16_t cAddress = 0x0600 + cChannel + 1;
        LOG(INFO) << BOLDBLUE << "Configuring register 0x" << std::hex << cAddress << std::dec << " to 0x" << std::hex << pValue << std::dec << " for channel " << +cChannel << RESET;

        uint8_t pAnalogueCalib  = 0;
        uint8_t pDigitalCalib   = 1;
        uint8_t pHitCounter     = 0;
        uint8_t pSignalPolarity = 0;
        uint8_t pStripEnable    = 1;
        uint8_t cRegValue       = (pAnalogueCalib << 4) | (pDigitalCalib << 3) | (pHitCounter << 2) | (pSignalPolarity << 1);
        cRegValue               = cRegValue | (pStripEnable << 0);
        bool cEnableAnalogue    = WriteChipSingleReg(pSSA, "ENFLAGS", cRegValue, false);
        if(cEnableAnalogue)
            return this->WriteReg(pSSA, cAddress, pValue, pVerifLoop);
        else
            return cEnableAnalogue;
    }
    else if(pRegName == "InjectedCharge")
    {
        LOG(INFO) << BOLDBLUE << "Setting "
                  << " bias calDac to " << +pValue << " on SSA" << +pSSA->getId() << RESET;
        return WriteChipSingleReg(pSSA, "Bias_CALDAC", pValue, pVerifLoop);
    }
    else if(pRegName == "Threshold")
    {
        LOG(DEBUG) << BOLDRED << "Setting threshold to " << +pValue << RESET;
        return WriteChipSingleReg(pSSA, "Bias_THDAC", (pValue), pVerifLoop);
    }
    else
    {
        return this->WriteChipSingleReg(pSSA, pRegName, pValue, pVerifLoop);
    }
}
bool SSAInterface::ConfigureAmux(Chip* pChip, const std::string& pRegister)
{
    // first make sure amux is set to 0 to avoid shorts
    // from SSA python methods
    uint8_t                  cHighZValue = 0x00;
    std::vector<std::string> cRegNames{"Bias_TEST_LSB", "Bias_TEST_MSB"};
    for(auto cReg: cRegNames)
    {
        bool cSuccess = this->WriteChipSingleReg(pChip, cReg, cHighZValue);
        if(!cSuccess)
            return cSuccess;
        else
            LOG(DEBUG) << BOLDBLUE << "Set " << cReg << " to 0x" << std::hex << +cHighZValue << std::dec << RESET;
    }
    if(pRegister != "HighZ")
    {
        auto cMapIterator = fAmuxMap.find(pRegister);
        if(cMapIterator != fAmuxMap.end())
        {
            uint16_t cValue = (1 << cMapIterator->second);
            LOG(DEBUG) << BOLDBLUE << "Select test_Bias 0x" << std::hex << cValue << std::dec << RESET;
            uint8_t cIndex = 0;
            for(auto cReg: cRegNames)
            {
                uint8_t cRegValue = (cValue & (0xFF << 8 * cIndex)) >> 8 * cIndex;
                bool    cSuccess  = this->WriteChipSingleReg(pChip, cReg, cRegValue);
                if(!cSuccess)
                    return cSuccess;
                else
                    LOG(DEBUG) << BOLDBLUE << "Set " << cReg << " to 0x" << std::hex << +cRegValue << std::dec << RESET;
                cIndex++;
            }
            return true;
        }
        else
            return false;
    }
    else
        return true;
}
uint8_t SSAInterface::ReadChipId(Chip* pChip)
{
    bool cVerifLoop = true;
    // start chip id read operation
    if(!this->WriteChipSingleReg(pChip, "Fuse_Mode", 0x0F, cVerifLoop))
    {
        // chip id - 8 LSBs of e-fuse register
        return 0;
    }
    else
        throw std::runtime_error(std::string("Failed to start e-fuse read operation from SSA ") + std::to_string(pChip->getId()));
}

bool SSAInterface::WriteReg(Chip* pChip, uint16_t pRegisterAddress, uint16_t pRegisterValue, bool pVerifLoop)
{
    bool cSuccess = false;
    setBoard(pChip->getBeBoardId());
    // write
    if(flpGBTInterface == nullptr)
    {
        std::vector<uint32_t> cVec;
        ChipRegItem           cRegItem;
        cRegItem.fPage    = 0x00;
        cRegItem.fAddress = pRegisterAddress;
        cRegItem.fValue   = pRegisterValue & 0xFF;
        fBoardFW->EncodeReg(cRegItem, pChip->getId(), pChip->getId(), cVec, pVerifLoop, true);
        uint8_t cWriteAttempts = 0;
        cSuccess               = fBoardFW->WriteChipBlockReg(cVec, cWriteAttempts, pVerifLoop);
    }
    else
    {
        LOG(DEBUG) << BOLDBLUE << "Writing address 0x" << std::hex << +pRegisterAddress << std::dec << RESET;
        cSuccess = flpGBTInterface->ssaWrite(flpGBT, pChip->getHybridId(), pChip->getId(), pRegisterAddress, pRegisterValue, pVerifLoop);
    }
    return cSuccess;
}

bool SSAInterface::WriteRegs(Chip* pChip, const std::vector<std::pair<uint16_t, uint16_t>> pRegs, bool pVerifLoop)
{
    setBoard(pChip->getBeBoardId());
    bool cSuccess = true;
    if(flpGBTInterface == nullptr)
    {
        std::vector<uint32_t> cVec;
        cVec.clear();
        for(const auto& cReg: pRegs)
        {
            ChipRegItem cRegItem;
            cRegItem.fPage    = 0x00;
            cRegItem.fAddress = cReg.first;
            cRegItem.fValue   = cReg.second & 0xFF;
            fBoardFW->EncodeReg(cRegItem, pChip->getId(), pChip->getId(), cVec, pVerifLoop, true);
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
        }
        uint8_t cWriteAttempts = 0;
        cSuccess               = fBoardFW->WriteChipBlockReg(cVec, cWriteAttempts, pVerifLoop);
#ifdef COUNT_FLAG
        fTransactionCount++;
#endif
    }
    else
    {
        int cRegCount = 0;
        for(const auto& cReg: pRegs)
        {
            if(cRegCount % 50 == 0) LOG(INFO) << BOLDBLUE << "Writing SSA register with address " << std::hex << +cReg.first << std::dec << RESET;
            cSuccess = flpGBTInterface->ssaWrite(flpGBT, pChip->getHybridId(), pChip->getId(), cReg.first, cReg.second, pVerifLoop);
            if(!cSuccess) continue;
#ifdef COUNT_FLAG
            fRegisterCount++;
#endif
            cRegCount++;
        }
    }
    return cSuccess;
}

uint16_t SSAInterface::ReadReg(Chip* pChip, uint16_t pRegisterAddress, bool pVerifLoop)
{
    setBoard(pChip->getBeBoardId());
    ChipRegItem cRegItem;
    cRegItem.fPage    = 0x00;
    cRegItem.fAddress = pRegisterAddress;
    cRegItem.fValue   = 0;
    if(flpGBTInterface == nullptr)
    {
        bool                  cFailed = false;
        bool                  cRead;
        std::vector<uint32_t> cVecReq;
        fBoardFW->EncodeReg(cRegItem, pChip->getId(), pChip->getId(), cVecReq, true, false);
        fBoardFW->ReadChipBlockReg(cVecReq);
        uint8_t cSSAId;
        fBoardFW->DecodeReg(cRegItem, cSSAId, cVecReq[0], cRead, cFailed);
    }
    else
    {
        // FIXME the FeId is hard coded for now, need to get the FeId info here
        cRegItem.fValue = flpGBTInterface->ssaRead(flpGBT, pChip->getHybridId(), pChip->getId(), pRegisterAddress);
    }
    return cRegItem.fValue & 0xFF;
}

bool SSAInterface::WriteChipSingleReg(Chip* pChip, const std::string& pRegNode, uint16_t pValue, bool pVerifLoop)
{
    setBoard(pChip->getBeBoardId());
    bool        cSuccess = true;
    ChipRegItem cRegItem = pChip->getRegItem(pRegNode);
    cRegItem.fValue      = pValue & 0xFF;
    if(flpGBTInterface == nullptr)
    {
        std::vector<uint32_t> cVec;
        fBoardFW->EncodeReg(cRegItem, pChip->getHybridId(), pChip->getId(), cVec, pVerifLoop, true);
        uint8_t cWriteAttempts = 0;
        cSuccess               = fBoardFW->WriteChipBlockReg(cVec, cWriteAttempts, pVerifLoop);
    }
    else
    {
        cSuccess = flpGBTInterface->ssaWrite(flpGBT, pChip->getHybridId(), pChip->getId(), cRegItem.fAddress, cRegItem.fValue, pVerifLoop);
    }
    if(cSuccess)
    {
        pChip->setReg(pRegNode, pValue);
        if(pVerifLoop)
        {
            if(pRegNode != "ENFLAGS" && pRegNode != "DigCalibPattern_L" && pRegNode != "DigCalibPattern_H")
            {
                uint16_t cReadBack = ReadChipReg(pChip, pRegNode);
                if(cReadBack != pValue)
                {
                    LOG(INFO) << BOLDRED << "Read back value from " << pRegNode << BOLDBLUE << " at I2C address " << std::hex << cRegItem.fAddress << std::dec << " not equal to write value of "
                              << std::hex << +cRegItem.fValue << std::dec << RESET;
                    return false;
                }
            }
        }
    }
#ifdef COUNT_FLAG
    fRegisterCount++;
    fTransactionCount++;
#endif
    return cSuccess;
}
bool SSAInterface::WriteChipMultReg(Chip* pSSA, const std::vector<std::pair<std::string, uint16_t>>& pVecReq, bool pVerifLoop)
{
    setBoard(pSSA->getBeBoardId());
    std::vector<uint32_t> cVec;
    ChipRegItem           cRegItem;
    for(const auto& cReg: pVecReq)
    {
        cRegItem        = pSSA->getRegItem(cReg.first);
        cRegItem.fValue = cReg.second;
        fBoardFW->EncodeReg(cRegItem, pSSA->getHybridId(), pSSA->getId(), cVec, pVerifLoop, true);
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }
    uint8_t cWriteAttempts = 0;
    bool    cSuccess       = fBoardFW->WriteChipBlockReg(cVec, cWriteAttempts, pVerifLoop);
#ifdef COUNT_FLAG
    fTransactionCount++;
#endif
    if(cSuccess)
    {
        for(const auto& cReg: pVecReq)
        {
            cRegItem = pSSA->getRegItem(cReg.first);
            pSSA->setReg(cReg.first, cReg.second);
        }
    }
    return cSuccess;
}
bool SSAInterface::WriteChipAllLocalReg(ReadoutChip* pChip, const std::string& dacName, ChipContainer& localRegValues, bool pVerifLoop)
{
    assert(localRegValues.size() == pChip->getNumberOfChannels());
    std::string dacTemplate;
    // bool isMask = false;

    if(dacName == "GainTrim")
        dacTemplate = "GAINTRIMMING_S%d";
    else if(dacName == "ThresholdTrim")
        dacTemplate = "THTRIMMING_S%d";
    // else if(dacName == "Mask") isMask = true;
    else
        LOG(ERROR) << "Error, DAC " << dacName << " is not a Local DAC";

    std::vector<std::pair<std::string, uint16_t>> cRegVec;
    ChannelGroup<NCHANNELS, 1>                    channelToEnable;

    std::vector<uint32_t> cVec;
    cVec.clear();
    bool cSuccess = true;
    for(uint8_t iChannel = 0; iChannel < pChip->getNumberOfChannels(); ++iChannel)
    {
        char dacName1[20];
        sprintf(dacName1, dacTemplate.c_str(), 1 + iChannel);
        LOG(DEBUG) << BOLDBLUE << "Setting register " << dacName1 << " to " << (localRegValues.getChannel<uint16_t>(iChannel) & 0x1F) << RESET;
        cSuccess = cSuccess && this->WriteChipSingleReg(pChip, dacName1, (localRegValues.getChannel<uint16_t>(iChannel) & 0x1F), pVerifLoop);
    }
    return cSuccess;
}
// Definitely needed:

void SSAInterface::ReadASEvent(ReadoutChip* pSSA, std::vector<uint32_t>& pData, std::pair<uint32_t, uint32_t> pSRange)
{
    if(pSRange == std::pair<uint32_t, uint32_t>{0, 0}) pSRange = std::pair<uint32_t, uint32_t>{1, pSSA->getNumberOfChannels()};
    for(uint32_t i = pSRange.first; i <= pSRange.second; i++)
    {
        char cRegName[100];
        std::sprintf(cRegName, "CounterStrip%d", static_cast<int>(i));
        pData.push_back(this->ReadChipReg(pSSA, cRegName));
        // uint8_t cRP1 = this->ReadChipReg(pSSA, "ReadCounter_LSB_S" + std::to_string(i));
        // uint8_t cRP2 = this->ReadChipReg(pSSA, "ReadCounter_MSB_S" + std::to_string(i));
        // pData.push_back((cRP2*256) + cRP1);
    }
}
uint16_t SSAInterface::ReadChipReg(Chip* pSSA, const std::string& pRegNode)
{
    setBoard(pSSA->getBeBoardId());
    std::vector<uint32_t> cVecReq;
    ChipRegItem           cRegItem;
    if(pRegNode.find("CounterStrip") != std::string::npos)
    {
        int cChannel = 0;
        sscanf(pRegNode.c_str(), "CounterStrip%d", &cChannel);
        cRegItem.fPage         = 0x00;
        cRegItem.fAddress      = 0x0901 + cChannel;
        cRegItem.fValue        = 0;
        uint8_t cRPLSB         = this->ReadReg(pSSA, cRegItem.fAddress) & 0xFF;
        cRegItem.fPage         = 0x00;
        cRegItem.fAddress      = 0x0801 + cChannel;
        uint8_t  cRPMSB        = this->ReadReg(pSSA, cRegItem.fAddress) & 0xFF;
        uint16_t cCounterValue = (cRPMSB << 8) | cRPLSB;
        LOG(DEBUG) << BOLDBLUE << "Counter MSB is 0x" << std::bitset<8>(cRPMSB) << " Counter LSB is 0x" << std::bitset<8>(cRPLSB) << " Counter value is " << std::hex << +cCounterValue << std::dec
                   << RESET;
        return cCounterValue;
    }
    else if(pRegNode == "ChipId")
    {
        return this->ReadChipId(pSSA);
    }
    else if(pRegNode == "Threshold")
    {
        return this->ReadChipReg(pSSA, "Bias_THDAC");
    }
    else
    {
        cRegItem = pSSA->getRegItem(pRegNode);
        return this->ReadReg(pSSA, cRegItem.fAddress) & 0xFF;
    }
}
} // namespace Ph2_HwInterface
