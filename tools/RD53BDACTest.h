#ifndef RD53BDACTest_H
#define RD53BDACTest_H

#include <array>
#include <chrono>
#include <cmath>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "RD53B.h"
#include "RD53BCommands.h"
#include "RD53BInterface.h"
#include "RD53BRegister.h"
#include "RD53BATLASRegisters.h"
#include "RD53BCMSRegisters.h"
#include "RD53BTool.h"

#include "TAxis.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TGraphErrors.h"

namespace RD53BTools {

template <class>
class RD53BDACTest;

template <class F>
const auto ToolParameters<RD53BDACTest<F>> = make_named_tuple(
    std::make_pair("voltmeterId"_s, std::string("dacCalib")),
    std::make_pair("voltmeterChannelId"_s, std::string("dacCalib")),
    std::make_pair("voltmeterSamples"_s, 2),
    std::make_pair("dacRegisters"_s, std::vector<std::string>()),
    std::make_pair("currentMuxResistance"_s, -1.0f),
    std::make_pair("vcalDelay"_s, 0.0f),
    std::make_pair("keithSpeed"_s, 0.01f),
    std::make_pair("n"_s, std::map<std::string, uint16_t>{}),
    std::make_pair("add"_s, std::map<std::string, std::vector<uint16_t>>{}),
    std::make_pair("fitThresholdV"_s, INFINITY)
);

template <class F>
class RD53BDACTest : public RD53BTool<RD53BDACTest, F> {
  public:
    struct DACData {
        std::vector<float> code, volt, voltErr;
    };

    using Base = RD53BTool<RD53BDACTest, F>;
    using Base::Base;
    using Base::param;
    using Results = ChipDataMap<std::map<RD53BConstants::Register, DACData>>;

    void init() {
        nSamp = param("voltmeterSamples"_s);
        vmetId = param("voltmeterId"_s);
        vmetChId = param("voltmeterChannelId"_s);
        imuxR = param("currentMuxResistance"_s);
        
        code["VCAL"] = {0,1,2,3,4,7,8,15,16,31,32,63,64,127,128,255,256,511,512,1023,1024,2047,2048,4095};
        for(const std::pair<std::string, std::array<uint16_t, 2>>& p : lim) {
            auto nIt = param("n"_s).find(p.first);
            uint16_t n = (nIt == param("n"_s).cend()) ? 2 : nIt->second;
            if(n < 2) throw std::logic_error("The number of points for " + p.first + " cannot be < 2");
            for(uint16_t i = 0; i < n; ++i) {
                float min = lim.at(p.first)[0];
                float max = lim.at(p.first)[1];
                float c = (max * i + min * (n - 1 - i)) / (n - 1);
                code[p.first].insert(std::round(c));
            }
            auto addIt = param("add"_s).find(p.first);
            if(addIt != param("add"_s).cend()) code[p.first].insert(addIt->second.cbegin(), addIt->second.cend());
        }
    }

    Results run() {
        Results res;
        Base::system().fPowerSupplyClient->sendAndReceivePacket(
            "K2410:SetSpeed"
            ",PowerSupplyId:" + vmetId +
            ",ChannelId:" + vmetChId +
            ",IntegrationTime:" + std::to_string(param("keithSpeed"_s))
        );
        Base::for_each_chip([this, &res] (Chip* chip) {
            std::vector<RD53BConstants::Register> dac;
            RD53BInterface<F>& chipIface = *static_cast<RD53BInterface<F>*>(Base::system().fReadoutChipInterface);
            /* first set of DACs */
            dac = {
                F::Reg::DAC_PREAMP_L_LIN,
                F::Reg::DAC_PREAMP_R_LIN,
                F::Reg::DAC_PREAMP_TL_LIN,
                F::Reg::DAC_PREAMP_TR_LIN,
                F::Reg::DAC_PREAMP_T_LIN,
                F::Reg::DAC_PREAMP_M_LIN,
                F::Reg::DAC_FC_LIN,
                F::Reg::DAC_KRUM_CURR_LIN,
                F::Reg::DAC_COMP_TA_LIN,
                F::Reg::DAC_GDAC_L_LIN,
                F::Reg::DAC_GDAC_R_LIN,
                F::Reg::DAC_GDAC_M_LIN,
                F::Reg::DAC_LDAC_LIN,
                F::Reg::VCAL_HIGH,
                F::Reg::VCAL_MED
            };
            chipIface.WriteReg(chip, F::Reg::DAC_REF_KRUM_LIN, 300);
            chipIface.WriteReg(chip, F::Reg::DAC_COMP_LIN, 250);
            this->calibChip(chip, res[chip], dac);
            /* last set of DACs */
            dac = {
                F::Reg::DAC_REF_KRUM_LIN,
                F::Reg::DAC_COMP_LIN
            };
            chipIface.WriteReg(chip, F::Reg::DAC_LDAC_LIN, 80);
            for(const auto& reg : {F::Reg::DAC_GDAC_L_LIN, F::Reg::DAC_GDAC_R_LIN, F::Reg::DAC_GDAC_M_LIN}) {
                chipIface.WriteReg(chip, reg, 900);
            }
            this->calibChip(chip, res[chip], dac);
        });
        return res;
    }

    void draw(const Results& results);

  private:
    static const std::map<std::string, std::array<uint16_t, 2>> lim;

    // static const std::map<RD53BConstants::Register, std::string> dacSett;
    static const std::map<std::reference_wrapper<const RD53BConstants::Register>, std::string> dacSett;
    
    // static const std::map<RD53BConstants::Register, uint16_t> muxSett;
    static const std::map<std::reference_wrapper<const RD53BConstants::Register>, uint16_t> muxSett;

    std::map<std::string, std::set<uint16_t>> code;
    
    size_t nSamp;
    float imuxR;
    std::string vmetId, vmetChId;
    
    bool isCurr(const Ph2_HwDescription::RD53BConstants::Register& dac) const {
        uint16_t vmux;
        std::tie(std::ignore, vmux) = bits::unpack<6, 6>(muxSett.at(std::ref(dac)));
        return vmux == (uint8_t)RD53B<F>::VMux::IMUX_OUT;
    }
    
    void calibChip(Chip* chip,
                   std::map<RD53BConstants::Register, DACData>& res,
                   const std::vector<RD53BConstants::Register>& dac) {
        using RD53BConstants::Register;
        
        auto &chipIface = *static_cast<RD53BInterface<F>*>(Base::system().fReadoutChipInterface);
        TCPClient &psIface = *Base::system().fPowerSupplyClient;
        
        std::ostringstream psReq;
        psReq = std::ostringstream("ReadVoltmeter,VoltmeterId:", std::ostringstream::ate);
        psReq << vmetId << ",ChannelId:" << vmetChId;
        if(nSamp > 1) psReq << ",Stats,N:" << nSamp;
        
        std::map<Register, std::set<uint16_t>> code;
        for(const Register& reg : dac) code[reg] = this->code.at(dacSett.at(std::ref(reg)));

        psIface.sendAndReceivePacket("VoltmeterSetRange"
                                      ",VoltmeterId:" + vmetId +
                                      ",ChannelId:" + vmetChId +
                                      ",Value:1.3");

        chipIface.WriteReg(chip, "SEL_CAL_RANGE", 1);

        BitVector<uint16_t> cmdQ;
        while(true) {
            // setting all the DACs
            cmdQ.clear();
            for(std::pair<const Register, std::set<uint16_t>>& p : code) {
                if(p.second.empty()) continue;
                
                chipIface.template SerializeCommand<RD53BCmd::Sync>(chip, cmdQ);
                chipIface.template SerializeCommand<RD53BCmd::WrReg>(chip, cmdQ, p.first.address, *p.second.begin());
            }
            
            if(cmdQ.size() == 0) return;
            
            chipIface.SendCommandStream(chip, cmdQ);
            auto start = std::chrono::steady_clock::now();
            // all DACs set
            // measuring all DAC outputs
            for(const Register& reg : dac) {
                if(code.at(reg).empty()) continue;
                
                chipIface.WriteChipReg(chip, "MonitorConfig", muxSett.at(reg));
                if(reg == F::Reg::VCAL_HIGH || reg == F::Reg::VCAL_MED) {
                    std::this_thread::sleep_until(start + std::chrono::duration<float>(param("vcalDelay"_s)));
                }
                std::istringstream psResp(psIface.sendAndReceivePacket(/*psReq.str()*/""));
                float a, b;
                psResp >> a;
                if(nSamp > 1) psResp >> b;
                if (psResp) {
                    if (isCurr(reg)) {
                        a /= imuxR;
                        if(nSamp > 1) b /= imuxR * std::sqrt(nSamp);
                    }
                    res[reg].code.push_back(*code.at(reg).cbegin());
                    res[reg].volt.push_back(a);
                    if(nSamp > 1) res[reg].voltErr.push_back(b);
                }
            }
            for(std::pair<const Register, std::set<uint16_t>>& p : code) {
                if(!p.second.empty()) p.second.erase(p.second.begin());
            }
            // all DAC outputs measured for a single configuration
        }
        // all codes tested for all DACs
    }
};

}

#endif
