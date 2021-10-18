#ifndef BITSERIALIZATION__TYPES__ALIGNED_HPP
#define BITSERIALIZATION__TYPES__ALIGNED_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, size_t Size>
struct AlignedType {
    static constexpr bool ignores_input_value = ignores_input_value_v<Type>;

    AlignedType(const Type& type) : _type(type) {}

    static constexpr const char name[] = "Aligned";

    using _SizeError = SizeError<name>;

    using ParseError = ErrorVariant<_SizeError, parse_error_t<Type>>;

    template <class T, class U=VoidValue>
    ParseResult<value_type_t<Type>, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        auto result = _type.parse(bits, parent);
        if (!result)
            return {std::move(result.error())};
        size_t leftover_bits = result.size() % Size;
        if (leftover_bits > 0) {
            int padding = Size - leftover_bits;
            if (bits.size() < result.size() + padding)
                return {_SizeError{bits.size()}};
            return {std::move(result.value()), result.size() + padding};
        }
        return {std::move(result.value()), result.size()};
    }

    struct SerializeSizeError {
        size_t size;

        friend std::ostream& operator<<(std::ostream& os, const SerializeSizeError& self) {
            return (os << "Padded error: too many bits (" << self.size << '/' << Size << ")");
        }
    };

    template <class T, class U=VoidValue>
    SerializeResult<serialize_error_t<Type>>
    serialize(value_type_t<Type>& value, BitVector<T>& bits, const U& parent={}) const {
        size_t original_size = bits.size();
        auto result = BitSerialization::serialize(_type, value, bits, parent);
        if (!result)
            return {std::move(result.error())};
        size_t leftover_bits = (bits.size() - original_size) % Size;
        if (leftover_bits > 0)
            bits.append_zeros(Size - leftover_bits);
        return {};
    }

private:
    Type _type;
};

template <class Type, size_t Size>
constexpr const char AlignedType<Type, Size>::name[];

template <size_t Size, class Type>
auto Aligned(const Type& type) {
    return AlignedType<Type, Size>(type);
}

} // namespace BitSerialization

#endif