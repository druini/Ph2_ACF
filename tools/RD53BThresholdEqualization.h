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
        auto tuningTask                 = progress.subTask({0.2, 1});

        auto& injectionTool = param("injectionTool"_s);
        auto& offset = injectionTool.param("offset"_s);
        auto& size = injectionTool.param("size"_s);

        auto scanOccMap = param("thresholdScan"_s).run(system, thresholdMeasurementTask);
        auto thresholdMap = param("thresholdScan"_s).analyze(scanOccMap)[0];

        ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDAC;
        ChipDataMap<xt::xtensor<double, 2>> bestOcc;
        ChipDataMap<xt::xtensor<double, 2>> minOcc;
        ChipDataMap<xt::xtensor<double, 2>> maxOcc;
        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
            chipInterface.WriteReg(rd53b, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
            rd53b->pixelConfig.tdac.fill(15);
            chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);

            bestTDAC.insert({rd53b, xt::zeros<uint8_t>({size[0], size[1]})});
            bestOcc.insert({rd53b, xt::ones<double>({size[0], size[1]})});
            
            minOcc.insert({rd53b, xt::ones<double>({size[0], size[1]})});
            maxOcc.insert({rd53b, xt::zeros<double>({size[0], size[1]})});
        });
        
        xt::print_options::set_line_width(160);
        xt::print_options::set_threshold(2000);

        // size_t nSteps = 7;
        // for (size_t i = 0; i < nSteps; ++i) {
        //     size_t tdacStep = i < 4 ? (8 >> i) : 1;
        for (size_t i = 0; i < 32; ++i) {

            for_each_device<Chip>(system, [&] (Chip* chip) {
                chipInterface.WriteReg(rd53b, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
                rd53b->pixelConfig.tdac.fill(i);
                chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true);

                double meanThreshold = xt::mean(thresholdMap[chip])();
                chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("thresholdScan"_s).param("vcalMed"_s) + meanThreshold);
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

                std::cout << "stuck: " << xt::count_nonzero(stuck) << std::endl;

                if (i == 0) {
                    bestOcc[chip] = occ;
                    bestTDAC[chip] = tdacView;
                }
                else {
                    auto isBest = !stuck && (xt::abs(occ - 0.5) <= xt::abs(bestOcc[chip] - .5));
                    xt::masked_view(bestOcc[chip], isBest) = occ;
                    xt::masked_view(bestTDAC[chip], isBest) = tdacView;
                }

                minOcc[chip] = xt::minimum(occ, minOcc[chip]);
                maxOcc[chip] = xt::maximum(occ, maxOcc[chip]);
                
                if (i < nSteps - 1) {
                    tdacView = xt::where(!stuck && (occ < 0.5), tdacView + tdacStep, xt::where(tdacView > tdacStep, tdacView - tdacStep, 0u));
                    // std::cout << tdacView << std::endl;

                    chipInterface.UpdatePixelConfig(rd53b, rd53b->pixelConfig, false, true); // update tdacs
                }
            });
        }


        for_each_device<Chip>(system, [&] (Chip* chip) {

            std::cout << "minOcc:\n" << minOcc[chip] << std::endl;
            std::cout << "maxOcc:\n" << maxOcc[chip] << std::endl;
            
            std::cout << "bestOcc:\n" << bestOcc[chip] << std::endl;
            std::cout << "bestTDAC:\n" << bestTDAC[chip] << std::endl;
            
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