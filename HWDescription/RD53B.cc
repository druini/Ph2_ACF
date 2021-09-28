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


const decltype(RD53BReg::GetRegisters()) RD53B::Registers = RD53BReg::GetRegisters();
const decltype(RD53BReg::GetRegisterIndexMap()) RD53B::RegisterIndexMap = RD53BReg::GetRegisterIndexMap();

const decltype(RD53BConstants::GetGlobalPulseRoutes()) RD53B::GlobalPulseRoutes = RD53BConstants::GetGlobalPulseRoutes();
const decltype(RD53BConstants::GetIMUX()) RD53B::IMUX = RD53BConstants::GetIMUX();
const decltype(RD53BConstants::GetVMUX()) RD53B::VMUX = RD53BConstants::GetVMUX();

template <>
uint8_t RD53B::ChipIdFor<Chip>(const Chip* chip) { return chip->getId(); }

RD53B::RD53B(uint8_t pBeId, uint8_t pFMCId, uint8_t pHybridId, uint8_t pRD53Id, uint8_t pRD53Lane, const std::string& fileName) 
  : RD53Base(pBeId, pFMCId, pHybridId, pRD53Id, pRD53Lane)
{
    fMaxRegValue      = 0xFFFF;
    fChipOriginalMask = new ChannelGroup<nRows, nCols>;
    configFileName    = fileName;
    setFrontEndType(FrontEndType::RD53B);

    for (size_t i = 0; i < Registers.size(); ++i) {
        registerValues[i] = Registers[i].defaultValue;
    }
    
    RD53B::loadfRegMap(configFileName);
}

RD53B::RD53B(const RD53B& chipObj) : RD53Base(chipObj) {}


uint32_t RD53B::getNumberOfChannels() const { return nRows * nCols; }

bool RD53B::isDACLocal(const std::string& regName)
{
    if(regName != "PIX_PORTAL") return false;
    return true;
}

uint8_t RD53B::getNumberOfBits(const std::string& regName)
{
    auto it = fRegMap.find(regName);
    if(it == fRegMap.end()) return 0;
    return it->second.fBitSize;
}


void RD53B::loadfRegMap(const std::string& fileName)
{
    if (boost::filesystem::exists(fileName)) {
        auto data = toml::parse(fileName);
        if (data.contains("Registers")) {
            for (const auto& key_value : data.at("Registers").as_table()) {
                setRegValue(key_value.first, key_value.second.as_integer());
            }
        }
    }
}

void RD53B::saveRegMap(const std::string& fName2Add)
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
    file << toml::value(toml::table({{"Registers", std::move(register_table)}}));
}


} // namespace RD53Cmd
