#ifndef BITSERIALIZATION__TYPES__CONVERTED_HPP
#define BITSERIALIZATION__TYPES__CONVERTED_HPP

#include "Compose3.hpp"
#include "Transformed.hpp"

namespace BitSerialization {


    template <class ValueType, class Type>
    struct ConvertedType {
        ConvertedType(const Type& type) :_type(type) {}
        
        template <class T, class U=VoidValue>
        ParseResult<ValueType, parse_error_t<Type>> parse(const BitView<T>& bits, const U& parent={}) const
        {
            auto result = _type.parse(bits, parent);
            if (!result)
                return {std::move(result.error())};
            return {std::move(result.value()), result.size()};
        }

        template <class T, class U=VoidValue>
        auto serialize(const ValueType& value, BitVector<T>& bits, const U& parent={}) const
        {
            value_type_t<Type> new_value{value};
            return BitSerialization::serialize(_type, new_value, bits, parent);
        }

    private:
        Type _type;
    };
    
    template <class ValueType, class Type>
    auto Converted(const Type& type) {
        return ConvertedType<ValueType, Type>(type);
    }

    template <class InputType, class OutputType, class Transform>
    auto Converted(const InputType& input_type, const OutputType& output_type, const Transform& transform) {
        return Compose3(input_type, Transformed(input_type, transform), output_type);
    }

} // namespace BitSerialization

#endif
