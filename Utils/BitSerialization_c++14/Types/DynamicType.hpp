#ifndef BITSERIALIZATION__TYPES__DYNAMICTYPE_HPP
#define BITSERIALIZATION__TYPES__DYNAMICTYPE_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class ValueType, class TypeGetter, bool IgnoresInputValue = false>
struct _DynamicType {
    static constexpr bool ignores_input_value = IgnoresInputValue;

    using value_type = ValueType;

    _DynamicType(const TypeGetter& type_getter) : _type_getter(type_getter) {}

    template <class T, class U=VoidValue>
    auto parse(const BitView<T>& bits, const U& parent={}) const {
        return _type_getter(parent).parse(bits, parent);
    }
    
    template <class T, class U=VoidValue>
    auto serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        return _type_getter(parent).serialize(value, bits, parent);
    }
    
private:
    TypeGetter _type_getter;
};

template <class ValueType, bool IgnoresInputValue = false, class TypeGetter = void>
auto DynamicType(TypeGetter&& type_getter) {
    return _DynamicType<ValueType, TypeGetter, IgnoresInputValue>(std::forward<TypeGetter>(type_getter));
}

} // namespace BitSerialization

#endif