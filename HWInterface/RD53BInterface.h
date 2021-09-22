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

// #############
// # CONSTANTS #
// #############
#define VCALSLEEP 50000 // [microseconds]

namespace Ph2_HwInterface
{

class RD53BInterface : public RD53InterfaceBase
{
    using Chip = Ph2_HwDescription::Chip;
    using ReadoutChip = Ph2_HwDescription::ReadoutChip;
    using BeBoard = Ph2_HwDescription::BeBoard;
    using Hybrid = Ph2_HwDescription::Hybrid;
    using RD53B = Ph2_HwDescription::RD53B;

public:
    RD53BInterface(const BeBoardFWMap& pBoardMap);

    // #############################
    // # Override member functions #
    // #############################
    bool     ConfigureChip(Chip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override;
    bool     WriteChipReg(Chip* pChip, const std::string& regName, uint16_t data, bool pVerifLoop = true) override;
    void     WriteBoardBroadcastChipReg(const BeBoard* pBoard, const std::string& regName, uint16_t data) override;
    bool     WriteChipAllLocalReg(ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue, bool pVerifLoop = true) override { return true; }
    void     ReadChipAllLocalReg(ReadoutChip* pChip, const std::string& regName, ChipContainer& pValue) override {}
    uint16_t ReadChipReg(Chip* pChip, const std::string& regName) override;
    bool     ConfigureChipOriginalMask(ReadoutChip* pChip, bool pVerifLoop = true, uint32_t pBlockSize = 310) override { return true; }
    bool     MaskAllChannels(ReadoutChip* pChip, bool mask, bool pVerifLoop = true) override { return true; }
    bool     maskChannelsAndSetInjectionSchema(ReadoutChip* pChip, const ChannelGroupBase* group, bool mask, bool inject, bool pVerifLoop = false) override  { return true; }

    void WriteReg(Chip* chip, const RD53B::Register& reg, uint16_t value);
    void WriteReg(const Hybrid* chip, const RD53B::Register& reg, uint16_t value);
    void WriteReg(const BeBoard* board, const RD53B::Register& reg, uint16_t value);

    void WriteRegField(RD53B* chip, const RD53B::Register& reg, size_t field, uint16_t value) {
        uint16_t newValue = chip->setRegField(reg, field, value);
        WriteReg(chip, reg, newValue);
    }

    void ChipErrorReport(ReadoutChip* pChip);

    void InitRD53Downlink(const BeBoard* pBoard);
    void InitRD53Uplinks(ReadoutChip* pChip, int nActiveLanes = 1);

private:
    template <class CmdType>
    BitVector<uint16_t> SerializeCommand(const CmdType& cmdType, BitSerialization::value_type_t<CmdType>& cmdValue) {
        BitVector<uint16_t> bits;
        auto result = cmdType.serialize(cmdValue, bits);
        if (!result) {
            std::stringstream ss;
            ss << "Command serialization error: " << result.error();
            throw std::runtime_error(ss.str());
        }
        return bits;
    }

public:
    template <class Device, class CmdType, typename std::enable_if_t<!std::is_same<Device, BeBoard>::value, int> = 0>
    void SendCommand(const Device* device, const CmdType& cmdType, BitSerialization::value_type_t<CmdType> cmdValue) {
        SendCommandStream(device->getHybridId(), SerializeCommand(cmdType, cmdValue));
    }

    template <class Device, class CmdType, class FirstArg, class... Args, typename std::enable_if_t<!RD53BCmd::isBroadcast<CmdType> && !std::is_same<Device, BeBoard>::value, int> = 0>
    void SendCommand(const Device* device, const CmdType& cmdType, FirstArg firstArg, Args... args) {
        auto cmdValue = cmdType.Create({RD53B::ChipIdFor(device), firstArg, args...});
        SendCommandStream(device->getHybridId(), SerializeCommand(cmdType, cmdValue));
    }

    template <class Device, class CmdType, typename std::enable_if_t<!RD53BCmd::isBroadcast<CmdType> && !std::is_same<Device, BeBoard>::value, int> = 0>
    void SendCommand(const Device* device, const CmdType& cmdType) {
        auto cmdValue =  cmdType.Create({RD53B::ChipIdFor(device)});
        SendCommandStream(device->getHybridId(), SerializeCommand(cmdType, cmdValue));
    }

    template <class Device, class CmdType, typename std::enable_if_t<RD53BCmd::isBroadcast<CmdType> && !std::is_same<Device, BeBoard>::value, int> = 0>
    void SendCommand(const Device* device, const CmdType& cmdType) {
        BitSerialization::value_type_t<CmdType> cmdValue{};
        SendCommandStream(device->getHybridId(), SerializeCommand(cmdType, cmdValue));
    }


    template <class CmdType>
    void SendCommand(const BeBoard* pBoard, const CmdType& cmdType, BitSerialization::value_type_t<CmdType>& cmdValue) {
        for (const auto* opticalGroup : *pBoard)
            for (const auto* hybrid : *opticalGroup)
                SendCommandStream(hybrid->getHybridId(), SerializeCommand(cmdType, cmdValue));
    }

    template <class CmdType, class FirstArg, class... Args, typename std::enable_if_t<!RD53BCmd::isBroadcast<CmdType>, int> = 0>
    void SendCommand(const BeBoard* pBoard, const CmdType& cmdType, FirstArg firstArg, Args... args) {
        auto cmdValue = cmdType.Create({RD53B::BroadcastId, firstArg, args...});
        SendCommand(pBoard, cmdType, cmdValue);
    }

    template <class CmdType, typename std::enable_if_t<!RD53BCmd::isBroadcast<CmdType>, int> = 0>
    void SendCommand(const BeBoard* pBoard, const CmdType& cmdType) {
        auto cmdValue = cmdType.Create({RD53B::BroadcastId});
        SendCommand(pBoard, cmdType, cmdValue);
    }

    template <class CmdType, typename std::enable_if_t<RD53BCmd::isBroadcast<CmdType>, int> = 0>
    void SendCommand(const BeBoard* pBoard, const CmdType& cmdType) {   
        BitSerialization::value_type_t<CmdType> cmdValue{};
        SendCommand(pBoard, cmdType, cmdValue);
    }

    void SendCommandStream(uint8_t hybridId, const BitVector<uint16_t>& cmdStream) {
        static_cast<RD53FWInterface*>(fBoardFW)->WriteChipCommand(cmdStream.blocks(), hybridId);
    }

    void SendCommandStream(ReadoutChip* pChip, const BitVector<uint16_t>& cmdStream) {
        SendCommandStream(pChip->getHybridId(), cmdStream);
    }

    template <class Device>
    void SendGlobalPulse(Device* device, const std::vector<std::string>& routes, uint8_t width = 1) {
        uint16_t route_value = 0;

        for (const auto& name : routes) 
            route_value |= (1 << RD53B::GlobalPulseRoutes.at(name));

        WriteReg(device, Ph2_HwDescription::RD53BReg::GlobalPulseConf, route_value);
        WriteReg(device, Ph2_HwDescription::RD53BReg::GlobalPulseWidth, width);
        SendCommand(device, RD53BCmd::GlobalPulse);
    }

    void StartPRBSpattern(Ph2_HwDescription::ReadoutChip* pChip) {}
    void StopPRBSpattern(Ph2_HwDescription::ReadoutChip* pChip) {}

private:
    RD53FWInterface& setup(const Chip* pChip) {
        this->setBoard(pChip->getBeBoardId());
        return *static_cast<RD53FWInterface*>(fBoardFW);
    }

    RD53FWInterface& setup(const BeBoard* pBoard) {
        this->setBoard(pBoard->getId());
        return *static_cast<RD53FWInterface*>(fBoardFW);
    }

    // ###########################
    // # Dedicated to minitoring #
    // ###########################
public:
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