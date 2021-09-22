#ifndef BITSERIALIZATION__TYPES__CONSTANT_HPP
#define BITSERIALIZATION__TYPES__CONSTANT_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, value_type_t<Type> Value>
struct ConstantType {
    static constexpr bool ignores_input_value = true;

    static constexpr const char name[] = "Constant";

    using value_type = value_type_t<Type>;

    ConstantType(const Type& type) : _type(type) {}

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
        return _type.serialize(value, bits, parent);
    }

private:
    Type _type;
};


template <class Type, value_type_t<Type> Value>
constexpr const char ConstantType<Type, Value>::name[];

template <size_t Value, class Type>
auto Constant(const Type& type) {
    return ConstantType<Type, Value>(type);
}

} // namespace BitSerialization

#endif