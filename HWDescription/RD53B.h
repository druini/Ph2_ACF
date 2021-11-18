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

#include "RD53BATLASRegisters.h"
#include "RD53BCMSRegisters.h"

#include <iomanip>

#include "../Utils/xtensor/xfixed.hpp"

#include "../Utils/NamedTuple.h"

#include <boost/optional.hpp>



namespace Ph2_HwInterface {

template <class>
class RD53BInterface;

}

namespace Ph2_HwDescription
{

namespace RD53BFlavor {

    enum class Flavor {
        ATLAS,
        CMS
    };

    struct ATLAS {
        static constexpr Flavor flavor = Flavor::ATLAS;
        static constexpr size_t nRows = 384;
        static constexpr size_t nCols = 400;

        using Reg = RD53BConstants::ATLASRegisters;

        static constexpr FrontEndType feType = FrontEndType::RD53B; 

        static constexpr char name[] = "ATLAS";

        static constexpr uint8_t encodeTDAC(uint8_t value) { 
            if (value < 16)
                return 1 << 4 | ~(value & 0xF);
            else
                return value & 0xF;
        }

        static constexpr uint8_t decodeTDAC(uint8_t code) { 
            bool sign = code & 0x10; 
            uint8_t value = code & 0xF;
            return sign ? 16 + value : ~value;
        }
    };

    struct CMS {
        static constexpr Flavor flavor = Flavor::CMS;
        static constexpr size_t nRows = 336;
        static constexpr size_t nCols = 432;

        using Reg = RD53BConstants::CMSRegisters;

        static constexpr FrontEndType feType = FrontEndType::CROC;

        static constexpr char name[] = "CMS";

        static constexpr uint8_t encodeTDAC(uint8_t value) { return value; }
        static constexpr uint8_t decodeTDAC(uint8_t code) { return code; }
    };
    
}

template <class Flavor, class T>
using pixel_matrix_t = xt::xtensor_fixed<T, xt::xshape<Flavor::nRows, Flavor::nCols>>;

template <class Flavor>
class RD53B : public RD53Base
{
public:
    friend class Ph2_HwInterface::RD53BInterface<Flavor>;
    using flavor = Flavor;

    using Reg = typename Flavor::Reg;
    using Register = RD53BConstants::Register;

    static constexpr size_t nRows = Flavor::nRows;
    static constexpr size_t nCols = Flavor::nCols;
    static constexpr uint8_t BroadcastId = 0b11111;

    static constexpr const auto& Regs = Reg::Regs;
    static constexpr const auto& vRegs = Reg::vRegs;

    static const decltype(RD53BConstants::GetGlobalPulseRoutes()) GlobalPulseRoutes;
    static const decltype(RD53BConstants::GetIMUX()) IMUX;
    static const decltype(RD53BConstants::GetVMUX()) VMUX;

    template <class T>
    using PixelMatrix = pixel_matrix_t<Flavor, T>;

    struct PixelConfig {
        PixelMatrix<bool> enable = xt::zeros<bool>({nRows, nCols});
        PixelMatrix<bool> enableInjections = xt::zeros<bool>({nRows, nCols});
        PixelMatrix<bool> enableHitOr = xt::zeros<bool>({nRows, nCols});
        PixelMatrix<uint8_t> tdac = 15 * xt::ones<uint8_t>({nRows, nCols});
    };

    static const auto& pixelConfigFields() {
        using namespace compile_time_string_literal;
        static const auto pixelConfigFields = NamedTuple::make_named_tuple(
            std::make_pair("enable"_s, &PixelConfig::enable),
            std::make_pair("enableInjections"_s, &PixelConfig::enableInjections),
            std::make_pair("enableHitOr"_s, &PixelConfig::enableHitOr),
            std::make_pair("tdac"_s, &PixelConfig::tdac)
        );
        return pixelConfigFields;
    }

    template <class T>
    static uint8_t ChipIdFor(const T* device) { return BroadcastId; }

    static const Register& getRegister(const std::string& name) {
        const auto& fields = vRegs.at(name);
        if (fields.size() > 1 || fields[0].size < fields[0].reg.size)
            throw std::runtime_error(name + " is not an actual register.");
        return fields[0].reg;
    }

    RD53B(uint8_t pBeId, uint8_t pFMCId, uint8_t pHybridId, uint8_t pRD53Id, uint8_t pRD53Lane, const std::string& fileName);
    RD53B(const RD53B& chipObj);

    void     loadfRegMap(const std::string& fileName) override;
    void     saveRegMap(const std::string& fName2Add) override;
    uint32_t getNumberOfChannels() const override;
    bool     isDACLocal(const std::string& regName) override;
    uint8_t  getNumberOfBits(const std::string& regName) override;

    void configureRegister(std::string name, size_t value) {
        registerConfig[name] = value;
    }

    PixelConfig pixelConfig;
    PixelConfig defaultPixelConfig;

    std::vector<bool> coreColEnable;
    std::vector<bool> coreColEnableInjections;

  private:
    std::array<boost::optional<uint16_t>, 256> registerValues;
    std::unordered_map<std::string, size_t> registerConfig;
    std::string configFileName;
    std::map<std::string, std::string> pixelConfigFileNames;
    uint16_t currentRow = 0;
};

} // namespace Ph2_HwDescription



#endif
