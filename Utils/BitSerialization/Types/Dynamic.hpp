#ifndef BITSERIALIZATION__TYPES__DYNAMIC_HPP
#define BITSERIALIZATION__TYPES__DYNAMIC_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, class ValueGetter>
struct DynamicType {
    static constexpr bool ignores_input_value = true;

    using value_type = value_type_t<Type>;

    DynamicType(const Type& type, const ValueGetter& value_getter) 
      : _type(type) 
      , _value_getter(value_getter)  
    {}

    static constexpr const char name[] = "Dynamic";

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
        if (!std::is_same<U, VoidValue>::value)
            if (result.value() != _value_getter(parent))
                return {_ValueError{std::move(result.value())}};
        return {std::move(result.value()), result.size()};
    }
    
    template <class T, class U=VoidValue>
    auto serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const
    {
        if (!std::is_same<U, VoidValue>::value)
            value = _value_getter(parent);
        return _type.serialize(value, bits, parent);
    }
    
private:
    Type _type;
    ValueGetter _value_getter;
};


template <class Type, class ValueGetter>
constexpr const char DynamicType<Type, ValueGetter>::name[];

template <class... Args>
auto Dynamic(const Args&... args) {
    return DynamicType<Args...>(args...);
}

} // namespace BitSerialization

#endif