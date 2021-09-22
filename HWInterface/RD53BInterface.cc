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

RD53BInterface::RD53BInterface(const BeBoardFWMap& pBoardMap) : RD53InterfaceBase(pBoardMap) {}

bool RD53BInterface::ConfigureChip(Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
{
    setup(pChip);

    auto* chip = static_cast<RD53B*>(pChip);

    for (const auto& reg : RD53B::Registers) {
        if (!reg.readOnly && chip->getRegValue(reg) != reg.defaultValue)
            WriteReg(chip, reg, chip->getRegValue(reg));
    }
    
    WriteReg(chip, RD53BReg::PIX_DEFAULT_CONFIG, 0x9CE2);
    WriteReg(chip, RD53BReg::PIX_DEFAULT_CONFIG_B, 0x631D);
    WriteRegField(chip, RD53BReg::TriggerConfig, 0, 0); // trigger mode
    WriteRegField(chip, RD53BReg::DataConcentratorConf, 3, 1); // events / stream

    LOG(INFO) << "VCAL_HIGH = " << ReadChipReg(chip, "VCAL_HIGH") << std::endl;

    return true;
}

void RD53BInterface::InitRD53Downlink(const BeBoard* pBoard)
{
    setup(pBoard);

    LOG(INFO) << GREEN << "Down-link phase initialization..." << RESET;

    WriteReg(pBoard, RD53BReg::GCR_DEFAULT_CONFIG, 0xac75);
    WriteReg(pBoard, RD53BReg::GCR_DEFAULT_CONFIG_B, 0x538a);
    WriteReg(pBoard, RD53BReg::CmdErrCnt, 0);
    SendGlobalPulse(pBoard, {"ResetChannelSynchronizer", "ResetCommandDecoder", "ResetGlobalConfiguration"}, 0xff);
    WriteReg(pBoard, RD53BReg::RingOscConfig, 0x7fff);
    WriteReg(pBoard, RD53BReg::RingOscConfig, 0x5eff);
    SendGlobalPulse(pBoard, {"ResetEfuses"}, 0xff);

    std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
    LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;
}

void RD53BInterface::InitRD53Uplinks(ReadoutChip* pChip, int nActiveLanes)
{
    
    LOG(INFO) << GREEN << "Configuring up-link lanes and monitoring..." << RESET;

    WriteReg(pChip, RD53BReg::SER_SEL_OUT, 0x0055);
    WriteReg(pChip, RD53BReg::CML_CONFIG, 0x0001);
    WriteReg(pChip, RD53BReg::AuroraConfig, 0x0167);
    WriteReg(pChip, RD53BReg::ServiceDataConf, (1 << 8) | 50);
    WriteReg(pChip, RD53BReg::AURORA_CB_CONFIG0, 0x0FF1);
    WriteReg(pChip, RD53BReg::AURORA_CB_CONFIG0, 0x0000);
    SendGlobalPulse(pChip, {"ResetSerializers", "ResetAurora"}, 0xff);
    BitVector<uint16_t> bits;
    auto cmdValue = RD53BCmd::Clear.Create({pChip->getId()});
    RD53BCmd::Clear.serialize(cmdValue, bits);
    RD53BCmd::Clear.serialize(cmdValue, bits);
    SendCommandStream(pChip, bits);
    // SendCommand(pChip, RD53BCmd::Clear);
    // SendCommand(pChip, RD53BCmd::Clear);

    LOG(INFO) << BOLDBLUE << "\t--> Done" << RESET;
}
bool RD53BInterface::WriteChipReg(Chip* pChip, const std::string& regName, const uint16_t data, bool pVerifLoop)
{
    setup(pChip);

    auto* chip = static_cast<RD53B*>(pChip);
    auto& reg = RD53B::Reg(regName);

    WriteReg(chip, reg, data);

    if((reg == RD53BReg::VCAL_HIGH) || (reg == RD53BReg::VCAL_MED)) std::this_thread::sleep_for(std::chrono::microseconds(VCALSLEEP)); // @TMP@

    if(pVerifLoop == true && (reg != RD53BReg::PIX_PORTAL || chip->getRegField(RD53BReg::PIX_MODE, 2) == 0))
    {
        uint16_t value = ReadChipReg(pChip, regName);

        if (value != data) {
            LOG(ERROR) << BOLDRED << "Error when reading back what was written into RD53 reg. " << BOLDYELLOW << regName << RESET;
            return false;
        }
    }

    return true;
}


void RD53BInterface::WriteReg(Chip* pChip, const RD53B::Register& reg, const uint16_t value)
{
    SendCommand(pChip, RD53BCmd::WrReg, reg.address, value);
    static_cast<RD53B*>(pChip)->setRegValue(reg, value);

    if((reg == RD53BReg::VCAL_HIGH) || (reg == RD53BReg::VCAL_MED)) std::this_thread::sleep_for(std::chrono::microseconds(VCALSLEEP)); // @TMP@
}


void RD53BInterface::WriteReg(const Hybrid* hybrid, const RD53B::Register& reg, const uint16_t value) {
    SendCommand(hybrid, RD53BCmd::WrReg, reg.address, value);
    
    for (auto* chip : *hybrid)
        static_cast<RD53B*>(chip)->setRegValue(reg, value);

    if((reg == RD53BReg::VCAL_HIGH) || (reg == RD53BReg::VCAL_MED)) std::this_thread::sleep_for(std::chrono::microseconds(VCALSLEEP)); // @TMP@
}

void RD53BInterface::WriteReg(const BeBoard* pBoard, const RD53B::Register& reg, const uint16_t data)
{
    setup(pBoard);

    for (const auto* opticalGroup : *pBoard) {
        for (const auto* hybrid : *opticalGroup) {
            WriteReg(hybrid, reg, data);
        }
    }
}

void RD53BInterface::WriteBoardBroadcastChipReg(const BeBoard* pBoard, const std::string& regName, const uint16_t data)
{
    WriteReg(pBoard, RD53B::Reg(regName), data);
}

uint16_t RD53BInterface::ReadChipReg(Chip* pChip, const std::string& regName)
{
    auto& boardFW = setup(pChip);
    auto& reg = RD53B::Reg(regName);
    auto* chip = static_cast<RD53B*>(pChip);

    const int nAttempts = 2; // @CONST@
    for(auto attempt = 0; attempt < nAttempts; attempt++)
    {
        SendCommand(chip, RD53BCmd::RdReg, reg.address);

        uint16_t address = reg == RD53BReg::PIX_PORTAL ? chip->getCurrentRow() : reg.address;

        auto regReadback = boardFW.ReadChipRegisters(chip);

        auto it = std::find_if(regReadback.begin(), regReadback.end(), [=] (auto& readback) {
            return readback.first == address;
        });

        if (it == regReadback.end())
        {
            LOG(WARNING) << BLUE << "Register readback error, attempt n. " << YELLOW << attempt + 1 << BLUE << "/" << YELLOW << nAttempts << RESET;
            std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::DEEPSLEEP));
        }
        else {
            chip->setRegValue(reg, it->second);
            return it->second;
        }
    }

    LOG(ERROR) << BOLDRED << "Invalid register readback after " << BOLDYELLOW << nAttempts << BOLDRED " attempts" << RESET;

    return 0;
}



void RD53BInterface::ChipErrorReport(ReadoutChip* pChip)
{
    LOG(INFO) << BOLDBLUE << "LockLossCnt        = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "LockLossCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "BitFlipWngCnt      = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "BitFlipWngCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "BitFlipErrCnt      = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "BitFlipErrCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "CmdErrCnt          = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "CmdErrCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "RdWrFifoErrorCount = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "RdWrFifoErrorCount") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "SkippedTriggerCnt  = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "SkippedTriggerCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_0_Cnt        = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "HitOr_0_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_1_Cnt        = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "HitOr_1_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_2_Cnt        = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "HitOr_2_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "HitOr_3_Cnt        = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "HitOr_3_Cnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "BCIDCnt            = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "BCIDCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "TrigCnt            = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "TrigCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
    LOG(INFO) << BOLDBLUE << "ReadTrigCnt        = " << BOLDYELLOW << RD53BInterface::ReadChipReg(pChip, "ReadTrigCnt") << std::setfill(' ') << std::setw(8) << "" << RESET;
}


float RD53BInterface::ReadHybridTemperature(ReadoutChip* pChip)
{
    auto& boardFW = setup(pChip);
    return boardFW.ReadHybridTemperature(pChip->getHybridId());
}

float RD53BInterface::ReadHybridVoltage(ReadoutChip* pChip)
{
    auto& boardFW = setup(pChip);
    return boardFW.ReadHybridVoltage(pChip->getHybridId());
}

} // namespace Ph2_HwInterface
