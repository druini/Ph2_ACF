#ifndef NAMEDTUPLE_H
#define NAMEDTUPLE_H

#include <tuple>
#include <functional>


#ifndef COMPILETIMESTRINGLITERAL
#define COMPILETIMESTRINGLITERAL

namespace compile_time_string_literal {

    template <char... Chars>
    struct c_str {
        static const char value[sizeof...(Chars) + 1];
    };

    template <char... Chars>
    const char c_str<Chars...>::value[] = {Chars..., 0};

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"

    template<typename Char, Char... Cs>
    constexpr c_str<Cs...> operator"" _s(){
        return {};
    }

    #pragma GCC diagnostic pop

}

#endif

namespace NamedTuple {

namespace detail {

    // constexpr variant of std::find
    template<class InputIt, class T, class Cmp = std::equal_to<>>
    constexpr InputIt find(InputIt first, InputIt last, const T& value, Cmp&& cmp = {}) {
        for (; first != last; ++first) 
            if (std::forward<Cmp>(cmp)(*first, value)) 
                return first;
        return last;
    }

    constexpr bool strings_equal(char const * a, char const * b) {
        return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
    }


    template <class T>
    std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
        if (vec.size() == 0)
            return os << "[]";
        os << "[ ";
        for (int i = 0; i < (int)vec.size() - 1; ++i) {
            os << vec[i] << ", ";
        }
        os << vec.back() << " ]";
        return os;
    }

}

struct NamedTupleBase {};


template <class... Fields>
struct NamedTuple : public NamedTupleBase {
    using names_tuple = std::tuple<typename Fields::first_type...>;
    using value_type = std::tuple<typename Fields::second_type...>;
    static constexpr const char* names[] = {Fields::first_type::value...};

    template <size_t I>
    using field_type = std::tuple_element_t<I, value_type>;

    static constexpr size_t size = sizeof...(Fields);

    // template <class Self = NamedTuple, typename std::enable_if_t<(Self::size > 0), int> = 0>
    constexpr NamedTuple() {}

    template <size_t Size = size, class... Us, std::enable_if_t<(Size > 0 && std::is_constructible<value_type, Us...>::value), int> En = 0>
    constexpr NamedTuple(Us&&... values) : _values(std::forward<Us>(values)...) {}

    template <size_t Size = size, std::enable_if_t<(Size > 0), int> En = 0>
    constexpr NamedTuple(const typename Fields::second_type&... values) : _values(values...) {}

    // template <size_t Size = sizeof...(Fields), std::enable_if_t<(Size > 0), int> En = 0>
    // constexpr NamedTuple(typename Fields::second_type&&... values) : _values(std::move(values)...) {}

    template <class Name>
    static constexpr size_t index = 
        detail::find(std::begin(names), std::end(names), Name::value, detail::strings_equal) - std::begin(names);

    template <class Name>
    constexpr auto& operator[](Name) { return std::get<index<Name>>(_values); }

    template <class Name>
    constexpr const auto& operator[](Name) const { return std::get<index<Name>>(_values); }

    constexpr value_type& values() { return _values; }
    constexpr const value_type& values() const { return _values; }

    
    template <class F, size_t... Is>
    static void for_each_index(F&& f, std::index_sequence<Is...>) {
        int unused[] = {0, (std::forward<F>(f)(std::integral_constant<size_t, Is>{}), 0)...};
        (void)unused;
    }

    template <class F>
    static void for_each_index(F&& f) {
        for_each_index(std::forward<F>(f), std::make_index_sequence<size>());
    }

    template <class F> void for_each(F&& f) const { 
        for_each_index([&] (auto index) {
            constexpr auto i = decltype(index)::value;
            std::forward<F>(f)(std::tuple_element_t<i, names_tuple>{}, std::get<i>(_values));
        });
    }

    template <class F> void for_each(F&& f) { 
        for_each_index([&] (auto index) {
            constexpr auto i = decltype(index)::value;
            std::forward<F>(f)(std::tuple_element_t<i, names_tuple>{}, std::get<i>(_values));
        });
    }

    template <class T, size_t... Is>
    static constexpr size_t index_of(std::index_sequence<Is...>) {
        size_t i = size;
        size_t unused[] = {0, (i = (i == size && std::is_same<std::tuple_element_t<Is, value_type>, std::decay_t<T>>::value) ? Is : i)...};
        return i;
    }

    template <class T>
    static constexpr size_t index_of() {
        return index_of<T>(std::make_index_sequence<size>{});
    }

    friend std::ostream& operator<<(std::ostream& os, const NamedTuple& t) {
        using detail::operator<<;
        os << "{ ";
        t.for_each([&] (auto name, const auto& value) {
            os << '\"'<< name.value << "\" : " << value;
            if (!std::is_same<decltype(name), std::tuple_element_t<size - 1, names_tuple>>::value)
                os << ", ";
        });
        os << " }";
        return os;
    }

    template <class T>
    using name_of = std::tuple_element_t<index_of<T>(), names_tuple>;

private:
    value_type _values;
};

template <class... Fields>
const char* const NamedTuple<Fields...>::names[];

template <class... Names, class... Ts>
constexpr auto make_named_tuple(const std::pair<Names, Ts>&... fields) {
    return NamedTuple<std::pair<Names, Ts>...>(fields.second...);
}

template <class T, class Name>
constexpr std::pair<Name, T> tuple_field(Name, T&& value = {}) {
    return {Name{}, std::forward<T>(value)};
}


template <class Tuple, class F, size_t... Is>
void for_each_index(Tuple&& tuple, F&& f, std::index_sequence<Is...>) {
    int unused[] = {0, (std::forward<F>(f)(std::integral_constant<size_t, Is>{}), 0)...};
    (void)unused;
}

template <class Tuple, class F>
void for_each_index(Tuple&& tuple, F&& f) {
    for_each_index(std::forward<Tuple>(tuple), std::forward<F>(f), std::make_index_sequence<Tuple::size>());
}

template <class Tuple, class F> 
void for_each(Tuple&& tuple, F&& f) { 
    for_each_index(std::forward<Tuple>(tuple), [&] (auto index) {
        constexpr auto i = decltype(index)::value;
        std::forward<F>(f)(std::tuple_element_t<i, typename Tuple::names_tuple>{}, std::get<i>(tuple.values()));
    });
}


// ======> named_tuple_cat <=============================================================
// template <class... NamesA, class... NamesB>
// constexpr auto names_cat(Named<NamesA...>, Named<NamesB...>)
//     -> Named<NamesA..., NamesB...>;

// template <
//     class... TypesA, template <class...> class InnerA,
//     class... TypesB, template <class...> class InnerB
// >
// constexpr auto named_tuple_cat(const InnerA<TypesA...>& a, const InnerB<TypesB...>& b) 
//     -> typename decltype(names_cat(typename InnerA<TypesA...>::outter{}, typename InnerB<TypesB...>::outter{}))::template Tuple<TypesA..., TypesB...>
// {
//     return {std::tuple_cat(a.values(), b.values())};
// }

template <size_t I, class T>
auto get(T&& tuple) {
    return std::get<I>(std::forward<T>(tuple).values());
}

}

#endif