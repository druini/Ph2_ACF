#ifndef RD53BGainTuning_H
#define RD53BGainTuning_H

#include "RD53BInjectionTool.h"

namespace RD53BTools {

template <class>
struct RD53BGainTuning; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BGainTuning<Flavor>> = make_named_tuple(
    std::make_pair("vcalMed"_s, 300ul),
    std::make_pair("targetVcal"_s, 1090),
    std::make_pair("targetToT"_s, 5.0),
    std::make_pair("krumCurrRange"_s, std::vector<size_t>({0, 300})),
    std::make_pair("injectionTool"_s, RD53BInjectionTool<Flavor>())
);

template <class Flavor>
struct RD53BGainTuning : public RD53BTool<RD53BGainTuning, Flavor> {
    using Base = RD53BTool<RD53BGainTuning, Flavor>;
    using Base::Base;
    using Base::param;

    bool run(Task progress) const;
};

}

#endif