#ifndef BITSERIALIZATION__TYPES__TRANSFORMED_HPP
#define BITSERIALIZATION__TYPES__TRANSFORMED_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, class Transform>
struct TransformedType {
    static constexpr bool ignores_input_value = ignores_input_value_v<Type>;
    using value_type = decltype(
        std::declval<Transform>().decode(std::declval<value_type_t<Type>>())
    );

    TransformedType(const Type& type, const Transform& transform)
      : _type(type)
      , _transform(transform)
    {}

    template <class T, class U=VoidValue>
    ParseResult<value_type, parse_error_t<Type>> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        auto result = _type.parse(bits, parent);
        if (!result)
            return {std::move(result.error())};
        return {_transform.decode(result.value()), result.size()};
    }
    
    template <class T, class U=VoidValue>
    auto serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        if (ignores_input_value_v<Type>) {
            value_type_t<Type> tmp = {};
            auto result = _type.serialize(tmp, bits, parent);
            if (result)
                value = _transform.decode(tmp);
            return result;
        }
        else {
            auto encoded_value = _transform.encode(value);
            return _type.serialize(encoded_value, bits, parent);
        }
    }

private:
    Type _type;
    Transform _transform;
};

template <class Type, class Transform>
auto Transformed(const Type& type, const Transform& transform) {
    return TransformedType<Type, Transform>(type, transform);
}

} // namespace BitSerialization

#endif