#ifndef RD53BCMD_H
#define RD53BCMD_H

#include "../Utils/BitSerialization/Types/All.hpp"

namespace RD53BCmd {

    using namespace BitSerialization;
    
    struct ByteTransform {
        constexpr ByteTransform(std::initializer_list<uint8_t> encoding) 
        : _size(encoding.size())
        , encoding_array{{0}}
        , decoding_array{{0}}
        {
            auto it = encoding.begin();
            for (size_t i = 0; i < _size; ++i) {
                encoding_array[i] = *it++;
                decoding_array[encoding_array[i]] = i;
            }
            for (int i = _size; i < 256; ++i)
                decoding_array[i] = _size;
        }

        uint8_t encode(uint8_t value) const { return encoding_array[value]; }
        uint8_t decode(uint8_t value) const { return decoding_array[value]; }

        size_t size() const { return _size; }

    private:
        size_t _size;
        uint8_t encoding_array[256];
        uint8_t decoding_array[256];
    };

    constexpr auto FieldEncodingTransform = ByteTransform({
        0b01101010,
        0b01101100,
        0b01110001,
        0b01110010,
        0b01110100,
        0b10001011,
        0b10001101,
        0b10001110,
        0b10010011,
        0b10010101,
        0b10010110,
        0b10011001,
        0b10011010,
        0b10011100,
        0b10100011,
        0b10100101,
        0b10100110,
        0b10101001,
        0b01011001,
        0b10101100,
        0b10110001,
        0b10110010,
        0b10110100,
        0b11000011,
        0b11000101,
        0b11000110,
        0b11001001,
        0b11001010,
        0b11001100,
        0b11010001,
        0b11010010,
        0b11010100
    });

    constexpr auto TriggerPatternTransform = ByteTransform({
        0b00101011,
        0b00101101,
        0b00101110,
        0b00110011,
        0b00110101,
        0b00110110,
        0b00111001,
        0b00111010,
        0b00111100,
        0b01001011,
        0b01001101,
        0b01001110,
        0b01010011,
        0b01010101,
        0b01010110
    });

    constexpr auto TriggerTagTransform = ByteTransform({
        0b01101010,
        0b01101100,
        0b01110001,
        0b01110010,
        0b01110100,
        0b10001011,
        0b10001101,
        0b10001110,
        0b10010011,
        0b10010101,
        0b10010110,
        0b10011001,
        0b10011010,
        0b10011100,
        0b10100011,
        0b10100101,
        0b10100110,
        0b10101001,
        0b01011001,
        0b10101100,
        0b10110001,
        0b10110010,
        0b10110100,
        0b11000011,
        0b11000101,
        0b11000110,
        0b11001001,
        0b11001010,
        0b11001100,
        0b11010001,
        0b11010010,
        0b11010100,
        0b01100011,
        0b01011010,
        0b01011100,
        0b10101010,
        0b01100101,
        0b01101001,
        0b00101011,
        0b00101101,
        0b00101110,
        0b00110011,
        0b00110101,
        0b00110110,
        0b00111001,
        0b00111010,
        0b00111100,
        0b01001011,
        0b01001101,
        0b01001110,
        0b01010011,
        0b01010101,
        0b01010110,
        0b01100110
    });


    template <uint8_t OpCode, class... Fields>
    auto Command(const Fields&... fields) {
        return Object(
            Field("op_code"_s, Constant<OpCode>(Uint<8>())),
            Field("fields"_s, Compose3(
                Object(fields...),
                Many(Uint<5>()),
                Many(Validated(Transformed(Uint<8>(), FieldEncodingTransform), [] (auto value) { 
                        return value < FieldEncodingTransform.size(); 
                }))
            ))
        );
    }

    const auto Sync = Constant<0b1000000101111110>(Uint<16>());

    const auto PLLlock = Constant<0b1010101010101010>(Uint<16>());

    const auto GlobalPulse = Command<0b01011100>(Field("chip_id"_s, Uint<5>()));

    const auto Clear = Command<0b01011010>(Field("chip_id"_s, Uint<5>()));

    const auto RdReg = Command<0b01100101>(
        Field("chip_id"_s, Uint<5>()),
        Field("padding"_s, Constant<0>(Bool())),
        Field("address"_s, Uint<9>())
    );

    const auto WrReg = Command<0b01100110>(
        Field("chip_id"_s, Uint<5>()),
        Field("padding0"_s, Constant<0>(Bool())),
        Field("address"_s, Uint<9>()),
        Field("data"_s, Uint<16>()),
        Field("padding1"_s, Constant<0>(Uint<4>()))
    );

    const auto WrRegLong = Command<0b01100110>(
        Field("chip_id"_s, Uint<5>()),
        Field("padding"_s, Constant<0b1000000000>(Uint<10>())),
        Field("data"_s, Many<1>(Uint<10>()))
    );

    const auto Cal = Command<0b01100011>(
        Field("chip_id"_s, Uint<5>()),
        Field("mode"_s, Bool()),
        Field("edge_delay"_s, Uint<5>()),
        Field("edge_duration"_s, Uint<8>()),
        Field("aux_enable"_s, Bool()),
        Field("aux_delay"_s, Uint<5>())
    );

    const auto ReadTrigger = Command<0b01101001>(
        Field("chip_id"_s, Uint<5>()),
        Field("padding"_s, Constant<0>(Uint<2>())),
        Field("tag"_s, Uint<8>())
    );

    const auto Trigger = Object(
        Field("pattern"_s, Converted(Uint<4>(), Uint<8>(), TriggerPatternTransform)),
        Field("tag"_s, Converted(Uint<6>(), Uint<8>(), TriggerTagTransform))
    );

    template <class CmdType>    constexpr bool isBroadcast                    = false;
    template <>                 constexpr bool isBroadcast<decltype(Sync)>    = true;
    template <>                 constexpr bool isBroadcast<decltype(PLLlock)> = true;
    template <>                 constexpr bool isBroadcast<decltype(Trigger)> = true;

} // namespace RD53BCmd

#endif