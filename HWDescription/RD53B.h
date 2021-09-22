/*!
  \file                  RD53B.h
  \brief                 RD53B description class, config of the RD53B
  \author                Alkis Papadopoulos
  \version               1.0
  \date                  19/9/2021
*/

#ifndef RD53B_H
#define RD53B_H

#include "../Utils/ConsoleColor.h"
#include "../Utils/RD53Shared.h"
#include "../Utils/bit_packing.h"
#include "../Utils/easylogging++.h"
#include "RD53Base.h"

#include "RD53BConstants.h"
#include "RD53BRegisters.h"

#include <iomanip>


namespace Ph2_HwDescription
{

class RD53B : public RD53Base
{
  public:
    static constexpr size_t nRows = 384;
    static constexpr size_t nCols = 400;

    static constexpr uint8_t BroadcastId = 0b11111;

    template <class T>
    static uint8_t ChipIdFor(const T* device) { return BroadcastId; }

    using Register = RD53BReg::Register;

    static const decltype(RD53BReg::Registers)& Registers;
    static const decltype(RD53BReg::RegisterIndexMap)& RegisterIndexMap;

    static constexpr auto& GlobalPulseRoutes = RD53BConstants::GlobalPulseRoutes;
    static constexpr auto& IMUX = RD53BConstants::IMUX;
    static constexpr auto& VMUX = RD53BConstants::VMUX;


    static size_t getRegIndex(size_t index) {
        return index;
    }

    static size_t getRegIndex(const Register& reg) {
        return &reg - &Registers[0];
    }

    // static size_t getRegIndex(const char* name) {
    //     return RegisterIndexMap().at(name);
    // }

    static size_t getRegIndex(const std::string& name) {
        return RegisterIndexMap.at(name);
    }


    template <class Key>
    static const Register& Reg(Key&& key) {
        return Registers[getRegIndex(std::forward<Key>(key))];
    }


    RD53B(uint8_t pBeId, uint8_t pFMCId, uint8_t pHybridId, uint8_t pRD53Id, uint8_t pRD53Lane, const std::string& fileName);
    RD53B(const RD53B& chipObj);

    void     loadfRegMap(const std::string& fileName) override;
    void     saveRegMap(const std::string& fName2Add) override;
    uint32_t getNumberOfChannels() const override;
    bool     isDACLocal(const std::string& regName) override;
    uint8_t  getNumberOfBits(const std::string& regName) override;


    // get/set register values

    template <class Key>
    uint16_t getRegValue(Key&& key) const {
        return registerValues[getRegIndex(std::forward<Key>(key))];
    }

    // uint16_t getRegValue(Reg reg) const {
    //     return registerValues[int(reg)];
    // }
    
    // uint16_t getRegValue(const char* name) const {
    //     return registerValues[RegisterIndexMap.at(name)];
    // }

    // uint16_t getRegValue(const std::string& name) const {
    //     return registerValues[RegisterIndexMap.at(name.c_str())];
    // }

    template <class Key>
    void setRegValue(Key&& key, uint16_t value) {
        auto& reg = Reg(std::forward<Key>(key));
        registerValues[getRegIndex(reg)] = value;
        if (reg == RD53BReg::PIX_PORTAL) {
            if (getRegField(RD53BReg::PIX_MODE, 2))
                ++currentRow;
        }
        else if (reg == RD53BReg::REGION_ROW) {
            currentRow = value;
        }
    }

    // void setRegValue(const Register& reg, uint16_t value) {
    //     registerValues[&reg - &Registers[0]] = value;
    // }

    // void setRegValue(Reg reg, uint16_t value) {
    //     registerValues[int(reg)] = value;
    // }
    
    // void setRegValue(const char* name, uint16_t value) {
    //     registerValuesRegisterIndexMap.at(name)] = value;
    // }

    // void setRegValue(const std::string& name, uint16_t value) {
    //     registerValues[RegisterIndexMap.at(name.c_str())] = value;
    // }



    // get/set register fields

    
    template <class Key>
    uint16_t getRegField(Key&& key, size_t field_index) const {
        auto& reg = Reg(std::forward<Key>(key));
        int offset = std::accumulate(reg.fieldSizes.begin(), reg.fieldSizes.begin() + field_index, 0);
        uint16_t mask = (1 << reg.fieldSizes[field_index]) - 1;
        return (registerValues[getRegIndex(reg)] >> offset) & mask;
    }

    // uint16_t getRegField(const Register& reg, size_t field_index) const {
    //     return getRegField(int(reg));
    // }

    // uint16_t getRegField(Reg reg, size_t field_index) const {
    //     return getRegField(int(reg));
    // }
    
    // uint16_t getRegField(const char* reg_name, size_t field_index) const {
    //     return getRegField(RegisterIndexMap.at(name));
    // }

    // uint16_t getRegField(const std::string& reg_name, size_t field_index) const {
    //     return getRegField(RegisterIndexMap.at(name.c_str()));
    // }

    template <class Key>
    uint16_t setRegField(Key&& key, size_t field_index, uint16_t value) {
        size_t reg_index = getRegIndex(std::forward<Key>(key));
        auto& reg = Registers[reg_index];
        int offset = std::accumulate(reg.fieldSizes.begin(), reg.fieldSizes.begin() + field_index, 0);
        uint16_t mask = ((1 << reg.fieldSizes[field_index]) - 1) << offset;
        auto regValue = registerValues[reg_index];
        regValue ^= ((regValue ^ (value << offset)) & mask);
        setRegValue(key, regValue); // https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge
        return regValue;
    }

    // void setRegField(Reg reg, size_t field_index, uint16_t value) {
    //     setRegField(int(reg), field_index, value));
    // }
    
    // void setRegField(const char* reg_name, size_t field_index, uint16_t value) {
    //     setRegField(RegisterIndexMap.at(name), field_index, value));
    // }

    // void setRegField(const std::string& reg_name, size_t field_index, uint16_t value) {
    //     setRegField(RegisterIndexMap.at(name.c_str()), field_index, value));
    // }

    uint16_t getCurrentRow() const { return currentRow; }

  private:
    std::array<uint16_t, RD53BReg::nRegs> registerValues;
    std::string configFileName;
    uint16_t currentRow = 0;
};

} // namespace Ph2_HwDescription



#endif
