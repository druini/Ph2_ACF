#ifndef RD53BEVENTDECODING_H
#define RD53BEVENTDECODING_H


#include <BitSerialization/Types/All.hpp>

#include <numeric>

namespace RD53BEventDecoding {

using namespace BitSerialization;

template <bool EnableHitmapCompression, bool EnableToT>
struct Config {
    static constexpr bool enable_hitmap_compression = EnableHitmapCompression;
    static constexpr bool enable_tot = EnableToT;
};

struct BitPairCompression {
    uint8_t encode(uint8_t value) const { return value == 1 ? 0 : value; }
    uint8_t decode(uint8_t value) const { return value == 0 ? 1 : value; }
};


const auto CompressedBitPair = Transformed(
    OneOf(
        ValidatedConstant<0>(Uint<1>), 
        Uint<2>
    ),
    BitPairCompression{}
);


const auto CompressedHitMapRow = Object(
    Field("s2"_s, CompressedBitPair),
    Field("s3"_s, List(CompressedBitPair, [] (const auto& parent) {
        return __builtin_popcount(parent["s2"_s]);
    })),
    Field("hit_pairs"_s, List(CompressedBitPair, [] (const auto& parent) {
        return std::accumulate(
            parent["s3"_s].begin(), 
            parent["s3"_s].end(), 0, 
            [] (int sum, const auto& value) {
                return sum + __builtin_popcount(value);
            }
        );
    }))
);


const auto CompressedHitMap = Object(
    Field("s1"_s, CompressedBitPair),
    Field("rows"_s, List(CompressedHitMapRow, [] (const auto& parent) {
        return __builtin_popcount(parent["s1"_s]);
    })),
    Field("n_hits"_s, Property<size_t>([] (const auto& parent) {
        size_t n_hits = 0;
        for (const auto& row : parent["rows"_s])
            for (const auto& hit_pair : row["hit_pairs"_s])
                n_hits += __builtin_popcount(hit_pair);
        return n_hits;
    }))
);


const auto HitMap = Object(
    Field("hits"_s, Array<16>(Uint<1>)),
    Field("n_hits"_s, Property<size_t>([] (auto parent) { 
        return std::count(parent["hits"_s].begin(), parent["hits"_s].end(), 1); 
    }))
);


template <class Cfg, bool IsLast = false>
const auto QRow = Object(
    Field("is_last"_s, Constant<IsLast>(Bool)),
    Field("is_neighbor"_s, Bool),
    Field("qrow"_s, Optional(Uint<8>, [] (const auto& parent) {
        return !(bool)parent["is_neighbor"_s];
    })),
    Field("hitmap"_s, conditional<Cfg::enable_hitmap_compression>(CompressedHitMap, HitMap)),
    Field("tots"_s, conditional<Cfg::enable_tot>(
        List(Uint<4>, [] (const auto& parent) {
            return parent["hitmap"_s]["n_hits"_s];
        }),
        Void
    ))
);


template <class Cfg>
const auto CCol = Object(
    Field("ccol"_s, Validated(Uint<6>, [] (const auto& value) { 
        return (value > 0); 
    })),
    Field("qrows"_s, String(QRow<Cfg>, QRow<Cfg, true>))
);


template <class Cfg>
const auto Event = Object(
    Field("tag"_s, Uint<8>),
    Field("ccols"_s, Many(CCol<Cfg>))
);


template <class Cfg>
const auto EventList = SeparatedBy(
    Event<Cfg>, 
    Constant<0b111>(Uint<3>)
);


template <class Cfg, bool HasChipId=false>
const auto EventStream = Object(
    Field("new_stream"_s, Constant<1>(Bool)),
    Field("chip_id"_s, conditional<HasChipId>(Uint<2>, Void)),
    Field("events"_s, Compose3(
        Aligned<(HasChipId ? 61 : 63)>(EventList<Cfg>),
        Many(Uint<(HasChipId ? 61 : 63)>),
        SeparatedBy(
            Uint<(HasChipId ? 61 : 63)>,
            conditional<HasChipId>(Dynamic(Uint<3>, [] (const auto& self) { return self["chip_id"_s]; }), Constant<0>(Bool))
        )
    ))
);


struct EventSequence {
    template <class Cfg, bool HasChipId>
    EventSequence(value_type_t<decltype(EventStream<Cfg, HasChipId>)))
};


} // namespace RD53BEventDecoding

#endif