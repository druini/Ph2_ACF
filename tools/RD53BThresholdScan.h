#ifndef RD53BTHRESHOLDSCAN_H
#define RD53BTHRESHOLDSCAN_H

#include "RD53BInjectionTool.h"

namespace RD53BTools {

template <class>
struct RD53BThresholdScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdScan<Flavor>> = make_named_tuple(
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("vcalMed"_s, 300u),
    std::make_pair("vcalHighRange"_s, std::vector<size_t>({500, 1000})),
    std::make_pair("vcalHighStep"_s, 20u),
    std::make_pair("TDAC"_s, -1)
);

template <class Flavor>
struct RD53BThresholdScan : public RD53BTool<RD53BThresholdScan, Flavor> {
    using Base = RD53BTool<RD53BThresholdScan, Flavor>;
    using Base::Base;
    using Base::param;

    using OccupancyMap = ChipDataMap<xt::xtensor<double, 3>>;

    OccupancyMap run(Ph2_System::SystemController& system, Task progress);

    std::array<ChipDataMap<xt::xtensor<double, 2>>, 2> analyze(const OccupancyMap& occMap) const;

    void draw(const OccupancyMap& occMap) const;

};

}

#endif