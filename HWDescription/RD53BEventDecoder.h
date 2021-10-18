#ifndef RD53BEVENTDECODING_H
#define RD53BEVENTDECODING_H

#include "RD53B.h"

#include "../Utils/BitSerialization/Types/All.hpp"

#include <numeric>

namespace RD53BEventDecoding {

using namespace BitSerialization;
using namespace Ph2_HwDescription;


struct RD53BEvent {

    struct Hit {
        Hit() {}
        Hit(uint16_t row, uint16_t col, uint8_t tot) 
          : row(row), col(col), tot(tot) {}

        friend std::ostream& operator<<(std::ostream& os, const Hit& e) {
            return os << "{ row: " << +e.row << ", col: " << +e.col << ", tot: " << +e.tot << " }";
        }

        uint16_t row;
        uint16_t col;
        uint8_t tot;
    };

    // struct PToTHit {
    //     uint16_t colPair;
    //     uint16_t timeOfArrival;
    //     uint16_t ToT;
    // };

    std::vector<Hit> hits;
    // std::vector<PToTHit> pToTHits;
    uint16_t BCID;
    uint8_t triggerTag;
    uint8_t triggerPos;
};


template <RD53BFlavor::Flavor Flavor, bool IsHitmapCompressed, bool HasToT, bool HasChipId>
struct Config {
    static constexpr bool isHitMapCompressed = IsHitmapCompressed;
    static constexpr bool hasToT = HasToT;
    static constexpr bool hasChipId = HasChipId;
    static constexpr RD53BFlavor::Flavor flavor = Flavor;
};

struct BitPairCompression {
    uint8_t encode(uint8_t value) const { return value == 1 ? 0 : value; }
    uint8_t decode(uint8_t value) const { return value == 0 ? 1 : value; }
};

const auto CompressedBitPair = Transformed(
    OneOf(
        ValidatedConstant<0>(Uint<1>()), 
        Uint<2>()
    ),
    BitPairCompression{}
);


const auto CompressedHitMapRow = Object(
    Field("quad_mask"_s, CompressedBitPair),
    Field("pair_masks"_s, List(CompressedBitPair, [] (const auto& self) {
        return __builtin_popcount(self["quad_mask"_s]);
    })),
    Field("pixel_masks"_s, List(CompressedBitPair, [] (const auto& self) {
        return std::accumulate(self["pair_masks"_s].begin(), self["pair_masks"_s].end(), 0, [] (int sum, const auto& value) {
            return sum + __builtin_popcount(value);
        });
    })),
    Field("hits"_s, Property<std::array<bool, 8>>([] (const auto& self) {
        std::array<bool, 8> hits;
        int index = 0;
        int current_quad = 0;
        int current_pair = 0;
        for (int pixel_quad = 0; pixel_quad < 2; ++pixel_quad, index += 4) {
            if (self["quad_mask"_s] & (1 << pixel_quad)) {
                for (int pixel_pair = 0; pixel_pair < 2; ++pixel_pair, index += 2) {
                    if (self["pair_masks"_s][current_quad] & (1 << pixel_pair)) {
                        hits[index]     = self["pixel_masks"_s][current_pair] & 1;
                        hits[index + 1] = self["pixel_masks"_s][current_pair] & 2;
                        ++current_pair;
                    }
                }
                ++current_quad;
            }
        }
        return hits;
    }))
);


const auto CompressedHitMap = Object(
    Field("row_mask"_s, CompressedBitPair),
    Field("rows"_s, List(CompressedHitMapRow, [] (const auto& self) {
        return __builtin_popcount(self["row_mask"_s]);
    })),
    Field("hits"_s, Property<std::array<bool, 16>>([] (const auto& self) {
        std::array<bool, 16> hits = {0};
        int current_row;
        for (int row = 0; row < 2; ++row) {
            if (self["row_mask"_s] & (1 << row)) {
                std::copy(
                    self["rows"_s][current_row]["hits"_s].begin(), 
                    self["rows"_s][current_row]["hits"_s].end(), 
                    hits.begin() + row * 8
                );
                ++current_row;
            }
        }
        return hits;
    }))
);


const auto HitMap = Object(
    Field("hits"_s, Array<16>(Bool())),
    Field("n_hits"_s, Property<size_t>([] (auto parent) { 
        return std::count(parent["hits"_s].begin(), parent["hits"_s].end(), 1); 
    }))
);


template <class Cfg, bool IsLast = false>
const auto QRow = Object(
    Field("is_last"_s, Constant<IsLast>(Bool())),
    Field("is_neighbor"_s, Bool()),
    Field("qrow"_s, Optional(Uint<8>(), [] (const auto& parent) {
        return !(bool)parent["is_neighbor"_s];
    })),
    Field("hitmap"_s, conditional<Cfg::isHitMapCompressed>(CompressedHitMap, HitMap)),
    Field("tots"_s, conditional<Cfg::hasToT>(
        List(Uint<4>(), [] (const auto& parent) {
            return std::count(parent["hitmap"_s]["hits"_s].begin(), parent["hitmap"_s]["hits"_s].end(), 1);
        }),
        Void()
    ))
);


template <class Cfg>
const auto CCol = Object(
    Field("ccol"_s, Validated(Uint<6>(), [] (const auto& self, const auto& value) { 
        return (value > 0); 
    })),
    Field("qrows"_s, String(QRow<Cfg>, QRow<Cfg, true>))
);


template <class Cfg>
const auto Event = Object(
    Field("trigger_tag"_s, Uint<6>()),
    Field("trigger_pos"_s, Uint<2>()),
    Field("ccols"_s, Many(CCol<Cfg>)),
    Field("hits"_s, Property<std::vector<RD53BEvent::Hit>>([] (const auto& parent) {
        std::vector<RD53BEvent::Hit> hits;
        for (const auto& ccol : parent["ccols"_s]) {
            size_t last_qrow = 0;
            for (const auto& qrow : ccol["qrows"_s]) {
                size_t qrow_address = qrow["is_neighbor"_s] ? ++last_qrow : (last_qrow = *qrow["qrow"_s]);
                int hit_id = 0;
                for (size_t row = 0; row < 2; ++row) {
                    for (size_t col = 0; col < 8; ++col) {
                        if (qrow["hitmap"_s]["hits"_s][row * 8 + col]) {
                            hits.emplace_back(
                                qrow_address * 2 + row, 
                                (ccol["ccol"_s] - 1) * 8 + col, 
                                qrow["tots"_s][hit_id]
                            );
                            ++hit_id;
                        }
                    }
                }
            }
        }
        return hits;
    }))
);


template <class Cfg>
const auto EventList = SeparatedBy(
    Event<Cfg>, 
    Constant<0b111>(Uint<3>())
);


template <bool IsDelimiter, bool HasChipId>
const auto StreamPacket = Object(
    Field("is_delimiter"_s, Constant<IsDelimiter>(Bool())),
    Field("chip_id"_s, conditional<HasChipId>(Uint<2>(), Void())),
    Field("payload"_s, Uint<(HasChipId ? 61 : 63)>())
);


template <class Cfg>
const auto EventStream = Compose3(
    Aligned<63>(EventList<Cfg>),
    Many(Uint<63>()),
    conditional<Cfg::flavor == RD53BFlavor::Flavor::ATLAS>(
        String(StreamPacket<false, false>, StreamPacket<true, false>), 
        Stream(StreamPacket<true, false>, StreamPacket<false, false>)
    )
);

// template <class Flavor>
// const auto FWEvent = Object(
//     Field("header"_s, Constant<0xA>(Uint<4>())),
//     Field("error_code"_s, Uint<4>()),
//     Field("hybrid_id"_s, Uint<8>()),
//     Field("chip_id"_s, Uint<4>()),
//     Field("l1a_size"_s, Uint<12>()),
//     Field("padding"_s, Uint<16>()),
//     Field("chip_type"_s, Uint<4>()),
//     Field("frame_delay"_s, Uint<12>()),
//     Field("chip_data"_s, Aligned<128>(
//         Validated(EventStream<Config<Flavor::flavor, false, true, false>>, [] (const auto& self, const auto& value) {
//             return value.size() == 1; // && value[0].size() == 1;
//         })
//     ))
// );

// // FW event encoding
// template <class Flavor>
// const auto FWEventData = 
// // Many<1>(
//     // Aligned<256>(
//         Object(
//             Field("header"_s, Constant<0xFFFF>(Uint<16>())),
//             Field("block_size"_s, Uint<16>()),
//             Field("tlu_trigger_id"_s, Uint<16>()),
//             Field("trigger_tag"_s, Uint<8>()),
//             Field("dummy_size"_s, Uint<8>()),
//             Field("tdc"_s, Uint<8>()),
//             Field("l1a_counter"_s, Uint<24>()),
//             Field("bx_counter"_s, Uint<32>()),
//             Field("events"_s, 
//                 Many<1>(FWEvent<Flavor>)
//                 // Compose3(
//                 //     Many<1>(FWEvent<Flavor>),
//                 //     // Many<1>(Validated(
//                 //     //     FWEvent<Flavor>,
//                 //     //     [] (const auto& self, const auto& value) {
//                 //     //         return value["chip_data"_s][0][0]["trigger_tag"_s] == self["trigger_tag"_s];
//                 //     //     }
//                 //     // )),
//                 //     Many(Uint<128>()),
//                 //     List(Uint<128>(), [] (const auto& self) { return self["block_size"_s] - 1 - self["dummy_size"_s]; })
//                 // )
//             )
//         )
//     // )
// // )
// ;


// template <class Flavor>
// auto decodeFWEventData(const std::vector<uint32_t>& data) {
//     auto result = FWEventData<Flavor>.parse(data);
//     if (!result) {
//         std::stringstream ss;
//         ss << result.error();
//         throw std::runtime_error(ss.str());
//     }
//     std::map<std::pair<uint8_t, uint8_t>, std::vector<RD53BEvent>> eventsMap;
//     for (const auto& fwEventContainer : result.value()) {
//         for (const auto& fwEvent : fwEventContainer["events"_s]) {
//             auto& event = fwEventContainer["events"_s]["chip_data"_s][0][0];
//             auto& outputEvents = eventsMap[{fwEvent["hybrid_id"_s], fwEvent["chip_id"_s]}];
//             outputEvents.push_back({
//                 event["hits"],
//                 fwEventContainer["bx_counter"_s],
//                 event["trigger_tag"_s],
//                 event["trigger_pos"_s]
//             });
//             outputEvents.back().BCID = fwEventContainer["bx_counter"_s];
//             outputEvents.back().triggerTag = event["trigger_tag"_s];
//             outputEvents.back().triggerPos = event["trigger_pos"_s];
//             std::copy(event["hits"].begin());
//         }
//     }
// }


} // namespace RD53BEventDecoding

#endif