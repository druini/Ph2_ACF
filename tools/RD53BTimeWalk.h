#ifndef RD53BTimeWalk_H
#define RD53BTimeWalk_H

#include "RD53BInjectionTool.h"

namespace RD53BTools {

template <class>
struct RD53BTimeWalk; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BTimeWalk<Flavor>> = make_named_tuple(
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>()),
    std::make_pair("vcalMed"_s, 300u),
    std::make_pair("vcalRange"_s, std::vector<size_t>({200, 800})),
    std::make_pair("vcalStep"_s, 20u),
    std::make_pair("fineDelayStep"_s, 1u),
    std::make_pair("storeHits"_s, false)
);

template <class Flavor>
struct RD53BTimeWalk : public RD53BTool<RD53BTimeWalk, Flavor> {
    using Base = RD53BTool<RD53BTimeWalk, Flavor>;
    using Base::Base;
    using Base::param;

    using Result = xt::xtensor<tool_result_t<RD53BInjectionTool<Flavor>>, 2>;
    // using Result = xt::xtensor<ChipDataMap<std::vector<RD53BEvent>>, 2>;

    void init();
    Result run(Task progress) const;
    void draw(const Result& lateHitRatio);

private:
    size_t nVcalSteps;
    size_t nDelaySteps;
};

}

#endif