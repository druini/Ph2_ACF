#ifndef BITSERIALIZATION__TYPES__OPTIONAL_HPP
#define BITSERIALIZATION__TYPES__OPTIONAL_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, class Condition>
struct OptionalType {
    static constexpr bool ignores_input_value = ignores_input_value_v<Type>;

    using value_type = boost::optional<value_type_t<Type>>;

    OptionalType(const Type& type, const Condition& condition)
      : _type(type)
      , _condition(condition)
    {}

    template <class T, class U>
    ParseResult<value_type, parse_error_t<Type>> 
    parse(const BitView<T>& bits, const U& parent) const {
        if (_condition(parent)) {
            auto result = _type.parse(bits, parent);
            if (result)
                return {std::move(result.value()), result.size()};
            else
                return {std::move(result.error())};
        }
        else {
            return {boost::none, 0};
        }
    }

    using SerializeError = serialize_error_t<Type>;

    template <class T, class U=VoidValue>
    SerializeResult<serialize_error_t<Type>> 
    serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        if (_condition(parent)) {
            auto v = value.get();
            auto result = _type.serialize(v, bits, parent);
            if (!result)
                return {std::move(result.error())};
            else {
                value = v;
                return {};
            }
        }
        else {
            return {};
        }
    }

private:
    Type _type;
    Condition _condition;
};

template <class... Args>
auto Optional(const Args&... args) {
    return OptionalType<Args...>(args...);
}

} // namespace BitSerialization

#endif