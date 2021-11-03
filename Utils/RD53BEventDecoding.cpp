#include "RD53BEventDecoding.h"

#include <array>

#include <cstddef>
#include <cstdint>

#include "../Utils/BitSerialization/Core.hpp"
#include "../Utils/BitSerialization/Types/All.hpp"

#include <numeric>

using namespace compile_time_string_literal;

using namespace BitSerialization;

namespace RD53BEventDecoding {

struct BitPairCompression {
    uint8_t encode(uint8_t value) const { return value == 1 ? 0 : value; }
    uint8_t decode(uint8_t value) const { return value == 0 ? 1 : value; }
};

using CompressedBitPair = Transformed<
    OneOf<
        ValidatedConstant<Uint<1>, 0>, 
        Uint<2>
    >,
    BitPairCompression{}
>;

using CompressedHitMapRow = BitSerialization::NamedTuple<
    NamedField<"quad_mask", CompressedBitPair>,
    NamedField<"pair_masks", List<CompressedBitPair, [] (const auto& self) {
        return __builtin_popcount(self["quad_mask"_s]);
    }>>,
    NamedField<"pixel_masks", List<CompressedBitPair, [] (const auto& self) {
        return std::accumulate(
            self["pair_masks"_s].begin(), 
            self["pair_masks"_s].end(), 0, 
            [] (int sum, const auto& value) {
                return sum + __builtin_popcount(value);
            }
        );
    }>>,
    NamedField<"hits", Property<std::array<bool, 8>, [] (const auto& self) {
        std::array<bool, 8> hits = {0};
        int current_quad = 0;
        int current_pair = 0;
        for (int pixel_quad = 0; pixel_quad < 2; ++pixel_quad) {
            if (self["quad_mask"_s] & (2 >> pixel_quad)) {
                for (int pixel_pair = 0; pixel_pair < 2; ++pixel_pair) {
                    if (self["pair_masks"_s][current_quad] & (2 >> pixel_pair)) {
                        hits[pixel_quad * 4 + pixel_pair * 2]     = self["pixel_masks"_s][current_pair] & 2;
                        hits[pixel_quad * 4 + pixel_pair * 2 + 1] = self["pixel_masks"_s][current_pair] & 1;
                        ++current_pair;
                    }
                }
                ++current_quad;
            }
        }
        return hits;
    }>>
    // ,
    // NamedField<"print", Property<bool, [] (const auto& self) {
    //     std::cout << std::ref(self) << std::endl;
    //     return true;
    // }>>
>;

using CompressedHitMap = BitSerialization::NamedTuple<
    NamedField<"row_mask", CompressedBitPair>,
    NamedField<"rows", List<CompressedHitMapRow,  [] (const auto& self) {
        return __builtin_popcount(self["row_mask"_s]);
    }>>,
    NamedField<"hits", Property<std::array<bool, 16>, [] (const auto& self) {
        std::array<bool, 16> hits = {0};
        int current_row = 0;
        for (int row = 0; row < 2; ++row) {
            if (self["row_mask"_s] & (2 >> row)) {
                std::copy(
                    self["rows"_s][current_row]["hits"_s].begin(), 
                    self["rows"_s][current_row]["hits"_s].end(), 
                    hits.begin() + row * 8
                );
                ++current_row;
            }
        }
        return hits;
    }>>
    // ,
    // NamedField<"print", Property<bool, [] (const auto& self) {
    //     std::cout << std::ref(self) << std::endl;
    //     return true;
    // }>>
>;

using HitMap = BitSerialization::NamedTuple<
    NamedField<"hits", Array<Uint<1>, 16>>
>;

// Type aliase templates (ie. templated "using" definitions) containing lambdas
// fail to compile probably due to some bug in gcc. In this case a new class
// definition with public inheritance can be used instead.
template <bool CompressedHitmap, bool EnableToT, bool IsLast = false>
struct QRow : public BitSerialization::NamedTuple<
    NamedField<"is_last", Constant<Bool, IsLast>>,
    NamedField<"is_neighbor", Bool>,
    NamedField<"qrow", Optional<Uint<8>, [] (const auto& self) {
        return !(bool)self["is_neighbor"_s];
    }>>,
    NamedField<"hitmap", std::conditional_t<CompressedHitmap, CompressedHitMap, HitMap>>,
    NamedField<"tots", std::conditional_t<EnableToT,
        List<Uint<4>, [] (const auto& self) {
            return std::count(self["hitmap"_s]["hits"_s].begin(), self["hitmap"_s]["hits"_s].end(), 1);
        }>,
        Void
    >>
    // ,
    // NamedField<"print", Property<bool, [] (const auto& self) {
    //     std::cout << std::ref(self) << std::endl;
    //     return true;
    // }>>
> {};


template <bool CompressedHitmap, bool EnableToT>
struct CCol : public BitSerialization::NamedTuple<
    NamedField<"ccol", Validated<Uint<6>, [] (const auto&, const auto& value) { 
        return (value > 0); 
    }>>,
    NamedField<"qrows", String<QRow<CompressedHitmap, EnableToT>, QRow<CompressedHitmap, EnableToT, true>>>
> {};


template <bool CompressedHitmap, bool EnableToT>
struct Event : public BitSerialization::NamedTuple<
    NamedField<"trigger_tag", Uint<6>>,
    NamedField<"trigger_pos", Uint<2>>,
    NamedField<"ccols", Many<CCol<CompressedHitmap, EnableToT>>>,
    NamedField<"hits", Property<std::vector<RD53BEvent::Hit>, [] (const auto& parent) {
        std::vector<RD53BEvent::Hit> hits;
        std::array<int, 54> last_qrow;
        last_qrow.fill(-1);
        for (const auto& ccol : parent["ccols"_s]) {
            size_t ccol_address = ccol["ccol"_s] - 1;
            for (const auto& qrow : ccol["qrows"_s]) {
                size_t qrow_address = qrow["is_neighbor"_s] ? last_qrow[ccol_address] + 1 : *qrow["qrow"_s];
                last_qrow[ccol_address] = qrow_address;
                // if ()
                //     qrow_address = ++last_qrow
                // else
                //     qrow_address = *qrow["qrow"_s];
                // last_qrow = qrow_address;
                int hit_id = 0;
                for (size_t row = 0; row < 2; ++row) {
                    for (size_t col = 0; col < 8; ++col) {
                        if (qrow["hitmap"_s]["hits"_s][row * 8 + col]) {
                            hits.emplace_back(
                                qrow_address * 2 + row, 
                                ccol_address * 8 + col, 
                                qrow["tots"_s][hit_id]
                            );
                            ++hit_id;
                        }
                    }
                }
            }
        }
        return hits;
    }>>
> {};


template <bool CompressedHitmap, bool EnableToT>
using EventList = SeparatedBy<Event<CompressedHitmap, EnableToT>, Constant<Uint<3>, 0b111>>;


template <bool IsDelimiter, bool HasChipId>
using StreamPacket = BitSerialization::NamedTuple<
    NamedField<"is_delimiter", Constant<Bool, IsDelimiter>>,
    NamedField<"chip_id", std::conditional_t<HasChipId, Uint<2>, Void>>,
    NamedField<"payload", Uint<(HasChipId ? 61 : 63)>>
>;


template <Config Cfg>
struct EventStream : public Compose3<
    Aligned<
        EventList<Cfg.compressed_hitmap, Cfg.enable_tot>,
        // Validated<
        //     EventList<Cfg.compressed_hitmap, Cfg.enable_tot>, 
        //     [] (const auto&, const auto& value) { return (value.size() > 0); }
        // >, 
        63
    >,
    Many<Uint<63>>,
    std::conditional_t<(Cfg.flavor == Ph2_HwDescription::RD53BFlavor::Flavor::ATLAS),
        Stream<StreamPacket<true, Cfg.has_chip_id>, StreamPacket<false, Cfg.has_chip_id>>,
        String<StreamPacket<false, Cfg.has_chip_id>, StreamPacket<true, Cfg.has_chip_id>>
    >
> {};



template <Config Cfg>
struct FWEvent : public Aligned<
    BitSerialization::NamedTuple<
        NamedField<"header", Constant<Uint<4>, 0xA>>,
        NamedField<"error_code", Uint<4>>,
        NamedField<"hybrid_id", Uint<8>>,
        NamedField<"chip_id", Uint<4>>,
        NamedField<"l1a_size", Uint<12>>,
        NamedField<"padding", Uint<16>>,
        NamedField<"chip_type", Uint<4>>,
        NamedField<"frame_delay", Uint<12>>,
        NamedField<"chip_data", 
            // EventStream<Cfg>
            Compose3<
                EventStream<Cfg>,
                Many<Uint<64>>,
                List<Uint<64>, [] (const auto& self) { return 2 * self["l1a_size"_s] - 1; }>
            >
        >
        // ,
        // NamedField<"print", 
        //     Property<bool, [] (const auto& self) {
        //         std::cout << std::ref(self) << std::endl;
        //         return true;
        //     }>
        // >
    >,
    128
> {};

template <Config Cfg>
struct FWEventData : public 
// NamedTuple<
//     NamedField<"data",
        Many<
            BitSerialization::NamedTuple<
                NamedField<"header", Constant<Uint<16>, 0xFFFF>>,
                NamedField<"block_size", Uint<16>>,
                NamedField<"tlu_trigger_id", Uint<16>>,
                NamedField<"trigger_tag", Uint<8>>,
                NamedField<"dummy_size", Uint<8>>,
                NamedField<"tdc", Uint<8>>,
                NamedField<"l1a_counter", Uint<24>>,
                NamedField<"bx_counter", Uint<32>>,
                NamedField<"events",
                    Compose3<
                        Many<
                            FWEvent<Cfg>,
                            // Validated<
                            //     FWEvent<Cfg>,
                            //     [] (const auto& self, const auto& value) {
                            //         std::cout << "trigger_tag: " << +self["trigger_tag"_s] << std::endl;
                            //         return value["chip_data"_s][0]["trigger_tag"_s] == self["trigger_tag"_s];
                            //     }
                            // >,
                            1
                        >,
                        Many<Array<Uint<64>, 2>>,
                        List<
                            Array<Uint<64>, 2>, 
                            [] (const auto& self) { return self["block_size"_s] - 1 - self["dummy_size"_s]; }
                        >
                    >
                >
                ,
                NamedField<"dummy_words", List<
                    Array<Uint<64>, 2>, 
                    [] (const auto& self) { return self["dummy_size"_s]; }
                >>
            >,
            1
        >
    // >
    // ,
    // NamedField<"print", 
    //     Property<bool, [] (const auto& self) {
    //         std::cout << std::ref(self) << std::endl;
    //         return true;
    //     }>
    // >
// >
{};

template <Ph2_HwDescription::RD53BFlavor::Flavor Flavor>
BoardEventsMap decode_events(const std::vector<uint32_t>& data) {
    BoardEventsMap eventsMap;
    auto bits = bit_view(data);

    auto result = FWEventData<Config{Flavor, true, true, false}>::parse(bits);

    if (!result) {
        std::stringstream ss;
        ss << "Event decoding error: " << result.error();
        throw std::runtime_error(ss.str());
    }

    for (const auto& fwEventContainer : result.value()) {
        for (const auto& fwEvent : fwEventContainer["events"_s]) {
            for (const auto& event : fwEvent["chip_data"_s]) {
                eventsMap[{fwEvent["hybrid_id"_s], fwEvent["chip_id"_s]}].push_back({
                    event["hits"_s],
                    fwEventContainer["bx_counter"_s],
                    event["trigger_tag"_s],
                    event["trigger_pos"_s],
                    fwEventContainer["dummy_size"_s]
                });
            }
        }
    }

    return eventsMap;
}

template BoardEventsMap decode_events<Ph2_HwDescription::RD53BFlavor::Flavor::ATLAS>(const std::vector<uint32_t>&);
template BoardEventsMap decode_events<Ph2_HwDescription::RD53BFlavor::Flavor::CMS>(const std::vector<uint32_t>&);

}
