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

    std::vector<std::pair<const Register&, uint16_t>> regValuePairs;

    for (const auto& reg : RD53B::Registers) {
        if (!reg.readOnly)
            regValuePairs.emplace_back(reg, chip->getRegValue(reg));
    }

    WriteRegs(pChip, regValuePairs);
    
    WriteReg(chip, Reg::PIX_DEFAULT_CONFIG, 0x9CE2);
    WriteReg(chip, Reg::PIX_DEFAULT_CONFIG_B, 0x631D);
    WriteRegField(chip, Reg::TriggerConfig, 0, 0); // trigger mode
    WriteRegField(chip, Reg::DataConcentratorConf, 3, 1); // events / stream

    return true;
}

template <class Flavor>
void RD53BInterface<Flavor>::InitRD53Downlink(const BeBoard* pBoard)
{
    auto& fwInterface = setup(pBoard);

    LOG(INFO) << GREEN << "Down-link phase initialization (RD53B<" << Flavor::name << ">)..." << RESET;

    WriteReg(pBoard, Reg::GCR_DEFAULT_CONFIG, 0xac75);
    WriteReg(pBoard, Reg::GCR_DEFAULT_CONFIG_B, 0x538a);
    WriteReg(pBoard, Reg::CmdErrCnt, 0);
    WriteReg(pBoard, Reg::CdrConf, fwInterface.getUplinkDataRate() == RD53FWInterface::UplinkDataRate::x1280 ? 0 : 1);
    SendGlobalPulse(pBoard, {"ResetChannelSynchronizer", "ResetCommandDecoder", "ResetGlobalConfiguration"}, 0xff);
    WriteReg(pBoard, Reg::RingOscConfig, 0x7fff);
    WriteReg(pBoard, Reg::RingOscConfig, 0x5eff);
    SendGlobalPulse(pBoard, {"ResetEfuses"}, 0xff);

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
    SendCommand(pChip, RD53BCmd::Clear);
    SendCommand(pChip, RD53BCmd::Clear);

    LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;
}

template <class Flavor>
bool RD53BInterface<Flavor>::WriteChipReg(Chip* pChip, const std::string& regName, const uint16_t data, bool pVerifLoop)
{
    setup(pChip);

    auto* chip = static_cast<RD53B*>(pChip);
    auto& reg = RD53B::getRegister(regName);

    WriteReg(chip, reg, data);

    if((reg == Reg::VCAL_HIGH) || (reg == Reg::VCAL_MED)) std::this_thread::sleep_for(std::chrono::microseconds(VCALSLEEP)); // @TMP@

    if(pVerifLoop == true && (reg != Reg::PIX_PORTAL || chip->getRegField(Reg::PIX_MODE, 2) == 0))
    {
        uint16_t value = ReadChipReg(pChip, regName);

        if (value != data) {
            LOG(ERROR) << BOLDRED << "Error when reading back what was written into RD53 reg. " << BOLDYELLOW << regName << RESET;
            return false;
        }
    }

    return true;
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(Chip* pChip, const Register& reg, const uint16_t value)
{
    SendCommand(pChip, RD53BCmd::WrReg, reg.address, value);
    static_cast<RD53B*>(pChip)->setRegValue(reg, value);

    if((reg == Reg::VCAL_HIGH) || (reg == Reg::VCAL_MED)) std::this_thread::sleep_for(std::chrono::microseconds(VCALSLEEP)); // @TMP@
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(const Hybrid* hybrid, const Register& reg, const uint16_t value) {
    SendCommand(hybrid, RD53BCmd::WrReg, reg.address, value);
    
    for (auto* chip : *hybrid)
        static_cast<RD53B*>(chip)->setRegValue(reg, value);

    if((reg == Reg::VCAL_HIGH) || (reg == Reg::VCAL_MED)) std::this_thread::sleep_for(std::chrono::microseconds(VCALSLEEP)); // @TMP@
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteReg(const BeBoard* pBoard, const Register& reg, const uint16_t data)
{
    setup(pBoard);

    for (const auto* opticalGroup : *pBoard) {
        for (const auto* hybrid : *opticalGroup) {
            WriteReg(hybrid, reg, data);
        }
    }
}

template <class Flavor>
void RD53BInterface<Flavor>::WriteBoardBroadcastChipReg(const BeBoard* pBoard, const std::string& regName, const uint16_t data)
{
    WriteReg(pBoard, RD53B::getRegister(regName), data);
}

template <class Flavor>
uint16_t RD53BInterface<Flavor>::ReadChipReg(Chip* pChip, const std::string& regName)
{
    auto& boardFW = setup(pChip);
    auto& reg = RD53B::getRegister(regName);
    auto* chip = static_cast<RD53B*>(pChip);

    const int nAttempts = 10; // @CONST@
    for(auto attempt = 0; attempt < nAttempts; attempt++)
    {
        SendCommand(chip, RD53BCmd::RdReg, reg.address);

        uint16_t address = reg == Reg::PIX_PORTAL ? bits::pack<1, 9>(1, chip->getCurrentRow()) : reg.address;

        auto regReadback = boardFW.ReadChipRegisters(chip);

        auto it = std::find_if(regReadback.begin(), regReadback.end(), [=] (auto& readback) {
            return readback.first == address;
        });

        if (it == regReadback.end())
        {
            LOG(WARNING) << BLUE << "Register readback (" << regName << ") error, attempt n. " << YELLOW << attempt + 1 << BLUE << "/" << YELLOW << nAttempts << RESET;
            if (regReadback.size()) {
                LOG(WARNING) << BLUE << "Readback entries:" << RESET;
                for (const auto& item : regReadback)
                    LOG(WARNING) << BLUE << "\taddress: " << item.first << ", value: " << item.second << RESET;
            }
            else
                LOG(WARNING) << BLUE << "No readback entries." << RESET;
        }
        else {
            chip->setRegValue(reg, it->second);
            return it->second;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
    }

    LOG(ERROR) << BOLDRED << "Invalid register readback (" << regName << ") after " << BOLDYELLOW << nAttempts << BOLDRED " attempts" << RESET;

    return 0;
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

// template <class Flavor> template <class Device>
// void RD53BInterface<Flavor>::UpdatePixelConfig(Device* device, const RD53B::PixelConfig& cfg) {
//     setup(pRD53);
//     const auto& cfg = pRD53->pixelConfig;

//     for (uint16_t col_pair = 0; col_pair < RD53B::nCols / 2; ++col_pair) {
//         uint16_t col = col_pair * 2;
//         BitVector<uint16_t> cmd_stream;

//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrReg, Reg::REGION_COL.address, col_pair);

//         // configure masks
//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrReg, Reg::PIX_MODE.address, uint16_t{1}); // mask + auto-row
//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrReg, Reg::REGION_ROW.address, uint16_t{0});
        
//         typename RD53B::PixelMatrix<uint16_t> mask_data(0);
//         mask_data |= cfg.enable.col(col);
//         mask_data |= cfg.enableInjections.col(col) << 1;
//         mask_data |= cfg.enableHitOr.col(col) << 2;
//         mask_data <<= 5;
//         mask_data |= cfg.enable.col(col + 1);
//         mask_data |= cfg.enableInjections.col(col + 1) << 1;
//         mask_data |= cfg.enableHitOr.col(col + 1) << 2;

//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrRegLong, std::vector<uint16_t>(mask_data.begin(), mask_data.end()));


//         // configure tdac
//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrReg, Reg::PIX_MODE.address, uint16_t{3}); // tdac + auto-row
//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrReg, Reg::REGION_ROW.address, uint16_t{0});
        
//         typename RD53B::PixelMatrix<uint16_t> tdac_data(0);
//         tdac_data |= cfg.tdac.col(col);
//         tdac_data |= cfg.tdacSign.col(col) << 4;
//         tdac_data <<= 5;
//         tdac_data |= cfg.tdac.col(col + 1);
//         tdac_data |= cfg.tdacSign.col(col + 1) << 4;

//         SerializeCommand(pRD53, cmd_stream, RD53BCmd::WrRegLong, std::vector<uint16_t>(tdac_data.begin(), tdac_data.end()));

//         SendCommandStream(pRD53, cmd_stream);
//     }

// }

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
