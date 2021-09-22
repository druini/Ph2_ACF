#ifndef BITSERIALIZATION__UTILITIY_HPP
#define BITSERIALIZATION__UTILITIY_HPP

#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <functional>
#include <array>

namespace BitSerialization {

template <size_t N>
using uint_t =  std::conditional_t<N <=  8, uint8_t,
                std::conditional_t<N <= 16, uint16_t,
                std::conditional_t<N <= 32, uint32_t, uint64_t>>>;


constexpr int iabs(int x) { return x < 0 ? -x : x; }

constexpr size_t cilog2(size_t val) { return val ? 1 + cilog2(val >> 1) : -1; }

constexpr size_t cilog(size_t val, size_t base) { return cilog2(val) / cilog2(base); }


template <class F, class... Args>
constexpr void variadic_apply(F&& f, Args&&... args) {
    int unused[] = {0, (std::forward<F>(f)(std::forward<Args>(args)), 0)... };
    (void)unused; // blocks warnings
}


template <class F, class T, class... Args>
constexpr auto variadic_accumulate(const T& init, F&& f, Args&&... args) {
    T value = init;
    int unused[] = {0, (value = std::forward<F>(f)(value, std::forward<Args>(args)), 0)... };
    (void)unused; // blocks warnings
    return value;
}

template <class F, size_t... Is>
constexpr void for_each_index(F&& f, std::index_sequence<Is...>) {
    variadic_apply(f, std::integral_constant<size_t, Is>()...);
}

template <size_t N, class F>
constexpr void for_each_index(F&& f) {
    for_each_index(std::forward<F>(f), std::make_index_sequence<N>());
}


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

constexpr bool strings_equal(char const * a, char const * b) {
    return *a == *b && (*a == '\0' || strings_equal(a + 1, b + 1));
}



template <class C> 
constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}

template <class T, size_t N>
constexpr size_t size(const T (&array)[N]) noexcept
{
    return N;
}


template<class InputIt, class T>
constexpr InputIt find(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (*first == value) {
            return first;
        }
    }
    return last;
}

template<class InputIt, class UnaryPredicate>
constexpr InputIt find_if(InputIt first, InputIt last, UnaryPredicate p)
{
    for (; first != last; ++first) {
        if (p(*first)) {
            return first;
        }
    }
    return last;
}


template <bool B, class T, class U>
auto conditional(T&& a, U&& b) {
    return std::get<!B>(std::make_tuple(std::forward<T>(a), std::forward<U>(b)));
}



template <typename... >
struct index_of;

// found it
template <typename T, typename... R>
struct index_of<T, T, R...>
: std::integral_constant<size_t, 0>
{ };

// still looking
template <typename T, typename F, typename... R>
struct index_of<T, F, R...>
: std::integral_constant<size_t, 1 + index_of<T,R...>::value>
{ };

template <class T, class... Ts>
std::tuple<T, Ts...> _tuple_prepend(std::tuple<Ts...>);

template <class T, class Tuple>
using tuple_prepend_t = decltype(_tuple_prepend<T>(Tuple()));

template <template <class> class Predicate, class... Ts>
struct filtered_tuple;

template <template <class> class Predicate, class... Ts>
using filtered_tuple_t = typename filtered_tuple<Predicate, Ts...>::type;

template <template <class> class Predicate, class T, class... Ts>
struct filtered_tuple<Predicate, T, Ts...> {
    using type = std::conditional_t<
        Predicate<T>::value,
        tuple_prepend_t<T, filtered_tuple_t<Predicate, Ts...>>,
        filtered_tuple_t<Predicate, Ts...>
    >;
};

template <template <class> class Predicate>
struct filtered_tuple<Predicate> {
    using type = std::tuple<>;
};


template <size_t I, size_t... Is>
std::index_sequence<I, Is...> _index_seq_prepend(std::index_sequence<Is...>);

template <size_t I, class Seq>
using index_seq_prepend_t = decltype(_index_seq_prepend<I>(Seq()));

template <template <class> class Predicate, size_t I, class... Ts>
struct indices_where;

template <template <class> class Predicate, size_t I, class T, class... Ts>
struct indices_where<Predicate, I, T, Ts...> {
    using type = std::conditional_t<
        Predicate<T>::value,
        index_seq_prepend_t<I, typename indices_where<Predicate, I + 1, Ts...>::type>,
        typename indices_where<Predicate, I + 1, Ts...>::type
    >;
};

template <template <class> class Predicate, size_t I>
struct indices_where<Predicate, I> {
    using type = std::index_sequence<>;
};

template <template <class> class Predicate, class... Ts>
using indices_where_t = typename indices_where<Predicate, 0, Ts...>::type;

template <class Tuple, size_t... Is>
std::tuple<std::tuple_element_t<Is, Tuple>...> _tuple_subset(std::index_sequence<Is...>);

template <class Tuple, class Seq>
using tuple_subset_t = decltype(_tuple_subset<Tuple>(Seq()));

} // namespace BitSerialization

#endif