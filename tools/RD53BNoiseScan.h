#ifndef RD53BNOISESCAN_H
#define RD53BNOISESCAN_H

#include "RD53BTool.h"
#include "../Utils/RD53BEventDecoding.h"

class TH1;

namespace RD53BTools {

template <class>
struct RD53BNoiseScan; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BNoiseScan<Flavor>> = make_named_tuple(
    std::make_pair("nTriggers"_s, 10ul),
    std::make_pair("triggerPeriod"_s, 100ul),
    std::make_pair("triggerDuration"_s, 10ul),
    std::make_pair("triggerLatency"_s, 133ul),
    std::make_pair("readoutPeriod"_s, 0ul),
    std::make_pair("offset"_s, std::vector<size_t>({0, 0})),
    std::make_pair("size"_s, std::vector<size_t>({0, 0})),
    std::make_pair("occupancyThreshold"_s, 10e-6),
    std::make_pair("maskNoisyPixels"_s, false)
);



template <class Flavor>
struct RD53BNoiseScan : public RD53BTool<RD53BNoiseScan, Flavor> {
    using Base = RD53BTool<RD53BNoiseScan, Flavor>;
    using Base::Base;
    using Base::param;

    using ChipEventsMap = ChipDataMap<std::vector<RD53BEventDecoding::RD53BEvent>>;

    struct ChipResult {
        pixel_matrix_t<Flavor, bool> enabled;
        std::vector<RD53BEventDecoding::RD53BEvent> events;
    };

    void init();

    ChipDataMap<ChipResult> run(Task progress) const;

    ChipDataMap<pixel_matrix_t<Flavor, size_t>> hitCount(const ChipDataMap<ChipResult>& data) const;

    ChipDataMap<std::array<double, 16>> totDistribution(const ChipDataMap<ChipResult>& data) const;

    void draw(const ChipDataMap<ChipResult>& result);
};

}

#endif