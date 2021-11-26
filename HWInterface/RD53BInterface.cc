/*!
  \file                  RD53BInterface.cc
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53BInterface.h"


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{

template <class Flavor>
RD53BInterface<Flavor>::RD53BInterface(const BeBoardFWMap& pBoardMap) : RD53InterfaceBase(pBoardMap) {}

template <class Flavor>
bool RD53BInterface<Flavor>::ConfigureChip(Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    setup(pChip);

    auto* chip = static_cast<RD53B*>(pChip);
    
    WriteReg(chip, Reg::PIX_DEFAULT_CONFIG, 0x9CE2);
    WriteReg(chip, Reg::PIX_DEFAULT_CONFIG_B, 0x631D);
    usleep(100000);

    std::vector<std::pair<const Register&, uint16_t>> regValuePairs;

    for (const auto& item : chip->registerConfig) {
        WriteReg(chip, item.first, item.second);
    }

    WriteReg(chip, "TwoLevelTrigger", 0);
    WriteReg(chip, "EnEoS", 0);
    WriteReg(chip, "NumOfEventsInStream", 0);
    WriteReg(chip, "BinaryReadOut", 0);
    WriteReg(chip, "RawData", 0);
    WriteReg(chip, "EnOutputDataChipId", 0);

    // WriteReg(chip, Reg::RingOscConfig, 0x7fff);
    // WriteReg(chip, Reg::RingOscConfig, 0x5eff);

    UpdateCoreColumns(chip);

    UpdatePixelConfig(chip, chip->pixelConfig);

    return true;
}

template <class Flavor>
void RD53BInterface<Flavor>::InitRD53Downlink(const BeBoard* pBoard)
{
    auto& fwInterface = setup(pBoard);

    LOG(INFO) << GREEN << "Down-link phase initialization (RD53B<" << Flavor::name << ">)..." << RESET;

    RD53BUtils::for_each_device<Hybrid>(pBoard, [&] (Hybrid* hybrid) {
        WriteReg(hybrid, Reg::GCR_DEFAULT_CONFIG, 0xac75);
        WriteReg(hybrid, Reg::GCR_DEFAULT_CONFIG_B, 0x538a);
        WriteReg(hybrid, Reg::CmdErrCnt, 0);
        WriteReg(hybrid, Reg::CdrConf, fwInterface.getUplinkDataRate() == RD53FWInterface::UplinkDataRate::x1280 ? 0 : 1);
        SendGlobalPulse(hybrid, {"ResetChannelSynchronizer", "ResetCommandDecoder", "ResetGlobalConfiguration"}, 0xff);
        WriteReg(hybrid, Reg::RingOscConfig, 0x7fff);
        WriteReg(hybrid, Reg::RingOscConfig, 0x5eff);
        SendGlobalPulse(hybrid, {"ResetEfuses"}, 0xff);
    });

    std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
    LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;
}

template <class Flavor>
void RD53BInterface<Flavor>::InitRD53Uplinks(ReadoutChip* pChip, int nActiveLanes)
{
    LOG(INFO) << GREEN << "Configuring up-link lanes and monitoring..." << RESET;

    WriteReg(pChip, Reg::SER_SEL_OUT, 0x0055);
    WriteReg(pChip, Reg::CML_CONFIG, 0x0001);
    WriteReg(pChip, Reg::AuroraConfig, 0x0167);
    WriteReg(pChip, Reg::ServiceDataConf, (1 << 8) | 50);
    WriteReg(pChip, Reg::AURORA_CB_CONFIG0, 0x0FF1);
    WriteReg(pChip, Reg::AURORA_CB_CONFIG0, 0x0000);
    SendGlobalPulse(pChip, {"ResetSerializers", "ResetAurora"}, 0xff);
    // BitVector<uint16_t> bits;
    // auto cmdValue = RD53BCmd::Clear.Create({pChip->getId()});
    // RD53BCmd::Clear.serialize(cmdValue, bits);
    // RD53BCmd::Clear.serialize(cmdValue, bits);
    // SendCommandStream(pChip, bits);
    SendCommand<RD53BCmd::Clear>(pChip);
    SendCommand<RD53BCmd::Clear>(pChip);

    LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;
}


template <class Flavor>
boost::optional<uint16_t> RD53BInterface<Flavor>::ReadChipReg(RD53B* rd53b, const Register& reg) {
    auto& boardFW = setup(rd53b);

    const int nAttempts = 10; // @CONST@
    for(auto attempt = 0; attempt < nAttempts; attempt++)
    {
        SendCommand<RD53BCmd::RdReg>(rd53b, reg.address);

        uint16_t address = reg == Reg::PIX_PORTAL ? bits::pack<1, 9>(1, rd53b->currentRow) : reg.address;

        auto regReadback = boardFW.ReadChipRegisters(rd53b);

        auto it = std::find_if(regReadback.begin(), regReadback.end(), [=] (auto& readback) {
            return readback.first == address;
        });

        if (it != regReadback.end()) {
            if (!reg.isVolatile)
                rd53b->registerValues[reg.address] = it->second;
            if (reg == Reg::REGION_ROW)
                rd53b->currentRow = it->second;
            if (reg == Reg::PIX_PORTAL && (ReadReg(rd53b, Reg::PIX_MODE) & 1)) // auto-row
                ++rd53b->currentRow;
            return it->second;
        }

        LOG(WARNING) << BLUE << "Register readback (" << reg.name << ") error, attempt n. " << YELLOW << attempt + 1 << BLUE << "/" << YELLOW << nAttempts << RESET;
        if (regReadback.size()) {
            LOG(WARNING) << BLUE << "Readback entries (expected address: " << address << "): " << RESET;
            for (const auto& item : regReadback)
                LOG(WARNING) << BLUE << "\taddress: " << item.first << ", value: " << item.second << RESET;
        }
        else
            LOG(WARNING) << BLUE << "No readback entries." << RESET;

        std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
    }

    LOG(ERROR) << BOLDRED << "Invalid register readback (" << reg.name << ") after " << BOLDYELLOW << nAttempts << BOLDRED " attempts" << RESET;

    return boost::none;
}


template <class Flavor>
uint16_t RD53BInterface<Flavor>::ReadReg(Chip* chip, const Register& reg, bool update) {
    auto& rd53b = *static_cast<RD53B*>(chip);
    if (!update && !reg.isVolatile && rd53b.registerValues[reg.address])
        return *rd53b.registerValues[reg.address];
    auto value = ReadChipReg(&rd53b, reg);
    if (value)
        return *value;
    else
        throw std::runtime_error("could not read register: " + reg.name);
}

template <class Flavor>
size_t RD53BInterface<Flavor>::ReadReg(Chip* chip, const std::string& regName, bool update) {
    size_t result = 0;
    size_t offset = 0;
    for (const auto& field : RD53B::vRegs.at(regName)) {
        uint16_t regValue = ReadReg(chip, field.reg, update);
        size_t fieldValue = ((regValue >> field.offset) & ((1u << field.size) - 1));
        result |= fieldValue << offset;
        offset += field.size;
    }
    return result;
}


template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(Chip* chip, const Register& reg, uint16_t value) {
    SendCommand<RD53BCmd::WrReg>(chip, reg.address, value);
    static_cast<RD53B*>(chip)->registerValues[reg.address] = value;
    if (reg == RD53B::Reg::REGION_ROW)
        static_cast<RD53B*>(chip)->currentRow = value;
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteRegField(Chip* chip, const RegisterField& field, uint16_t value, bool update){
    uint16_t regValue = 0;
    if (field.size < field.reg.size)
        regValue = ReadReg(chip, field.reg, update);
    uint16_t mask = ((1 << field.size) - 1) << field.offset;
    regValue = regValue ^ ((regValue ^ (value << field.offset)) & mask);
    WriteReg(chip, field.reg, regValue);
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(Chip* chip, const std::string& regName, size_t value, bool update) {
    size_t offset = 0;
    auto it = RD53B::vRegs.find(regName);
    if (it == RD53B::vRegs.end())
        LOG(ERROR) << BOLDRED << "Reg not found: " << regName << RESET;
    else {
        for (const auto& field : it->second) {
            uint16_t fieldValue = (value >> offset) & ((1u << field.size) - 1);
            WriteRegField(chip, field, fieldValue, update);
            offset += field.size;
        }
    }
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(Hybrid* hybrid, const Register& reg, uint16_t value) {
    SendCommand<RD53BCmd::WrReg>(hybrid, reg.address, value);
    
    for (auto* chip : *hybrid) {
        static_cast<RD53B*>(chip)->registerValues[reg.address] = value;
        if (reg == RD53B::Reg::REGION_ROW)
            static_cast<RD53B*>(chip)->currentRow = value;
    }
}


template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(Hybrid* hybrid, const std::string& regName, size_t value, bool update) {
    size_t offset = 0;
    for (const auto& field : RD53B::vRegs.at(regName)) {
        uint16_t fieldValue = (value >> offset) & ((1u << field.size) - 1);
        if (field.size == field.reg.size)
            WriteReg(hybrid, field.reg, fieldValue);
        else {
            for (auto* chip : *hybrid)
                WriteRegField(chip, field, fieldValue, update);
        }
        offset += field.size;
    }
}


template <class Flavor>
uint16_t RD53BInterface<Flavor>::ReadChipReg(Chip* chip, const std::string& regName)
{
    return ReadReg(chip, regName, true);
}

template <class Flavor>
bool RD53BInterface<Flavor>::WriteChipReg(Chip* chip, const std::string& regName, const uint16_t data, bool pVerifLoop)
{
    setup(chip);

    WriteReg(chip, regName, data);

    if(pVerifLoop && ReadReg(chip, regName, true) != data)
    {
        LOG(ERROR) << BOLDRED << "Error when reading back what was written into RD53 reg. " << BOLDYELLOW << regName << RESET;
        return false;
    }

    return true;
}


template <class Flavor>
void RD53BInterface<Flavor>::WriteBoardBroadcastChipReg(const BeBoard* pBoard, const std::string& regName, uint16_t data)
{
    RD53BUtils::for_each_device<Hybrid>(pBoard, [&] (Hybrid* hybrid) {
        WriteReg(hybrid, regName, data);
    });
}

template <class Flavor>
void RD53BInterface<Flavor>::ChipErrorReport(ReadoutChip* pChip)
{
    LOG(INFO) << BOLDBLUE << "LockLossCnt        = " << BOLDYELLOW << ReadChipReg(pChip, "LockLossCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "BitFlipWngCnt      = " << BOLDYELLOW << ReadChipReg(pChip, "BitFlipWngCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "BitFlipErrCnt      = " << BOLDYELLOW << ReadChipReg(pChip, "BitFlipErrCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "CmdErrCnt          = " << BOLDYELLOW << ReadChipReg(pChip, "CmdErrCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "RdWrFifoErrorCount = " << BOLDYELLOW << ReadChipReg(pChip, "RdWrFifoErrorCount") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "SkippedTriggerCnt  = " << BOLDYELLOW << ReadChipReg(pChip, "SkippedTriggerCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_0_Cnt        = " << BOLDYELLOW << ReadChipReg(pChip, "HitOr_0_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_1_Cnt        = " << BOLDYELLOW << ReadChipReg(pChip, "HitOr_1_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_2_Cnt        = " << BOLDYELLOW << ReadChipReg(pChip, "HitOr_2_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_3_Cnt        = " << BOLDYELLOW << ReadChipReg(pChip, "HitOr_3_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "BCIDCnt            = " << BOLDYELLOW << ReadChipReg(pChip, "BCIDCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "TrigCnt            = " << BOLDYELLOW << ReadChipReg(pChip, "TrigCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "ReadTrigCnt        = " << BOLDYELLOW << ReadChipReg(pChip, "ReadTrigCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
}

template <class Flavor>
bool RD53BInterface<Flavor>::ConfigureChipOriginalMask(ReadoutChip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    RD53B* pRD53 = static_cast<RD53B*>(pChip);

    pRD53->pixelConfig = pRD53->defaultPixelConfig;
    UpdatePixelConfig(pRD53);

    return true;
}

template <class Flavor>
bool RD53BInterface<Flavor>::MaskAllChannels(ReadoutChip* pChip, bool mask, bool pVerifLoop)
{
    RD53B* pRD53 = static_cast<RD53B*>(pChip);

    pRD53->pixelConfig.enable = mask;

    UpdatePixelConfig(pRD53);
    // RD53Interface::WriteRD53Mask(pRD53, false, false);

    return true;
}

template <class Flavor>
bool RD53BInterface<Flavor>::maskChannelsAndSetInjectionSchema(ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop)
{
    RD53B* pRD53 = static_cast<RD53B*>(pChip);

    for (auto row = 0u; row < RD53::nRows; row++)
        for (auto col = 0u; col < RD53::nCols; col++)
        {
            if (mask == true) 
                pRD53->pixelConfig.enable(row, col) = group->isChannelEnabled(row, col) && pRD53->defaultPixelConfig.enable(row, col);
            if (inject == true)
                pRD53->pixelConfig.enableInjections(row, col) = group->isChannelEnabled(row, col) && pRD53->defaultPixelConfig.enable(row, col);
            else
                pRD53->pixelConfig.enableInjections(row, col) = group->isChannelEnabled(row, col) && pRD53->defaultPixelConfig.enable(row, col) && pRD53->defaultPixelConfig.enableInjections(row, col);
        }

    UpdatePixelConfig(pRD53);

    return true;
}

template <class Flavor>
bool RD53BInterface<Flavor>::WriteChipAllLocalReg(ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop)
{
    RD53B* pRD53 = static_cast<RD53B*>(pChip);

    for(auto row = 0u; row < RD53::nRows; row++)
        for(auto col = 0u; col < RD53::nCols; col++) 
            pRD53->pixelConfig.tdac(row, col) = pValue.getChannel<uint16_t>(row, col);

    UpdatePixelConfig(pRD53);
    // RD53Interface::WriteRD53Mask(pRD53, false, false);

    return true;
}

template <class Flavor>
void RD53BInterface<Flavor>::ReadChipAllLocalReg(ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue)
{
    for(auto row = 0u; row < RD53::nRows; row++)
        for(auto col = 0u; col < RD53::nCols; col++) 
            pValue.getChannel<uint16_t>(row, col) = static_cast<RD53B*>(pChip)->pixelConfig.tdac(row, col);
}


template <class Flavor>
float RD53BInterface<Flavor>::ReadHybridTemperature(ReadoutChip* pChip)
{
    auto& boardFW = setup(pChip);
    return boardFW.ReadHybridTemperature(pChip->getHybridId());
}

template <class Flavor>
float RD53BInterface<Flavor>::ReadHybridVoltage(ReadoutChip* pChip)
{
    auto& boardFW = setup(pChip);
    return boardFW.ReadHybridVoltage(pChip->getHybridId());
}

template class RD53BInterface<RD53BFlavor::ATLAS>;
template class RD53BInterface<RD53BFlavor::CMS>;

} // namespace Ph2_HwInterface