#include "RD53BThresholdEqualization.h"

#include "../Utils/xtensor/xmasked_view.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"
#include "../Utils/xtensor/xrandom.hpp"

#include <TH2F.h>
#include <TApplication.h>
#include <TFile.h>

namespace RD53BTools {

template <class Flavor>
ChipDataMap<xt::xtensor<uint8_t, 2>> RD53BThresholdEqualization<Flavor>::run(Task progress) {
    auto& chipInterface = Base::chipInterface();

    auto thresholdMeasurementTask   = progress.subTask({0, 0.2});
    auto tuningTask                 = progress.subTask({0.2, 1});

    auto& injectionTool = param("injectionTool"_s);
    auto& offset = injectionTool.param("offset"_s);
    auto& size = injectionTool.param("size"_s);

    ChipDataMap<pixel_matrix_t<Flavor, uint8_t>> tdac;
    ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDAC;
    ChipDataMap<xt::xtensor<double, 2>> bestOcc;

    Base::for_each_chip([&] (Chip* chip) {
        tdac[chip].fill(param("initialTDAC"_s));
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
        bestTDAC.insert({chip, param("initialTDAC"_s) * xt::ones<uint8_t>({size[0], size[1]})});
        bestOcc.insert({chip, xt::zeros<double>({size[0], size[1]})});
        // auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
        // rd53b->pixelConfig.tdac.fill(param("initialTDAC"_s));
        // chipInterface.UpdatePixelConfig(rd53b, false, true);
        chipInterface.UpdatePixelTDACUniform(chip, param("initialTDAC"_s));
    });

    ChipDataMap<size_t> VCAL_HIGH;
    if (param("targetThreshold"_s) == 0) {
        auto scanOccMap = param("thresholdScan"_s).run(thresholdMeasurementTask);
        auto thresholdMap = param("thresholdScan"_s).analyze(scanOccMap)[0];
        
        Base::for_each_chip([&] (Chip* chip) {
            double meanThreshold = xt::mean(xt::filter(thresholdMap[chip], xt::view(scanOccMap[chip], -1, xt::all(), xt::all()) > .9))();
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
        int tdacStep = i < 4 ? (8 >> i) : 1;
        
        Base::for_each_chip([&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, VCAL_HIGH[chip]);
        });
        
        auto events = injectionTool.run(tuningTask.subTask({i / double(nSteps), (i + .5) / double(nSteps)}));
        auto occMap = injectionTool.occupancy(events);

        Base::for_each_hybrid([&] (Hybrid* hybrid) {
            chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, 0xFFFF);
        });
        
        auto eventsHighCharge = injectionTool.run(tuningTask.subTask({(i + .5) / double(nSteps), (i + 1) / double(nSteps)}));
        auto occMapHighCharge = injectionTool.occupancy(eventsHighCharge);

        Base::for_each_chip([&] (Chip* chip) {
            auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
            auto tdacView = xt::view(tdac[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
            const auto occ = xt::view(occMap[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
            const auto occHighCharge = xt::view(occMapHighCharge[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));

            const auto stuck = occHighCharge < .9;

            // auto isBest = !stuck && (xt::abs(occ - 0.5) <= xt::abs(bestOcc[chip] - .5) + 1e-10);
            // xt::masked_view(bestOcc[chip], isBest) = occ;
            // xt::masked_view(bestTDAC[chip], isBest) = tdacView;

            auto cost = xt::abs(occ - 0.5);
            auto minCost = xt::abs(bestOcc[chip] - 0.5);

            xt::xtensor<bool, 2> isBest = cost < minCost || xt::isclose(occ, bestOcc[chip]);

            if (param("eliminateBias"_s) && i == nSteps - 1)
                isBest |= (xt::isclose(cost, minCost) && xt::random::randint<int>(size, 0, 1));

            isBest &= !stuck;
            
            LOG(INFO) << "Step: " << i;
            LOG(INFO) << "nStuck: " <<  xt::count_nonzero(stuck);
            LOG(INFO) << "nUpdated: " <<  xt::count_nonzero(isBest);
            LOG(INFO) << "nEquivalent: " << xt::count_nonzero(xt::isclose(cost, minCost));
            LOG(INFO) << "nActuallyBetter: " << xt::count_nonzero(cost < minCost && !xt::isclose(cost, minCost));
            LOG(INFO) << "nUncertain: " <<  xt::count_nonzero(minCost > 0.4);
            LOG(INFO) << "==========================================";

            xt::masked_view(bestOcc[chip], isBest) = occ;
            xt::masked_view(bestTDAC[chip], isBest) = tdacView;

            if (i < nSteps - 1) {
                tdacView = xt::clip(xt::where(!stuck && (occ < 0.5), tdacView + tdacStep, tdacView - tdacStep), 0, 31);

                chipInterface.UpdatePixelTDAC(rd53b, tdac[chip]); // update tdacs
            }
        });
    }


    Base::for_each_chip([&] (RD53B<Flavor>* chip) {
        auto tdacView = xt::view(chip->pixelConfig().tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
        tdacView = bestTDAC[chip];
        chipInterface.UpdatePixelConfig(chip, false, true);
    }); 


    return bestTDAC;
}

template <class Flavor>
void RD53BThresholdEqualization<Flavor>::draw(const ChipDataMap<xt::xtensor<uint8_t, 2>>& bestTDAC) {
    Base::createRootFile();
    auto& offset = param("injectionTool"_s).param("offset"_s);

    for (const auto& item : bestTDAC) {
        const auto& tdac = item.second;
        
        Base::drawMap(tdac, "TDAC Map", "TDAC", offset[0], offset[1]);
        Base::drawHist(tdac, "TDAC Distribution", 32, 0, 32, "TDAC");
    }
}


template class RD53BThresholdEqualization<RD53BFlavor::ATLAS>;
template class RD53BThresholdEqualization<RD53BFlavor::CMS>;

}