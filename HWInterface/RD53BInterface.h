/*!
  \file                  RD53BInterface.h
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef RD53BInterface_H
#define RD53BInterface_H

#include "../HWDescription/ChipRegItem.h"
#include "BeBoardFWInterface.h"
#include "RD53FWInterface.h"
#include "RD53InterfaceBase.h"
#include "../Utils/Bits/BitVector.hpp"
#include "../HWDescription/RD53B.h"
#include "../HWDescription/RD53BCommands.h"
#include "../Utils/RD53BUtils.h"

#include "../Utils/xtensor/xview.hpp"

// #############
// # CONSTANTS #
// #############
#define VCALSLEEP 50000 // [microseconds]

namespace Ph2_HwInterface
{

template <class Flavor>
class RD53BInterface : public RD53InterfaceBase
{
    using Chip = Ph2_HwDescription::Chip;
    using ReadoutChip = Ph2_HwDescription::ReadoutChip;
    using BeBoard = Ph2_HwDescription::BeBoard;
    using Hybrid = Ph2_HwDescription::Hybrid;
    using RD53B = Ph2_HwDescription::RD53B<Flavor>;

    using Reg = typename RD53B::Reg;
    using Register = Ph2_HwDescription::RD53BConstants::Register;
    using RegisterField = Ph2_HwDescription::RD53BConstants::RegisterField;

public:
    RD53BInterface(const BeBoardFWMap& pBoardMap);

    // #############################
    // # Override member functions #
    // #############################
    bool     ConfigureChip(Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    bool     WriteChipReg(Chip* pChip, const std::string& regName, uint16_t data, bool pVerifLoop = true) override;
    void     WriteBoardBroadcastChipReg(const BeBoard* pBoard, const std::string& regName, uint16_t data) override;
    bool     WriteChipAllLocalReg(ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop = true) override;
    void     ReadChipAllLocalReg(ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue) override;
    uint16_t ReadChipReg(Chip* pChip, const std::string& regName) override;
    bool     ConfigureChipOriginalMask(ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    bool     MaskAllChannels(ReadoutChip* pChip, bool mask, bool pVerifLoop = true) override;
    bool     maskChannelsAndSetInjectionSchema(ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop = false) override;

    boost::optional<uint16_t> ReadChipReg(RD53B* chip, const Register& reg);
    uint16_t ReadReg(Chip* chip, const Register& reg, bool update = false);
    size_t ReadReg(Chip* chip, const std::string& regName, bool update = false);

    void WriteReg(Chip* chip, const Register& reg, uint16_t value);
    void WriteRegField(Chip* chip, const RegisterField& field, uint16_t value, bool update = false);
    void WriteReg(Chip* chip, const std::string& regName, size_t value, bool update = false);
    void WriteReg(Hybrid* chip, const Register& reg, uint16_t value);
    void WriteReg(Hybrid* chip, const std::string& regName, size_t value, bool update = false);
    
    void ChipErrorReport(ReadoutChip* pChip);

    void InitRD53Downlink(const BeBoard* pBoard);
    void InitRD53Uplinks(ReadoutChip* pChip, int nActiveLanes = 1);

    void UpdatePixelConfig(RD53B* pRD53) {
        UpdatePixelConfig(pRD53, pRD53->pixelConfig);
    }

public:
    template <class Device>
    void UpdatePixelConfig(Device* device, const typename RD53B::PixelConfig& cfg, bool updateMasks = true, bool updateTdac = true) {
        setup(device);

        std::vector<uint16_t> pixModeValues;
        RD53BUtils::for_each_device<Chip>(device, [&] (Chip* chip) {
            pixModeValues.push_back(ReadReg(chip, Reg::PIX_MODE));
        });

        for (uint16_t col_pair = 0; col_pair < RD53B::nCols / 2; ++col_pair) {
            const uint16_t col = col_pair * 2;
            BitVector<uint16_t> cmd_stream;

            SerializeCommand<RD53BCmd::WrReg>(device, cmd_stream, Reg::REGION_COL.address, col_pair);

            if (updateMasks) {
                SerializeCommand<RD53BCmd::WrReg>(device, cmd_stream, Reg::REGION_ROW.address, uint16_t{0});
                SerializeCommand<RD53BCmd::WrReg>(device, cmd_stream, Reg::PIX_MODE.address, uint16_t{1}); // mask + auto-row
                
                xt::xarray<uint16_t> mask_data = xt::col(cfg.enable, col + 1);
                mask_data |= xt::left_shift(xt::col(cfg.enableInjections, col + 1), 1);
                mask_data |= xt::left_shift(xt::col(cfg.enableHitOr, col + 1),  2);
                mask_data = xt::left_shift(mask_data, 5);
                mask_data |= xt::col(cfg.enable, col);
                mask_data |= xt::left_shift(xt::col(cfg.enableInjections, col), 1);
                mask_data |= xt::left_shift(xt::col(cfg.enableHitOr, col), 2);

                SerializeCommand<RD53BCmd::WrRegLong>(device, cmd_stream, std::vector<uint16_t>(mask_data.begin(), mask_data.end()));
            }

            if (updateTdac) {
                SerializeCommand<RD53BCmd::WrReg>(device, cmd_stream, Reg::REGION_ROW.address, uint16_t{0});
                SerializeCommand<RD53BCmd::WrReg>(device, cmd_stream, Reg::PIX_MODE.address, uint16_t{3}); // tdac + auto-row
                
                xt::xarray<uint16_t> tdac_data = xt::col(cfg.tdac, col + 1) & 0x1F;
                tdac_data |= xt::left_shift(xt::col(cfg.tdac, col)  & 0x1F, 5);

                SerializeCommand<RD53BCmd::WrRegLong>(device, cmd_stream, std::vector<uint16_t>(tdac_data.begin(), tdac_data.end()));
            }

            SendCommandStream(device, cmd_stream);
        }

        auto pixMode = pixModeValues.begin();
        RD53BUtils::for_each_device<Chip>(device, [&] (Chip* chip) {
            static_cast<RD53B*>(chip)->pixelConfig = cfg;
            WriteReg(chip, Reg::PIX_MODE, *pixMode);
            ++pixMode;
        });

    }

    // serialize a command for any destination device into an existing BitVector
    template <class Cmd, class Device, class... Args>
    void SerializeCommand(const Device* destination, BitVector<uint16_t>& bits, Args&&... args) {
        RD53BCmd::serialize<Cmd>(bits, RD53B::ChipIdFor(destination), std::forward<Args>(args)...);
    }

    // serialize a command for any destination device into a new BitVector
    template <class Cmd, class Device, class... Args>
    auto SerializeCommand(const Device* destination, Args&&... args) {
        BitVector<uint16_t> bits;
        SerializeCommand<Cmd>(destination, bits, std::forward<Args>(args)...);
        return bits;
    }

    // send a single command to any destination device
    template <class Cmd, class Device, class... Args>
    void SendCommand(const Device* destination, Args&&... args) {
        auto bits = SerializeCommand<Cmd>(destination, std::forward<Args>(args)...);
        SendCommandStream(destination, bits);
    }

    // send a BitVector containing serialized commands to the hybrid which is/contains the given destination device
    template <class Device, typename std::enable_if_t<!std::is_same<Device, BeBoard>::value, int> = 0>
    void SendCommandStream(const Device* destination, const BitVector<uint16_t>& cmdStream) {
        setup(destination).WriteChipCommand(cmdStream.blocks(), destination->getHybridId());
    }

    // send a BitVector containing serialized commands to all hybrids of a BeBoard
    void SendCommandStream(const BeBoard* pBoard, const BitVector<uint16_t>& cmdStream) {
        auto& fwInterface = setup(pBoard);
        for (const auto* opticalGroup : *pBoard)
            for (const auto* hybrid : *opticalGroup)
                fwInterface.WriteChipCommand(cmdStream.blocks(), hybrid->getHybridId());
    }

    template <class Device>
    void SendGlobalPulse(Device* destination, const std::vector<std::string>& routes, uint8_t width = 1) {
        uint16_t route_value = 0;

        for (const auto& name : routes) 
            route_value |= (1 << RD53B::GlobalPulseRoutes.at(name));

        WriteReg(destination, Reg::GlobalPulseConf, route_value);
        WriteReg(destination, Reg::GlobalPulseWidth, width);
        SendCommand<RD53BCmd::GlobalPulse>(destination);
    }

    void StartPRBSpattern(Ph2_HwDescription::ReadoutChip* pChip) {}
    void StopPRBSpattern(Ph2_HwDescription::ReadoutChip* pChip) {}

    RD53FWInterface& setup(const Ph2_HwDescription::FrontEndDescription* device) {
        this->setBoard(device->getBeBoardId());
        return *static_cast<RD53FWInterface*>(fBoardFW);
    }

    RD53FWInterface& setup(const BeBoard* pBoard) {
        this->setBoard(pBoard->getId());
        return *static_cast<RD53FWInterface*>(fBoardFW);
    }

    void UpdateCoreColumns(Chip* chip);
    void UpdateCoreColRegs(Chip* chip, const std::string& prefix, const std::vector<bool>& core_col_en);

    // ###########################
    // # Dedicated to minitoring #
    // ###########################
    void ReadChipMonitor(ReadoutChip* pChip, const std::vector<std::string>& args)
    {
        for(const auto& arg: args) ReadChipMonitor(pChip, arg);
    }
    float    ReadChipMonitor(ReadoutChip* pChip, const std::string& observableName) { return 0; }
    float    ReadHybridTemperature(ReadoutChip* pChip);
    float    ReadHybridVoltage(ReadoutChip* pChip);
    uint32_t ReadChipADC(ReadoutChip* pChip, const std::string& observableName) { return 0; }

    // private:
    // uint32_t getADCobservable(const std::string& observableName, bool* isCurrentNotVoltage);
    // uint32_t measureADC(ReadoutChip* pChip, uint32_t data);
    // float    measureVoltageCurrent(ReadoutChip* pChip, uint32_t data, bool isCurrentNotVoltage);
    // float    measureTemperature(ReadoutChip* pChip, uint32_t data);
    // float    convertADC2VorI(ReadoutChip* pChip, uint32_t value, bool isCurrentNotVoltage = false);
};

} // namespace Ph2_HwInterface

#endif