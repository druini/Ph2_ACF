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
#include "../Utils/xtensor/xsort.hpp"
#include "../Utils/xtensor/xhistogram.hpp"
#include "../Utils/xtensor/xio.hpp"
#include "../Utils/xtensor/xindex_view.hpp"


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

    // update masks only
    void UpdatePixelMasks(
        Chip* chip,
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& enable, 
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& enableInjections, 
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& enableHitOr
    )
    {
        UpdatePixelConfig(chip, &enable, &enableInjections, &enableHitOr, nullptr, true, false);
    }
    
    // update tdac only
    void UpdatePixelTDAC(
        Chip* chip,
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& tdac
    ) 
    {
        UpdatePixelConfig(chip, nullptr, nullptr, nullptr, &tdac, false, true);
    }

    // update both masks and tdac
    void UpdatePixelConfig(
        Chip* chip,
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& enable, 
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& enableInjections, 
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& enableHitOr,
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>& tdac
    ) 
    {
        UpdatePixelConfig(chip, &enable, &enableInjections, &enableHitOr, &tdac, true, true);
    }

    void UpdatePixelConfig(Chip* chip, bool updateMasks = true, bool updateTdac = true) {
        auto rd53b = static_cast<RD53B*>(chip);
        auto& cfg = rd53b->pixelConfig();
        if (updateMasks && updateTdac)
            UpdatePixelConfig(chip, cfg.enable, cfg.enableInjections, cfg.enableHitOr, cfg.tdac);
        else if (updateMasks)
            UpdatePixelMasks(chip, cfg.enable, cfg.enableInjections, cfg.enableHitOr);
        else
            UpdatePixelTDAC(chip, cfg.tdac);
    }

    template <class Device>
    void UpdatePixelMasksUniform(Device* device, bool enable, bool enableInjections, bool enableHitOr) {
        UpdatePixelConfigUniform(device, enable, enableInjections, enableHitOr, 0, true, false);
    }

    template <class Device>
    void UpdatePixelTDACUniform(Device* device, uint8_t tdac) {
        UpdatePixelConfigUniform(device, false, false, false, tdac, false, true);
    }

    template <class Device>
    void UpdatePixelConfigUniform(Device* device, bool enable, bool enableInjections, bool enableHitOr, uint8_t tdac) {
        UpdatePixelConfigUniform(device, enable, enableInjections, enableHitOr, tdac, true, true);
    }

private:
    template <class Device>
    void UpdatePixelConfigUniform(Device* device, bool enable, bool enableInjections, bool enableHitOr, uint8_t tdac, bool updateMasks, bool updateTdac) {
        setup(device);
        BitVector<uint16_t> cmdStream;

        uint8_t masks = bits::pack<1, 1, 1>(enableHitOr, enableInjections, enable);

        tdac = Flavor::encodeTDAC(tdac);
        std::vector<uint16_t> maskData = std::vector<uint16_t>(RD53B::nRows, masks << 5 | masks);
        std::vector<uint16_t> tdacData = std::vector<uint16_t>(RD53B::nRows, tdac << 5 | tdac);

        if (updateMasks && !updateTdac)
            StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::PIX_MODE.address, uint16_t{5});
                
        if (!updateMasks && updateTdac)
            StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::PIX_MODE.address, uint16_t{7});

        for (uint16_t colPair = 0; colPair < 4; ++colPair) {
            StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::REGION_COL.address, colPair);

            if (updateMasks) {
                if (updateTdac)
                    StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::PIX_MODE.address, uint16_t{5});
                StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::REGION_ROW.address, uint16_t{0});
                StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                StreamCommand<RD53BCmd::WrRegLong>(device, cmdStream, maskData);
                
                for (int i = 0; i < 6; ++i) {
                    StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                    StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                    for (int j = 0; j < 32; ++j) 
                        StreamCommand<RD53BCmd::PLLlock>(device, cmdStream);
                }
            }

            if (updateTdac) {
                if (updateMasks)
                    StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::PIX_MODE.address, uint16_t{7});

                StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                for (int j = 0; j < 32; ++j) 
                    StreamCommand<RD53BCmd::PLLlock>(device, cmdStream);
                StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                StreamCommand<RD53BCmd::Sync>(device, cmdStream);

                StreamCommand<RD53BCmd::WrReg>(device, cmdStream, Reg::REGION_ROW.address, uint16_t{0});

                StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                StreamCommand<RD53BCmd::Sync>(device, cmdStream);

                StreamCommand<RD53BCmd::WrRegLong>(device, cmdStream, tdacData);

                 for (int i = 0; i < 4; ++i) {
                    StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                    StreamCommand<RD53BCmd::Sync>(device, cmdStream);
                    for (int j = 0; j < 32; ++j) 
                        StreamCommand<RD53BCmd::PLLlock>(device, cmdStream);
                }
            }
            
            StreamCommand<RD53BCmd::Sync>(device, cmdStream);
            StreamCommand<RD53BCmd::Sync>(device, cmdStream);
            
        }
        if (cmdStream.blocks().size()) {
            SendCommandStream(device, cmdStream);
            // cmdStream.clear();
        }
    }

    void UpdatePixelConfig(
        Chip* chip,
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>* enable, 
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>* enableInjections, 
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>* enableHitOr,
        const Ph2_HwDescription::pixel_matrix_t<Flavor, bool>* tdac, 
        bool updateMasks, 
        bool updateTdac
    ) {
        setup(chip);

        auto pixelConfig = static_cast<RD53B*>(chip)->pixelConfig();

        size_t pixMode = ReadReg(chip, Reg::PIX_MODE);

        BitVector<uint16_t> cmdStream;

        if (updateMasks && !updateTdac)
            StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::PIX_MODE.address, uint16_t{1});
                
        if (!updateMasks && updateTdac)
            StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::PIX_MODE.address, uint16_t{3});

        for (uint16_t colPair = 0; colPair < RD53B::nCols / 2; ++colPair) {
            const uint16_t col = colPair * 2;

            StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::REGION_COL.address, colPair);
            
            if (updateMasks) {
                StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::REGION_ROW.address, uint16_t{0});
                if (updateTdac)
                    StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::PIX_MODE.address, uint16_t{1});

                auto actuallyEnable = *enable && pixelConfig.enable;
                auto actuallyEnableInjections = *enableInjections && pixelConfig.enableInjections;
                auto actuallyEnableHitOr = *enableHitOr && pixelConfig.enableHitOr;

                xt::xarray<uint16_t> mask_data = xt::col(actuallyEnable, col + 1);
                mask_data |= xt::left_shift(xt::col(actuallyEnableInjections, col + 1), 1);
                mask_data |= xt::left_shift(xt::col(actuallyEnableHitOr, col + 1),  2);
                mask_data = xt::left_shift(mask_data, 5);
                mask_data |= xt::col(actuallyEnable, col);
                mask_data |= xt::left_shift(xt::col(actuallyEnableInjections, col), 1);
                mask_data |= xt::left_shift(xt::col(actuallyEnableHitOr, col), 2);

                StreamCommand<RD53BCmd::WrRegLong>(chip, cmdStream, std::vector<uint16_t>(mask_data.begin(), mask_data.end()));
            }

            if (updateTdac) {
                StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::REGION_ROW.address, uint16_t{0});
                if (updateMasks)
                    StreamCommand<RD53BCmd::WrReg>(chip, cmdStream, Reg::PIX_MODE.address, uint16_t{3});
                
                xt::xarray<uint16_t> tdac_data = xt::left_shift(RD53B::encodeTDAC(xt::col(*tdac, col + 1)) & 0x1F, 5);
                tdac_data |= RD53B::encodeTDAC(xt::col(*tdac, col)) & 0x1F;

                StreamCommand<RD53BCmd::WrRegLong>(chip, cmdStream, std::vector<uint16_t>(tdac_data.begin(), tdac_data.end()));
            }

            for (int i = 0; i < 6; ++i) {
                StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
                for (int j = 0; j < 32; ++j) 
                    StreamCommand<RD53BCmd::PLLlock>(chip, cmdStream);
            }
            StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
            StreamCommand<RD53BCmd::Sync>(chip, cmdStream);
        }
        
        if (cmdStream.blocks().size())
            SendCommandStream(chip, cmdStream);

        WriteReg(chip, Reg::PIX_MODE, pixMode);

        // auto pixMode = pixModeValues.begin();
        // RD53BUtils::for_each_device<Chip>(device, [&] (Chip* chip) {
        //     auto& chipConfig = static_cast<RD53B*>(chip)->pixelConfig();
        //     if (&cfg != &chipConfig)
        //         chipConfig = cfg;
        //     WriteReg(chip, Reg::PIX_MODE, *pixMode);
        //     ++pixMode;
        // });
    }

public:
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
    
    template <class Cmd, class Device, class... Args>
    size_t StreamCommand(Device* destination, BitVector<uint16_t>& bits, Args&&... args) {
        auto& fwInterface = setup(destination);
        auto size = bits.blocks().size();
        SerializeCommand<Cmd>(destination, bits, std::forward<Args>(args)...);
        // if (size % 2048 > 1024 && bits.blocks().size() % 2048 < 1024) {
        //     for (int i = 0; i < 6; ++i) {
        //         StreamCommand<RD53BCmd::Sync>(destination, bits);
        //         StreamCommand<RD53BCmd::Sync>(destination, bits);
        //         for (int j = 0; j < 32; ++j) 
        //             StreamCommand<RD53BCmd::PLLlock>(destination, bits);
        //     }
        //     StreamCommand<RD53BCmd::Sync>(destination, bits);
        //     StreamCommand<RD53BCmd::Sync>(destination, bits);
        // }
        // RD53BCmd::serialize<Cmd>(bits, RD53B::ChipIdFor(destination), std::forward<Args>(args)...);
        if (bits.blocks().size() > 60000) {
            fwInterface.WriteChipCommand(
                bits.blocks().begin(),
                bits.blocks().begin() + size,
                destination->getHybridId()
            );
            bits.erase_blocks(bits.blocks().begin(), bits.blocks().begin() + size);
            return size;
        }
        return 0;
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
    void UpdateCoreColRegs(
        Chip* chip, 
        const std::string& prefix, 
        const std::array<bool, RD53B::nCoreCols>& core_col_en
    );

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
    uint32_t ReadChipADC(ReadoutChip* pChip, const std::string& observableName);

    // private:
    // uint32_t getADCobservable(const std::string& observableName, bool* isCurrentNotVoltage);
    // uint32_t measureADC(ReadoutChip* pChip, uint32_t data);
    // float    measureVoltageCurrent(ReadoutChip* pChip, uint32_t data, bool isCurrentNotVoltage);
    // float    measureTemperature(ReadoutChip* pChip, uint32_t data);
    // float    convertADC2VorI(ReadoutChip* pChip, uint32_t value, bool isCurrentNotVoltage = false);
};

} // namespace Ph2_HwInterface

#endif
