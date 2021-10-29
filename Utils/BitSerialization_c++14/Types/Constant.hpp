#ifndef BITSERIALIZATION__TYPES__CONSTANT_HPP
#define BITSERIALIZATION__TYPES__CONSTANT_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, value_type_t<Type> Value>
struct StaticConstantType {
    static constexpr bool ignores_input_value = true;

    static constexpr const char name[] = "Constant";

    using value_type = value_type_t<Type>;

    StaticConstantType(const Type& type) : _type(type) {}

    using _ValueError = ValueError<name, value_type>;

    using ParseError = ErrorVariant<
        _ValueError, 
        parse_error_t<Type>
    >;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> parse(const BitView<T>& bits, const U& parent={}) const
    {
        auto result = _type.parse(bits, parent);
        if (!result)
            return {std::move(result.error())};
        if (result.value() != Value)
            return {_ValueError{std::move(result.value())}};
        return {std::move(result.value()), result.size()};
    }

    template <class T, class U=VoidValue>
    auto serialize(const value_type&, BitVector<T>& bits, const U& parent={}) const
    {
        value_type value = Value;
        return BitSerialization::serialize(_type, value, bits, parent);
    }

private:
    Type _type;
};

template <class Type, value_type_t<Type> Value>
constexpr const char StaticConstantType<Type, Value>::name[];


template <class Type>
struct DynamicConstantType {
    static constexpr bool ignores_input_value = true;

    static constexpr const char name[] = "Constant";

    using value_type = value_type_t<Type>;

    DynamicConstantType(const Type& type, const value_type& value) : _type(type), _value(value) {}

    using _ValueError = ValueError<name, value_type>;

    using ParseError = ErrorVariant<
        _ValueError, 
        parse_error_t<Type>
    >;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> parse(const BitView<T>& bits, const U& parent={}) const
    {
        auto result = _type.parse(bits, parent);
        if (!result)
            return {std::move(result.error())};
        if (result.value() != _value)
            return {_ValueError{std::move(result.value())}};
        return {std::move(result.value()), result.size()};
    }

    template <class T, class U=VoidValue>
    auto serialize(const value_type&, BitVector<T>& bits, const U& parent={}) const
    {
        value_type value = _value;
        return BitSerialization::serialize(_type, value, bits, parent);
    }

private:
    Type _type;
    value_type _value;
};

template <class Type>
constexpr const char DynamicConstantType<Type>::name[];

template <size_t Value, class Type>
auto Constant(const Type& type) {
    return StaticConstantType<Type, Value>(type);
}

template <class Type>
auto Constant(const Type& type, const value_type_t<Type>& value) {
    return DynamicConstantType<Type>(type, value);
}

} // namespace BitSerialization

#endif