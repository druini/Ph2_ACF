#ifndef MATRIX_HPP
#define MATRIX_HPP

// This file contains a fixed-size Matrix class template implemented using std::valarray.
// It supports slicing, masking, arithmetic and comparison operations 

// Autor: Alkis Papadopoulos (alkiviadis.papadopoulos@cern.ch)

#include <sstream>
#include <array>
#include <valarray>
#include <vector>
#include <numeric>

// =======> Slice <=========================================================================================
// Represents a slice of a 1-D sequence

struct Slice {
    Slice() {}

    explicit Slice(size_t stop) : start(0), stop(stop), stride(1) {}

    Slice(size_t start, size_t stop) : start(start), stop(stop), stride(1) {}

    Slice(size_t start, size_t stop, size_t stride) : start(start), stop(stop), stride(stride) {}

    size_t size() const { return (stop - start) / stride; }

    size_t start;
    size_t stop;
    size_t stride;
};


// =======> gslice_array <==================================================================================
// This is a replacement for std::gslice_array<T> which is used for slicing and was needed because:
// The std::gsclice constructor creates a std::valarray<size_t> containing the selected 
// indices and the constructed std::gsclice object acts as a reference counted pointer 
// to it. The std::gslice_array<T> class stores a reference to the std::valarray<size_t> 
// created by std::gslice which is destroyed when the last std::gslice pointing to it 
// is destroyed invalidating the std::gslice_array. This means that a std::gslice_array
// created with a temporary std::gslice object is only usable within the same expression
// in which it was constructed. The following class solves this issue by storing the
// std::gslice object together with the std::gslice_array<T>.

template <class T>
struct gslice_array : public std::gslice_array<T> {
    using std::gslice_array<T>::operator=;
    using std::gslice_array<T>::operator+=;
    using std::gslice_array<T>::operator-=;
    using std::gslice_array<T>::operator*=;
    using std::gslice_array<T>::operator/=;
    using std::gslice_array<T>::operator%=;
    using std::gslice_array<T>::operator&=;
    using std::gslice_array<T>::operator|=;
    using std::gslice_array<T>::operator^=;
    using std::gslice_array<T>::operator<<=;
    using std::gslice_array<T>::operator>>=;
	
    gslice_array(std::valarray<T>& data, std::gslice&& slice) 
      : std::gslice_array<T>(data[slice])
      , _slice(std::move(slice)) 
    {}

    operator std::valarray<T>() const {
        return {_slice};
    }

private:
    std::gslice _slice;
};


// =======> Matrix <====================================================================================
// A fixed-size Matrix class supporting slicing, masking and element-wise arithmetic operations and comparisons.

template <class T, size_t Rows, size_t Cols>
struct Matrix {
    template<class, size_t, size_t> friend class Matrix; // make all Matrix<...> instantiations friends

    static constexpr size_t rows = Rows;
    static constexpr size_t cols = Cols;
    static constexpr size_t size = Rows * Cols;
    
    // =======> Constructors <==============================================================================
    Matrix() : _data(size) {}

    Matrix(const T& value) : _data(value, size) {}

    Matrix(const std::valarray<T>& data) : _data(data) {
        if (_data.size() != size) {
            std::stringstream ss;
            ss << "Matrix error1: Invalid data size: " << _data.size() << " instead of " << size;
            throw std::runtime_error(ss.str());
        }
    }

    Matrix(std::valarray<T>&& data) : _data(std::move(data)) {
        if (_data.size() != size) {
            std::stringstream ss;
            ss << "Matrix error2: Invalid data size: " << _data.size() << " instead of " << size;
            throw std::runtime_error(ss.str());
        }
    }

    template <class U, typename std::enable_if_t<std::is_constructible<U, T>::value, int> = 0>
    Matrix(const std::valarray<U>& data) {
        if (_data.size() != size) {
            std::stringstream ss;
            ss << "Matrix error3: Invalid data size: " << _data.size() << " instead of " << size;
            throw std::runtime_error(ss.str());
        }
        std::copy(std::begin(data), std::end(data), std::begin(_data));
    }

    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix(ReplacementType&& data) : Matrix(std::valarray<T>(std::forward<ReplacementType>(data))) {}

    template <class U>
    Matrix(const Matrix<U, Rows, Cols>& other) : _data(size) {
        for (size_t i = 0; i < size; ++i)
            _data[i] = other._data[i];
    }

    Matrix(const gslice_array<T>& slice) : _data(slice) {
        if (_data.size() != size)
            throw std::runtime_error("Matrix error: Invalid data size.");
    }

    // =======> Utilities <=================================================================================
    T* data() { return &_data[0]; }
    T* begin() { return data(); }
    T* end() { return data() + _data.size(); }

    // =======> Assignment operators <======================================================================
    Matrix& operator=(const T& value) { _data = value; return *this; }
    
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator=(const ReplacementType& value) { _data = value; return *this; }

    // =======> Conversion operators <======================================================================
    operator std::valarray<T>&() { return _data; }
    operator const std::valarray<T>&() const { return _data; }


    // =======> Indexing <==================================================================================
    T& operator[](const std::array<size_t, 2>& indices) {
        return _data[indices[0] * Cols + indices[1]];
    }

    const T& operator[](const std::array<size_t, 2>& indices) const {
        return _data[indices[0] * Cols + indices[1]];
    }

    T& operator()(size_t row, size_t col) {
        return _data[row * Cols + col];
    }

    const T& operator()(size_t row, size_t col) const {
        return _data[row * Cols + col];
    }

    // =======> Slicing <===================================================================================
    std::gslice gslice(const std::vector<Slice>& slices) const {
        static constexpr std::array<size_t, 2> strides = {Cols, 1};
        static constexpr std::array<size_t, 2> sizes = {Rows, Cols};
        size_t start = 0;
        std::valarray<size_t> slice_sizes(2);
        std::valarray<size_t> slice_strides(2);


        for (size_t i = 0; i < slices.size(); ++i) {
            start += slices[i].start * strides[i];
            slice_sizes[i] = slices[i].size();
            slice_strides[i] = strides[i] + slices[i].stride - 1;
        }

        for (size_t i = slices.size(); i < 2; ++i) {
            slice_sizes[i] = sizes[i];
            slice_strides[i] = strides[i];
        }

        return {start, std::move(slice_sizes), std::move(slice_strides)};
    }

    gslice_array<T> operator[](const std::vector<Slice>& slices) {
        return {_data, gslice(slices)};
    }

    std::valarray<T> operator[](const std::vector<Slice>& slices) const {
        return _data[gslice(slices)];
    }

    Matrix<T, 1, Cols> row(size_t i) {
        return {operator[]({Slice(i, i + 1), Slice(0, Cols)})};
    }

    Matrix<T, 1, Cols> row(size_t i) const {
        return {operator[]({Slice(i, i + 1), Slice(0, Cols)})};
    }
    
    Matrix<T, Rows, 1> col(size_t i) {
        return {operator[]({Slice(0, Rows), Slice(i, i + 1)})};
    }

    Matrix<T, Rows, 1> col(size_t i) const {
        return {operator[]({Slice(0, Rows), Slice(i, i + 1)})};
    }
    
    // =======> Printing <==================================================================================
    friend std::ostream& operator<<(std::ostream& os, const Matrix& mat) {
        for (size_t row = 0; row < Rows - 1; ++row) {
            for (size_t col = 0; col < Cols - 1; ++col) {
                os << mat(row, col) << ", ";
            }
            os << mat(row, Cols - 1) << '\n';
        }
        for (size_t col = 0; col < Cols - 1; ++col) {
            os << mat(Rows - 1, col) << ", ";
        }
        os << mat(Rows - 1, Cols - 1);
        return os;
    }

    std::string to_string() const {
        std::ostringstream os;
        os << *this;
        return std::move(os.str());
    }

    // =======> Parsing <===================================================================================

    friend std::istream& operator>>(std::istream& is, Matrix& mat) {
        std::string line;
        size_t index = 0;
        while (std::getline(is, line)) {
            std::istringstream line_stream(line);
            std::string item;
            while (std::getline(line_stream, item, ',')) {
                std::istringstream item_stream(item);
                item_stream >> mat._data[index++];
            }
        }
        return is;
    }

    // =======> Masking <===================================================================================
    Matrix operator[](const Matrix<bool, Rows, Cols>& mask) const { return _data[mask._data]; }
    std::mask_array<T> operator[](const Matrix<bool, Rows, Cols>& mask) { return _data[mask._data]; }
    
    // =======> Unary arithmetic operators <================================================================
    Matrix operator+() { return _data.operator+(); }
    Matrix operator-() { return _data.operator-(); }
    Matrix operator~() { return _data.operator~(); }
    Matrix operator!() { return _data.operator!(); }

    // =======> Compound assignment operators <=============================================================
    Matrix& operator+=(const Matrix& other) { _data.operator+=(other._data); return *this; }
    Matrix& operator-=(const Matrix& other) { _data.operator-=(other._data); return *this; }
    Matrix& operator*=(const Matrix& other) { _data.operator*=(other._data); return *this; }
    Matrix& operator/=(const Matrix& other) { _data.operator/=(other._data); return *this; }
    Matrix& operator%=(const Matrix& other) { _data.operator%=(other._data); return *this; }
    Matrix& operator&=(const Matrix& other) { _data.operator&=(other._data); return *this; }
    Matrix& operator|=(const Matrix& other) { _data.operator|=(other._data); return *this; }
    Matrix& operator^=(const Matrix& other) { _data.operator^=(other._data); return *this; }
    Matrix& operator<<=(const Matrix& other) { _data.operator<<=(other._data); return *this; }
    Matrix& operator>>=(const Matrix& other) { _data.operator>>=(other._data); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator+=(const std::valarray<T>& other) { _data.operator+=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator-=(const std::valarray<T>& other) { _data.operator-=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator*=(const std::valarray<T>& other) { _data.operator*=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator/=(const std::valarray<T>& other) { _data.operator/=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator%=(const std::valarray<T>& other) { _data.operator%=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator&=(const std::valarray<T>& other) { _data.operator&=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator|=(const std::valarray<T>& other) { _data.operator|=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator^=(const std::valarray<T>& other) { _data.operator^=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator<<=(const std::valarray<T>& other) { _data.operator<<=(other); return *this; }
    
    // template <class U, typename std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    Matrix& operator>>=(const std::valarray<T>& other) { _data.operator>>=(other); return *this; }


    Matrix& operator+=(const T& value) { _data.operator+=(value); return *this; }
    Matrix& operator-=(const T& value) { _data.operator-=(value); return *this; }
    Matrix& operator*=(const T& value) { _data.operator*=(value); return *this; }
    Matrix& operator/=(const T& value) { _data.operator/=(value); return *this; }
    Matrix& operator%=(const T& value) { _data.operator%=(value); return *this; }
    Matrix& operator&=(const T& value) { _data.operator&=(value); return *this; }
    Matrix& operator|=(const T& value) { _data.operator|=(value); return *this; }
    Matrix& operator^=(const T& value) { _data.operator^=(value); return *this; }
    Matrix& operator<<=(const T& value) { _data.operator<<=(value); return *this; }
    Matrix& operator>>=(const T& value) { _data.operator>>=(value); return *this; }

    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator+=(const ReplacementType& values) { _data.operator+=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator-=(const ReplacementType& values) { _data.operator-=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator*=(const ReplacementType& values) { _data.operator*=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator/=(const ReplacementType& values) { _data.operator/=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator%=(const ReplacementType& values) { _data.operator%=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator&=(const ReplacementType& values) { _data.operator&=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator|=(const ReplacementType& values) { _data.operator|=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator^=(const ReplacementType& values) { _data.operator^=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator<<=(const ReplacementType& values) { _data.operator<<=(values); return *this; }
    template <class ReplacementType, typename std::enable_if_t<std::is_constructible<std::valarray<T>, ReplacementType>::value, int> = 0>
    Matrix& operator>>=(const ReplacementType& values) { _data.operator>>=(values); return *this; }

    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator+=(const gslice_array<U>& values) { _data.operator+=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator-=(const gslice_array<U>& values) { _data.operator-=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator*=(const gslice_array<U>& values) { _data.operator*=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator/=(const gslice_array<U>& values) { _data.operator/=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator%=(const gslice_array<U>& values) { _data.operator%=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator&=(const gslice_array<U>& values) { _data.operator&=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator|=(const gslice_array<U>& values) { _data.operator|=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator^=(const gslice_array<U>& values) { _data.operator^=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator<<=(const gslice_array<U>& values) { _data.operator<<=(values); return *this; }
    // template <class U, std::enable_if_t<std::is_convertible<U, T>::value, int> = 0>
    // Matrix& operator>>=(const gslice_array<U>& values) { _data.operator>>=(values); return *this; }

    // =======> Binary arithmetic operators <=============================================================
    friend Matrix operator+(const Matrix& a, const Matrix& b) { return std::valarray<T>(a._data + b._data); }
    friend Matrix operator-(const Matrix& a, const Matrix& b) { return a._data - b._data; }
    friend Matrix operator*(const Matrix& a, const Matrix& b) { return a._data * b._data; }
    friend Matrix operator/(const Matrix& a, const Matrix& b) { return a._data / b._data; }
    friend Matrix operator%(const Matrix& a, const Matrix& b) { return a._data % b._data; }
    friend Matrix operator&(const Matrix& a, const Matrix& b) { return a._data & b._data; }
    friend Matrix operator|(const Matrix& a, const Matrix& b) { return a._data | b._data; }
    friend Matrix operator^(const Matrix& a, const Matrix& b) { return a._data ^ b._data; }
    friend Matrix operator<<(const Matrix& a, const Matrix& b) { return a._data << b._data; }
    friend Matrix operator>>(const Matrix& a, const Matrix& b) { return a._data >> b._data; }
    friend Matrix operator&&(const Matrix& a, const Matrix& b) { return a._data && b._data; }
    friend Matrix operator||(const Matrix& a, const Matrix& b) { return a._data || b._data; }


    
    template <class U>
    friend auto operator+(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() + std::declval<U>()), Rows, Cols>
    { return mat._data + value; }
    
    template <class U>
    friend auto operator-(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() - std::declval<U>()), Rows, Cols>
    { return mat._data - value; }
    
    template <class U>
    friend auto operator*(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() * std::declval<U>()), Rows, Cols>
    { return mat._data * value; }
    
    template <class U>
    friend auto operator/(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() / std::declval<U>()), Rows, Cols>
    { return mat._data / value; }
    
    template <class U>
    friend auto operator%(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() % std::declval<U>()), Rows, Cols>
    { return mat._data % value; }
    
    template <class U>
    friend auto operator&(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() & std::declval<U>()), Rows, Cols>
    { return mat._data & value; }
    
    template <class U>
    friend auto operator|(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() | std::declval<U>()), Rows, Cols>
    { return mat._data | value; }
    
    template <class U>
    friend auto operator^(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() ^ std::declval<U>()), Rows, Cols>
    { return mat._data ^ value; }
    
    template <class U>
    friend auto operator<<(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() << std::declval<U>()), Rows, Cols>
    { 
        // using value_type = decltype(std::declval<T>() << std::declval<U>());
        return {std::valarray<T>(mat._data << value)}; 
    }
    
    template <class U>
    friend auto operator>>(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() >> std::declval<U>()), Rows, Cols>
    { return mat._data >> value; }
    
    template <class U>
    friend auto operator&&(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() && std::declval<U>()), Rows, Cols>
    { return mat._data && value; }
    
    template <class U>
    friend auto operator||(const Matrix& mat, const U& value) 
        -> Matrix<decltype(std::declval<T>() || std::declval<U>()), Rows, Cols>
    { return mat._data || value; }

    
    template <class U>
    friend auto operator+(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() + std::declval<T>()), Rows, Cols>
    { return value + mat._data; }
    
    template <class U>
    friend auto operator-(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() - std::declval<T>()), Rows, Cols>
    { return value - mat._data; }
    
    template <class U>
    friend auto operator*(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() * std::declval<T>()), Rows, Cols>
    { return value * mat._data; }
    
    template <class U>
    friend auto operator/(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() / std::declval<T>()), Rows, Cols>
    { return value / mat._data; }
    
    template <class U>
    friend auto operator%(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() % std::declval<T>()), Rows, Cols>
    { return value % mat._data; }
    
    template <class U>
    friend auto operator&(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() & std::declval<T>()), Rows, Cols>
    { return value & mat._data; }
    
    template <class U>
    friend auto operator|(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() | std::declval<T>()), Rows, Cols>
    { return value | mat._data; }
    
    template <class U>
    friend auto operator^(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() ^ std::declval<T>()), Rows, Cols>
    { return value ^ mat._data; }
    
    // template <class U, std::enable_if_t<!std::is_base_of<std::ostream, U>::value, int> = 0>
    // friend auto operator<<(const U& value, const Matrix& mat) 
    //     ->Matrix<decltype(std::declval<U>() << std::declval<T>()), Rows, Cols>
    // { return value << mat._data; }
    
    // template <class U, std::enable_if_t<!std::is_base_of<std::istream, U>::value, int> = 0>
    // friend auto operator>>(const U& value, const Matrix& mat) 
    //     ->Matrix<decltype(std::declval<U>() >> std::declval<T>()), Rows, Cols>
    // { return value >> mat._data; }
    
    template <class U>
    friend auto operator&&(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() && std::declval<T>()), Rows, Cols>
    { return value && mat._data; }
    
    template <class U>
    friend auto operator||(const U& value, const Matrix& mat) 
        ->Matrix<decltype(std::declval<U>() || std::declval<T>()), Rows, Cols>
    { return value || mat._data; }

    // friend Matrix operator+(const Matrix& mat, const T& value) { return mat._data + value; }
    // friend Matrix operator-(const Matrix& mat, const T& value) { return mat._data - value; }
    // friend Matrix operator*(const Matrix& mat, const T& value) { return mat._data * value; }
    // friend Matrix operator/(const Matrix& mat, const T& value) { return mat._data / value; }
    // friend Matrix operator%(const Matrix& mat, const T& value) { return mat._data % value; }
    // friend Matrix operator&(const Matrix& mat, const T& value) { return mat._data & value; }
    // friend Matrix operator|(const Matrix& mat, const T& value) { return mat._data | value; }
    // friend Matrix operator^(const Matrix& mat, const T& value) { return mat._data ^ value; }
    // friend Matrix operator<<(const Matrix& mat, const T& value) { return mat._data << value; }
    // friend Matrix operator>>(const Matrix& mat, const T& value) { return mat._data >> value; }
    // friend Matrix operator&&(const Matrix& mat, const T& value) { return mat._data && value; }
    // friend Matrix operator||(const Matrix& mat, const T& value) { return mat._data || value; }

    // friend Matrix operator+(const T& value, const Matrix& mat) { return value + mat._data; }
    // friend Matrix operator-(const T& value, const Matrix& mat) { return value - mat._data; }
    // friend Matrix operator*(const T& value, const Matrix& mat) { return value * mat._data; }
    // friend Matrix operator/(const T& value, const Matrix& mat) { return value / mat._data; }
    // friend Matrix operator%(const T& value, const Matrix& mat) { return value % mat._data; }
    // friend Matrix operator&(const T& value, const Matrix& mat) { return value & mat._data; }
    // friend Matrix operator|(const T& value, const Matrix& mat) { return value | mat._data; }
    // friend Matrix operator^(const T& value, const Matrix& mat) { return value ^ mat._data; }
    // friend Matrix operator<<(const T& value, const Matrix& mat) { return value << mat._data; }
    // friend Matrix operator>>(const T& value, const Matrix& mat) { return value >> mat._data; }
    // friend Matrix operator&&(const T& value, const Matrix& mat) { return value && mat._data; }
    // friend Matrix operator||(const T& value, const Matrix& mat) { return value || mat._data; }

    // =======> Comparison operators <====================================================================
    friend Matrix<bool, Rows, Cols> operator==(const Matrix& a, const Matrix& b) { return a._data == b._data; }
    friend Matrix<bool, Rows, Cols> operator!=(const Matrix& a, const Matrix& b) { return a._data != b._data; }
    friend Matrix<bool, Rows, Cols> operator<(const Matrix& a, const Matrix& b) { return a._data < b._data; }
    friend Matrix<bool, Rows, Cols> operator<=(const Matrix& a, const Matrix& b) { return a._data <= b._data; }
    friend Matrix<bool, Rows, Cols> operator>(const Matrix& a, const Matrix& b) { return a._data > b._data; }
    friend Matrix<bool, Rows, Cols> operator>=(const Matrix& a, const Matrix& b) { return a._data >= b._data; }

    friend Matrix<bool, Rows, Cols> operator==(const Matrix& mat, const T& value) { return mat._data == value; }
    friend Matrix<bool, Rows, Cols> operator!=(const Matrix& mat, const T& value) { return mat._data != value; }
    friend Matrix<bool, Rows, Cols> operator<(const Matrix& mat, const T& value) { return mat._data < value; }
    friend Matrix<bool, Rows, Cols> operator<=(const Matrix& mat, const T& value) { return mat._data <= value; }
    friend Matrix<bool, Rows, Cols> operator>(const Matrix& mat, const T& value) { return mat._data > value; }
    friend Matrix<bool, Rows, Cols> operator>=(const Matrix& mat, const T& value) { return mat._data >= value; }
    
    friend Matrix<bool, Rows, Cols> operator==(const T& value, const Matrix& mat) { return value == mat._data; }
    friend Matrix<bool, Rows, Cols> operator!=(const T& value, const Matrix& mat) { return value != mat._data; }
    friend Matrix<bool, Rows, Cols> operator<(const T& value, const Matrix& mat) { return value < mat._data; }
    friend Matrix<bool, Rows, Cols> operator<=(const T& value, const Matrix& mat) { return value <= mat._data; }
    friend Matrix<bool, Rows, Cols> operator>(const T& value, const Matrix& mat) { return value > mat._data; }
    friend Matrix<bool, Rows, Cols> operator>=(const T& value, const Matrix& mat) { return value >= mat._data; }

private:
    std::valarray<T> _data;
};

#endif