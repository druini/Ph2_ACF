#ifndef BITSERIALIZATION__TYPES__VARIABLE_HPP
#define BITSERIALIZATION__TYPES__VARIABLE_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class ValueType>
struct ConstantProperty {
    static constexpr bool ignores_input_value = true;

    ConstantProperty() {}

    ConstantProperty(const ValueType)

    template <class T, class U=VoidValue>
    ParseResult<ValueType> parse(const BitView<T>& bits, const U& parent={}) const {
        return {_value};
    }

    template <class T, class U=VoidValue>
    auto serialize(ValueType& value, BitVector<T>& bits, const U& parent={}) const {
        value = _value;
        return SerializeResult<>{};
    }

private:
    ValueType _value;
};

} // namespace BitSerialization

#endif