#ifndef RD53BGLOBALTHRESHOLDTUNING_H
#define RD53BGLOBALTHRESHOLDTUNING_H

#include "RD53BInjectionTool.h"

namespace RD53BTools {

template <class>
struct RD53BGlobalThresholdTuning; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BGlobalThresholdTuning<Flavor>> = make_named_tuple(
    std::make_pair("targetThreshold"_s, 300),
    std::make_pair("gdacRange"_s, std::vector<size_t>({380, 600})),
    std::make_pair("vcalMed"_s, 300),
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("maxStuckPixelRatio"_s, 0.1)
);

template <class Flavor>
struct RD53BGlobalThresholdTuning : public RD53BTool<RD53BGlobalThresholdTuning, Flavor> {
    using Base = RD53BTool<RD53BGlobalThresholdTuning, Flavor>;
    using Base::Base;
    using Base::param;

    ChipDataMap<size_t> run(Task progress) const;

    void draw(const ChipDataMap<size_t>& bestGDAC) const;
};

}

#endif