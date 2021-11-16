#include "RD53BThresholdScan.h"

#include "../Utils/xtensor/xview.hpp"
#include "../Utils/xtensor/xhistogram.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xstrided_view.hpp"
#include "../Utils/xtensor/xio.hpp"
#include "../Utils/xtensor/xoperation.hpp"
#include "../Utils/xtensor/xmath.hpp"

#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TApplication.h>
#include <TGaxis.h>


namespace RD53BTools {

void ReverseYAxis(TH1 *h)
{
    // Remove the current axis
    h->GetYaxis()->SetLabelOffset(999);
    h->GetYaxis()->SetTickLength(0);
    // Redraw the new axis
    gPad->Update();
    TGaxis *newaxis = new TGaxis(gPad->GetUxmin(),
                                    gPad->GetUymax(),
                                    gPad->GetUxmin()-0.001,
                                    gPad->GetUymin(),
                                    h->GetYaxis()->GetXmin(),
                                    h->GetYaxis()->GetXmax(),
                                    510,"+");
    newaxis->SetLabelOffset(-0.03);
    newaxis->Draw();
}

template <class Flavor>
typename RD53BThresholdScan<Flavor>::OccupancyMap RD53BThresholdScan<Flavor>::run(Ph2_System::SystemController& system, Task progress) {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

    size_t nSteps = (param("vcalHighRange"_s)[1] - param("vcalHighRange"_s)[0]) / param("vcalHighStep"_s);
    const auto& offset = param("injectionTool"_s).param("offset"_s);
    const auto& size = param("injectionTool"_s).param("size"_s);

    for_each_device<Chip>(system, [&] (Chip* chip) {
        auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
        chipInterface.WriteReg(rd53b, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        if (param("TDAC"_s) >= 0) {
            rd53b->pixelConfig.tdac.fill(param("TDAC"_s));
            chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);
        }
    });

    param("injectionTool"_s).configureInjections(system);

    std::vector<tool_result_t<RD53BInjectionTool<Flavor>>> events(nSteps);

    size_t nFrames = param("injectionTool"_s).nFrames();

    for (size_t frame = 0; frame < nFrames; ++frame) {
        param("injectionTool"_s).setupMaskFrame(system, frame);
        for (size_t i = 0, vcalHigh = param("vcalHighRange"_s)[0]; i < nSteps; ++i, vcalHigh += param("vcalHighStep"_s)) {
            for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
                chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, vcalHigh);
            });
            
            param("injectionTool"_s).inject(system, events[i]);
            progress.update(double(frame * nSteps + i) / (nFrames * nSteps));
        }
    }

    OccupancyMap overallOccMap;
    for (size_t i = 0; i < nSteps; ++i) {
        for (const auto& item : param("injectionTool"_s).occupancy(events[i])) {
            overallOccMap.insert({item.first, xt::zeros<double>({nSteps, size[0], size[1]})});
            xt::view(overallOccMap[item.first], i, xt::all(), xt::all()) = 
                xt::view(item.second, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
        }
    }
    return overallOccMap;
}



template <class Flavor>
std::array<ChipDataMap<xt::xtensor<double, 2>>, 2> RD53BThresholdScan<Flavor>::analyze(const OccupancyMap& occMap) const {
    ChipDataMap<xt::xtensor<double, 2>> thresholdMap;
    ChipDataMap<xt::xtensor<double, 2>> noiseMap;

    auto vcalBins = xt::arange(param("vcalHighRange"_s)[0], param("vcalHighRange"_s)[1], param("vcalHighStep"_s)) - param("vcalMed"_s);

    for (const auto& item : occMap) {
        const auto& occ = item.second;

        auto diff = xt::diff(occ, 1, 0);

        auto vcalDiffBins = xt::view(vcalBins + param("vcalHighStep"_s) / 2.0, xt::range(0, -1));

        thresholdMap.insert({item.first, xt::zeros<double>({occ.shape()[1], occ.shape()[2]})});
        noiseMap.insert({item.first, xt::zeros<double>({occ.shape()[1], occ.shape()[2]})});

        for (size_t row = 0; row < occ.shape()[1]; ++row) {
            for (size_t col = 0; col < occ.shape()[2]; ++col) {
                if (xt::all(xt::view(occ, xt::all(), row, col) >= 1.))
                    thresholdMap[item.first](row, col) = vcalDiffBins.periodic(0);
                else if (xt::allclose(xt::view(occ, xt::all(), row, col), 0.))
                    thresholdMap[item.first](row, col) = vcalDiffBins(-1);
                else {
                    auto weights = xt::view(diff, xt::all(), row, col);
                    thresholdMap[item.first](row, col) = xt::average(vcalDiffBins, xt::abs(weights))();
                    noiseMap[item.first](row, col) = sqrt(xt::average(xt::square(vcalDiffBins - thresholdMap[item.first](row, col)), weights)());
                }
            }
        }
    }
    return {thresholdMap, noiseMap};
}


template <class Flavor>
void RD53BThresholdScan<Flavor>::draw(const OccupancyMap& occMap) const {
    TApplication app("app", nullptr, nullptr);
    
    TFile* file = new TFile(Base::getResultPath(".root").c_str(), "NEW");

    const auto& offset = param("injectionTool"_s).param("offset"_s);
    const size_t nInjections = param("injectionTool"_s).param("nInjections"_s);
    const size_t nPixels = param("injectionTool"_s).param("size"_s)[0] * param("injectionTool"_s).param("size"_s)[1];

    auto thresholdAndNoise = analyze(occMap);
    auto vcalBins = xt::arange(param("vcalHighRange"_s)[0], param("vcalHighRange"_s)[1], param("vcalHighStep"_s)) - param("vcalMed"_s);

    for (const auto& item : occMap) {
        std::stringstream ss;
        ss << "Chip " << item.first;
        file->cd();
        file->mkdir(ss.str().c_str())->cd();

        TH2F* scurves = new TH2F("scurves", "S-Curves", 
            item.second.shape()[0], param("vcalHighRange"_s)[0] - param("vcalMed"_s), param("vcalHighRange"_s)[1] - param("vcalMed"_s), 
            nInjections + 1, 0, 101
        );

        // xt::xtensor<double, 2> scurveData = xt::zeros<double>({vcalBins.size(), nInjections});
        // auto arr = xt::xarray<double>(item.second);
        // arr.reshape({int(vcalBins.size()), -1});
        // for (size_t i = 0; i < vcalBins.size(); ++i)
        //     xt::col(scurveData, i) = xt::histogram(xt::view(arr, i, xt::all()), nInjections);

        // std::cout << scurveData << std::endl;

        for (size_t i = 0; i < item.second.shape()[0]; ++i)
            for (const double occ : xt::view(item.second, i, xt::all(), xt::all()))
                scurves->Fill(vcalBins(i), occ * 100 + .5 * 100.0 / nInjections, 1);

        const auto& threshold = thresholdAndNoise[0][item.first];
        const auto& noise = thresholdAndNoise[1][item.first];

        LOG(INFO) << "Mean threshold: " << xt::mean(threshold)();
        LOG(INFO) << "Mean noise: " << xt::mean(noise)();

        auto vcalDiffBins = xt::view(vcalBins + param("vcalHighStep"_s) / 2.0, xt::range(0, -1));

        TH2F* thresholdMap = new TH2F("thresholdMap", "Threshold Map", Flavor::nCols, 0, Flavor::nCols, Flavor::nRows, 0, Flavor::nRows);
        for (size_t row = 0; row < threshold.shape()[0]; ++row)
            for (size_t col = 0; col < threshold.shape()[1]; ++col)
                thresholdMap->Fill(offset[1] + col, Flavor::nRows - (offset[0] + row), threshold(row, col));

        TH1F* thresholdHist = new TH1F("threshold", "Threshold Distribution", vcalDiffBins.size(), vcalDiffBins(0), vcalDiffBins(vcalDiffBins.size() - 1));
        for (const auto& val : threshold)
            thresholdHist->Fill(val, 1.0 / nPixels);

        TH2F* noiseMap = new TH2F("noiseMap", "Noise Map", Flavor::nCols, 0, Flavor::nCols, Flavor::nRows, 0, Flavor::nRows);
        for (size_t row = 0; row < noise.shape()[0]; ++row)
            for (size_t col = 0; col < noise.shape()[1]; ++col)
                noiseMap->Fill(offset[1] + col, Flavor::nRows - (offset[0] + row), noise(row, col));

        TH1F* noiseHist = new TH1F("noise", "Noise Distribution", 100, 0, 1.1 * xt::amax(noise)());
        for (const auto& val : noise)
            noiseHist->Fill(val, 1.0 / nPixels);

        TCanvas* c1 = new TCanvas("scurves", "Threshold Scan S-Curves", 600, 600); (void)c1;
        scurves->Draw("COLZ");
        c1->Write();

        TCanvas* c2 = new TCanvas("thresholdMap", "Threshold Scan Results", 600, 600); (void)c2;
        thresholdMap->Draw("COLZ");
        ReverseYAxis(thresholdMap);
        c2->Write();
        
        TCanvas* c3 = new TCanvas("threshold", "Threshold Scan Results", 600, 600); (void)c3;
        thresholdHist->Draw("HIST");
        c3->Write();
        c3->Print(Base::getResultPath(".pdf").c_str());
        
        TCanvas* c4 = new TCanvas("noiseMap", "Threshold Scan Results", 600, 600); (void)c4;
        noiseMap->Draw("COLZ");
        ReverseYAxis(noiseMap);
        c4->Write();

        TCanvas* c5 = new TCanvas("noise", "Threshold Scan Results", 600, 600); (void)c5;
        noiseHist->Draw("HIST");
        c5->Write();
    }

    file->Write();

    // TQObject::Connect("TGMainFrame", "CloseWindow()", "TApplication", &app, "Terminate()");
    app.Run(true);
}

template class RD53BThresholdScan<RD53BFlavor::ATLAS>;
template class RD53BThresholdScan<RD53BFlavor::CMS>;

}