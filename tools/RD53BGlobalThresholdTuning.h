#ifndef RD53BGLOBALTHRESHOLDTUNING_H
#define RD53BGLOBALTHRESHOLDTUNING_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BGlobalThresholdTuning; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BGlobalThresholdTuning<Flavor>> = make_named_tuple(
    std::make_pair("targetThreshold"_s, 300),
    std::make_pair("GdacRange"_s, std::vector<size_t>({380, 600}))
    std::make_pair("vcalMed", 300)
);

template <class Flavor>
struct RD53BGlobalThresholdTuning : public RD53BTool<RD53BGlobalThresholdTuning, Flavor> {
    using Base = RD53BTool<RD53BGlobalThresholdTuning, Flavor>;
    using Base::Base;

    using result_type = ChipDataMap<std::array<size_t, 256>>;

    ChipDataMap<size_t> run(Ph2_System::SystemController& system, Task progress) const {
        auto& chipInterface = *static_cast<RD53BInterface<Flavor>*>(system.fReadoutChipInterface);

        auto thresholdMeasurementTask   = progress.subTask({0, 0.2});
        auto tuningTask                 = progress.subTask({0.2, 1});

        auto& injectionTool = param("injectionTool"_s);
        auto& offset = injectionTool.param("offset"_s);
        auto& size = injectionTool.param("size"_s);

        for_each_device<Chip>(system, [&] (Chip* chip) {
            auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
            rd53b->pixelConfig.tdac.fill(15);
            chipInterface.UpdatePixelConfig(rd53b, false, true);
        });


        ChipDataMap<size_t> GDAC;
        ChipDataMap<size_t> bestGDAC;
        ChipDataMap<double> bestOcc;

        auto& GdacRange = param("GdacRange"_s);

        for_each_device<Chip>(system, [&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("VcalMed"_s));
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("targetThreshold"_s));
            GDAC[chip] =  GdacRange[0];
            bestGDAC[chip] = GdacRange[0];
            bestOcc[chip] = 0;
        });
        
        xt::print_options::set_line_width(160);
        xt::print_options::set_threshold(2000);

        auto size = GdacRange[1] - GdacRange[2];
        auto step = size / 2;
        while (step > 0) {

            for_each_device<Chip>(system, [&] (Chip* chip) {
                chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_L_LIN, GDAC[chip]);
                chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_R_LIN, GDAC[chip]);
                chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_M_LIN, GDAC[chip]);
            });

            auto events = injectionTool.run(system, tuningTask.subTask({i / double(nSteps), (i + .5) / double(nSteps)}));
            auto occMap = injectionTool.occupancy(events);

            for_each_device<Chip>(system, [&] (Chip* chip) {
                auto mean_occ = xt::mean(occMap[]);
                if (std::abs(mean_occ - 0.5) < std::abs(bestOcc[chip] - 0.5) + 1e-10) {
                    bestGDAC[chip] = GDAC[chip];
                    bestOcc[chip] = mean_occ;
                }
                if (mean_occ > 0.5)
                    GDAC[chip] += step;
                else 
                    GDAC[chip] -= step;
            });

            step = step / 2;
        }


        for_each_device<Chip>(system, [&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_L_LIN, bestGDAC[chip]);
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_R_LIN, bestGDAC[chip]);
            chipInterface.WriteReg(chip, Flavor::Reg::DAC_GDAC_M_LIN, bestGDAC[chip]);
        }); 

        return bestGDAC;
    }

    void draw(const ChipDataMap<size_t>& bestGDAC) const {

        for (const auto& item : bestGDAC) {
            LOG(INFO) << "Best GDAC is " << item.second << " for chip: " << item.first << RESET;
        }
    }
};

}

#endif