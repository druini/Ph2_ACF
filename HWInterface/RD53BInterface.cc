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

    for (const auto& reg : RD53B::Registers) {
        if (!reg.readOnly && chip->getRegValue(reg) != reg.defaultValue)
            WriteReg(chip, reg, chip->getRegValue(reg));
    }
    
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

    LOG(INFO) << GREEN << "Down-link phase initialization..." << RESET;

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
    // SendGlobalPulse(pChip, {"ResetSerializers", "ResetAurora"}, 0xff);
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

    const int nAttempts = 2; // @CONST@
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
            std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
        }
        else {
            chip->setRegValue(reg, it->second);
            return it->second;
        }
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
