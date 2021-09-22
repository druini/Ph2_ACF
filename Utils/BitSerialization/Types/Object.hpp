#ifndef BITSERIALIZATION__TYPES__OBJECT_HPP
#define BITSERIALIZATION__TYPES__OBJECT_HPP

#include "../Core.hpp"

#include <string.h>


namespace BitSerialization {

template <class Key, class Type>
struct _Field {
    static constexpr auto key = Key::value;

    using type = Type;

    _Field(const Type& type) : _type(type) {}

    const auto& get_type() const { return _type; }

private:
    Type _type;
};

template <class Key, class Type>
auto Field(Key, const Type& type) {
    return _Field<Key, Type>(type);
}


template <class... Fields>
struct _Object {
    static constexpr size_t n_fields = sizeof...(Fields);

    using fields_tuple = std::tuple<Fields...>;

    using values_tuple = std::tuple<value_type_t<typename Fields::type>...>;
    
    template <class T>
    using is_required = std::integral_constant<bool, (!ignores_input_value_v<T>)>;

    using required_field_indices = indices_where_t<is_required, typename Fields::type...>;

    using required_fields_tuple = tuple_subset_t<fields_tuple, required_field_indices>;
    

    template <class>
    struct ObjectValue;

    template <class>
    struct ObjectType;

    template <class... RequiredFields>
    struct ObjectValue<std::tuple<RequiredFields...>> {
        static constexpr char const* keys[] = {Fields::key...};

        template <class Field>
        using field_value_type = value_type_t<typename Field::type, ObjectValue>;

        ObjectValue() {}

        template <size_t N = n_fields, typename std::enable_if_t<(N > 0), int> = 0>
        ObjectValue(const field_value_type<Fields>&... values) 
        : _values(std::move(values)...)
        {}
        
        template <size_t N = sizeof...(RequiredFields), typename std::enable_if_t<(N < n_fields), int> = 0>
        ObjectValue(const field_value_type<RequiredFields>&... args) {
            get_values_tuple(required_field_indices()) = std::forward_as_tuple(std::move(args)...);
        }

        template <class Other, typename std::enable_if_t<std::is_convertible<decltype(std::declval<Other>().values()), values_tuple>::value, int> = 0>
        ObjectValue(const Other& other)
          : _values(other.values()) 
        {}


        template <size_t... Is>
        auto get_values_tuple(std::index_sequence<Is...>) {
            return std::tie(std::get<Is>(_values)...);
        }

        template <size_t... Is>
        const auto get_values_tuple(std::index_sequence<Is...>) const {
            return std::tie(std::get<Is>(_values)...);
        }

        friend bool operator==(const ObjectValue& a, const ObjectValue& b) {
            return a.get_values_tuple(required_field_indices()) == b.get_values_tuple(required_field_indices());
        }

        struct NameFinder {
            const char* name;

            constexpr bool operator()(const char* value) {
                return strings_equal(name, value);
            }
        };

        template <class Key>
        auto& operator[](Key) {
            constexpr auto i = find_if(std::begin(keys), std::end(keys), NameFinder{Key::value}) - std::begin(keys);
            return std::get<i>(_values);
        }

        template <class Key>
        const auto& operator[](Key) const {
            constexpr auto i = find_if(std::begin(keys), std::end(keys), NameFinder{Key::value}) - std::begin(keys);
            return std::get<i>(_values);
        }

        template <size_t I>
        auto& get() {
            return std::get<I>(_values);
        }

        template <size_t I>
        const auto& get() const {
            return std::get<I>(_values);
        }

        values_tuple& values() { return _values; }
        const values_tuple& values() const { return _values; }
        
        friend std::ostream& operator<<(std::ostream& os, const ObjectValue& obj) {
            os << "{" << increase_indent << endl_indent;
            os << '\"' << ObjectValue::keys[0] << "\": " << std::ref(obj.template get<0>());
            for_each_index<n_fields - 1>([&] (auto index) {
                os << ',' << endl_indent << '\"' <<  ObjectValue::keys[index.value + 1] << "\": " << std::ref(obj.template get<index.value + 1>());
            });
            os << decrease_indent << endl_indent << "}";
            return os;
        }

    private:
        values_tuple _values;

    };

    using value_type = ObjectValue<required_fields_tuple>;

    template <class... RequiredFields>
    struct ObjectType<std::tuple<RequiredFields...>> {
        static constexpr bool ignores_input_value = variadic_accumulate(true, std::logical_and<>(), ignores_input_value_v<typename Fields::type>...);

        using value_type = ObjectValue<required_fields_tuple>;

        ObjectType(const Fields&... fields)
          : _types(fields.get_type()...)
        {}

        static auto Create() { return value_type{}; }

        template <size_t N = n_fields, typename std::enable_if_t<(N > 0), int> = 0>
        static auto Create(const typename value_type::field_value_type<Fields>&... values)
        {
            return  value_type(values...);
        }
        
        template <size_t N = sizeof...(RequiredFields), typename std::enable_if_t<(N < n_fields), int> = 0>
        static auto Create(const typename value_type::field_value_type<RequiredFields>&... values)
        {
            return  value_type(values...);
        }
    
        template <class Field, template <class Type, class Parent> class error_getter>
        struct FieldError {
            typename error_getter<typename Field::type, value_type>::type sub_error;

            friend std::ostream& operator<<(std::ostream& os, const FieldError& self) {
                os << "Object error (field \"" << Field::key << "\"): " << self.sub_error;
                return os;
            };
        };

        template <class Field>
        using FieldParseError = FieldError<Field, parse_error>;

        using ParseError = ErrorVariant<FieldParseError<Fields>...>;

        template <class T, class U=VoidValue>
        ParseResult<value_type, ParseError> parse(const BitView<T>& bits, const U& parent={}) const {
            value_type object;
            ParseError error;
            bool has_error = false;
            size_t offset = 0;

            for_each_index<n_fields>([&] (auto index) {
                if (has_error)
                    return;
                using Field = std::tuple_element_t<index.value, std::tuple<Fields...>>;
                auto parse_result = std::get<index.value>(_types).parse(bits.slice(offset), object);
                if (!parse_result) {
                    error = FieldParseError<Field>{std::move(parse_result.error())};
                    has_error = true;
                }
                else {
                    object.template get<index.value>() = std::move(parse_result.value());
                    offset += parse_result.size();
                }
            });

            if (has_error)
                return {std::move(error)};
            else 
                return {std::move(object), offset};
        }


        template <class Field>
        using FieldSerializeError = FieldError<Field, serialize_error>;

        using SerializeError = ErrorVariant<FieldSerializeError<Fields>...>;

        template <class T, class U=VoidValue>
        SerializeResult<SerializeError> 
        serialize(value_type& value, BitVector<T>& bits, const U& parent={}) const {
            // return serialize_impl(value, bits, std::make_index_sequence<n_fields>());
            SerializeError error;
            bool has_error = false;
            
            for_each_index<n_fields>([&] (auto index) {
                if (has_error)
                    return;
                using Field = std::tuple_element_t<index.value, std::tuple<Fields...>>;
                auto serialize_result = std::get<index.value>(_types).serialize(value.template get<index.value>(), bits, value);
                if (!serialize_result) {
                    error = FieldSerializeError<Field>{std::move(serialize_result.error())};
                    has_error = true;
                }
            });

            if (has_error)
                return {std::move(error)};
            else
                return {};
        }

    private:
        std::tuple<typename Fields::type...> _types;
    };

    using type = ObjectType<required_fields_tuple>;
};


template <class... Fields>
auto Object(const Fields&... fields) {
    return typename _Object<Fields...>::type(fields...);
}


template <class... Fields>
std::ostream& operator<<(std::ostream& os, std::reference_wrapper<const typename _Object<Fields...>::type> obj) {
    return (os << obj.get());
}

} // namespace BitSerialization

#endif