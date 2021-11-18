#ifndef RD53BINJECTIONTOOL_H
#define RD53BINJECTIONTOOL_H

#include "RD53BTool.h"
#include "../Utils/RD53BEventDecoding.h"

class TH1;

namespace RD53BTools {

template <class>
struct RD53BInjectionTool; // forward declaration

using MaskStep = decltype(make_named_tuple(
    std::make_pair("dim"_s, size_t{}),
    std::make_pair("size"_s, size_t{}),
    std::make_pair("parallel"_s, bool{}),
    std::make_pair("shift"_s, std::vector<size_t>{})
));

template <class Flavor>
const auto ToolParameters<RD53BInjectionTool<Flavor>> = make_named_tuple(
    std::make_pair("nInjections"_s, 10ul),
    std::make_pair("triggerDuration"_s, 10ul),
    std::make_pair("triggerLatency"_s, 133ul),
    std::make_pair("injectionType"_s, std::string("Analog")),
    std::make_pair("readoutPeriod"_s, 0ul),
    std::make_pair("injectionPeriod"_s, 800ul),
    std::make_pair("fineDelay"_s, 0ul),
    std::make_pair("pulseDuration"_s, 2ul),
    std::make_pair("offset"_s, std::vector<size_t>({0, 0})),
    std::make_pair("size"_s, std::vector<size_t>({0, 0})),
    std::make_pair("maskGen"_s, std::vector<MaskStep>({
        MaskStep(0, 0, 0, {8, 1}),
        MaskStep(1, 0, 1, {}),
        MaskStep(0, 21, 1, {})
    })),
    std::make_pair("showPlots"_s, true)
);

template <class Flavor>
struct RD53BInjectionTool : public RD53BTool<RD53BInjectionTool, Flavor> {
    using Base = RD53BTool<RD53BInjectionTool, Flavor>;
    using Base::Base;
    using Base::param;

    using ChipEventsMap = ChipDataMap<std::vector<RD53BEventDecoding::RD53BEvent>>;

    void init();

    ChipEventsMap run(Ph2_System::SystemController& system, Task progress) const;

    void configureInjections(Ph2_System::SystemController& system) const;

    void setupMaskFrame(Ph2_System::SystemController& system, size_t frameId) const;

    void inject(Ph2_System::SystemController& system, ChipEventsMap& events) const;

    ChipDataMap<pixel_matrix_t<Flavor, double>> occupancy(const ChipEventsMap& data) const;

    ChipDataMap<std::array<double, 16>> totDistribution(const ChipEventsMap& data) const;

    size_t nFrames() const { return _nFrames; }

    void draw(const ChipEventsMap& result) const;

private:
    auto generateInjectionMask(size_t i) const;

    static void ReverseYAxis(TH1 *h);

    size_t _nFrames;
};

}

#endif