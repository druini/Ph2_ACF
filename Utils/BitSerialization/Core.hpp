#ifndef BITSERIALIZATION__CORE_HPP
#define BITSERIALIZATION__CORE_HPP

#include "../Bits/BitVector.hpp"

#include "Utility.hpp"
#include "Results.hpp"
#include "Printing.hpp"



namespace BitSerialization {

template <class...> using void_t = void;

struct VoidType;
struct VoidValue;

template <class Type, class Parent=VoidValue>
using parse_result_t = decltype(
    std::declval<Type>().parse(
        std::declval<BitView<uint32_t>>(), 
        std::declval<const Parent&>()
    )
);

template <class Type, class Parent=VoidValue>
using value_type_t = typename parse_result_t<Type, Parent>::value_type;

template <class Type, class Parent=VoidValue>
using parse_error_t = typename parse_result_t<Type, Parent>::error_type;

template <class Type, class Parent=VoidValue>
struct parse_error { using type = parse_error_t<Type, Parent>; };

template <class Type, class Parent=VoidValue>
using serialize_result_t = decltype(
    std::declval<Type>().serialize(
        std::declval<value_type_t<Type>&>(), 
        std::declval<BitVector<uint32_t>&>(), 
        std::declval<const Parent&>()
    )
);

template <class Type, class Parent=VoidValue>
using serialize_error_t = typename serialize_result_t<Type, Parent>::error_type;

template <class Type, class Parent=VoidValue>
struct serialize_error { using type = serialize_error_t<Type, Parent>; };

// template <typename T>
// concept ignores_input_value = std::same_as<
//     std::integral_constant<bool, T::ignores_input_value>, 
//     std::integral_constant<bool, 1>
// >;

// template <typename T>
// constexpr bool ignores_input_value_v = ignores_input_value<T>;

template <class T, class Enable = void>
struct ignores_input_value : public std::false_type {};

template <class T>
struct ignores_input_value<T, void_t<decltype(T::ignores_input_value)>> {
    static constexpr bool value = T::ignores_input_value;
};

template <typename T>
constexpr bool ignores_input_value_v = ignores_input_value<T>::value;

struct VoidValue {
    VoidValue() {}

    template <class T>
    VoidValue(const T&) {}

    template <class T>
    operator T() const; // { return {}; }

    template <class T>
    VoidValue operator[](T) const;
};

struct VoidType {
    static constexpr bool ignores_input_value = true;

    template <class T, class U=VoidValue>
    ParseResult<VoidValue> parse(const BitView<T>& bits, const U& parent={}) const {
        return {VoidValue(), 0};
    }

    template <class T, class U=VoidValue>
    SerializeResult<> 
    serialize(VoidValue& value, BitVector<T>& bits, const U& parent={}) const {
        return {};
    }
};

inline auto Void() {
    return VoidType{};
}

inline std::ostream& operator<<(std::ostream& os, std::reference_wrapper<const VoidValue> wrapper) {
    return (os << "Void");
}

struct EndOfStream {
    static constexpr bool ignores_input_value = true;

    struct ParseError {
        friend std::ostream& operator<<(std::ostream& os, const ParseError& wrapper) {
            return (os << "EndOfStream error.");
        }
    };

    template <class T, class U=VoidValue>
    ParseResult<VoidValue, ParseError> parse(const BitView<T>& bits, const U& parent={}) const {
        if (bits.size() > 0)
            return ParseError{};
        return {VoidValue(), 0};
    }

    template <class T, class U=VoidValue>
    SerializeResult<> serialize(VoidValue& value, BitVector<T>& bits, const U& parent={}) const {
        return {};
    }
};

template <size_t Size>
constexpr const char* to_char_ptr(const std::array<char, Size>& array) {
    return &std::get<0>(array);
}

template <size_t N>
struct UintType {
    static constexpr const char name[] = "Uint";

    using value_type = uint_t<N>;

    using ParseError = SizeError<name>;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        if (bits.size() < N)
            return ParseError{bits.size()};
        else
            return {bits.slice(0, N).template get<value_type>(), N};
    }

    template <class T, class U=VoidValue>
    SerializeResult<> 
    serialize(const value_type& value, BitVector<T>& bits, const U& parent={}) const {
        bits.append_zeros(N).template set<value_type>(value);
        return {};
    }
};

template <size_t N>
constexpr const char UintType<N>::name[];

// template <size_t N>
// auto Uint(std::integral_constant<size_t, N> = {}) {
//     return UintType<N>();
// }

template <size_t N>
inline auto Uint() {
    return UintType<N>{};
}


inline auto Bool() {
    return UintType<1>();
}

} // namespace BitSerialization

#endif