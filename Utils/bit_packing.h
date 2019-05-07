/*!
  \file                  bit_packing.h
  \brief                 Tools to pack and unpack sequence of bits
  \author                Alkiviadis Papadopoulos
  \version               1.0
  \date                  01/04/19
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#pragma once

#include <cstdint>
#include <tuple>

namespace detail
{
  // needed for c++11
  // template <class T, size_t I>
  // struct Identity {
  //     using type = T;
  // };
  
  // sum template parameter pack
  template<size_t size, size_t... sizes>
    struct size_sum
    {
      static constexpr size_t value = size + size_sum<sizes...>::value;
    };
  
  template<size_t size>
    struct size_sum<size>
    {
      static constexpr size_t value = size;
    };
  
  template <size_t Size, size_t I>
    struct ArrayPackerImpl
    {
      template <class U, class T, size_t N>
        static void unpack(std::array<T, N>& array, const U& u)
      {
	array[I] =  (u >> ((N - I - 1) * Size)) & ((1 << Size) - 1);
	ArrayPackerImpl<Size, I - 1>::unpack(array, u);
      }
      
      template <class R = uint64_t, class T, size_t N>
        static R pack(const std::array<T, N>& array)
      {
	return (array[I] & ((1 << Size) - 1)) << ((N - 1 - I) * Size) | ArrayPackerImpl<Size, I-1>::pack(array);
      }
	
        template <size_t N, class R = uint64_t, class It>
        static R pack(It begin)
      {
	return (begin[I] & ((1 << Size) - 1)) << ((N - 1 - I) * Size) | ArrayPackerImpl<Size, I-1>::template pack<N>(begin);
      }
	  
	  template <class R = uint64_t, class T, size_t N>
        static R pack_reverse(const std::array<T, N>& array)
      {
	return (array[N - 1 - I] & ((1 << Size) - 1)) << ((N - 1 - I) * Size) | ArrayPackerImpl<Size, I-1>::pack_reverse(array);
      }
	    
	    template <size_t N, class R = uint64_t, class It>
        static R pack_reverse(It begin)
      {
	return (begin[N - 1 - I] & ((1 << Size) - 1)) << ((N - 1 - I) * Size) | ArrayPackerImpl<Size, I-1>::template pack_reverse<N>(begin);
      }
    };
  
  template <size_t Size>
    struct ArrayPackerImpl<Size, 0>
    {
      template <class U, class T, size_t N>
	static void unpack(std::array<T, N>& array, const U& u)
      {
	array[0] = (u >> ((N-1) * Size)) & ((1 << Size) - 1);
      }
      
      template <class R = uint64_t, class T, size_t N>
	static R pack(const std::array<T, N>& array)
	{
	  return (array[0] & ((1 << Size) - 1)) << ((N - 1) * Size);
	}
    
      template <size_t N, class R = uint64_t, class It>
	static R pack(It begin)
	{
	  return (begin[0] & ((1 << Size) - 1)) << ((N - 1) * Size);
	}
    
      template <class R = uint64_t, class T, size_t N>
	static R pack_reverse(const std::array<T, N>& array)
	{
	  return (array[N-1] & ((1 << Size) - 1)) << ((N - 1) * Size);
	}
    
      template <size_t N, class R = uint64_t, class It>
	static R pack_reverse(It begin)
	{
	  return (begin[N-1] & ((1 << Size) - 1)) << ((N - 1) * Size);
	}
    };
}

// general case
template <size_t CurrentSize, size_t... Sizes>
  struct BitPacker
  {
    template <size_t CurrentOffset = CurrentSize + detail::size_sum<Sizes...>::value, class T>
      static auto unpack(const T& value) 
      // -> std::tuple<T, typename detail::Identity<T, Sizes>::type...>
    {
      return std::tuple_cat(std::make_tuple((value >> (CurrentOffset - CurrentSize)) & ((1 << CurrentSize) - 1)), BitPacker<Sizes...>::template unpack<CurrentOffset - CurrentSize>(value));
    }
      
      template <size_t CurrentOffset = CurrentSize + detail::size_sum<Sizes...>::value, class T = uint64_t, class First, class... Args>
      static T pack(First first, Args... args)
    {
      return (first & ((1 << CurrentSize) - 1)) << (CurrentOffset - CurrentSize) | BitPacker<Sizes...>::template pack<CurrentOffset - CurrentSize>(args...);
    }
  };

// last field
template <size_t LastSize>
struct BitPacker<LastSize> 
{
  template <size_t CurrentOffset = LastSize, class T>
    static auto unpack(const T& value) 
  // -> std::tuple<T>
    {
      return std::make_tuple((value >> (CurrentOffset - LastSize)) & ((1 << LastSize) - 1));
    }
  
  template <size_t CurrentOffset = LastSize, class T = uint64_t, class Last>
    static T pack(Last last)
    {
      return (last & ((1 << LastSize) - 1));
    }
};

// upacks a value into a tuple
template <size_t... Sizes, class T>
  auto unpack_bits(const T& data)
  // -> std::tuple<typename detail::Identity<T, Sizes>::type...>
{
  return BitPacker<Sizes...>::unpack(data);
}

// packs multiple variables into a single value
template <size_t... Sizes, class... Args>
  uint64_t pack_bits(Args... args)
{
  return BitPacker<Sizes...>::pack(args...);
}


template <size_t Size>
struct ArrayPacker
{
  template <class U, class T, size_t N>
    static void unpack(std::array<T, N>& array, const U& value)
  {
    detail::ArrayPackerImpl<Size, N-1>::unpack(array, value);
  }
  
  template <class R = uint64_t, class T, size_t N>
    static R pack(const std::array<T, N>& array)
  {
    return detail::ArrayPackerImpl<Size, N-1>::pack(array);
  }
    
  // pack a range of specified length N
    template <size_t N, class R = uint64_t, class It>
    static R pack(It begin)
  {
    return detail::ArrayPackerImpl<Size, N-1>::template pack<N>(begin);
  }
      
      template <class R = uint64_t, class T, size_t N>
    static R pack_reverse(const std::array<T, N>& array)
  {
    return detail::ArrayPackerImpl<Size, N-1>::pack_reverse(array);
  }
	
      // pack a range of specified length N
	template <size_t N, class R = uint64_t, class It>
    static R pack_reverse(It begin)
  {
    return detail::ArrayPackerImpl<Size, N-1>::template pack_reverse<N>(begin);
  }
};

// unpack value into fields of size NBITS and store them in array
template <size_t NBits, class U, class T, size_t N>
  void unpack_array(std::array<T, N>& array, const U& value)
{
  ArrayPacker<NBits>::unpack(array, value);
}
