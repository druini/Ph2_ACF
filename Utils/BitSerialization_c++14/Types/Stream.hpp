#ifndef BITSERIALIZATION__TYPES__STREAM_HPP
#define BITSERIALIZATION__TYPES__STREAM_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class InitiatorType, class ElementType, bool IncludeInitiator=true>
struct StreamType {
    static constexpr bool ignores_input_value = ignores_input_value_v<ElementType>;

    // using value_type = std::vector<std::conditional<IncludeInitiator,
    //     std::common_type_t<value_type_t<InitiatorType>, value_type_t<ElementType>>,
    //     value_type_t<ElementType>
    // >>;
    
    using value_type = ConvertibleVector<value_type_t<ElementType>>;

    StreamType(const InitiatorType& initiator, const ElementType& element)
      : _initiator(initiator)
      , _element(element)
    {}

    static constexpr const char name[] = "Stream";

    using ParseError = ElementError<parse_error_t<InitiatorType>, name>;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        value_type results;
        size_t offset = 0;
        auto initiator_result = _initiator.parse(bits.slice(offset), parent);
        if (!initiator_result)
            return {ParseError{IncludeInitiator - 1, std::move(initiator_result.error())}};
        offset += initiator_result.size();
        if (IncludeInitiator)
            results.push_back(initiator_result.value());
        while (offset < bits.size()) {
            auto element_result = _element.parse(bits.slice(offset), parent);
            if (!element_result) {
                break;
            }
            results.push_back(std::move(element_result.value()));
            offset += element_result.size();
        }
        return {std::move(results), offset};
    }

    using SerializeError = ErrorVariant<
        ElementError<serialize_error_t<InitiatorType>, name>,
        ElementError<serialize_error_t<ElementType>, name>
    >;

    template <class T, class U=VoidValue>
    SerializeResult<SerializeError>
    serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        auto initiator_result = BitSerialization::serialize(_initiator, IncludeInitiator ? value[0] : value_type_t<InitiatorType>{}, bits, parent);
        if (!initiator_result)
            return SerializeError{IncludeInitiator - 1, std::move(initiator_result.error())};
        for (size_t i = IncludeInitiator ? 1 : 0; i < value.size(); ++i) {
            auto element_result = BitSerialization::serialize(_element, value[i], bits, parent);
            if (!element_result)
                return SerializeError{i, std::move(element_result.error())};
        }
        return {};
    }

private:
    InitiatorType _initiator;
    ElementType _element;
};

template <class ElementType, class TerminatorType, bool IncludeTerminator>
constexpr const char StreamType<ElementType, TerminatorType, IncludeTerminator>::name[];

template <bool IncludeTerminator=true, class ElementType, class InitiatorType>
auto Stream(const InitiatorType& initiator, const ElementType& element, std::integral_constant<bool, IncludeTerminator> = {}) {
    return StreamType<InitiatorType, ElementType, IncludeTerminator>(initiator, element);
}

} // namespace BitSerialization

#endif
