/*!
  \file                  RD53B.cc
  \brief                 RD53B implementation class, config of the RD53B
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "RD53B.h"

#include <boost/filesystem.hpp>

#include "../Utils/xtensor/xcsv.hpp"
#include "../Utils/xtensor/xio.hpp"

namespace Ph2_HwDescription
{

namespace RD53BFlavor {
    constexpr char ATLAS::name[];
    constexpr char CMS::name[];
}

template <> template <>
uint8_t RD53B<RD53BFlavor::ATLAS>::ChipIdFor<Chip>(const Chip* chip) { return chip->getId(); }

template <> template <>
uint8_t RD53B<RD53BFlavor::CMS>::ChipIdFor<Chip>(const Chip* chip) { return chip->getId(); }

template <class Flavor>
RD53B<Flavor>::RD53B(uint8_t pBeId, uint8_t pFMCId, uint8_t pHybridId, uint8_t pRD53Id, uint8_t pRD53Lane, const std::string& fileName) 
  : RD53Base(pBeId, pFMCId, pHybridId, pRD53Id, pRD53Lane)
  , coreColEnable(nCols / 8, true)
  , coreColEnableInjections(nCols / 8, true)
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
void RD53B<Flavor>::setDefaultState() {
    for (const auto& reg : Regs)
        registerValues[reg.address] = reg.defaultValue;
}

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
        config = toml::parse<toml::preserve_comments, tsl::ordered_map>(fileName);

        if (config.contains("Registers"))
            for (const auto& key_value : config.at("Registers").as_table())
                configureRegister(key_value.first, key_value.second.as_integer());

        if (config.contains("CoreColumns")) {
            auto coreCols = config["CoreColumns"];

            if (coreCols.contains("disable"))
                for (const auto& range : toml::get<std::vector<std::array<size_t, 2>>>(coreCols["disable"]))
                    std::fill_n(coreColEnable.begin() + range[0], std::min(range[1], coreColEnable.size()) - range[0], false);
                    
            if (coreCols.contains("disableInjections"))
                for (const auto& range : toml::get<std::vector<std::array<size_t, 2>>>(coreCols["disableInjections"]))
                    std::fill_n(coreColEnableInjections.begin() + range[0], std::min(range[1], coreColEnableInjections.size()) - range[0], false);
        }

        if (config.contains("Pixels")) {
            auto&& pixelsConfig = config.at("Pixels").as_table();
            pixelConfigFields().for_each([&] (const auto& fieldName, auto ptr) {
                auto it = pixelsConfig.find(fieldName.value);
                if (it != pixelsConfig.end()) {
                    if (it->second.is_integer())
                        (pixelConfig.*ptr).fill(it->second.as_integer());
                    else {
                        std::string csvFileName = it->second.as_string();
                        // pixelConfigFileNames[fieldName.value] = csvFileName;
                        if (boost::filesystem::exists(csvFileName)) {
                            std::ifstream csvFile(csvFileName);
                            auto data = xt::load_csv<double>(csvFile);
                            if (data.size() == 1)
                                (pixelConfig.*ptr).fill(data.flat(0));
                            else
                                pixelConfig.*ptr = data;
                        }
                    }
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
    
    // toml::table register_table;
    for (size_t i = 0; i < Regs.size(); ++i) {
        if (Regs[i].type == RD53BConstants::RegType::ReadWrite && registerValues[i]) {
            config["Registers"][Regs[i].name] = *registerValues[i];
            // register_table.insert({Regs[i].name, *registerValues[i]});
        }
    }
    
    // toml::table pixels_table;
    if (config.contains("Pixels")) {
        pixelConfigFields().for_each([&] (const auto& fieldName, auto ptr) {
            // LOG(INFO) << "save pixel config: " << fieldName.value;
            // auto it = pixelConfigFileNames.find(fieldName.value);
            auto data = pixelConfig.*ptr;
            if (config["Pixels"].contains(fieldName.value)) {
                if (xt::all(xt::equal(pixelConfig.*ptr, (pixelConfig.*ptr).flat(0)))) {
                    config["Pixels"][fieldName.value] = (int)(pixelConfig.*ptr).flat(0);
                }
                else {
                    std::ostringstream csvFileNameStream;
                    if (config["Pixels"][fieldName.value].is_string())
                        csvFileNameStream.str(config["Pixels"][fieldName.value].as_string());
                    else
                        csvFileNameStream << fieldName.value << ".csv";
                    std::ofstream out_file(csvFileNameStream.str());
                    xt::dump_csv(out_file, xt::cast<int>(pixelConfig.*ptr));
                    config["Pixels"][fieldName.value] = csvFileNameStream.str();
                }
                // pixels_table.insert({fieldName.value, it->second});
            }
            // else {
            //     auto firstPixel = (pixelConfig.*ptr)(0, 0);
            //     if (xt::all(pixelConfig.*ptr == firstPixel)) {
            //         config["Pixels"].as_table().insert({fieldName.value, firstPixel});
            //     }
            // }
        });
    }

    // file << toml::value(toml::table({
    //     {"Registers", std::move(register_table)},
    //     {"Pixels", std::move(pixels_table)}
    // }));
    file << config;
}

template class RD53B<RD53BFlavor::ATLAS>;
template class RD53B<RD53BFlavor::CMS>;

} // namespace RD53Cmd
