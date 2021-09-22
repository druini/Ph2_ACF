#ifndef BITSERIALIZATION__TYPES__MANY_HPP
#define BITSERIALIZATION__TYPES__MANY_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, size_t MinSize = 0>
struct ManyType {
    static constexpr bool ignores_input_value = ignores_input_value_v<Type>;
    using value_type = std::vector<value_type_t<Type>>;

    ManyType(const Type& type) : _type(type) {}

    static constexpr const char name[] = "Many";

    using ParseError = ElementError<parse_error_t<Type>, name>;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        value_type results;
        size_t offset = 0;
        while (offset < bits.size()) {
            auto result = _type.parse(bits.slice(offset), parent);
            if (!result) {
                if (results.size() < MinSize)
                    return ParseError(results.size(), std::move(result.error()));
                break;
            }
            results.push_back(std::move(result.value()));
            offset += result.size();
        }
        return {std::move(results), offset};
    }

    using SerializeError = ElementError<serialize_error_t<Type>, name>;

    template <class ValueType, class T, class U=VoidValue>
    SerializeResult<SerializeError>
    serialize(ValueType& value, BitVector<T>& bits, const U& parent={}) const {
        for (size_t i = 0; i < value.size(); ++i) {
            auto result = _type.serialize(value[i], bits, parent);
            if (!result) 
                return SerializeError{i, std::move(result.error())};
        }
        return {};
    }
    
private:
    Type _type;
};

template <class Type, size_t MinSize>
constexpr const char ManyType<Type, MinSize>::name[];

template <size_t MinSize=0, class Type>
auto Many(const Type& type) {
    return ManyType<Type, MinSize>(type);
}

} // namespace BitSerialization

#endif