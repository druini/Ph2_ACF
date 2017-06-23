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
#include "../Utils/easylogging++.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/BeBoard.h"


using namespace Ph2_HwDescription;

namespace Ph2_HwInterface {

    using EventDataMap = std::map<uint16_t, std::vector<uint32_t>>;

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

    class Stub
    {
      public:
        Stub (uint8_t pPosition, uint8_t pBend) : fPosition (pPosition), fBend (pBend)
        {
            //with Strips starting at 0
            fCenter = static_cast<float> ( (pPosition / 2.) - 1);
        }
        uint8_t getPosition()
        {
            return fPosition;
        }
        uint8_t getBend()
        {
            return fBend;
        }
        float getCenter()
        {
            return fCenter;
        }
      private:
        uint8_t fPosition;
        uint8_t fBend;
        float fCenter;
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
      protected:
        //general members for Event Header
        uint32_t fEventCount;           /*!< Event Counter */
        uint32_t fTDC;                  /*!< TDC value*/

        // data map for CBC Data
      public:
        EventDataMap fEventDataMap;

      protected:
        //for CBC2 use
        uint32_t fBunch;                /*!< Bunch value */
        uint32_t fOrbit;                /*!< Orbit value */
        uint32_t fLumi;                 /*!< LuminositySection value */
        uint32_t fEventCountCBC;        /*!< Cbc Event Counter */

        //for CBC3 use
        uint8_t fBeId;
        uint8_t fBeFWType;
        uint8_t fCBCDataType;
        uint8_t fNCbc;
        uint16_t fEventDataSize;
        uint32_t fBeStatus;

        // d19c variables
        uint16_t fTLUTriggerID;
        uint8_t fDummySize;


      public:
        // size of an event
        uint32_t fEventSize;

      protected:

        uint16_t encodeId (const uint8_t& pFeId, const uint8_t& pCbcId) const
        {
            return (pFeId << 8 | pCbcId);
        }

        void decodeId (const uint16_t& pKey, uint8_t& pFeId, uint8_t& pCbcId) const
        {
            pFeId = (pKey >> 8) & 0x00FF;
            pCbcId = pKey & 0xFF;
        }

      public:
        /*!
         * \brief Constructor of the Event Class
         * \param pBoard : Board to work with
         * \param pNbCbc
         * \param pEventBuf : the pointer to the raw Event buffer of this Event
         */
        Event () ;
        /*!
         * \brief Copy Constructor of the Event Class
         */
        Event ( const Event& pEvent );
        /*!
         * \brief Destructor of the Event Class
         */
        virtual ~Event()
        {
        }
        /*!
         * \brief Clear the Event Map
         */
        void Clear()
        {
            fEventDataMap.clear();
        }
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
         * \brief Get TDC value ??
         * \return TDC value
         */
        uint32_t GetTDC() const
        {
            return fTDC;
        }
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
        void GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint8_t >& cbcData )  const;
        /*!
         * \brief Function to get the bit at the global data string position
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param pPosition : Position in the data buffer
         * \return Bit
         */
        bool Bit ( uint8_t pFeId, uint8_t pCbcId, uint32_t pPosition ) const;
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
         * \brief Function to get char at the global data string at position 8*i
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param pBytePosition : Position of the byte
         * \return Char in given position
         */
        unsigned char Char ( uint8_t pFeId, uint8_t pCbcId, uint32_t pBytePosition );

        const EventDataMap& GetEventDataMap() const
        {
            return fEventDataMap;
        }

        bool operator== (const Event& pEvent) const;


        ///////////////////////////////////////
        //VIRTUAL METHODS                    //
        ///////////////////////////////////////
        /*!
         * \brief Set an Event to the Event map
         * \param pEvent : Event to set
         * \return Aknowledgement of the Event setting (1/0)
         */
        virtual void SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list ) = 0;
        /*!
         * \brief Convert Data to Hex string
         * \return Data string in hex
         */
        virtual std::string HexString() const = 0;

        //user interface
        /*!
         * \brief Get the Cbc Event counter
         * \return Cbc Event counter
         */
        virtual uint32_t GetEventCountCBC() const = 0;
        /*!
         * \brief Function to get bit string in hexadecimal format for CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit string in Hex
         */
        virtual std::string DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Function to get bit string of CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit string
         */
        virtual std::string DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Function to get bit vector of CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit vector
         */
        virtual std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Function to get Error bit
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param i : Error bit number i
         * \return Error bit
         */
        virtual bool Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const = 0;
        /*!
         * \brief Function to get all Error bits
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Error bit
         */
        virtual uint32_t Error ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Function to get pipeline address
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Pipeline address
         */
        virtual uint32_t PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Function to get a CBC pixel bit data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param i : pixel bit data number i
         * \return Data Bit
         */
        virtual bool DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const = 0;
        virtual std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const = 0;
        /*!
         * \brief Function to get GLIB flag string
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Glib flag string
         */
        virtual std::string GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Function to get Stub bit
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return stub bit?
         */
        virtual std::string StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
        * \brief Function to get Stub bit
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return stub bit?
        */
        virtual bool StubBit ( uint8_t pFeId, uint8_t pCbcId ) const = 0;
        /*!
         * \brief Get a vector of Stubs - will be empty for Cbc2
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        */
        virtual std::vector<Stub> StubVector (uint8_t pFeId, uint8_t pCbcId ) const = 0;


        /*!
        * \brief Function to count the Hits in this event
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return number of hits
        */
        virtual uint32_t GetNHits (uint8_t pFeId, uint8_t pCbcId) const = 0;
        /*!
        * \brief Function to get a sparsified hit vector
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return vector with hit channels
        */
        virtual std::vector<uint32_t> GetHits (uint8_t pFeId, uint8_t pCbcId) const = 0;


        friend std::ostream& operator<< ( std::ostream& out, const Event& ev )
        {
            ev.print (out);
            return out;
        }

        virtual std::vector<Cluster> getClusters ( uint8_t pFeId, uint8_t pCbcId) const = 0;

      protected:
        virtual void print (std::ostream& out) const = 0;

    };
}
#endif

