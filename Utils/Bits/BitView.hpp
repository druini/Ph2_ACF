#ifndef BITS__BIT_VIEW_HPP
#define BITS__BIT_VIEW_HPP

// #include <sys/cdefs.h>
// #include <stddef.h>
// #include <cstdint>
#include <array>
#include <vector>

#include <iostream>
#include <bitset>


namespace BitViewDetails {

template <class T>
T high_mask(int n) {
    if (n <= 0 || n > int(sizeof(T) * 8))
        return 0;
    return (T)~T{0} << uint8_t(sizeof(T) * 8 - n);
}

template <class T>
T low_mask(int n) {
    if (n <= 0 || n > int(sizeof(T) * 8))
        return 0;
    return (T)~T{0} >> uint8_t(sizeof(T) * 8 - n);
}

template <class T>
T mid_mask(int start, int end) {
    return ~(high_mask<T>(start) | low_mask<T>(sizeof(T) * 8 - end));
}

}

template <class BlockType>
class BitView {
    static constexpr size_t block_size = 8 * sizeof(BlockType);

public:
    BitView(BlockType* data, size_t start, size_t end)
      : _data(data)
      , _start(start)
      , _end(end)
    {}

    BitView(std::vector<BlockType>& vec)
      : _data(vec.data())
      , _start(0)
      , _end(block_size * vec.size())
    {}

    explicit BitView(BlockType& val)
      : _data(&val)
      , _start(0)
      , _end(block_size)
    {}

    size_t size() const { return _end - _start; }

    BitView slice(size_t start) const {
        if (start > size())
            throw std::out_of_range("BitView::slice: out of range.");
        return {_data, _start + start, _end};
    }

    BitView slice(size_t start, size_t end) const {
        if (start > end || end > size()) {
            throw std::out_of_range("BitView::slice: out of range.");
        }
        return {_data, _start + start, _start + end};
    }

    template <class T>
    union U {
        T val;
        std::array<std::uint8_t, sizeof(T)> raw;
    };

private:
    template<typename T> struct type_tag { };

    template <class T>
    T get(type_tag<T>, bool big_endian) const {
        U<T> result;
        int block_offset = _start / block_size;
        size_t bit_offset = _start % block_size;
        auto begin = big_endian ? std::end(result.raw) - 1 : std::begin(result.raw);
        const auto end = big_endian ? std::begin(result.raw) - 1 : std::end(result.raw);
        while (begin != end) {
            if (bit_offset + 8 > block_size) {
                int left = block_size - bit_offset;
                int right = 8 - left;
                *begin = (_data[block_offset] & BitViewDetails::low_mask<BlockType>(left)) << right;
                *begin |= (_data[block_offset + 1] & BitViewDetails::high_mask<BlockType>(right)) >> (block_size - right);
                bit_offset += 8 - block_size;
                ++block_offset;
            }
            else {
                *begin = (_data[block_offset] & BitViewDetails::mid_mask<BlockType>(bit_offset, bit_offset + 8)) >> (block_size - 8 - bit_offset);
                bit_offset += 8;
            }
            if (big_endian)
                --begin;
            else
                ++begin;
        }
        if (size() < 8 * sizeof(T)) {
            if (big_endian)
                result.val = result.val >> (8 * sizeof(T) - (_end - _start));
            else
                result.val = result.val & BitViewDetails::low_mask<T>(_end - _start);
        }
        // std::cout << "(" << _start << ", " << _end << "): " << std::bitset<64>(result.val) << std::endl;
        return result.val;
    }

    // specialization for bool
    bool get(type_tag<bool>, bool) const {
        int block_offset = _start / block_size;
        size_t bit_offset = _start % block_size;
        return _data[block_offset] & BitViewDetails::mid_mask<BlockType>(bit_offset, bit_offset + 1);
    }

public:

    template <class T>
    T get(bool big_endian=true) const {
       return get(type_tag<T>{}, big_endian); 
    }

    template <class T>
    void set(const T& value, bool big_endian=true) {
        if (size() == 0)
            return;
        
        int first_block = _start / block_size;
        int last_block = (_end - 1) / block_size;
        
        size_t bit_start = _start % block_size;
        size_t bit_end = (_end - 1) % block_size + 1;

        int n_bits;
        if (first_block == last_block) {
            n_bits = bit_end - bit_start;
        }
        else {
            n_bits = block_size - bit_start;
        }
        _data[first_block] &= ~BitViewDetails::mid_mask<BlockType>(bit_start, bit_start + n_bits);
        BlockType new_value = (value >> (size() - n_bits)) & BitViewDetails::low_mask<T>(n_bits);
        if (bit_start + n_bits < block_size) {
            new_value <<= block_size - (bit_start + n_bits);
        }
        _data[first_block] |= new_value;

        for (int i = first_block + 1; i <= last_block; ++i) {
            if (i < last_block) {
                _data[i] = (value >> (size() - i * block_size - n_bits)) & BitViewDetails::low_mask<T>(block_size);
            }
            else {
                _data[i] &= BitViewDetails::low_mask<BlockType>(block_size - bit_end);
                _data[i] |= (BlockType)(value & BitViewDetails::low_mask<T>(bit_end)) << (block_size - bit_end);
            }
        }
    }


    BitView& operator=(const BitView& bits) {
        int offset = 0;
        while (offset <= bits.size() - 8) {
            uint8_t byte = bits.slice(offset, offset + 8).template get<uint8_t>();
            slice(offset, offset + 8).set(byte);
            offset += 8;
        }
        if (bits.size() > offset) {
            uint8_t byte = bits.slice(offset, bits.size()).template get<uint8_t>();
            slice(offset, bits.size()).set(byte);
        }
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const BitView& bit_view) {
        for (size_t i = 0; i < bit_view.size(); ++i)
            os << bit_view.slice(i, i+1).template get<bool>();
        return os;
    };

private:
    BlockType* _data;
    const size_t _start;
    const size_t _end;
};

template <class T>
BitView<T> bit_view(T* data, size_t start, size_t end) {
    return {data, start, end};
}

template <class T>
BitView<T> bit_view(std::vector<T>& vec) {
    return {vec};
}

template <class T>
BitView<const T> bit_view(const T* data, size_t start, size_t end) {
    return {data, start, end};
}

template <class T>
BitView<const T> bit_view(const std::vector<T>& vec) {
    return {vec};
}



#endif