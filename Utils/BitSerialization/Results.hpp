#ifndef BITSERIALIZATION__RESULTS_HPP
#define BITSERIALIZATION__RESULTS_HPP

#include "Errors.hpp"

#include <boost/optional.hpp>


namespace BitSerialization {

template <class T, class Error=VoidError>
class ParseResult {

    struct _Data {
        T value;
        size_t size;
    };

public:
    using value_type = T;
    using error_type = Error;

    ParseResult() {}

    ParseResult(const ParseResult&) = default;

    ParseResult(ParseResult&&) = default;
    
    ParseResult(T&& value, size_t size) 
      : _storage(_Data{std::move(value), size})
    {}

    ParseResult(Error&& error) 
      : _storage(std::move(error))
    {}

    operator bool() const {
        return _storage.type() != typeid(Error);
    }

    T& value() { return boost::get<_Data>(_storage).value; }
    const T& value() const { return boost::get<_Data>(_storage).value; }

    size_t& size() { return boost::get<_Data>(_storage).size; }
    const size_t& size() const { return boost::get<_Data>(_storage).size; }

    Error& error() { return boost::get<Error>(_storage); }
    const Error& error() const { return boost::get<Error>(_storage); }

private:
    boost::variant<_Data, Error> _storage;
};



template <class Error=VoidError>
struct SerializeResult {
    using error_type = Error;

    SerializeResult() {}

    SerializeResult(Error&& error)
      : _storage(std::move(error))
    {}

    operator bool() const {
        return !bool(_storage);
    }

    Error& error() { return _storage.get(); }
    const Error& error() const { return _storage.get(); }

private:
    boost::optional<Error> _storage;
};

} // namespace BitSerialization

#endif