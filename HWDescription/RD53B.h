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
#include "../Utils/xtensor/xvectorize.hpp"

#include "../Utils/NamedTuple.h"
#include "../Utils/toml.hpp"
#include "../Utils/tsl/ordered_map.h"

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
        static constexpr auto& IMuxMap = RD53BConstants::AtlasIMuxMap;
        static constexpr auto& VMuxMap = RD53BConstants::AtlasVMuxMap;

        using IMux = RD53BConstants::AtlasIMux;
        using VMux = RD53BConstants::AtlasVMux;

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

        static constexpr uint16_t globalPulseUnit = 2;
    };

    struct CMS {
        static constexpr Flavor flavor = Flavor::CMS;
        static constexpr size_t nRows = 336;
        static constexpr size_t nCols = 432;
        static constexpr auto& IMuxMap = RD53BConstants::CmsIMuxMap;
        static constexpr auto& VMuxMap = RD53BConstants::CmsVMuxMap;

        using IMux = RD53BConstants::CmsIMux;
        using VMux = RD53BConstants::CmsVMux;

        using Reg = RD53BConstants::CMSRegisters;

        static constexpr FrontEndType feType = FrontEndType::CROC;

        static constexpr char name[] = "CMS";

        static constexpr uint8_t encodeTDAC(uint8_t value) { return value; }
        static constexpr uint8_t decodeTDAC(uint8_t code) { return code; }

        static constexpr uint16_t globalPulseUnit = 1;
    };
    
}

template <class Flavor, class T>
using pixel_matrix_t = xt::xtensor_fixed<T, xt::xshape<Flavor::nRows, Flavor::nCols>>;

template <class Flavor, class T>
using ccol_array_t = std::array<T, Flavor::nCols / 8>;

struct ChipLocation {
    ChipLocation(Chip* pChip) : board_id(pChip->getBeBoardId()), hybrid_id(pChip->getHybridId()), chip_id(pChip->getId()) {}

    uint16_t board_id;
    uint16_t hybrid_id;
    uint16_t chip_id;

    size_t hash() const { return (board_id << 16) | (hybrid_id << 8) | chip_id; }

    friend bool operator<(const ChipLocation& l, const ChipLocation& r ) {
        return l.hash() < r.hash();
    }

    friend std::ostream& operator<<(std::ostream& os, const ChipLocation& loc) {
        return (os << "{ board: " << loc.board_id << ", hybrid: " << loc.hybrid_id << ", chip: " << loc.chip_id << " }");
    }
};


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
    static constexpr size_t nCoreCols = nCols / 8;
    static constexpr uint8_t BroadcastId = 0b11111;

    static constexpr const auto& Regs = Reg::Regs;
    static constexpr const auto& vRegs = Reg::vRegs;

    static constexpr auto& GlobalPulseRoutes = RD53BConstants::GlobalPulseRoutes;
    
    static constexpr auto& IMuxMap = Flavor::IMuxMap;
    static constexpr auto& VMuxMap = Flavor::VMuxMap;

    using IMux = typename Flavor::IMux;
    using VMux = typename Flavor::VMux;

    template <class T>
    using PixelMatrix = pixel_matrix_t<Flavor, T>;

    template <class T>
    using CoreColArray = ccol_array_t<Flavor, T>;

    static const auto& pixelConfigDefauls() {
        static const auto pixelConfigDefauls = std::map<std::string, uint8_t>({
            {"enable", 1},
            {"enableInjections", 1},
            {"enableHitOr", 1},
            {"tdac", 15}
        });
        return pixelConfigDefauls;
    }

    struct PixelConfig {
        PixelMatrix<bool> enable = xt::ones<bool>({nRows, nCols});
        PixelMatrix<bool> enableInjections = xt::ones<bool>({nRows, nCols});
        PixelMatrix<bool> enableHitOr = xt::ones<bool>({nRows, nCols});
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
    static auto encodeTDAC(T&& data) {
        static const auto f = xt::vectorize(Flavor::encodeTDAC);
        return f(std::forward<T>(data));
    }
    
    template <class T>
    static auto decodeTDAC(T&& data) {
        static const auto f = xt::vectorize(Flavor::decodeTDAC);
        return f(std::forward<T>(data));
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

    void setDefaultState();

    void     loadfRegMap(const std::string& fileName) override;
    void     saveRegMap(const std::string& fName2Add) override;
    uint32_t getNumberOfChannels() const override;
    bool     isDACLocal(const std::string& regName) override;
    uint8_t  getNumberOfBits(const std::string& regName) override;

    const auto& getConfigFileName() const { return configFileName; }

    const auto& getConfig() const { return config; }    

    void configureRegister(std::string name, size_t value) {
        registerConfig[name] = value;
    }

    const PixelConfig& pixelConfig() const { return _pixelConfig; }
    PixelConfig& pixelConfig() { return _pixelConfig; }

    // PixelConfig defaultPixelConfig;

    CoreColArray<bool> coreColEnable;
    CoreColArray<bool> coreColEnableInjections;

  private:
    PixelConfig _pixelConfig;
    std::array<boost::optional<uint16_t>, 256> registerValues;
    std::unordered_map<std::string, size_t> registerConfig;
    std::string configFileName;
    toml::basic_value<toml::preserve_comments, tsl::ordered_map> config;
    // std::map<std::string, std::string> pixelConfigFileNames;
    uint16_t currentRow = 0;
};

} // namespace Ph2_HwDescription



#endif
