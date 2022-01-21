#ifndef BITSERIALIZATION__TYPES__CONDITIONAL_HPP
#define BITSERIALIZATION__TYPES__CONDITIONAL_HPP

#include "../Core.hpp"

namespace BitSerialization {
    
template <auto Condition, class TypeA, class TypeB>
class Conditional {
public:
    static constexpr bool ignores_input_value = ignores_input_value<TypeA> && ignores_input_value<TypeB>;

    using value_type = std::variant<value_type_t<TypeA>, value_type_t<TypeB>>;

    using ParseError = ErrorVariant<parse_error_t<TypeA>, parse_error_t<TypeB>>;

    template <class T, class U=Void>
    static auto parse(const BitView<T>& bits, const U& parent={})  {
        return Condition(parent) ? parse_type<TypeA>(bits, parent) : parse_type<TypeB>(bits, parent);
    }

    using SerializeError = ErrorVariant<serialize_error_t<TypeA>, serialize_error_t<TypeB>>;

    template <class T, class U=Void>
    static auto serialize(value_type& value, const BitView<T>& bits, const U& parent={})  {
        return Condition(parent) ? serialize_type<TypeA>(value, bits, parent) : serialize_type<TypeB>(value, bits, parent);
    }

private:
    template <class Type, class T, class U>
    static ParseResult<value_type, ParseError> 
    parse_type(const BitView<T>& bits, const U& parent) {
        auto result = Type::parse(bits, parent);
        if (result)
            return {std::move(result.value()), result.size()};
        else
            return {std::move(result.error())};
    }

    template <class Type, class T, class U=Void>
    static SerializeResult<SerializeError> 
    serialize_type(value_type& value, BitVector<T>& bits, const U& parent) {
        auto result = Type::serialize(std::get<value_type_t<Type>>(value), bits, parent);
        if (!result)
            return {std::move(result.error())};
        else
            return {};
    }
};

}

#endif