#ifndef RD53BStuckPixelScan_H
#define RD53BStuckPixelScan_H

#include "RD53BInjectionTool.h"

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

    bool run(Task progress) const;

};

}

#endif