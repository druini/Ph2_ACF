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
    std::make_pair("fineDelayStep"_s, 1u)
);

template <class Flavor>
struct RD53BTimeWalk : public RD53BTool<RD53BTimeWalk, Flavor> {
    using Base = RD53BTool<RD53BTimeWalk, Flavor>;
    using Base::Base;
    using Base::param;

    using Result = ChipDataMap<xt::xtensor<double, 2>>;

    Result run(Task progress) const;
    void draw(const Result& lateHitRatio);
};

}

#endif