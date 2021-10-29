#ifndef BITSERIALIZATION__TYPES__VALIDATED_HPP
#define BITSERIALIZATION__TYPES__VALIDATED_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, class Predicate>
struct ValidatedType {
    using value_type = value_type_t<Type>;

    ValidatedType(const Type& type, const Predicate& predicate)
      : _type(type)
      , _predicate(predicate)
    {}

    static constexpr const char name[] = "Validated";

    using _ValueError = ValueError<name, value_type>;

    using ParseError = ErrorVariant<
        _ValueError, 
        parse_error_t<Type>
    >;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const
    {
        auto result = _type.parse(bits, parent);
        if (!result)
            return {std::move(result.error())};
        if (!_predicate(parent, result.value()))
            return {_ValueError{std::move(result.value())}};
        else
            return {std::move(result.value()), result.size()};
    }
    
    using SerializeError = ErrorVariant<
        _ValueError, 
        serialize_error_t<Type>
    >;

    template <class T, class U=VoidValue>
    SerializeResult<SerializeError> 
    serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        if (!_predicate(parent, value))
            return {_ValueError{value}};
        auto result = BitSerialization::serialize(_type, value, bits, parent);
        if (!result)
            return {std::move(result.error())};
        else
            return {};
    }

private:
    Type _type;
    Predicate _predicate;
};

template <class Type, class Predicate>
constexpr const char ValidatedType<Type, Predicate>::name[];

template <class Type, class Predicate>
auto Validated(const Type& type, const Predicate& predicate) {
    return ValidatedType<Type, Predicate>(type, predicate);
}

template <size_t Value>
struct ValueValidator {
    template <class T, class U>
    bool operator()(const T&, const U& value) const {
        return value == Value;
    }
};

template <class Type, size_t Value, class Base = ValidatedType<Type, ValueValidator<Value>>>
struct ValidatedConstantType : public Base {
    ValidatedConstantType(const Type& type)
      : Base(type, ValueValidator<Value>{})
    {}
};

template <size_t Value, class Type>
auto ValidatedConstant(const Type& type) {
    return ValidatedConstantType<Type, Value>(type);
}

} // namespace BitSerialization

#endif