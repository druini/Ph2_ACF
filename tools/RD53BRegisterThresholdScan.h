#ifndef RD53BREGISTERTHRESHOLDSCAN_H
#define RD53BREGISTERTHRESHOLDSCAN_H

#include "RD53BThresholdScan.h"

#include <xtensor/xaxis_slice_iterator.hpp>

#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TApplication.h>

namespace RD53BTools {

template <class>
struct RD53BRegisterThresholdScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegisterThresholdScan<Flavor>> = make_named_tuple(
    std::make_pair("thresholdScan"_s, RD53BThresholdScan<Flavor>()),
    std::make_pair("regNames"_s, std::vector<std::string>({"DAC_GDAC_L_LIN", "DAC_GDAC_R_LIN", "DAC_GDAC_M_LIN"})),
    std::make_pair("regValueRange"_s, std::vector<size_t>({500, 1000})),
    std::make_pair("regValueStep"_s, 20u)
);

template <class Flavor>
struct RD53BRegisterThresholdScan : public RD53BTool<RD53BRegisterThresholdScan, Flavor> {
    using Base = RD53BTool<RD53BRegisterThresholdScan, Flavor>;
    using Base::Base;
    using Base::param;

    using ThresholdMap = ChipDataMap<xt::xtensor<double, 3>>;

    ThresholdMap run(Task progress) {
        ThresholdMap result;
        auto& chipInterface = Base::chipInterface();
        
        uint16_t regValue = param("regValueRange"_s)[0];
        size_t nSteps = (param("regValueRange"_s)[1] - param("regValueRange"_s)[0]) / param("regValueStep"_s);

        const auto& size = param("thresholdScan"_s).param("injectionTool"_s).param("size"_s);
        Base::for_each_chip([&] (Chip* chip) {
            result.insert({chip, xt::zeros<double>({nSteps, size[0], size[1]})});
        });

        for (size_t i = 0; i < nSteps; ++i) {
            Base::for_each_chip([&] (Chip* chip) {
                for (const auto& regName : param("regNames"_s))
                    chipInterface.WriteReg(chip, regName, regValue);
            });
            
            auto occMap = param("thresholdScan"_s).run(progress.subTask({double(i) / nSteps, double(i + 1) / nSteps}));
            auto thresholdMap = param("thresholdScan"_s).analyze(occMap)[0];
            for (const auto item : thresholdMap)
                xt::view(result[item.first], i, xt::all(), xt::all()) = item.second;

            regValue += param("regValueStep"_s);
        }

        return result;
    }

    void draw(const ThresholdMap& thresholdMap) const {
        // TApplication app("app", nullptr, nullptr);
        
        TFile* file = new TFile(Base::getResultPath(".root").c_str(), "NEW");
        
        xt::xarray<double> x = xt::arange<double>(param("regValueRange"_s)[0], param("regValueRange"_s)[1], param("regValueStep"_s));

        for (const auto& item : thresholdMap) {
            const auto& threshold = item.second;
            
            TCanvas* c = new TCanvas("canvas", "", 600, 600);
            auto mg = new TMultiGraph();

            for (auto it = xt::axis_slice_begin(threshold, 0); it != xt::axis_slice_end(threshold, 0); ++it) {
                xt::xarray<double> y = *it;
                mg->Add(new TGraph(threshold.shape()[0], x.data(), y.data()));
            }

            mg->Draw("acp");
            mg->Write();
            c->Print(Base::getResultPath(".pdf").c_str());
        }


        file->Write();
        file->Close();

    }

};

}

#endif