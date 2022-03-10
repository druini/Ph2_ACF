#ifndef RD53BStuckPixelScan_H
#define RD53BStuckPixelScan_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BStuckPixelScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BStuckPixelScan<Flavor>> = make_named_tuple(
    std::make_pair("vcal"_s, 3000),
    std::make_pair("vcalMed"_s, 300),
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("occupancyThreshold"_s, 0.9)
);

template <class Flavor>
struct RD53BStuckPixelScan : public RD53BTool<RD53BStuckPixelScan, Flavor> {
    using Base = RD53BTool<RD53BStuckPixelScan, Flavor>;
    using Base::Base;
    using Base::param;

    using result_type = ChipDataMap<std::array<size_t, 256>>;

    auto run(Task progress) const {
        auto& chipInterface = Base::chipInterface();

        Base::for_each_chip([&] (Chip* chip) {
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("vcalMed"_s));
            chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, param("vcalMed"_s) + param("vcal"_s));
        });

        auto events = param("injectionTool"_s).run(progress);
        auto occupancy = param("injectionTool"_s).occupancy(events);

        Base::for_each_chip([&] (Chip* chip) {
            auto rd53b = static_cast<RD53B<Flavor>*>(chip);
            auto stuck = occupancy[chip] < param("occupancyThreshold"_s);
            LOG(INFO) << "Masking " << xt::count_nonzero(rd53b->pixelConfig().enable && stuck) << " stuck pixels for chip: " << ChipLocation(chip) << RESET;
            rd53b->pixelConfig().enable &= !stuck;
            chipInterface.UpdatePixelConfig(chip, true, false);
        });

        return true;
    }

};

}

#endif