#ifndef RD53BTHRESHOLDSCAN_H
#define RD53BTHRESHOLDSCAN_H

#include "RD53BInjectionTool.h"

#include "../Utils/xtensor/xoptional.hpp"

namespace RD53BTools {

template <class>
struct RD53BThresholdScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdScan<Flavor>> = make_named_tuple(
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("vcalMed"_s, 300u),
    std::make_pair("vcalRange"_s, std::vector<size_t>({200, 8000})),
    std::make_pair("vcalStep"_s, 20u)
);

template <class Flavor>
struct RD53BThresholdScan : public RD53BTool<RD53BThresholdScan, Flavor> {
    using Base = RD53BTool<RD53BThresholdScan, Flavor>;
    using Base::Base;
    using Base::param;

    using OccupancyMap = ChipDataMap<xt::xtensor<double, 3>>;

    void init();

    OccupancyMap run(Task progress) const;

    std::array<ChipDataMap<xt::xtensor_optional<double, 2>>, 2> analyze(const OccupancyMap& occMap) const;

    void draw(const OccupancyMap& occMap);

private:
    const auto& offset(size_t i) const { return param("injectionTool"_s).param("offset"_s)[i]; }
    const auto& size(size_t i) const { return param("injectionTool"_s).param("size"_s)[i]; }
    auto rowRange() const { return xt::range(offset(0), offset(0) + size(0)); }
    auto colRange() const { return xt::range(offset(1), offset(1) + size(1)); }

    xt::xtensor<size_t, 1> vcalBins;
};

}

#endif