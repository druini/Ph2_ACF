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
#include <boost/range/iterator_range.hpp>

#include "../Utils/xtensor/xcsv.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <regex>

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
{
    std::fill(coreColEnable.begin(), coreColEnable.end(), true);
    std::fill(coreColEnableInjections.begin(), coreColEnableInjections.end(), true);

    fMaxRegValue      = 0xFFFF;
    fChipOriginalMask = new ChannelGroup<nRows, nCols>;
    configFileName    = fileName;
    setFrontEndType(Flavor::feType);
    
    RD53B::loadfRegMap(configFileName);

    // defaultPixelConfig = pixelConfig;
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
                        (pixelConfig().*ptr).fill(it->second.as_integer());
                    else {
                        std::string csvFileName = it->second.as_string();
                        // pixelConfigFileNames[fieldName.value] = csvFileName;
                        if (boost::filesystem::exists(csvFileName)) {
                            std::ifstream csvFile(csvFileName);
                            auto data = xt::load_csv<double>(csvFile);
                            if (data.size() == 1)
                                (pixelConfig().*ptr).fill(data.flat(0));
                            else
                                pixelConfig().*ptr = data;
                        }
                    }
                }
            });
        }
    }
}

std::string getAvailablePath(const boost::filesystem::path& path) {
    if (!boost::filesystem::exists(path))
        return path.string();
    else {
        std::regex runNumberRegex(path.stem().string() + "\\(([0-9]+)\\)\\" + path.extension().string());
        std::vector<size_t> existingRunNumbers{{0}};
        for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path.parent_path()), {})) {
            if (boost::filesystem::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                std::smatch m;
                if (std::regex_match(filename, m, runNumberRegex) && m.size() > 1)
                    existingRunNumbers.push_back(std::stoul(m[1]));
            }
        }
        std::sort(existingRunNumbers.begin(), existingRunNumbers.end());
        auto it = std::adjacent_find(existingRunNumbers.begin(), existingRunNumbers.end(), [] (auto a, auto b) { return b != a + 1; });
        std::stringstream ss;
        ss <<  path.parent_path() << path.stem() << '(' << (*it + 1) << ')' << path.extension();
        return ss.str();
    }
}
    
template <class Flavor>
void RD53B<Flavor>::saveRegMap(const std::string& fName2Add)
{
    std::string fileName = configFileName;
    fileName.insert(fileName.rfind('/'), fName2Add);
    std::ofstream file(fileName);
    
    for (size_t i = 0; i < Regs.size(); ++i) {
        if (Regs[i].type == RD53BConstants::RegType::ReadWrite && registerValues[i]) {
            config["Registers"][Regs[i].name] = *registerValues[i];
        }
    }
    
    pixelConfigFields().for_each([&] (const auto& fieldName, auto ptr) {
        const auto& data = pixelConfig().*ptr;

        bool exists = config.contains("Pixels") && config["Pixels"].contains(fieldName.value);
        bool isUniform = xt::all(xt::equal(data, data(0)));
        bool isDefault = isUniform && data(0) == pixelConfigDefauls().at(fieldName.value);

        if (isDefault) {
            if (exists)
                config["Pixels"].as_table().erase(fieldName.value);
        }
        else {
            bool isString = exists && config["Pixels"][fieldName.value].is_string();
            if (isUniform && !isString)
                config["Pixels"][fieldName.value] = (int)data(0);
            else {
                std::string csvFileName;
                if (isString)
                    csvFileName = config["Pixels"][fieldName.value].as_string();
                else {
                    std::ostringstream csvFileNameStream;
                    csvFileNameStream << fieldName.value << ".csv";
                    csvFileName = getAvailablePath(csvFileNameStream.str());
                    config["Pixels"][fieldName.value] = csvFileName;
                }
                std::ofstream out_file(csvFileName);
                if (isUniform)
                    out_file << (int)data(0);
                else
                    xt::dump_csv(out_file, xt::cast<int>(pixelConfig().*ptr));
            }
        }
    });

    file << config;
}

template class RD53B<RD53BFlavor::ATLAS>;
template class RD53B<RD53BFlavor::CMS>;

} // namespace RD53Cmd
