#ifndef RD53BFWEVENTDECODING_H
#define RD53BFWEVENTDECODING_H

#include "RD53BFWEvent.h"

namespace RD53BEventDecoding {

using namespace BitSerialization;
using namespace Ph2_HwDescription;


// extern template decltype(FWEvent<RD53BFlavor::Flavor::ATLAS>) FWEvent<RD53BFlavor::Flavor::ATLAS>;
// extern template decltype(FWEvent<RD53BFlavor::Flavor::CMS>) FWEvent<RD53BFlavor::Flavor::CMS>;

// FW event encoding
template <RD53BFlavor::Flavor Flavor>
const auto FWEventData = 
Many<1>(
    Aligned<256>(
        Object(
            Field("header"_s, Constant<0xFFFF>(Uint<16>())),
            Field("block_size"_s, Uint<16>()),
            Field("tlu_trigger_id"_s, Uint<16>()),
            Field("trigger_tag"_s, Uint<8>()),
            Field("dummy_size"_s, Uint<8>()),
            Field("tdc"_s, Uint<8>()),
            Field("l1a_counter"_s, Uint<24>()),
            Field("bx_counter"_s, Uint<32>()),
            Field("events"_s, 
                // Many<1>(FWEvent<Flavor>)
                Compose3(
                    Many<1>(FWEvent<Flavor>),
                    // Many<1>(Validated(
                    //     FWEvent<Flavor>,
                    //     [] (const auto& self, const auto& value) {
                    //         return value["chip_data"_s][0][0]["trigger_tag"_s] == self["trigger_tag"_s];
                    //     }
                    // )),
                    Many(Uint<128>()),
                    List(Uint<128>(), [] (const auto& self) { return self["block_size"_s] - 1 - self["dummy_size"_s]; })
                )
            )
        )
    )
)
;



} // namespace RD53BEventDecoding

#endif