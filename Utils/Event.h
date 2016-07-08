/*

        \file                          Event.h
        \brief                         Event handling from DAQ
        \author                        Nicolas PIERRE
        \version                       1.0
        \date                                  10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include <string>
#include <bitset>
#include <sstream>
#include <cstring>
#include <iomanip>
#include "ConsoleColor.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/BeBoard.h"


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface {

    using EventDataMap = std::map<uint32_t, std::bitset<CBC_EVENT_SIZE_32 * 32> >;

    /*!
     * \class Cluster
     * \brief Cluster object for the Event
     */
    class Cluster
    {
      public:
        uint8_t fSensor;
        uint8_t fFirstStrip;
        uint8_t fClusterWidth;
        double getBaricentre();
    };


    /*!
     * \class Event
     * \brief Event container to manipulate event flux from the Cbc
     */
    class Event
    {

        /*
           id of FeEvent should be the order of FeEvents in data stream starting from 0
           id of CbcEvent also should be the order of CBCEvents in data stream starting from 0
         */
      private:
        uint32_t fBunch;                /*!< Bunch value */
        uint32_t fOrbit;                /*!< Orbit value */
        uint32_t fLumi;                 /*!< LuminositySection value */
        uint32_t fEventCount;           /*!< Event Counter */
        uint32_t fEventCountCBC;        /*!< Cbc Event Counter */
        uint32_t fTDC;                  /*!< TDC value*/
        //to mask out all non channel data
        std::bitset<CBC_EVENT_SIZE_32 * 32> fChannelMask;


      public:
        uint32_t fEventSize;                     /*!< Size of an Event */
        EventDataMap fEventDataMap;

      private:
        /*!
         * \brief: method to add the contents of a 32 bit word at in dex cBitsetIndex in a bitset
         * \param pCbcSet: bitset to hold CBC data
         * \parm pWord: 32 bit word
         * \param cBitsetInidex: index at which to insert cWord
         * \param cWordIndex: needed for recursive call, defaults to 0 if not explicitly set
         */
        void setBitsetValue (std::bitset<CBC_EVENT_SIZE_32 * 32>& pCbcSet, uint32_t pWord, size_t pBitsetIndex, size_t pWordIndex = 0)
        {
            if ( pWordIndex < pCbcSet.size() )
            {
                pCbcSet[pBitsetIndex] = (pWord >> (31 - pWordIndex) ) & 0x1;
                setBitsetValue (pCbcSet, pWord, pBitsetIndex + 1, pWordIndex + 1);
            }
        }

        uint32_t subset (const std::bitset<CBC_EVENT_SIZE_32 * 32>& pSet, uint32_t pPosition, uint32_t pWidth, uint32_t pMask = 0xFFFFFFFF) const
        {
            uint32_t cMask = 1;
            uint32_t cResult = 0;

            for (size_t cBit = pPosition; cBit < pPosition + pWidth; ++cBit)
            {
                if (pSet.test (cBit) )
                    cResult |= cMask;

                cMask <<= 1;
            }

            return (cResult & pMask);
        }

        uint32_t encodeId (/* const uint8_t& pBeI, */
            const uint8_t& pFeId, const uint8_t& pCbcId) const
        {
            return (/*pBeId << 24 |*/ pFeId << 16 | pCbcId );
        }

        void decodeId (const uint32_t& pKey, uint8_t& pFeId, uint8_t& pCbcId) const
        {
            pFeId = (pKey >> 16) & 0x000000FF;
            pCbcId = pKey & 0x000000FF;
        }

      public:
        /*!
         * \brief Constructor of the Event Class
         * \param pBoard : Board to work with
         * \param pNbCbc
         * \param pEventBuf : the pointer to the raw Event buffer of this Event
         */
        Event ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list );
        /*!
         * \brief Copy Constructor of the Event Class
         */
        Event ( const Event& pEvent );
        /*!
         * \brief Destructor of the Event Class
         */
        ~Event()
        {
        }
        /*!
         * \brief Clear the Event Map
         */
        void Clear()
        {
            fEventDataMap.clear();
        }
        /*!
         * \brief Set an Event to the Event map
         * \param pEvent : Event to set
         * \return Aknowledgement of the Event setting (1/0)
         */
        void SetEvent ( const BeBoard* pBoard, uint32_t pNbCb, const std::vector<uint32_t>& pList );

        /*! \brief Get raw data */
        //const std::vector<uint32_t>& GetEventData() const
        //{
        //return fEventData;
        //}

        /*! \brief Get the event size in bytes */
        uint32_t GetSize() const
        {
            return fEventSize;
        }

        //user interface
        /*!
         * \brief Get an event contained in a Cbc
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Event buffer
         */
        void GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint32_t >& cbcData ) const;
        /*!
         * \brief Get an event contained in a Cbc
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Event buffer
         */
        void GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint8_t >& cbcData ) const;
        /*!
         * \brief Get the bunch value
         * \return Bunch value
         */
        uint32_t GetBunch() const
        {
            return fBunch;
        }
        /*!
         * \brief Get the orbit value
         * \return Orbit value
         */
        uint32_t GetOrbit() const
        {
            return fOrbit;
        }
        /*!
         * \brief Get the luminence value
         * \return Luminence value
         */
        uint32_t GetLumi() const
        {
            return fLumi;
        }
        /*!
         * \brief Get the Event counter
         * \return Event counter
         */
        uint32_t GetEventCount() const
        {
            return fEventCount;
        }
        /*!
         * \brief Get the Cbc Event counter
         * \return Cbc Event counter
         */
        uint32_t GetEventCountCBC() const
        {
            return fEventCountCBC;
        }
        /*!
         * \brief Get TDC value ??
         * \return TDC value
         */
        uint32_t GetTDC() const
        {
            return fTDC;
        }

        /*!
         * \brief Function to get the bit at the global data string position
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param pPosition : Position in the data buffer
         * \return Bit
         */
        bool Bit ( uint8_t pFeId, uint8_t pCbcId, uint32_t pPosition ) const;
        /*!
         * \brief Function to get Error bit
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param i : Error bit number i
         * \return Error bit
         */
        bool Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const;
        /*!
         * \brief Function to get all Error bits
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Error bit
         */
        uint32_t Error ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
         * \brief Function to get pipeline address
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Pipeline address
         */
        uint32_t PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
         * \brief Function to get a CBC pixel bit data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param i : pixel bit data number i
         * \return Data Bit
         */
        bool DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const;
        /*!
         * \brief Function to get bit string from the data offset and width
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param pOffset : position Offset
         * \param pWidth : string width
         * \return Bit string
         */
        std::string BitString ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const;
        /*!
         * \brief Function to get bit vector from the data offset and width
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param pOffset : position Offset
         * \param pWidth : string width
         * \return Boolean/Bit vector
         */
        std::vector<bool> BitVector ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const;
        /*!
         * \brief Function to get bit string of CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit string
         */
        std::string DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
         * \brief Function to get bit vector of CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit vector
         */
        std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const;
        std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const;
        /*!
         * \brief Function to get bit string in hexadecimal format for CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit string in Hex
         */
        std::string DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
         * \brief Function to get GLIB flag string
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Glib flag string
         */
        std::string GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
         * \brief Function to get Stub bit
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return stub bit?
         */
        std::string StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
        * \brief Function to get Stub bit
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return stub bit?
        */
        bool StubBit ( uint8_t pFeId, uint8_t pCbcId ) const;
        /*!
        * \brief Function to count the number of hits
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return Number of Hits
        */
        uint32_t GetNHits (uint8_t pFeId, uint8_t pCbcId) const;
        /*!
        * \brief Function to get a sparsified vector of hits
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return vector of hit channels
        */
        std::vector<uint8_t> GetHits (uint8_t pFeId, uint8_t pCbcId) const;
        /*!
         * \brief Function to get char at the global data string at position 8*i
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param pBytePosition : Position of the byte
         * \return Char in given position
         */
        unsigned char Char ( uint8_t pFeId, uint8_t pCbcId, uint32_t pBytePosition );

        //const EventMap& GetEventMap() const
        //{
        //return fEventMap;
        //}
        friend std::ostream& operator<< ( std::ostream& out, const Event& ev );

        std::vector<Cluster> getClusters ( uint8_t pFeId, uint8_t pCbcId);

    };
    //const std::bitset<CBC_EVENT_SIZE_32 * 32> Event::fChannelMask  ( "000000000000000000000000111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110000000000" );
}
#endif
