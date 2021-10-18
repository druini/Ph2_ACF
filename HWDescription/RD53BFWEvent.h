#ifndef RD53BFWEVENT_H
#define RD53BFWEVENT_H

// #include "RD53BEventDecoder.h"

namespace RD53BEventDecoding {

using namespace BitSerialization;
using namespace Ph2_HwDescription;


// extern template decltype(EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::ATLAS, false, true, false>>;
// extern template decltype(EventStream<Config<RD53BFlavor::Flavor::CMS, false, true, false>>) EventStream<Config<RD53BFlavor::Flavor::CMS, false, true, false>>;

template <RD53BFlavor::Flavor Flavor>
const auto FWEvent = Object(
    Field("header"_s, Constant<0xA>(Uint<4>())),
    Field("error_code"_s, Uint<4>()),
    Field("hybrid_id"_s, Uint<8>()),
    Field("chip_id"_s, Uint<4>()),
    Field("l1a_size"_s, Uint<12>()),
    Field("padding"_s, Uint<16>()),
    Field("chip_type"_s, Uint<4>()),
    Field("frame_delay"_s, Uint<12>()),
    Field("chip_data"_s, Aligned<128>(EventStream<Config<Flavor, false, true, false>>))
);




} // namespace RD53BEventDecoding

#endif