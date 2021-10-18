#ifndef BITSERIALIZATION__TYPES__LIST_HPP
#define BITSERIALIZATION__TYPES__LIST_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, class SizeGetter>
struct ListType {
    static constexpr bool ignores_input_value = ignores_input_value_v<Type>;

    ListType(const Type& type, const SizeGetter& size_getter) 
      : _type(type) 
      , _size_getter(size_getter)  
    {}

    static constexpr const char name[] = "Array";

    template <class T, class U=VoidValue>
    ParseResult<ConvertibleVector<value_type_t<Type>>, ElementError<parse_error_t<Type>, name>>
    parse(const BitView<T>& bits, const U& parent={}) const {
        size_t size = _size_getter(parent);
        ConvertibleVector<value_type_t<Type>> results;
        results.reserve(size);
        size_t offset = 0;
        for (size_t i = 0; i < size; ++i) {
            auto parse_result = _type.parse(bits.slice(offset), parent);
            if (!parse_result) 
                return {{results.size(), std::move(parse_result.error())}};
            results.push_back(std::move(parse_result.value()));
            offset += parse_result.size();
        }
        return ParseResult<ConvertibleVector<value_type_t<Type>>, ElementError<parse_error_t<Type>, name>>{std::move(results), offset};
    }

    using SerializeError = ElementError<serialize_error_t<Type>, name>;

    template <class ValueType, class T, class U=VoidValue>
    SerializeResult<SerializeError>
    serialize(ValueType& value, BitVector<T>& bits, const U& parent={}) const {
        for (size_t i = 0; i < value.size(); ++i) {
            auto result = BitSerialization::serialize(_type, value[i], bits, parent);
            if (!result) 
                return SerializeError{i, std::move(result.error())};
        }
        return {};
    }

private:
    Type _type;
    SizeGetter _size_getter;
};

template <class Type, class SizeGetter>
constexpr const char ListType<Type, SizeGetter>::name[];

template <class... Args>
auto List(const Args&... args) {
    return ListType<Args...>(args...);
}

} // namespace BitSerialization

#endif