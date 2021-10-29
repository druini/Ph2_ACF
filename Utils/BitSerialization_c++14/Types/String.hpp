#ifndef BITSERIALIZATION__TYPES__STRING_HPP
#define BITSERIALIZATION__TYPES__STRING_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class ElementType, class TerminatorType, bool IncludeTerminator=true>
struct StringType {
    static constexpr bool ignores_input_value = ignores_input_value_v<ElementType>;

    using value_type = ConvertibleVector<value_type_t<ElementType>>;

    StringType(const ElementType& type, const TerminatorType& terminator)
      : _type(type)
      , _terminator(terminator)
    {}

    static constexpr const char name[] = "String";

    using ParseError = ElementError<parse_error_t<ElementType>, name>;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        value_type results;
        size_t offset = 0;
        while (true) {
            auto terminator_result = _terminator.parse(bits.slice(offset), parent);
            if (terminator_result) {
                if (IncludeTerminator) {
                    results.push_back(
                        value_type_t<ElementType>(std::move(terminator_result.value()))
                    );
                    offset += terminator_result.size();
                }
                break;
            }
            auto element_result = _type.parse(bits.slice(offset), parent);
            if (!element_result) {
                return ParseError{results.size(), std::move(element_result.error())};
            }
            results.push_back(std::move(element_result.value()));
            offset += element_result.size();
        }
        return {std::move(results), offset};
    }

    using SerializeElementError = ElementError<serialize_error_t<ElementType>, name>;

    using SerializeError = ErrorVariant<
        SerializeElementError,
        serialize_error_t<TerminatorType>
    >;

    template <class T, class U, class ValueType>
    auto serialize_terminator(std::true_type, ValueType& value, BitVector<T>& bits, const U& parent) const {
        value_type_t<TerminatorType> v{value};
        return BitSerialization::serialize(_terminator, v, bits, parent);
    }

    template <class T, class U, class ValueType>
    auto serialize_terminator(std::false_type, ValueType&, BitVector<T>& bits, const U& parent) const {
        value_type_t<TerminatorType> value{};
        return BitSerialization::serialize(_terminator, value, bits, parent);
    }
    

    template <class T, class U=VoidValue>
    SerializeResult<SerializeError>
    serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        for (size_t i = 0; i < value.size() - IncludeTerminator; ++i) {
            auto result = BitSerialization::serialize(_type, value[i], bits, parent);
            if (!result)
                return {SerializeElementError{i, std::move(result.error())}};
        }
        auto result = serialize_terminator(
            std::integral_constant<bool, IncludeTerminator>(), 
            value.back(), 
            bits,
            parent
        );
        if (!result)
            return {std::move(result.error())};
        return {};
    }

private:
    ElementType _type;
    TerminatorType _terminator;
};

template <class ElementType, class TerminatorType, bool IncludeTerminator>
constexpr const char StringType<ElementType, TerminatorType, IncludeTerminator>::name[];

template <bool IncludeTerminator=true, class ElementType, class TerminatorType>
auto String(const ElementType& type, const TerminatorType& terminator, std::integral_constant<bool, IncludeTerminator> = {}) {
    return StringType<ElementType, TerminatorType, IncludeTerminator>(type, terminator);
}

} // namespace BitSerialization

#endif
