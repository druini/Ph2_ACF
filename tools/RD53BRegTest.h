#ifndef RD53BREGTEST_H
#define RD53BREGTEST_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BRegTest; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegTest<Flavor>> = make_named_tuple(
    std::make_pair("testRegs"_s, true),
    std::make_pair("testPixels"_s, true),
    std::make_pair("ServiceFrameSkip"_s, 50)
);

template <class Flavor>
struct RD53BRegTest : public RD53BTool<RD53BRegTest, Flavor> {
    using Base = RD53BTool<RD53BRegTest, Flavor>;
    using Base::Base;

    bool run(Task progress) const;
};

}

#endif