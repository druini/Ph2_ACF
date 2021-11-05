#ifndef RD53BTHRESHOLDSCAN_H
#define RD53BTHRESHOLDSCAN_H

#include "RD53BInjectionTool.h"

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TApplication.h>


namespace RD53BTools {

template <class>
struct RD53BThresholdScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdScan<Flavor>> = make_named_tuple(
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("vcalMed"_s, 300u),
    std::make_pair("vcalHighRange"_s, std::vector<size_t>({500, 1000})),
    std::make_pair("vcalHighStep"_s, 20u)
);

template <class Flavor>
struct RD53BThresholdScan : public RD53BTool<RD53BThresholdScan<Flavor>> {
    using Base = RD53BTool<RD53BThresholdScan>;
    using Base::Base;
    using Base::param;

    // using OccupancyMap = ChipDataMap<std::vector<typename RD53B<Flavor>::PixelMatrix<double>>>;

    using OccupancyMap = ChipDataMap<xt::xtensor<double, 3>>;

    OccupancyMap run(Ph2_System::SystemController& system, Task progress) const {
        OccupancyMap overallOccMap;
        
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        size_t nSteps = (param("vcalHighRange"_s)[1] - param("vcalHighRange"_s)[0]) / param("vcalHighStep"_s);
        const auto& offset = param("injectionTool"_s).param("offset"_s);
        const auto& size = param("injectionTool"_s).param("size"_s);

        for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
            chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        });

        for_each_device<Chip>(system, [&] (Chip* chip) {
            overallOccMap.insert({chip, xt::zeros<double>({nSteps, size[0], size[1]})});
        });
        
        for (size_t vcalHigh = param("vcalHighRange"_s)[0], i = 0; vcalHigh < param("vcalHighRange"_s)[1]; vcalHigh += param("vcalHighStep"_s), ++i) {
            for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
                chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, vcalHigh);
            });
            usleep(100000);
            
            auto eventsMap = param("injectionTool"_s).run(system, progress.subTask({double(i) / nSteps, double(i + 1) / nSteps}));
            auto occMap = param("injectionTool"_s).occupancy(eventsMap);

            for (const auto& item : occMap)
                xt::view(overallOccMap[item.first], i, xt::all(), xt::all()) = xt::view(item.second, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
        }

        return overallOccMap;
    }

    void draw(const OccupancyMap& occMap) const {
        TApplication app("app", nullptr, nullptr);
        const auto& offset = param("injectionTool"_s).param("offset"_s);

        for (const auto& item : occMap) {
            TH2F* scurves = new TH2F("scurves", "S-Curves", 
                item.second.shape()[0], param("vcalHighRange"_s)[0] - param("vcalMed"_s), param("vcalHighRange"_s)[1] - param("vcalMed"_s), 
                1001, 0, 100
            );

            auto vcalBins = xt::arange(param("vcalHighRange"_s)[0], param("vcalHighRange"_s)[1], param("vcalHighStep"_s)) - param("vcalMed"_s);

            for (size_t i = 0; i < item.second.shape()[0]; ++i) {
                for (const double occ : xt::view(item.second, i, xt::all(), xt::all())) {
                    scurves->Fill(vcalBins(i), occ * 100, 1);
                }
            }

            TCanvas* c1 = new TCanvas("c1", "Threshold Scan Results", 600, 600); (void)c1;
            scurves->Draw("COLZ");

            auto diff = xt::diff(item.second, 1, 0);

            auto vcalDiffBins = xt::view(vcalBins + param("vcalHighStep"_s) / 2.0, xt::range(0, -1));
            
            // TH1F* threshold = new TH1F("threshold", "Threshold Distribution", vcalDiffBins.size(), vcalDiffBins(0), vcalDiffBins(vcalDiffBins.size() - 1));

            xt::xtensor<double, 2> threshold = xt::zeros<double>({item.second.shape()[1], item.second.shape()[2]});
            xt::xtensor<double, 2> noise = xt::zeros_like(threshold);

            // TH1F* noise = new TH1F("noise", "Noise Distribution", vcalDiffBins.size(), vcalDiffBins(0), vcalDiffBins(vcalDiffBins.size() - 1));

            size_t nPixels = item.second.shape()[1] * item.second.shape()[2];

            for (size_t row = 0; row < item.second.shape()[1]; ++row) {
                for (size_t col = 0; col < item.second.shape()[2]; ++col) {
                    auto weights = xt::view(diff, xt::all(), row, col);
                    threshold(row, col) = xt::average(vcalDiffBins, weights)();
                    noise(row, col) = sqrt(xt::average(xt::square(vcalDiffBins - threshold(row, col)), weights)());
                    // threshold->Fill(mean, 1.0 / nPixels);
                    // noise->Fill(std, 1.0 / nPixels);
                }
            }

            TH2F* thresholdMap = new TH2F("thresholdMap", "Threshold Map", Flavor::nCols, 0, Flavor::nCols, Flavor::nRows, 0, Flavor::nRows);
            for (size_t row = 0; row < threshold.shape()[0]; ++row)
                for (size_t col = 0; col < threshold.shape()[1]; ++col)
                    thresholdMap->Fill(offset[1] + col, offset[0] + row, threshold(row, col));
            TCanvas* c2 = new TCanvas("c2", "Threshold Scan Results", 600, 600); (void)c2;
            thresholdMap->Draw("COLZ");


            TH1F* thresholdHist = new TH1F("threshold", "Threshold Distribution", vcalDiffBins.size(), vcalDiffBins(0), vcalDiffBins(vcalDiffBins.size() - 1));
            for (const auto& val : threshold)
                thresholdHist->Fill(val, 1.0 / nPixels);
            TCanvas* c3 = new TCanvas("c3", "Threshold Scan Results", 600, 600); (void)c3;
            thresholdHist->Draw("HIST");


            TH2F* noiseMap = new TH2F("noiseMap", "Noise Map", Flavor::nCols, 0, Flavor::nCols, Flavor::nRows, 0, Flavor::nRows);
            for (size_t row = 0; row < noise.shape()[0]; ++row)
                for (size_t col = 0; col < noise.shape()[1]; ++col)
                    noiseMap->Fill(offset[1] + col, offset[0] + row, noise(row, col));
            TCanvas* c4 = new TCanvas("c4", "Threshold Scan Results", 600, 600); (void)c4;
            noiseMap->Draw("COLZ");


            TH1F* noiseHist = new TH1F("noise", "Noise Distribution", 100, 0, 1.1 * xt::amax(noise)());
            for (const auto& val : noise)
                noiseHist->Fill(val, 1.0 / nPixels);
            TCanvas* c5 = new TCanvas("c5", "Threshold Scan Results", 600, 600); (void)c5;
            noiseHist->Draw("HIST");
        }

        TQObject::Connect("TGMainFrame", "CloseWindow()", "TApplication", &app, "Terminate()");
        app.Run(true);
    }

};

}

#endif