
#ifndef __D19cSSAEvent_H__
#define __D19cSSAEvent_H__

#include "Event.h"

namespace Ph2_HwInterface
{
    /*!
     * \class SSAEvent
     */
    class D19cSSAEvent : public Event
    {
      public:
        /*!
         * \brief Constructor of the Event Class
         * \param pBoard : Board to work with
         * \param pNbSSA
         * \param pEventBuf : the pointer to the raw Event buffer of this Event
         */
        D19cSSAEvent ( const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNbSSA, const std::vector<uint32_t>& list );
        /*!
         * \brief Copy Constructor of the Event Class
         */
        //SSAEvent ( const Event& pEvent );
        /*!
         * \brief Destructor of the Event Class
         */
        ~D19cSSAEvent()
        {
        }
        /*!
         * \brief Set an Event to the Event map
         * \param pEvent : Event to set
         * \return Aknowledgement of the Event setting (1/0)
         */
        void SetEvent ( const Ph2_HwDescription::BeBoard* pBoard, uint32_t pNbSSA, const std::vector<uint32_t>& list ) override;

        /*!
         * \brief Get the SSA Event counter
         * \return SSA Event counter
         */
        uint32_t GetEventCountCBC() const override
        {
            return fEventCount;
        }

        //private members of SSA events only
        uint32_t GetBeId() const
        {
            return fBeId;
        }
        uint8_t GetFWType() const
        {
            return fBeFWType;
        }
        uint32_t GetCBCDataType() const
        {
            return fCBCDataType;
        }
        uint32_t GetNC() const
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
         * \brief Function to get bit string in hexadecimal format for SSA data
         * \param pFeId : FE Id
         * \param pSSAId : SSA Id
         * \return Data Bit string in Hex
         */
        std::string DataHexString ( uint8_t pFeId, uint8_t pSSAId ) const override;

        /*!
         * \brief Function to get Error bit
         * \param pFeId : FE Id
         * \param pSSAId : Cbc Id
         * \param i : Error bit number i
         * \return Error bit
         */
        bool Error ( uint8_t pFeId, uint8_t pSSAId, uint32_t i ) const override;
        /*!
         * \brief Function to get all Error bits
         * \param pFeId : FE Id
         * \param pSSAId : SSA Id
         * \return Error bit
         */
        uint32_t Error ( uint8_t pFeId, uint8_t pSSAId ) const override;

        /*!
        * \brief Function to count the Hits in this event
        * \param pFeId : FE Id
        * \param pSSAId : SSA Id
        * \return number of hits
        */
        uint32_t GetNHits (uint8_t pFeId, uint8_t pSSAId) const override;
        /*!
        * \brief Function to get a sparsified hit vector
        * \param pFeId : FE Id
        * \param pSSAId : SSA Id
        * \return vector with hit channels
        */
        std::vector<uint32_t> GetHits (uint8_t pFeId, uint8_t pSSAId) const override;

        std::vector<Cluster> getClusters ( uint8_t pFeId, uint8_t pSSAId) const override;
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



        uint32_t GetNStripClusters( uint8_t pFeId, uint8_t pSSAId ) const;

        std::vector<SCluster> GetStripClusters ( uint8_t pFeId, uint8_t pSSAId) const;

        uint32_t GetNPixelClusters( uint8_t pFeId, uint8_t pSSAId ) const;

        std::vector<PCluster> GetPixelClusters ( uint8_t pFeId, uint8_t pSSAId) const;

        uint32_t GetSync1( uint8_t pFeId, uint8_t pSSAId) const;

        uint32_t GetSync2( uint8_t pFeId, uint8_t pSSAId) const;

        uint32_t GetBX1_NStubs( uint8_t pFeId, uint8_t pSSAId) const;

        uint32_t DivideBy2RoundUp(uint32_t value) const;

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


        //Not sure what most of these do -- probably dont work
        void calculate_address (uint32_t& cWordP, uint32_t& cBitP, uint32_t i) const
        {
            // we have odd and even channels, so let's first define the oddness.
            if ( i % 2 == 1 )
            {
                // the word with channel 1 (from zero)
                cWordP = 7;
            }
            else
            {
                // the word with channel 0 (from zero)
                cWordP = 3;
            }

            // then let's find real position and word
            if (i <= 61)
            {
                cWordP = cWordP;
                cBitP = (int) (i / 2);
            }
            else if (i >= 62 && i <= 125)
            {
                cWordP = cWordP - 1;
                cBitP = (int) ( (i - 62) / 2);
            }
            else if (i >= 126 && i <= 189)
            {
                cWordP = cWordP - 2;
                cBitP = (int) ( (i - 62) / 2);
            }
            else if (i >= 190)
            {
                cWordP = cWordP - 3;
                cBitP = (int) ( (i - 190) / 2);
            }
        }

        void fillDataContainer(BoardDataContainer* boardContainer, const ChannelGroupBase *cTestChannelGroup) override
        {
            std::cout<< __PRETTY_FUNCTION__ << " YOU NEED TO IMPLEMENT ME!!!!";
            abort();
        }

        void printSSAHeader (std::ostream& os, uint8_t pFeId, uint8_t pSSAId) const;

        SLinkEvent GetSLinkEvent ( Ph2_HwDescription::BeBoard* pBoard) const override;
    };
}
#endif
