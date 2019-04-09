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

#include "../HWDescription/RD53.h"


using namespace Ph2_HwDescription;


namespace Ph2_HwInterface
{
  /*!
   * \class Cbc3Event
   * \brief Event container to manipulate event flux from the Cbc2
   */
  class RD53Event : public Event
  {
  public:

    // copy events into chip_events
    RD53Event(const std::vector<size_t>& chip_id, const std::vector<RD53::Event>& events)
      : chip_id_vec(chip_id)
      , chip_events(events)
    {}

    // move events into chip_events
    RD53Event(const std::vector<size_t>& chip_id, std::vector<RD53::Event>&& events)
      : chip_id_vec(chip_id)
      , chip_events(events)
    {}

    bool DataBit ( uint8_t /*module_id*/, uint8_t chip_id, uint32_t channel_id ) const {
      for (size_t i = 0; i < chip_events.size(); i++) {
        if (chip_id == chip_id_vec[i]) {
           for (const auto& hit : chip_events[i].data) {
              if ((hit.row * NROWS + hit.col) / 4 == channel_id / 4 && hit.tots[channel_id % 4]) {
                return true;
              } 
           }
        }
      }
      return false;
    }

  private:
    std::vector<RD53::Event> chip_events;
    std::vector<size_t> chip_id_vec;
    
  public:

    ///////////////////////////////////////
        //VIRTUAL METHODS                    //
        ///////////////////////////////////////
        /*!
         * \brief Set an Event to the Event map
         * \param pEvent : Event to set
         * \return Aknowledgement of the Event setting (1/0)
         */
        virtual void SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list ) {}
        /*!
         * \brief Convert Data to Hex string
         * \return Data string in hex
         */
        virtual std::string HexString() const {}

        //user interface
        /*!
         * \brief Get the Cbc Event counter
         * \return Cbc Event counter
         */
        virtual uint32_t GetEventCountCBC() const {}
        /*!
         * \brief Function to get bit string in hexadecimal format for CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit string in Hex
         */
        virtual std::string DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Function to get bit string of CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit string
         */
        virtual std::string DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Function to get bit vector of CBC data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Data Bit vector
         */
        virtual std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Function to get Error bit
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param i : Error bit number i
         * \return Error bit
         */
        virtual bool Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const {}
        /*!
         * \brief Function to get all Error bits
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Error bit
         */
        virtual uint32_t Error ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Function to get pipeline address
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Pipeline address
         */
        virtual uint32_t PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Function to get a CBC pixel bit data
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \param i : pixel bit data number i
         * \return Data Bit
         */
        // virtual bool DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const {}
        virtual std::vector<bool> DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const {}
        /*!
         * \brief Function to get GLIB flag string
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return Glib flag string
         */
        virtual std::string GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Function to get Stub bit
         * \param pFeId : FE Id
         * \param pCbcId : Cbc Id
         * \return stub bit?
         */
        virtual std::string StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
        * \brief Function to get Stub bit
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return stub bit?
        */
        virtual bool StubBit ( uint8_t pFeId, uint8_t pCbcId ) const {}
        /*!
         * \brief Get a vector of Stubs - will be empty for Cbc2
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        */
        virtual std::vector<Stub> StubVector (uint8_t pFeId, uint8_t pCbcId ) const {}


        /*!
        * \brief Function to count the Hits in this event
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return number of hits
        */
        virtual uint32_t GetNHits (uint8_t pFeId, uint8_t pCbcId) const {}
        /*!
        * \brief Function to get a sparsified hit vector
        * \param pFeId : FE Id
        * \param pCbcId : Cbc Id
        * \return vector with hit channels
        */
        virtual std::vector<uint32_t> GetHits (uint8_t pFeId, uint8_t pCbcId) const {}
        /*!
        * \brief Function to get an encoded SLinkEvent object
        * \param pBoard : pointer to BeBoard
        * \param pSet : set of condition data parsed from config file
        * \return SlinkEvent object
        */
        virtual SLinkEvent GetSLinkEvent (  BeBoard* pBoard) const {}

        virtual std::vector<Cluster> getClusters ( uint8_t pFeId, uint8_t pCbcId) const {}


      protected:
        virtual void print (std::ostream& out) const {};

  };
}

#endif
