#ifndef RD53BTHRESHOLDEQUALIZATION_H
#define RD53BTHRESHOLDEQUALIZATION_H

#include "RD53BInjectionTool.h"

#include "../Utils/xtensor/xmasked_view.hpp"
#include "../Utils/xtensor/xio.hpp"

namespace RD53BTools {

template <class>
struct RD53BThresholdEqualization; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdEqualization<Flavor>> = make_named_tuple(
    std::make_pair("thresholdScan"_s, RD53BThresholdScan<Flavor>()),
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>())
);

template <class Flavor>
struct RD53BThresholdEqualization : public RD53BTool<RD53BThresholdEqualization, Flavor> {
    using Base = RD53BTool<RD53BThresholdEqualization, Flavor>;
    using Base::Base;
    using Base::param;

    using OccupancyMap = ChipDataMap<xt::xtensor<double, 3>>;

    auto run(Ph2_System::SystemController& system, Task progress) {
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        auto thresholdMeasurementTask   = progress.subTask({0, 0.2});
        auto tuningTask                 = progress.subTask({0.2, 0.9});

        auto& injectionTool = param("injectionTool"_s);
        auto& offset = injectionTool.param("offset"_s);
        auto& size = injectionTool.param("size"_s);

        auto scanOccMap = param("thresholdScan"_s).run(system, thresholdMeasurementTask);
        auto thresholdMap = param("thresholdScan"_s).analyze(scanOccMap)[0];

        ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDAC;
        // ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDACMin;
        // ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDACMax;
        ChipDataMap<xt::xtensor<double, 2>> bestOcc;
        // ChipDataMap<xt::xtensor<double, 2>> bestOccMin;
        // ChipDataMap<xt::xtensor<double, 2>> bestOccMax;
        // ChipDataMap<xt::xtensor<double, 2>> lastOcc;
        ChipDataMap<xt::xtensor<double, 2>> minOcc;
        ChipDataMap<xt::xtensor<double, 2>> maxOcc;
        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
            chipInterface.WriteReg(rd53b, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
            std::cout << thresholdMap[chip] << std::endl;
            double meanThreshold = xt::mean(thresholdMap[chip])();
            std::cout << "meanThreshold = " << meanThreshold << std::endl;
            size_t VCAL_HIGH = param("thresholdScan"_s).param("vcalMed"_s) + meanThreshold;
            std::cout << "VCAL_HIGH = " << VCAL_HIGH << std::endl;
            chipInterface.WriteReg(rd53b, Flavor::Reg::VCAL_HIGH, VCAL_HIGH);
            rd53b->pixelConfig.tdac.fill(15);
            chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);
            bestTDAC.insert({rd53b, xt::zeros<uint8_t>({size[0], size[1]})});
            // bestTDACMax.insert({rd53b, xt::zeros<uint8_t>({size[0], size[1]})});
            // bestTDACMin.insert({rd53b, 31 * xt::ones<uint8_t>({size[0], size[1]})});
            bestOcc.insert({rd53b, xt::ones<double>({size[0], size[1]})});
            // bestOccMin.insert({rd53b, xt::zeros<double>({size[0], size[1]})});
            // bestOccMax.insert({rd53b, xt::ones<double>({size[0], size[1]})});
            // lastOcc.insert({rd53b, xt::ones<double>({size[0], size[1]})});
            minOcc.insert({rd53b, xt::ones<double>({size[0], size[1]})});
            maxOcc.insert({rd53b, xt::zeros<double>({size[0], size[1]})});
        });
        
        xt::print_options::set_line_width(160);
        xt::print_options::set_threshold(2000);

        for (size_t i = 0; i < 6; ++i) {
            std::cout << "step " << i << std::endl;
            size_t tdacStep = i < 4 ? (8 >> i) : 1;

            auto events = injectionTool.run(system, tuningTask.subTask({i / 5., (i + 1) / 5.}));
            auto occMap = injectionTool.occupancy(events);

            for_each_device<Chip>(system, [&] (Chip* chip) {
                auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
                auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
                auto occ = xt::view(occMap[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
                // std::cout << xt::adapt(occ.shape()) << std::endl;
                // std::copy(occ.shape().cbegin(), occ.shape().cend(), std::ostream_iterator<unsigned long>(std::cout, ", "));
                std::cout << occ << std::endl;
                // std::cout << xt::abs(occ - 0.5) << std::endl;
                // std::cout << xt::abs(bestOcc[chip] - .5) << std::endl;
                if (i == 0) {
                    // auto isAboveThreshold = occ > 0.5;
                    // xt::masked_view(bestOccMax[chip], isAboveThreshold) = occ;
                    // xt::masked_view(bestOccMin[chip], !isAboveThreshold) = occ;
                    // xt::masked_view(bestTDACMax[chip], isAboveThreshold) = tdacView;
                    // xt::masked_view(bestTDACMin[chip], !isAboveThreshold) = tdacView;
                    // xt::where(occ > 0.5, bestOccMax[chip], bestOccMin[chip]) = occ;
                    // xt::where(occ > 0.5, bestTDACMax[chip], bestTDACMin[chip]) = tdacView;
                    bestOcc[chip] = occ;
                    bestTDAC[chip] = tdacView;
                }
                else {
                    // auto isAboveThreshold = occ > 0.5;
                    // auto isBest = (!isAboveThreshold && occ > bestOccMin[chip]) || (isAboveThreshold && occ < bestOccMin[chip]);
                    auto isBest = xt::abs(occ - 0.5) <= xt::abs(bestOcc[chip] - .5);
                    std::cout << "nBest = " << xt::count_nonzero(isBest) << std::endl;
                    // xt::masked_view(bestOccMax[chip], isAboveThreshold && isBest) = occ;
                    // xt::masked_view(bestOccMin[chip], !isAboveThreshold && isBest) = occ;
                    // xt::masked_view(bestTDACMax[chip], isAboveThreshold && isBest) = tdacView;
                    // xt::masked_view(bestTDACMin[chip], !isAboveThreshold && isBest) = tdacView;
                    // xt::masked_view(xt::where(occ > 0.5, bestOccMax[chip], bestOccMin[chip]), isBest) = xt::masked_view(occ, isBest);
                    // xt::masked_view(xt::where(occ > 0.5, bestTDACMax[chip], bestTDACMin[chip]), isBest) = xt::masked_view(occ, isBest);
                    xt::masked_view(bestOcc[chip], isBest) = occ;
                    xt::masked_view(bestTDAC[chip], isBest) = tdacView;
                }

                minOcc[chip] = xt::minimum(occ, minOcc[chip]);
                maxOcc[chip] = xt::maximum(occ, maxOcc[chip]);
                // lastOcc[chip] = occ;

                // std::cout << bestTDAC[chip] << std::endl;
                if (i < 5) {
                    tdacView = xt::where(occ > 0.5, tdacView + tdacStep, xt::where(tdacView < tdacStep, 0u, tdacView - tdacStep));

                    std::cout << tdacView << std::endl;

                    chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true); // update tdacs
                }
            });
        }


        for_each_device<Chip>(system, [&] (Chip* chip) {

            std::cout << "minOcc:\n" << minOcc[chip] << std::endl;
            std::cout << "maxOcc:\n" << maxOcc[chip] << std::endl;
            // std::cout << "bestOccMin:\n" << bestOccMin[chip] << std::endl;
            // std::cout << "bestOccMax:\n" << bestOccMax[chip] << std::endl;

            // std::cout << "bestTDACMin:\n" << bestTDACMin[chip] << std::endl;
            // std::cout << "bestTDACMax:\n" << bestTDACMax[chip] << std::endl;
            
            std::cout << "bestOcc:\n" << bestOcc[chip] << std::endl;
            std::cout << "bestTDAC:\n" << bestTDAC[chip] << std::endl;

            std::cout << bestTDAC[chip] << std::endl; 
            auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
            auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
            tdacView = bestTDAC[chip];
            chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);
        }); 

        return true;
    }
};

}

#endif