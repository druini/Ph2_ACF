#ifndef BITSERIALIZATION__TYPES__ONEOF_HPP
#define BITSERIALIZATION__TYPES__ONEOF_HPP

#include "../Core.hpp"


namespace BitSerialization {

template <class... Types>
struct OneOfType {
    static constexpr bool ignores_input_value = variadic_accumulate(true, std::logical_and<>(), ignores_input_value_v<Types>...);
    using value_type = std::common_type_t<typename Types::value_type...>;

    OneOfType(const Types&... types) : _types(types...) {}

    template <class... SubErrors>
    struct Error {
        std::tuple<SubErrors...> errors;

        friend std::ostream& operator<<(std::ostream& os, const Error& self) {
            os << "OneOf error (all types failed): ";
            for_each_index<sizeof...(SubErrors)>([&] (auto index) {
                os << std::get<index.value>(self.errors) << "; ";
            });
            return os;
        }
    };

    using ParseError = Error<parse_error_t<Types>...>;
    
    template <class T, class U=VoidValue>
    ParseResult<value_type, ParseError> parse(const BitView<T>& bits, const U& parent={}) const {
        // return parse_impl(bits, parent, std::make_index_sequence<sizeof...(Types)>());
        ParseError error;        
        value_type overall_result;
        size_t size = 0;

        bool done = false;
        
        for_each_index<sizeof...(Types)>([&] (auto index) {
            if (done)
                return;
            auto result = std::get<index.value>(_types).parse(bits, parent);
            if (result) {
                overall_result = std::move(result.value());
                size = result.size();
                done = true;
            }
            else
                std::get<index.value>(error.errors) = std::move(result.error());
        });

        if (done)
            return {std::move(overall_result), size};
        else
            return std::move(error);
    }
    
    using SerializeError = Error<serialize_error_t<Types>...>;

    template <class T, class U=VoidValue>
    SerializeResult<SerializeError> serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
        SerializeError error;
        bool done = false;

        for_each_index<sizeof...(Types)>([&] (auto index) {
            if (done)
                return;
            auto result = std::get<index.value>(_types).serialize(value, bits, parent);
            if (result)
                done = true;
            else
                std::get<index.value>(error.errors) = std::move(result.error());
        });

        if (done)
            return {};
        else
            return std::move(error);
    }

private:
    std::tuple<Types...> _types;
};

template <class... Types>
auto OneOf(const Types&... types) {
    return OneOfType<Types...>(types...);
}

} // namespace BitSerialization

#endif