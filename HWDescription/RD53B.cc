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

namespace Ph2_HwDescription
{

template <class Flavor>
const decltype(Flavor::Reg::GetRegisters()) RD53B<Flavor>::Registers = Flavor::Reg::GetRegisters();

template <class Flavor>
const decltype(Flavor::Reg::GetRegisterIndexMap()) RD53B<Flavor>::RegisterIndexMap = Flavor::Reg::GetRegisterIndexMap();

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

    for (size_t i = 0; i < Registers.size(); ++i) {
        registerValues[i] = Registers[i].defaultValue;
    }
    
    RD53B::loadfRegMap(configFileName);
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
                setRegValue(key_value.first, key_value.second.as_integer());
            }
        }

        if (data.contains("Pixels")) {
            auto load_matrix = [] (std::string csvFileName, auto& matrix) {
                std::ifstream csvFile(csvFileName);
                csvFile >> matrix;
            };
            for (const auto& key_value : data.at("Pixels").as_table()) {
                if (key_value.first == "enable")
                    load_matrix(key_value.second.as_string(), pixelConfig.enable);
                else if (key_value.first == "enableInjections")
                    load_matrix(key_value.second.as_string(), pixelConfig.enableInjections);
                else if (key_value.first == "enableHitOr")
                    load_matrix(key_value.second.as_string(), pixelConfig.enableHitOr);
                else if (key_value.first == "tdac")
                    load_matrix(key_value.second.as_string(), pixelConfig.tdac);
                else if (key_value.first == "tdacSign")
                    load_matrix(key_value.second.as_string(), pixelConfig.tdacSign);
                else
                    throw std::runtime_error("Unkown pixel configuration field " + key_value.first + " in " + fileName);
            }

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
    for (size_t i = 0; i < Registers.size(); ++i) {
        if (getRegValue(i) != Registers[i].defaultValue) {
            register_table.insert({Registers[i].name, getRegValue(i)});
        }
    }

    file << toml::value(toml::table({
        {"Registers", std::move(register_table)},
        {"Pixels", toml::table({
            {"enable", pixelConfig.enable.to_string()},
            {"enableInjections", pixelConfig.enableInjections.to_string()},
            {"enableHitOr", pixelConfig.enableHitOr.to_string()},
            {"tdac", pixelConfig.tdac.to_string()},
            {"tdacSign", pixelConfig.tdacSign.to_string()}
        })}
    }));
}

template class RD53B<RD53BFlavor::ATLAS>;
template class RD53B<RD53BFlavor::CMS>;

} // namespace RD53Cmd
