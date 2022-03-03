#ifndef RD53BThresholdEqualizationUnbiased_H
#define RD53BThresholdEqualizationUnbiased_H

#include "RD53BInjectionTool.h"
#include "RD53BThresholdScan.h"

namespace RD53BTools {

template <class>
struct RD53BThresholdEqualizationUnbiased; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdEqualizationUnbiased<Flavor>> = make_named_tuple(
    std::make_pair("thresholdScan"_s, RD53BThresholdScan<Flavor>()),
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("targetThreshold"_s, 0ul)
);

template <class Flavor>
struct RD53BThresholdEqualizationUnbiased : public RD53BTool<RD53BThresholdEqualizationUnbiased, Flavor> {
    using Base = RD53BTool<RD53BThresholdEqualizationUnbiased, Flavor>;
    using Base::Base;
    using Base::param;

    ChipDataMap<xt::xtensor<uint8_t, 2>> run(Task progress);

    void draw(const ChipDataMap<xt::xtensor<uint8_t, 2>>& bestTDAC);
};

}

#endif