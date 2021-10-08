#ifndef BITSERIALIZATION__TYPES__VARIABLE_HPP
#define BITSERIALIZATION__TYPES__VARIABLE_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class ValueType>
struct Variable {
    static constexpr bool ignores_input_value = true;

    Variable() {}

    template <class T, class U=VoidValue>
    ParseResult<ValueType> parse(const BitView<T>& bits, const U& parent={}) const {
        return {ValueType{}};
    }

    template <class T, class U=VoidValue>
    auto serialize(ValueType& value, BitVector<T>& bits, const U& parent={}) const {
        return SerializeResult<>{};
    }
};

} // namespace BitSerialization

#endif