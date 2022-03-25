#include "RD53BThresholdEqualization.h"

#include "../Utils/xtensor/xmasked_view.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"
#include "../Utils/xtensor/xrandom.hpp"
#include "../Utils/xtensor/xoptional.hpp"

#include <TH2F.h>
#include <TApplication.h>
#include <TFile.h>

namespace RD53BTools {

template <class Flavor>
ChipDataMap<pixel_matrix_t<Flavor, uint8_t>> RD53BThresholdEqualization<Flavor>::run(Task progress) {
    auto& chipInterface = Base::chipInterface();

    auto thresholdMeasurementTask   = progress.subTask({0, 0.2});
    auto tuningTask                 = progress.subTask({0.2, 1});

    auto& injectionTool = param("injectionTool"_s);
    // auto& offset = injectionTool.param("offset"_s);
    auto& size = injectionTool.param("size"_s);
    // auto rowRange = xt::range(offset[0], offset[0] + size[0]);
    // auto colRange = xt::range(offset[1], offset[1] + size[1]);
    
    auto usedPixels = injectionTool.usedPixels();

    ChipDataMap<pixel_matrix_t<Flavor, bool>> enabled;
    ChipDataMap<pixel_matrix_t<Flavor, uint8_t>> tdac;
    ChipDataMap<pixel_matrix_t<Flavor, uint8_t>> bestTDAC;
    ChipDataMap<pixel_matrix_t<Flavor, double>> bestOcc;

    Base::for_each_chip([&] (auto* chip) {
        enabled[chip] = usedPixels && chip->injectablePixels();
        tdac[chip].fill(param("initialTDAC"_s));
        bestTDAC[chip].fill(param("initialTDAC"_s));
        bestOcc[chip].fill(0);
        // bestTDAC.insert({chip, param("initialTDAC"_s) * xt::ones<uint8_t>({size[0], size[1]})});
        // bestOcc.insert({chip, xt::zeros<double>({size[0], size[1]})});
        // chipInterface.UpdatePixelTDACUniform(chip, param("initialTDAC"_s));
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
    });

    ChipDataMap<size_t> VCAL_HIGH;
    if (param("targetThreshold"_s) == 0) {
        auto scanOccMap = param("thresholdScan"_s).run(thresholdMeasurementTask);
        auto thresholdMap = param("thresholdScan"_s).analyze(scanOccMap)[0];
        
        Base::for_each_chip([&] (Chip* chip) {
            double meanThreshold = xt::mean(xt::value(xt::filter(thresholdMap[chip], xt::has_value(thresholdMap[chip]))))();
            VCAL_HIGH[chip] = param("thresholdScan"_s).param("vcalMed"_s) + meanThreshold;
        });
    }
    else {
        Base::for_each_chip([&] (Chip* chip) {
            VCAL_HIGH[chip] = param("thresholdScan"_s).param("vcalMed"_s) + param("targetThreshold"_s);
        });
    }
    
    xt::print_options::set_line_width(200);
    xt::print_options::set_threshold(3000);

    size_t nSteps = param("nSteps"_s);
    for (size_t i = 0; i < nSteps; ++i) {
        
        Base::for_each_chip([&] (Chip* chip) {
            chipInterface.UpdatePixelTDAC(chip, tdac[chip]); // update tdacs
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, VCAL_HIGH[chip]);
        });
        
        auto result = injectionTool.run(tuningTask.subTask({i / double(nSteps), (i + .5) / double(nSteps)}));
        auto occMap = injectionTool.occupancy(result);

        Base::for_each_hybrid([&] (Hybrid* hybrid) {
            chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, 0xFFFF);
        });
        
        auto resultHighCharge = injectionTool.run(tuningTask.subTask({(i + .5) / double(nSteps), (i + 1) / double(nSteps)}));
        auto occMapHighCharge = injectionTool.occupancy(resultHighCharge);

        Base::for_each_chip([&] (auto* chip) {
            
            const auto stuck = enabled[chip] && occMapHighCharge[chip] < .9;

            auto cost = xt::abs(occMap[chip] - 0.5);
            auto minCost = xt::abs(bestOcc[chip] - 0.5);

            xt::xtensor<bool, 2> isBest = enabled[chip] && !stuck && ((cost < minCost) || xt::isclose(occMap[chip], bestOcc[chip]));

            if (param("eliminateBias"_s) && i == nSteps - 1)
                isBest |= (xt::isclose(cost, minCost) && xt::random::randint<int>(size, 0, 1));
            
            LOG(INFO) << "Step: " << i;
            LOG(INFO) << "nStuck: " <<  xt::count_nonzero(stuck);
            LOG(INFO) << "==========================================";

            xt::masked_view(bestOcc[chip], isBest) = occMap[chip];
            xt::masked_view(bestTDAC[chip], isBest) = tdac[chip];

            if (i < nSteps - 1) {
                int tdacStep = i < 4 ? (8 >> i) : 1;
                xt::masked_view(tdac[chip], enabled[chip]) = 
                    xt::clip(xt::where(!stuck && (occMap[chip] < 0.5), tdac[chip] + tdacStep, tdac[chip] - tdacStep), 0, 31);
            }
        });
    }


    Base::for_each_chip([&] (auto* chip) {
        xt::masked_view(chip->pixelConfig().tdac, enabled[chip]) = bestTDAC[chip];
        chipInterface.UpdatePixelConfig(chip, false, true);
    }); 


    return bestTDAC;
}

template <class Flavor>
void RD53BThresholdEqualization<Flavor>::draw(const ChipDataMap<pixel_matrix_t<Flavor, uint8_t>>& bestTDAC) {
    Base::createRootFile();

    auto& offset = param("injectionTool"_s).param("offset"_s);
    auto& size = param("injectionTool"_s).param("size"_s);
    auto rowRange = xt::range(offset[0], offset[0] + size[0]);
    auto colRange = xt::range(offset[1], offset[1] + size[1]);

    for (const auto& item : bestTDAC) {
        const auto& tdac = xt::view(item.second, rowRange, colRange);
        
        // Base::drawMap(tdac, "TDAC Map", "TDAC", offset[0], offset[1]);
        Base::drawMap(tdac, "TDAC Map", "TDAC", offset[0], offset[1]);
        Base::drawHist(tdac, "TDAC Distribution", 32, 0, 32, "TDAC");
    }
}


template class RD53BThresholdEqualization<RD53BFlavor::ATLAS>;
template class RD53BThresholdEqualization<RD53BFlavor::CMS>;

}