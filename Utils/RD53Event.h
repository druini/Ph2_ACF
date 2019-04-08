/*!
  \file                  RD53Event.h
  \brief                 Event handling from DAQ
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53Event_h_
#define _RD53Event_h_

#include "Event.h"


using namespace Ph2_HwDescription;

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

namespace Ph2_HwInterface
{
  /*!
   * \class Cbc3Event
   * \brief Event container to manipulate event flux from the Cbc2
   */
  class RD53Event : public Event
  {
  public:
    /*!
     * \brief Constructor of the Event Class
     * \param pBoard : Board to work with
     * \param pNbCbc
     * \param pEventBuf : the pointer to the raw Event buffer of this Event
     */
    RD53Event ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list );
    /*!
     * \brief Copy Constructor of the Event Class
     */
    //Cbc3Event ( const Event& pEvent );
    /*!
     * \brief Destructor of the Event Class
     */
    ~RD53Event()
      {
      }
    /*!
     * \brief Set an Event to the Event map
     * \param pEvent : Event to set
     * \return Aknowledgement of the Event setting (1/0)
     */
    void SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list ) override;

    /*!
     * \brief Get the Cbc Event counter
     * \return Cbc Event counter
     */
    uint32_t GetEventCountCBC() const override
    {
      return fEventCountCBC;
    }

    //private members of cbc3 events only
    uint32_t GetBeId() const
    {
      return fBeId;
    }
    uint8_t GetFWType() const
    {
      return fBeFWType;
    }
    uint32_t GetCbcDataType() const
    {
      return fCBCDataType;
    }
    uint32_t GetNCbc() const
    {
      return fNCbc;
    }
    uint32_t GetEventDataSize() const
    {
      return fEventDataSize;
    }
    uint32_t GetBeStatus() const
    {
      return fBeStatus;
    }
    /*!
     * \brief Convert Data to Hex string
     * \return Data string in hex
     */
    std::string HexString() const override;
    /*!
     * \brief Function to get bit string in hexadecimal format for CBC data
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return Data Bit string in Hex
     */
    std::string DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const override;

    /*!
     * \brief Function to get Error bit
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \param i : Error bit number i
     * \return Error bit
     */
    bool Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const override;
    /*!
     * \brief Function to get all Error bits
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return Error bit
     */
    uint32_t Error ( uint8_t pFeId, uint8_t pCbcId ) const override;
    /*!
     * \brief Function to get pipeline address
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return Pipeline address
     */
    uint32_t PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const override;
    /*!
     * \brief Function to get a CBC pixel bit data
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \param i : pixel bit data number i
     * \return Data Bit
     */
    bool DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const override;
    /*!
     * \brief Function to get bit string of CBC data
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return Data Bit string
     */
    std::string DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const override;
    /*!
     * \brief Function to get bit vector of CBC data
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return Data Bit vector
     */
    std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const override;
    std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const override;
    /*!
     * \brief Function to get GLIB flag string
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return Glib flag string
     */
    std::string GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const override;
    /*!
     * \brief Function to get Stub bit
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return stub bit?
     */
    std::string StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const override;
    /*!
     * \brief Function to get Stub bit
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return stub bit?
     */
    bool StubBit ( uint8_t pFeId, uint8_t pCbcId ) const override;
    /*!
     * \brief Get a vector of Stubs - will be empty for Cbc2
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     */
    std::vector<Stub> StubVector (uint8_t pFeId, uint8_t pCbcId ) const override;

    /*!
     * \brief Function to count the Hits in this event
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return number of hits
     */
    uint32_t GetNHits (uint8_t pFeId, uint8_t pCbcId) const override;
    /*!
     * \brief Function to get a sparsified hit vector
     * \param pFeId : FE Id
     * \param pCbcId : Cbc Id
     * \return vector with hit channels
     */
    std::vector<uint32_t> GetHits (uint8_t pFeId, uint8_t pCbcId) const override;

    std::vector<Cluster> getClusters ( uint8_t pFeId, uint8_t pCbcId) const override;

    void print (std::ostream& out) const override;

  private:
    uint32_t reverse_bits ( uint32_t n) const
    {
      n = ( (n >> 1) & 0x55555555) | ( (n << 1) & 0xaaaaaaaa) ;
      n = ( (n >> 2) & 0x33333333) | ( (n << 2) & 0xcccccccc) ;
      n = ( (n >> 4) & 0x0f0f0f0f) | ( (n << 4) & 0xf0f0f0f0) ;
      n = ( (n >> 8) & 0x00ff00ff) | ( (n << 8) & 0xff00ff00) ;
      n = ( (n >> 16) & 0x0000ffff) | ( (n << 16) & 0xffff0000) ;
      return n;
    }
    void calculate_address (uint32_t& cWordP, uint32_t& cBitP, uint32_t i) const
    {
      // the first 3 words contain header data
      cWordP = 3 + (i-i%32)/32;
      cBitP = 31 - i%32;
    }

    void printCbcHeader (std::ostream& os, uint8_t pFeId, uint8_t pCbcId) const;

    SLinkEvent GetSLinkEvent ( BeBoard* pBoard) const override;
  };
}

#endif
