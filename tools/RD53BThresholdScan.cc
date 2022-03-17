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


template <class Flavor>
void RD53BThresholdScan<Flavor>::init() {
    vcalBins = xt::arange(param("vcalRange"_s)[0], param("vcalRange"_s)[1], param("vcalStep"_s));
}

template <class Flavor>
typename RD53BThresholdScan<Flavor>::OccupancyMap RD53BThresholdScan<Flavor>::run(Task progress) const {
    OccupancyMap overallOccMap;

    auto& chipInterface = Base::chipInterface();

    Base::for_each_chip([&] (auto* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
        overallOccMap.insert({chip, xt::zeros<double>({vcalBins.size(), size(0), size(1)})});
    });

    param("injectionTool"_s).configureInjections();

    std::vector<tool_result_t<RD53BInjectionTool<Flavor>>> events(vcalBins.size());

    size_t nFrames = param("injectionTool"_s).nFrames();

    for (size_t frame = 0; frame < nFrames; ++frame) {
        param("injectionTool"_s).setupMaskFrame(frame);
        for (size_t i = 0; i < vcalBins.size(); ++i) {
        // for (size_t i = 0, vcalHigh = param("vcalMed"_s) + param("vcalRange"_s)[0]; i < nSteps; ++i, vcalHigh += param("vcalStep"_s)) {
            Base::for_each_hybrid([&] (Hybrid* hybrid) {
                chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + vcalBins[i]);
            });
            
            param("injectionTool"_s).inject(events[i]);
            progress.update(double(frame * vcalBins.size() + i) / (nFrames * vcalBins.size()));
        }
    }

    for (size_t i = 0; i < vcalBins.size(); ++i) {
        for (const auto& item : param("injectionTool"_s).occupancy(events[i])) {
            xt::view(overallOccMap[item.first], i, xt::all(), xt::all()) = 
                xt::view(item.second, rowRange(), colRange());
        }
    }
    return overallOccMap;
}

template <class Flavor>
std::array<ChipDataMap<xt::xtensor_optional<double, 2>>, 2> RD53BThresholdScan<Flavor>::analyze(const OccupancyMap& occMap) const {
    ChipDataMap<xt::xtensor_optional<double, 2>> thresholdMap;
    ChipDataMap<xt::xtensor_optional<double, 2>> noiseMap;

    Base::for_each_chip([&] (auto* chip) {
        const auto& occ = occMap.at(chip);

        auto diff = xt::diff(occ, 1, 0);

        auto vcalDiffBins = xt::view(vcalBins + param("vcalStep"_s) / 2.0, xt::range(0, diff.shape()[0]));
        
        // thresholdMap[chip].fill(xtl::missing<double>());
        // noiseMap[chip].fill(xtl::missing<double>());
        thresholdMap.insert({chip, xt::xtensor_optional<double, 2>({size(0), size(1)})});
        noiseMap.insert({chip, xt::xtensor_optional<double, 2>({size(0), size(1)})});

        auto enabled = xt::view(chip->pixelConfig().enable && chip->pixelConfig().enableInjections, rowRange(), colRange());

        for (size_t row = 0; row < occ.shape()[1]; ++row) {
            for (size_t col = 0; col < occ.shape()[2]; ++col) {
                auto pixelOcc = xt::view(occ, xt::all(), row, col);
                if (enabled(row, col) && pixelOcc.front() < .01 && pixelOcc.back() > .99) {
                    auto weights = xt::view(diff, xt::all(), row, col);
                    thresholdMap[chip](row, col) = xt::average(vcalDiffBins, xt::abs(weights))();
                    noiseMap[chip](row, col) = sqrt(xt::average(xt::square(vcalDiffBins - thresholdMap[chip](row, col)), weights)());
                
                }
                else {
                    thresholdMap[chip](row, col) = xtl::missing<double>();
                    noiseMap[chip](row, col) = xtl::missing<double>();
                }
            }
        }
    });
    return {thresholdMap, noiseMap};
}


template <class Flavor>
void RD53BThresholdScan<Flavor>::draw(const OccupancyMap& occMap) {
    Base::createRootFile();

    const size_t nInjections = param("injectionTool"_s).param("nInjections"_s);
    // const size_t nPixels = param("injectionTool"_s).param("size"_s)[0] * param("injectionTool"_s).param("size"_s)[1];

    auto thresholdAndNoise = analyze(occMap);

    Base::for_each_chip([&] (RD53B<Flavor>* chip) {
    // for (const auto& item : occMap) {
        Base::createRootFileDirectory(chip);

        const auto& occ = occMap.at(chip);

        xt::xtensor<size_t, 2> scurves = xt::zeros<size_t>({vcalBins.size(), nInjections + 1});

        for (size_t i = 0; i < vcalBins.size(); ++i)
            for (size_t row = 0; row < occ.shape()[1]; ++row)
                for (size_t col = 0; col < occ.shape()[2]; ++col)
                    scurves(i, std::round(nInjections * occ(i, row, col))) += 1;

        Base::drawHist2D(scurves, "S-Curves", vcalBins(0), vcalBins.periodic(-1), 0, 1, "Delta VCAL", "Occupancy", "# of Pixels");
        
        const auto& threshold = thresholdAndNoise[0][chip];
        const auto& noise = thresholdAndNoise[1][chip];

        auto enabled = xt::view(chip->pixelConfig().enable && chip->pixelConfig().enableInjections, rowRange(), colRange());
        auto valid = xt::cast<bool>(xt::has_value(threshold));

        LOG(INFO) << "Enabled pixels: " << xt::count_nonzero(enabled)();
        LOG(INFO) << "Succesfully computed threshold and noise for " << xt::count_nonzero(valid)() << " pixels";
        const auto filtered_threshold = xt::filter(xt::value(threshold), valid);
        LOG(INFO) << "Mean threshold: " << xt::mean(filtered_threshold)();
        LOG(INFO) << "Threshold stddev: " << xt::stddev(filtered_threshold)();
        LOG(INFO) << "Min threshold: " << xt::amin(filtered_threshold)();
        LOG(INFO) << "Max threshold: " << xt::amax(filtered_threshold)();
        LOG(INFO) << "Mean noise: " << xt::mean(xt::filter(noise, valid))();
        LOG(INFO) << "Noise stddev: " << xt::stddev(xt::filter(noise, valid))();

        auto vcalDiffBins = xt::view(vcalBins + param("vcalStep"_s) / 2.0, xt::range(0, -1));

        Base::drawMap(xt::value(threshold), "Threshold Map", "Threshold", offset(0), offset(1));

        Base::drawHist(xt::value(threshold), "Threshold Distribution", vcalBins.size(), vcalBins(0), vcalBins.periodic(-1), "Delta VCAL");

        Base::drawMap(xt::value(noise), "Noise Map", "Noise", offset(0), offset(1));

        Base::drawHist(xt::filter(xt::value(noise), valid), "Noise Distribution", 32, 0, 32, "Delta VCAL");
    });
}

template class RD53BThresholdScan<RD53BFlavor::ATLAS>;
template class RD53BThresholdScan<RD53BFlavor::CMS>;

}