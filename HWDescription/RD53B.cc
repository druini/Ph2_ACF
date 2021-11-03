/*!
  \file                  RD53B.cc
  \brief                 RD53B implementation class, config of the RD53B
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "RD53B.h"

#include "../Utils/toml.hpp"
#include <boost/filesystem.hpp>

#include "../Utils/xtensor/xcsv.hpp"

namespace Ph2_HwDescription
{

namespace RD53BFlavor {
    constexpr char ATLAS::name[];
    constexpr char CMS::name[];
}

template <class Flavor>
const decltype(RD53BConstants::GetGlobalPulseRoutes()) RD53B<Flavor>::GlobalPulseRoutes = RD53BConstants::GetGlobalPulseRoutes();

template <class Flavor>
const decltype(RD53BConstants::GetIMUX()) RD53B<Flavor>::IMUX = RD53BConstants::GetIMUX();

template <class Flavor>
const decltype(RD53BConstants::GetVMUX()) RD53B<Flavor>::VMUX = RD53BConstants::GetVMUX();

template <> template <>
uint8_t RD53B<RD53BFlavor::ATLAS>::ChipIdFor<Chip>(const Chip* chip) { return chip->getId(); }

template <> template <>
uint8_t RD53B<RD53BFlavor::CMS>::ChipIdFor<Chip>(const Chip* chip) { return chip->getId(); }

template <class Flavor>
RD53B<Flavor>::RD53B(uint8_t pBeId, uint8_t pFMCId, uint8_t pHybridId, uint8_t pRD53Id, uint8_t pRD53Lane, const std::string& fileName) 
  : RD53Base(pBeId, pFMCId, pHybridId, pRD53Id, pRD53Lane)
{
    fMaxRegValue      = 0xFFFF;
    fChipOriginalMask = new ChannelGroup<nRows, nCols>;
    configFileName    = fileName;
    setFrontEndType(Flavor::feType);
    
    RD53B::loadfRegMap(configFileName);

    defaultPixelConfig = pixelConfig;
}

template <class Flavor>
RD53B<Flavor>::RD53B(const RD53B& chipObj) : RD53Base(chipObj) {}


template <class Flavor>
uint32_t RD53B<Flavor>::getNumberOfChannels() const { return nRows * nCols; }

template <class Flavor>
bool RD53B<Flavor>::isDACLocal(const std::string& regName)
{
    if(regName != "PIX_PORTAL") return false;
    return true;
}

template <class Flavor>
uint8_t RD53B<Flavor>::getNumberOfBits(const std::string& regName)
{
    auto it = fRegMap.find(regName);
    if(it == fRegMap.end()) return 0;
    return it->second.fBitSize;
}


template <class Flavor>
void RD53B<Flavor>::loadfRegMap(const std::string& fileName)
{
    if (boost::filesystem::exists(fileName)) {
        auto data = toml::parse(fileName);

        if (data.contains("Registers")) {
            for (const auto& key_value : data.at("Registers").as_table()) {
                configureRegister(key_value.first, key_value.second.as_integer());
            }
        }

        if (data.contains("Pixels")) {
            auto&& pixelsConfig = data.at("Pixels").as_table();
            pixelConfigFields().for_each([&] (const auto& fieldName, auto ptr) {
                auto it = pixelsConfig.find(fieldName.value);
                if (it != pixelsConfig.end()) {
                    auto fileName = it->second.as_string();
                    pixelConfigFileNames[fieldName.value] = fileName;
                    std::ifstream csvFile(fileName);
                    pixelConfig.*ptr = xt::load_csv<double>(csvFile);
                }
            });
        }
    }
}

template <class Flavor>
void RD53B<Flavor>::saveRegMap(const std::string& fName2Add)
{
    std::string fileName = configFileName;
    fileName.insert(fileName.rfind('/'), fName2Add);
    std::ofstream file(fileName);
    
    toml::table register_table;
    for (size_t i = 0; i < Regs.size(); ++i) {
        if (Regs[i].type == RD53BConstants::RegType::ReadWrite && registerValues[i]) {
            register_table.insert({Regs[i].name, *registerValues[i]});
        }
    }
    
    toml::table pixels_table;
    pixelConfigFields().for_each([&] (const auto& fieldName, auto ptr) {
        auto it = pixelConfigFileNames.find(fieldName.value);
        if (it != pixelConfigFileNames.end()) {
            std::ofstream out_file(it->second);
            xt::dump_csv(out_file, pixelConfig.*ptr);
            pixels_table.insert({fieldName.value, it->second});
        }
    });

    file << toml::value(toml::table({
        {"Registers", std::move(register_table)},
        {"Pixels", std::move(pixels_table)}
    }));
}

template class RD53B<RD53BFlavor::ATLAS>;
template class RD53B<RD53BFlavor::CMS>;

} // namespace RD53Cmd
