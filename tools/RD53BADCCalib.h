#ifndef RD53BADCCalib_H
#define RD53BADCCalib_H

#include <array>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "pugixml.hpp"

#include "TAxis.h"
#include "TDirectory.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TTree.h"

#include "../Utils/RD53BUtils.h"

namespace RD53BTools {

template <class>
class RD53BADCCalib;

template <class F>
const auto ToolParameters<RD53BADCCalib<F>> = make_named_tuple(
    std::make_pair("powerSupplyId"_s, std::string("analogCalib")),
    std::make_pair("powerSupplyChannelId"_s, std::string("analogCalib")),
    std::make_pair("minVoltage"_s, 0.0f),
    std::make_pair("maxVoltage"_s, 0.9f),
    std::make_pair("nVoltages"_s, 3),
    std::make_pair("adcSamples"_s, 2),
    std::make_pair("powerSupplySamples"_s, 2),
    std::make_pair("groundCorrection"_s, false),
    std::make_pair("voltmeterId"_s, std::string()),
    std::make_pair("voltmeterChannelId"_s, std::string()),
    std::make_pair("abortOnLimit"_s, false)
);

template <class F>
class RD53BADCCalib : public RD53BTool<RD53BADCCalib, F> {
public:
    struct Data {
        std::map<float, std::map<short, unsigned int>> adcData;
        std::array<std::vector<float>, 2>                       psData;
    };

    using Base = RD53BTool<RD53BADCCalib, F>;
    using Base::Base;
    using Base::param;
    using Results = ChipDataMap<Data>;

    void init() {
        psId = param("powerSupplyId"_s);
        psChId = param("powerSupplyChannelId"_s);
        min = param("minVoltage"_s);
        max = param("maxVoltage"_s);
        if(min > max) throw std::logic_error("Bad configuration: minimum voltage > maximum voltage.");
        if(min < 0.0f) throw std::logic_error("Bad configuration: minimum voltage is negative.");
        if(max > 1.3f) throw std::logic_error("Bad configuration: maximum voltage is dangerously high.");
        nVolt = param("nVoltages"_s);
        nSamp = param("adcSamples"_s);
        psMsg = "GetVoltage,PowerSupplyId:" + psId + ",ChannelId:" + psChId + ",Stats,Samples:" + std::to_string(param("powerSupplySamples"_s));
    }

    Results run(Ph2_System::SystemController& sysCtrl) {
        Results res;

        /* additional setup in case a Keithley SMU is being used */
        sysCtrl.fPowerSupplyClient->sendAndReceivePacket(
            "K2410:SetSpeed"
            ",PowerSupplyId:" + psId +
            ",ChannelId:" + psChId +
            ",IntegrationTime:0.01"
        );

        for_each_device<Chip>(sysCtrl, [this, &sysCtrl, &res](Chip* chip) { this->calibChip(chip, sysCtrl, res[chip]); });

        return res;
    }

    void draw(const Results& results) {
        TFile f(Base::getResultPath(".root").c_str(), "RECREATE");

        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child("adccalib");

        unsigned char boardId, hybridId, chipId;
        float setV, measV, errMeasV;

        std::vector<short> adcRdng;
        adcRdng.reserve(nSamp);

        TTree* t = new TTree("calibData", "Calibration data");
        t->Branch("boardId", &boardId);
        t->Branch("hybridId", &hybridId);
        t->Branch("chipId", &chipId);
        t->Branch("setV", &setV);
        t->Branch("measV", &measV);
        t->Branch("errMeasV", &errMeasV);
        t->Branch("adcRdng", &adcRdng);

        TF1 line("line", "[V(0)]+[dV/dADC]*x");
        std::vector<float> x, errX, diff;
        x.reserve(nVolt);
        errX.reserve(nVolt);
        diff.reserve(nVolt);
        for(const std::pair<const ChipLocation, Data>& chipRes: results) {
            const ChipLocation& chip = chipRes.first;

            boardId = chip.board_id;
            hybridId = chip.hybrid_id;
            chipId = chip.chip_id;

            f.mkdir(("board_" + std::to_string(chip.board_id)).c_str(), "", true)
                ->mkdir(("hybrid_" + std::to_string(chip.hybrid_id)).c_str(), "", true)
                ->mkdir(("chip_" + std::to_string(chip.chip_id)).c_str(), "", true)
                ->cd();

            pugi::xml_node dev = root.append_child("device");
            dev.append_attribute("board") = chip.board_id;
            dev.append_attribute("hybrid") = chip.hybrid_id;
            dev.append_attribute("chip") = chip.chip_id;

            std::vector<float> y = chipRes.second.psData[0];
            std::vector<float> yErr = chipRes.second.psData[1];

            x.clear();
            errX.clear();
            diff.clear();
            for(const std::pair<const float, std::map<short, unsigned int>>& p: chipRes.second.adcData) {
                adcRdng.clear();
                short min = 4096;
                short max = -1;
                unsigned long long n = 0;
                unsigned long long sum = 0;
                unsigned long long sum2 = 0;
                for(const std::pair<const short, unsigned int>& pp: p.second) {
                    if(pp.first < min) min = pp.first;
                    if(pp.first > max) max = pp.first;
                    unsigned long long buf = pp.first * pp.second;
                    sum += buf;
                    sum2 += buf * pp.first;
                    n += pp.second;
                    adcRdng.insert(adcRdng.end(), pp.second, pp.first);
                }
                if(n == 0) {
                    y.erase(y.cbegin()+x.size());
                    yErr.erase(yErr.cbegin()+x.size());
                }
                else {
                    x.push_back(sum / static_cast<double>(n));
                    errX.push_back(std::sqrt((sum2 * n - sum * sum) / static_cast<double>(n - 1)) / n);
                    diff.push_back(max-min);
                    setV = p.first;
                    measV = y[x.size()-1];
                    errMeasV = y[x.size()-1];
                    t->Fill();
                }
            }

            if(y.empty()) continue;

            std::vector<float>::size_type a = 0;
            std::vector<float>::size_type b = y.size() - 1;
            for(; a < y.size() - 1; ++a) if(!std::isnan(x[a]) && !std::isnan(y[a])) break;
            for(; b > 0; --b) if(!std::isnan(x[b]) && !std::isnan(y[b])) break;
            float m = (y[b] - y[a]) / (x[b] - x[a]);
            float q = y[a] - m * x[a];
            line.SetParameter("V(0)", q);
            line.SetParameter("dV/dADC", m);

            TGraphErrors g(x.size(), x.data(), y.data(), errX.data(), yErr.data());
            g.SetName("adcCalib");
            g.SetTitle("ADC calibration");
            g.GetXaxis()->SetTitle("ADC conversion [ADC counts]");
            g.GetYaxis()->SetTitle("Injected voltage [V]");
            g.Fit(&line, "Q");
            q = line.GetParameter(0);
            m = line.GetParameter(1);
            LOG(INFO) << std::scientific << "Offset [mV] = " << q << " +- " << line.GetParError(0) << RESET;
            LOG(INFO) << std::scientific << "Slope [mV/ADC LSB] = " << m << " +- " << line.GetParError(1) << RESET;
            g.Write();

            TGraph gDiff(y.size(), y.data(), diff.data());
            gDiff.SetName("adcDiff");
            gDiff.SetTitle("ADC conversions distribution width");
            gDiff.GetXaxis()->SetTitle("Injected voltage [V]");
            gDiff.GetYaxis()->SetTitle("Distribution width [ADC counts]");
            gDiff.Write();

            /* calculating residuals for next plot */ 
            std::vector<float> resid;
            resid.reserve(y.size());
            for(std::vector<float>::size_type i = 0; i < y.size(); ++i)
            {
                resid.push_back(y[i] - q - m * x[i]);
            }

            TGraphErrors gResid(x.size(), x.data(), resid.data(), errX.data(), yErr.data());
            gResid.SetName("adcResid");
            gResid.SetTitle("Residuals");
            gResid.GetXaxis()->SetTitle("ADC conversion [ADC counts]");
            gResid.GetYaxis()->SetTitle("Voltage residual [V]");
            gResid.Write();

            dev.append_attribute("offset") = line.GetParameter("V(0)");
            dev.append_attribute("slope") = line.GetParameter("dV/dADC");
            for(size_t i = 0; i < x.size(); ++i) {
                pugi::xml_node pt = dev.append_child("point");
                pt.append_attribute("x") = x[i];
                pt.append_attribute("y") = y[i];
                pt.append_attribute("xuncert") = errX[i];
                pt.append_attribute("yuncert") = yErr[i];
            }
        }

        t->Write();

        doc.save_file(Base::getResultPath(".xml").c_str());
    }

private:
    float min, max;
    int nSamp, nVolt;
    std::string psId, psChId, psMsg;

    void calibChip(Chip* chip, Ph2_System::SystemController& sysCtrl, Data& res) {
        auto& chipIface = *static_cast<RD53BInterface<F>*>(sysCtrl.fReadoutChipInterface);
        TCPClient& psIface = *sysCtrl.fPowerSupplyClient;

        double groundOffset = 0.0;

        if(param("groundCorrection"_s)) {
            psIface.sendAndReceivePacket("TurnOff,PowerSupplyId:" + psId + ",ChannelId:" + psChId);

            chipIface.WriteReg(chip, F::Reg::MonitorConfig, RD53B<F>::IMUX.at("high_Z") << 6 | RD53B<F>::VMUX.at("Analog_GND"));
            psIface.sendAndReceivePacket(
                "VoltmeterSetRange"
                ",VoltmeterId:" + param("voltmeterId"_s) +
                ",ChannelId:" + param("voltmeterChannelId"_s) +
                ",Value:1.3"
            );
            groundOffset = std::stod(psIface.sendAndReceivePacket(
                "ReadVoltmeter"
                ",VoltmeterId:" + param("voltmeterId"_s) +
                ",ChannelId:" + param("voltmeterChannelId"_s)
            ));
            LOG(INFO) << BOLDBLUE << "Analog ground offset = " << BOLDYELLOW << groundOffset << " V" << RESET;

            chipIface.WriteReg(chip, F::Reg::MonitorConfig, RD53B<F>::IMUX.at("high_Z") << 6 | RD53B<F>::VMUX.at("Vref_ADC"));
            std::string adcRef(psIface.sendAndReceivePacket(
                "ReadVoltmeter"
                ",VoltmeterId:" + param("voltmeterId"_s) +
                ",ChannelId:" + param("voltmeterChannelId"_s)
            ));
            LOG(INFO) << BOLDBLUE << "ADC reference voltage = " << BOLDYELLOW << adcRef << " V" << RESET;
        }

        chipIface.WriteReg(chip, F::Reg::MonitorConfig, 1u << 12 | RD53B<F>::IMUX.at("high_Z") << 6 | RD53B<F>::VMUX.at("high_Z"));

        /* additional setup in case a Keithley SMU is being used */
        psIface.sendAndReceivePacket(
            "K2410:setupVsource"
            ",PowerSupplyId:" + psId +
            ",ChannelId:" + psChId +
            ",Voltage:" + std::to_string(min) +
            ",CurrCompl:1e-6"
        );

        for(int i = 0; i < nVolt; ++i) {
            float volt = (max * i + min * (nVolt - 1 - i)) / static_cast<double>(nVolt - 1);
            psIface.sendAndReceivePacket("TurnOff,PowerSupplyId:" + psId + ",ChannelId:" + psChId);
            psIface.sendAndReceivePacket("SetVoltage,PowerSupplyId:" + psId + ",ChannelId:" + psChId + ",Voltage:" + std::to_string(volt));
            psIface.sendAndReceivePacket("TurnOn,PowerSupplyId:" + psId + ",ChannelId:" + psChId);

            double buf[2];
            std::istringstream resp(psIface.sendAndReceivePacket(psMsg));
            resp >> buf[0] >> buf[1];
            if(resp) {
                res.psData[0].push_back(buf[0] - groundOffset);
                res.psData[1].push_back(buf[1] / std::sqrt(param("powerSupplySamples"_s)));
            }
            else {
                res.psData[0].push_back(NAN);
                res.psData[1].push_back(NAN);
            }

            std::map<short, unsigned int>& m = res.adcData[volt];
            for(int j = 0; j < nSamp; ++j) {
                unsigned short x = chipIface.ReadChipADC(static_cast<RD53B<F>*>(chip), "high_Z");
                if(param("abortOnLimit"_s) && (x == 0 || x == 4095)) {
                    m.clear();
                    break;
                }
                m[x] += 1;
            }
        }

        psIface.sendAndReceivePacket("TurnOff,PowerSupplyId:" + psId + ",ChannelId:" + psChId);
    }

};

}

#endif
