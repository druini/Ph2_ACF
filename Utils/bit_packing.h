#include <tuple>

namespace Ph2_HwDescription
{
  namespace detail {
    // general case
    template <size_t CurrentSize, size_t... Sizes>
      struct Fields
      {
        template <size_t CurrentOffset, class T>
	  static auto unpack(const T& data) {
	  return std::tuple_cat(std::make_tuple((data >> (CurrentOffset - CurrentSize)) & ((1 << CurrentSize) - 1)), Fields<Sizes...>::template unpack<CurrentOffset - CurrentSize>(data));
        }

        template <size_t CurrentOffset, class First, class... Args>
	  static auto pack(First first, Args... args) {
	  return (first & ((1 << CurrentSize) - 1)) << (CurrentOffset - CurrentSize) | Fields<Sizes...>::template pack<CurrentOffset - CurrentSize>(args...);
        }
      };

    // last field
    template <size_t LastSize>
      struct Fields<LastSize> 
      {
        template <size_t CurrentOffset, class T>
	  static auto unpack(const T& data) {
	  return std::make_tuple((data >> (CurrentOffset - LastSize)) & ((1 << LastSize) - 1));
        }

        template <size_t CurrentOffset, class Last>
	  static auto pack(Last last) {
	  return (last & ((1 << LastSize) - 1));
        }
      };


    // sum template parameter pack
    template<size_t size, size_t... sizes>
      struct size_sum
      {
        static const size_t value = size + size_sum<sizes...>::value;
      };

    template<size_t size>
      struct size_sum<size>
      {
        static const size_t value = size;
      };


    template <size_t I>
      struct unpack_array_helper {
        template <size_t NBits, class U, class T, size_t N>
	  static void apply(T (&arr)[N], const U& u) {
	  arr[I] =  (u >> ((N - I - 1) * NBits)) & ((1 << NBits) - 1);
	  unpack_array_helper<I - 1>::template apply<NBits>(arr, u);
        }
      };

    template <>
      struct unpack_array_helper<0> {
      template <size_t NBits, class U, class T, size_t N>
        static void apply(T (&arr)[N], const U& u) {
	arr[0] = (u >> ((N-1) * NBits)) & ((1 << NBits) - 1);
      }
    };
  }

  // upacks a value into a tuple of fields
  template <size_t... Sizes, class T>
    auto unpack_bits(const T& data) {
    return detail::Fields<Sizes...>::template unpack<detail::size_sum<Sizes...>::value>(data);
  }

  // packs multiple fields into a single value
  template <size_t... Sizes, class... Args>
    auto pack_bits(Args... args) {
    return detail::Fields<Sizes...>::template pack<detail::size_sum<Sizes...>::value>(args...);
  }

  // unpack value into fields of size NBITS and store them in array
  template <size_t NBits, class U, class T, size_t N>
    void unpack_array(T (&array)[N], const U& value) {
    detail::unpack_array_helper<N-1>::template apply<NBits>(array, value);
  }
}