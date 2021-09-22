#ifndef BITSERIALIZATION__TYPES__COMPOSE3_HPP
#define BITSERIALIZATION__TYPES__COMPOSE3_HPP

#include "../Core.hpp"

namespace BitSerialization {

template <class SourceType, class IntermediateType, class DestinationType>
struct Compose3Type {
    static constexpr bool ignores_input_value = ignores_input_value_v<SourceType>;
    using value_type = value_type_t<SourceType>;

    Compose3Type(
        const SourceType& source_type, 
        const IntermediateType& intermediate_type,
        const DestinationType& destination_type
    ) 
      : _source_type(source_type)
      , _intermediate_type(intermediate_type)
      , _destination_type(destination_type)
    {}
    
    template <size_t I, class SubError>
    struct Error {
        static constexpr const char* stage_names[] = {"source", "intermediate", "destination"};
        
        SubError error;

        Error() {}

        Error(SubError&& error) : error(std::move(error)) {}

        friend std::ostream& operator<<(std::ostream& os, const Error& self) {
            return (os << "Compose3 " << stage_names[I] << " error: " << self.error);
        }
    };

    template <size_t I, class SubError>
    static Error<I, SubError> make_error(SubError&& error) {
        return {std::move(error)};
    }

    using ParseError = ErrorVariant<
        Error<0, parse_error_t<SourceType>>,
        Error<1, serialize_error_t<IntermediateType>>,
        Error<2, parse_error_t<DestinationType>>
    >;

    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> 
    parse(const BitView<T>& bits, const U& parent={}) const {
        BitVector<uint64_t> new_bits;

        auto destination_result = _destination_type.parse(bits, parent);
        if (!destination_result)
            return {make_error<2>(std::move(destination_result.error()))};
        
        auto intermediate_result = _intermediate_type.serialize(
            destination_result.value(), 
            new_bits, 
            parent
        );
        if (!intermediate_result)
            return {make_error<1>(std::move(intermediate_result.error()))};

        auto source_result = _source_type.parse(bit_view(new_bits), parent);
        if (!source_result)
            return {make_error<0>(std::move(source_result.error()))};

        return {std::move(source_result.value()), destination_result.size()};
    }

    using SerializeError = ErrorVariant<
        Error<0, serialize_error_t<SourceType>>,
        Error<1, parse_error_t<IntermediateType>>,
        Error<2, serialize_error_t<DestinationType>>
    >;
    
    template <class T, class U=VoidValue>
    SerializeResult<SerializeError> 
    serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        BitVector<uint64_t> new_bits;
        
        // std::cout << "Compose3 input: " << std::ref(value) << std::endl;

        auto source_result = _source_type.serialize(value, new_bits, parent);
        if (!source_result)
            return {make_error<0>(std::move(source_result.error()))};
        
        // std::cout << "Compose3 buffer: " << new_bits.view() << std::endl;

        auto intermediate_result = _intermediate_type.parse(bit_view(new_bits), parent);
        if (!intermediate_result)
            return {make_error<1>(std::move(intermediate_result.error()))};
            
        // std::cout << "Compose3 value: " << std::ref(intermediate_result.value()) << std::endl;

        auto destination_result = _destination_type.serialize(
            intermediate_result.value(), 
            bits, 
            parent
        );
        if (!destination_result)
            return {make_error<2>(std::move(destination_result.error()))};
        
        return {};
    }

private:
    SourceType _source_type;
    IntermediateType _intermediate_type;
    DestinationType _destination_type;
};

template <class... Args>
auto Compose3(const Args&... args) {
    return Compose3Type<Args...>(args...);
}

} // namespace BitSerialization

#endif