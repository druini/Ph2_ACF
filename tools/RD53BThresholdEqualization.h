#ifndef RD53BTHRESHOLDEQUALIZATION_H
#define RD53BTHRESHOLDEQUALIZATION_H

#include "RD53BInjectionTool.h"
#include "RD53BThresholdScan.h"

namespace RD53BTools {

template <class>
struct RD53BThresholdEqualization; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BThresholdEqualization<Flavor>> = make_named_tuple(
    std::make_pair("thresholdScan"_s, RD53BThresholdScan<Flavor>()),
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("targetThreshold"_s, 0ul),
    std::make_pair("initialTDAC"_s, 15ul),
    std::make_pair("nSteps"_s, 6ul),
    std::make_pair("eliminateBias"_s, false)
);

template <class Flavor>
struct RD53BThresholdEqualization : public RD53BTool<RD53BThresholdEqualization, Flavor> {
    using Base = RD53BTool<RD53BThresholdEqualization, Flavor>;
    using Base::Base;
    using Base::param;

    ChipDataMap<pixel_matrix_t<Flavor, uint8_t>> run(Task progress);

    void draw(const ChipDataMap<pixel_matrix_t<Flavor, uint8_t>>& bestTDAC);
};

}

#endif