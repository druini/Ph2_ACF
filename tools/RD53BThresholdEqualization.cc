#include "RD53BThresholdEqualization.h"

#include "../Utils/xtensor/xmasked_view.hpp"
#include "../Utils/xtensor/xindex_view.hpp"
#include "../Utils/xtensor/xio.hpp"

#include <TH2F.h>
#include <TApplication.h>
#include <TFile.h>

namespace RD53BTools {

template <class Flavor>
ChipDataMap<xt::xtensor<uint8_t, 2>> RD53BThresholdEqualization<Flavor>::run(Ph2_System::SystemController& system, Task progress) {
    auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

    auto thresholdMeasurementTask   = progress.subTask({0, 0.2});
    auto tuningTask                 = progress.subTask({0.2, 1});

    auto& injectionTool = param("injectionTool"_s);
    auto& offset = injectionTool.param("offset"_s);
    auto& size = injectionTool.param("size"_s);

    for_each_device<Chip>(system, [&] (Chip* chip) {
        auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
        rd53b->pixelConfig.tdac.fill(15);
        chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);
    });

    ChipDataMap<size_t> VCAL_HIGH;
    if (param("targetThreshold"_s) == 0) {
        auto scanOccMap = param("thresholdScan"_s).run(system, thresholdMeasurementTask);
        auto thresholdMap = param("thresholdScan"_s).analyze(scanOccMap)[0];
        
        for_each_device<Chip>(system, [&] (Chip* chip) {
            double meanThreshold = xt::mean(xt::filter(thresholdMap[chip], xt::view(scanOccMap[chip], -1, xt::all(), xt::all()) > .9))();
            VCAL_HIGH[chip] = param("thresholdScan"_s).param("vcalMed"_s) + meanThreshold;
        });
    }
    else {
        for_each_device<Chip>(system, [&] (Chip* chip) {
            VCAL_HIGH[chip] = param("thresholdScan"_s).param("vcalMed"_s) + param("targetThreshold"_s);
        });
    }

    ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDAC;
    ChipDataMap<xt::xtensor<double, 2>> bestOcc;

    for_each_device<Chip>(system, [&] (Chip* chip) {
        chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
        bestTDAC.insert({chip, 15 * xt::ones<uint8_t>({size[0], size[1]})});
        bestOcc.insert({chip, xt::zeros<double>({size[0], size[1]})});
    });
    
    xt::print_options::set_line_width(160);
    xt::print_options::set_threshold(2000);

    size_t nSteps = 6;
    for (size_t i = 0; i < nSteps; ++i) {
        size_t tdacStep = i < 4 ? (8 >> i) : 1;
    // for (size_t i = 0; i < nSteps; ++i) {

        // for_each_device<Chip>(system, [&] (Chip* chip) {
        //     auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
        //     auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
        //     tdacView.fill(i);
        //     chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);
        // });
        for_each_device<Chip>(system, [&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, VCAL_HIGH[chip]);
        });
        
        auto events = injectionTool.run(system, tuningTask.subTask({i / double(nSteps), (i + .5) / double(nSteps)}));
        auto occMap = injectionTool.occupancy(events);

        for_each_device<Hybrid>(system, [&] (Hybrid* hybrid) {
            chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, 0xFFFF);
        });
        
        auto eventsHighCharge = injectionTool.run(system, tuningTask.subTask({(i + .5) / double(nSteps), (i + 1) / double(nSteps)}));
        auto occMapHighCharge = injectionTool.occupancy(eventsHighCharge);

        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
            auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
            const auto occ = xt::view(occMap[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
            const auto occHighCharge = xt::view(occMapHighCharge[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));

            const auto stuck = occHighCharge < .9;

            // std::cout << "stuck: " << xt::count_nonzero(stuck) << std::endl;

            // if (i == 0) {
            //     bestOcc[chip] = occ;
            //     bestTDAC[chip] = tdacView;
            // }
            // else {
            auto isBest = !stuck && (xt::abs(occ - 0.5) <= xt::abs(bestOcc[chip] - .5) + 1e-10);
            xt::masked_view(bestOcc[chip], isBest) = occ;
            xt::masked_view(bestTDAC[chip], isBest) = tdacView;
            // }

            // std::cout << "isBest:\n" <<  xt::filter(isBest, xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
            // std::cout << "diff:\n" <<  xt::filter((xt::abs(occ - 0.5) - xt::abs(bestOcc[chip] - .5)), xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
            // std::cout << "occ:\n" <<  xt::filter(occ, xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
            // std::cout << "bestOcc:\n" <<  xt::filter(bestOcc[chip], xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
            // std::cout << "bestTDAC:\n" <<  xt::filter(bestTDAC[chip], xt::abs(bestOcc[chip] - .5) > .49) << std::endl;

            if (i < nSteps - 1) {
                tdacView = xt::where(!stuck && (occ < 0.5), tdacView + tdacStep, xt::where(tdacView > tdacStep, tdacView - tdacStep, 0u));
                // std::cout << tdacView << std::endl;

                chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true); // update tdacs
            }
        });
    }


    for_each_device<Chip>(system, [&] (Chip* chip) {
        
        // std::cout << "bestOcc:\n" << bestOcc[chip] << std::endl;
        // std::cout << "bestTDAC:\n" << bestTDAC[chip] << std::endl;
        
        auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
        auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
        tdacView = bestTDAC[chip];
        // std::cout << rd53b->pixelConfig.tdac << std::endl;
        chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);
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