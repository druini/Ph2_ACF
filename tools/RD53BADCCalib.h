#ifndef RD53BADCCalib_H
#define RD53BADCCalib_H

#include <array>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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
        std::array<std::vector<float>, 2> psData;
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
        psMsg = "GetVoltage,PowerSupplyId:" + psId + ",ChannelId:" + psChId;
        if(param("powerSupplySamples"_s) > 1) psMsg += ",Stats,Samples:" + std::to_string(param("powerSupplySamples"_s));
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
        std::ofstream json(Base::getResultPath(".json"));
        TFile f(Base::getResultPath(".root").c_str(), "RECREATE");

        uint16_t boardId, hybridId, chipId;
        float setV, measV;
        float errMeasV = NAN;

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
        std::vector<float> x, xErr, spread;

        const char* str = "{";
        json << std::scientific << "{\"chips\":[";
        for(const std::pair<const ChipLocation, Data>& chipRes: results) {
            const std::vector<float>& y = chipRes.second.psData[0];
            const std::vector<float>& yErr = chipRes.second.psData[1];

            if(y.empty()) continue;

            json << str;
            str = ",{";

            boardId = chipRes.first.board_id;
            hybridId = chipRes.first.hybrid_id;
            chipId = chipRes.first.chip_id;

            json << "\"board\":" << boardId << ","
                 << "\"hybrid\":" << hybridId << ","
                 << "\"id\":" << chipId << ",";

            f.mkdir(("board_" + std::to_string(boardId)).c_str(), "", true)
                ->mkdir(("hybrid_" + std::to_string(hybridId)).c_str(), "", true)
                ->mkdir(("chip_" + std::to_string(chipId)).c_str(), "", true)
                ->cd();

            x.reserve(y.size());
            xErr.reserve(y.size());
            spread.reserve(y.size());
            x.clear();
            xErr.clear();
            spread.clear();
            for(const std::pair<const float, std::map<short, unsigned int>>& p: chipRes.second.adcData) {
                short min = 4096;
                short max = -1;
                unsigned long long n = 0;
                unsigned long long sum = 0;
                unsigned long long sum2 = 0;
                adcRdng.clear();
                for(const std::pair<const short, unsigned int>& pp: p.second) {
                    if(pp.first < min) min = pp.first;
                    if(pp.first > max) max = pp.first;
                    unsigned long long buf = pp.first * pp.second;
                    sum += buf;
                    sum2 += buf * pp.first;
                    n += pp.second;
                    adcRdng.insert(adcRdng.end(), pp.second, pp.first);
                }
                x.push_back(sum / static_cast<double>(n));
                if(n > 1) {
                    xErr.push_back(std::sqrt((sum2 * n - sum * sum) / static_cast<double>(n - 1)) / n);
                }
                spread.push_back(max - min + 1);
                setV = p.first;
                measV = y[x.size() - 1];
                if(!yErr.empty()) errMeasV = yErr[x.size() - 1];
                t->Fill();
            }

            TGraph gSpread(y.size(), y.data(), spread.data());
            gSpread.SetName("adcSpread");
            gSpread.SetTitle("A/D conversions spread");
            gSpread.GetXaxis()->SetTitle("Injected voltage [V]");
            gSpread.GetYaxis()->SetTitle("Spread [ADC counts]");
            gSpread.Write();

            float m = (y.back() - y.front()) / (x.back() - x.front());
            float q = y.front() - m * x.front();
            line.SetParameter("V(0)", q);
            line.SetParameter("dV/dADC", m);

            TGraph* g;
            if(xErr.empty() && yErr.empty()) {
                g = new TGraph(x.size(), x.data(), y.data());
            }
            else if(xErr.empty()) {
                g = new TGraphErrors(x.size(), x.data(), y.data(), nullptr, yErr.data());
            }
            else if(yErr.empty()) {
                g = new TGraphErrors(x.size(), x.data(), y.data(), xErr.data(), nullptr);
            }
            else {
                g = new TGraphErrors(x.size(), x.data(), y.data(), xErr.data(), yErr.data());
            }
            g->SetName("adcCalib");
            g->SetTitle("ADC calibration");
            g->GetXaxis()->SetTitle("ADC conversion [ADC counts]");
            g->GetYaxis()->SetTitle("Injected voltage [V]");
            g->Fit(&line, "Q");
            g->Write();

            q = line.GetParameter("V(0)");
            m = line.GetParameter("dV/dADC");

            for(int i = 0; i < g->GetN(); ++i)
            {
                g->SetPointY(i, y[i] - q - m * x[i]);
            }

            g->SetName("adcResid");
            g->SetTitle("Residuals");
            g->GetXaxis()->SetTitle("ADC conversion [ADC counts]");
            g->GetYaxis()->SetTitle("Residual (voltage) [V]");
            g->Write();

            json << "\"fitted_line\":{\"intercept\":" << q << ",\"slope\":" << m << "},"
                 << "\"points\":[";
            for(size_t i = 0; i < x.size(); ++i) {
                json << "{\"ADC\":{\"val\":" << x[i];
                if(!xErr.empty()) json << ",\"err\":" << xErr[i];
                json << "},\"voltage\":{\"val\":" << y[i];
                if(!yErr.empty()) json << ",\"err\":" << yErr[i];
                json << "}}";
                if(i < x.size() - 1) json << ",";
            }
            json << "]}";
        }
        json << "]}";

        t->Write();
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

            double arr[2];
            std::istringstream resp(psIface.sendAndReceivePacket(psMsg));
            resp >> arr[0];
            if(param("powerSupplySamples"_s) > 1) resp >> arr[1];

            if(!resp) continue;

            std::map<short, unsigned int> m;
            for(int j = 0; j < nSamp; ++j) {
                unsigned short x = chipIface.ReadChipADC(static_cast<RD53B<F>*>(chip), "high_Z");
                if(param("abortOnLimit"_s) && (x == 0 || x == 4095)) {
                    m.clear();
                    break;
                }
                m[x] += 1;
            }
            if(!m.empty()) {
                res.adcData.emplace(volt, std::move(m));
                res.psData[0].push_back(arr[0] - groundOffset);
                if(param("powerSupplySamples"_s) > 1) res.psData[1].push_back(arr[1] / std::sqrt(param("powerSupplySamples"_s)));
            }
        }

        psIface.sendAndReceivePacket("TurnOff,PowerSupplyId:" + psId + ",ChannelId:" + psChId);
    }

};

}

#endif
