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
#include "CROCRegisters.h"

#include <iomanip>

#include "../Utils/Matrix.hpp"


namespace Ph2_HwDescription
{

namespace RD53BFlavor {

struct ATLAS { 
    static constexpr size_t nRows = 384;
    static constexpr size_t nCols = 400;

    using Reg = RD53BReg;

    static constexpr FrontEndType feType = FrontEndType::RD53B; 

    static constexpr char name[] = "ATLAS";
};

struct CMS {
    static constexpr size_t nRows = 336;
    static constexpr size_t nCols = 432;

    using Reg = CROCReg;

    static constexpr FrontEndType feType = FrontEndType::CROC;

    static constexpr char name[] = "CMS";
};

}

template <class Flavor>
class RD53B : public RD53Base
{
public:
    static constexpr size_t nRows = Flavor::nRows;
    static constexpr size_t nCols = Flavor::nCols;

    static constexpr uint8_t BroadcastId = 0b11111;

    using Reg = typename Flavor::Reg;
    using Register = typename Reg::Register;

    static const decltype(Reg::GetRegisters()) Registers;
    static const decltype(Reg::GetRegisterIndexMap()) RegisterIndexMap;

    static const decltype(RD53BConstants::GetGlobalPulseRoutes()) GlobalPulseRoutes;
    static const decltype(RD53BConstants::GetIMUX()) IMUX;
    static const decltype(RD53BConstants::GetVMUX()) VMUX;

    template <class T>
    using PixelMatrix = Matrix<T, nRows, nCols>;

    struct PixelConfig {
        PixelMatrix<bool> enable;
        PixelMatrix<bool> enableInjections;
        PixelMatrix<bool> enableHitOr;
        PixelMatrix<uint8_t> tdac;
        PixelMatrix<bool> tdacSign;
    };

    template <class T>
    static uint8_t ChipIdFor(const T* device) { return BroadcastId; }

    static size_t getRegIndex(size_t index) {
        return index;
    }

    static size_t getRegIndex(const Register& reg) {
        return &reg - &Registers[0];
    }


    static size_t getRegIndex(const std::string& name) {
        return RegisterIndexMap.at(name);
    }


    template <class Key>
    static const Register& getRegister(Key&& key) {
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

    template <class Key>
    void setRegValue(Key&& key, uint16_t value) {
        auto& reg = getRegister(std::forward<Key>(key));
        registerValues[getRegIndex(reg)] = value;
        if (reg == RD53BReg::PIX_PORTAL) {
            if (getRegField(RD53BReg::PIX_MODE, 2))
                ++currentRow;
        }
        else if (reg == RD53BReg::REGION_ROW) {
            currentRow = value;
        }
    }

    // get/set register fields
    template <class Key>
    uint16_t getRegField(Key&& key, size_t field_index) const {
        auto& reg = getRegister(std::forward<Key>(key));
        int offset = std::accumulate(reg.fieldSizes.begin(), reg.fieldSizes.begin() + field_index, 0);
        uint16_t mask = (1 << reg.fieldSizes[field_index]) - 1;
        return (registerValues[getRegIndex(reg)] >> offset) & mask;
    }

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

    uint16_t getCurrentRow() const { return currentRow; }

    PixelConfig pixelConfig;
    PixelConfig defaultPixelConfig;

  private:
    std::array<uint16_t, Reg::nRegs> registerValues;
    std::string configFileName;
    uint16_t currentRow = 0;
};

} // namespace Ph2_HwDescription



#endif
