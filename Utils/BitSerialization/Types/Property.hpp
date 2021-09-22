#ifndef BITSERIALIZATION__TYPES__PROPERTY_HPP
#define BITSERIALIZATION__TYPES__PROPERTY_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class ValueType, class F>
struct PropertyType {
    static constexpr bool ignores_input_value = true;

    PropertyType(const F& f) : _f(f) {}

    template <class T, class U=VoidValue>
    ParseResult<ValueType> parse(const BitView<T>& bits, const U& parent={}) const {
        return {T(_f(parent)), 0};
    }

    template <class T, class U=VoidValue>
    auto serialize(ValueType& value, BitVector<T>& bits, const U& parent={}) const {
        value = _f(parent);
        return SerializeResult<>{};
    }

private:
    F _f;
};

template <class ValueType, class F>
auto Property(const F& f) {
    return PropertyType<ValueType, F>(f);
}

} // namespace BitSerialization

#endif