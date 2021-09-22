#ifndef BITSERIALIZATION__ERRORS_HPP
#define BITSERIALIZATION__ERRORS_HPP

#include <ostream>
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>


namespace BitSerialization {

struct VoidError {
    friend std::ostream& operator<<(std::ostream& os, const VoidError&) {
        return (os << "Generic error (no message).");
    }
};

template <const char* TypeName>
struct SizeError {
    size_t size;

    friend std::ostream& operator<<(std::ostream& os, const SizeError& error) {
        return (os << TypeName << " error: not enough bits (" << error.size << ").");
    }
};


template <const char* TypeName, class T>
struct ValueError {
    T value;

    friend std::ostream& operator<<(std::ostream& os, const ValueError& error) {
        return (os << TypeName << " error: invalid value (" << std::ref(error.value) << ").");
    }
};


template <class SubError, const char* TypeName>
struct ElementError {
    size_t index;
    SubError error;

    ElementError() {}
    
    ElementError(size_t index, SubError&& error) 
      : index(index)
      , error(std::move(error)) 
    {}

    friend std::ostream& operator<<(std::ostream& os, const ElementError& self) {
        os << TypeName << " error at element #" << self.index << ": " << self.error;
        return os;
    };
};



template <class... Errors>
struct _ErrorVariant;

template<typename T, typename VARIANT_T>
struct isVariantMember;

template<typename T, typename... ALL_T>
struct isVariantMember<T, _ErrorVariant<ALL_T...>> 
  : public std::integral_constant<bool, variadic_accumulate(false, std::logical_or<>{}, std::is_same<T, ALL_T>::value...)> {};
//   : public std::disjunction<std::is_same<T, ALL_T>...> {};

template <class... Errors>
struct _ErrorVariant {
    boost::variant<Errors...> storage;

    _ErrorVariant() {}

    _ErrorVariant(const _ErrorVariant& other) = default;

    _ErrorVariant(_ErrorVariant&& other) noexcept = default;

    template <class Error, typename std::enable_if_t<isVariantMember<Error, _ErrorVariant>::value, int> = 0>
    _ErrorVariant(Error&& error)
      : storage(std::forward<Error>(error))
    {}

     
    _ErrorVariant& operator=(const _ErrorVariant& other) {
        storage = other.storage;
        return *this;
    }
    
    _ErrorVariant& operator=(_ErrorVariant&& other) noexcept {
        storage = std::move(other.storage);
        return *this;
    }

    template <class Error>
    _ErrorVariant& operator=(Error&& error) {
        storage = std::forward<Error>(error);
        return *this;
    }

    struct PrintingVisitor : public boost::static_visitor<void> {
        PrintingVisitor(std::ostream& os) : os(os) {}

        template <class T>
        void operator()(const T& error) const {
            os << error;
        }

    private:
        std::ostream& os;
    };

    friend std::ostream& operator<<(std::ostream& os, const _ErrorVariant& error) {
        // std::visit([&] (const auto& error) { os << error; }, error.storage);
        boost::apply_visitor(PrintingVisitor(os), error.storage);
        return os;
    }
};



template <class Variant, class... Ts>
struct variant_append;

template <class... Us, class T, class... Ts>
struct variant_append<_ErrorVariant<Us...>, T, Ts...> {
    using type = std::conditional_t<
        variadic_accumulate(false, std::logical_or<>{}, std::is_same<T, Us>::value...),
        // (std::is_same<T, Us>::value || ...),
        typename variant_append<_ErrorVariant<Us...>, Ts...>::type,
        typename variant_append<_ErrorVariant<Us..., T>, Ts...>::type
    >;
};

template <class Variant>
struct variant_append<Variant> {
    using type = Variant;
};

template <class... Ts>
struct make_variant;

template <class T, class... Ts>
struct make_variant<T, Ts...> {
    using type = typename variant_append<_ErrorVariant<T>, Ts...>::type;
};

template <>
struct make_variant<> {
    using type = _ErrorVariant<>;
};

template <class... Ts>
using ErrorVariant = typename make_variant<Ts...>::type;

} // namespace BitSerialization

#endif