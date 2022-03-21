#ifndef RD53BREGREADER_H
#define RD53BREGREADER_H

#include "RD53BTool.h"

namespace RD53BTools {

template <class>
struct RD53BRegReader; // forward declaration

template <class Flavor>
const auto ToolParameters<RD53BRegReader<Flavor>> = make_named_tuple();

template <class Flavor>
struct RD53BRegReader : public RD53BTool<RD53BRegReader, Flavor> {
    using Base = RD53BTool<RD53BRegReader, Flavor>;
    using Base::Base;

    bool run(Task progress) const;
};

}

#endif