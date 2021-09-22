#ifndef BITSERIALIZATION__TYPES__SEPARATEDBY_HPP
#define BITSERIALIZATION__TYPES__SEPARATEDBY_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class Type, class SeparatorType>
struct SeparatedByType {
    static_assert(ignores_input_value_v<SeparatorType>, "SeparatorType must ignore input value (ie. be a constant or something)");

    static constexpr bool ignores_input_value = ignores_input_value_v<Type>;
    using value_type = std::vector<value_type_t<Type>>;

    SeparatedByType(const Type& type, const SeparatorType& separator)
      : _type(type)
      , _separator(separator)
    {}

    static constexpr const char name[] = "SeparatedBy";

    using ParseError = ElementError<parse_error_t<Type>, name>;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        value_type results;
        size_t offset = 0;
        while (true) {
            auto element = _type.parse(bits.slice(offset), parent);
            if (!element) 
                return ParseError{results.size(), std::move(element.error())};
            results.push_back(std::move(element.value()));
            offset += element.size();
            auto sep = _separator.parse(bits.slice(offset), parent);
            if (!sep) 
                break;
            offset += sep.size();
        }
        return {std::move(results), offset};
    }

    using SerializeElementError = ElementError<serialize_error_t<Type>, name>;

    using SerializeError = ErrorVariant<
        SerializeElementError, 
        serialize_error_t<SeparatorType>
    >;

    template <class T, class U=VoidValue>
    SerializeResult<SerializeError> 
    serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        if (value.size() > 0) {
            for (size_t i = 0; i < value.size(); ++i) {
                auto element = _type.serialize(value[i], bits, parent);
                if (!element)
                    return {SerializeElementError{i, std::move(element.error())}};
                if (i < value.size() - 1) {
                    value_type_t<SeparatorType> v;
                    auto sep = _separator.serialize(v, bits, parent);
                    if (!sep)
                        return {std::move(sep.error())};
                }
            }
        }
        return {};
    }

private:
    Type _type;
    SeparatorType _separator;
};


template <class Type, class SeparatorType>
constexpr const char SeparatedByType<Type, SeparatorType>::name[];


template <class Type, class SeparatorType>
auto SeparatedBy(const Type& type, const SeparatorType& separator) {
    return SeparatedByType<Type, SeparatorType>(type, separator);
}

} // namespace BitSerialization

#endif