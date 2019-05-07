/*

    \file                          Data.h
    \brief                         Data handling from DAQ
    \author                        Nicolas PIERRE
    \version                       1.0
    \date                          10/07/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#ifndef __DATA_H__
#define __DATA_H__

//#include <uhal/uhal.hpp>
#include <memory>
#include <future>
#include <ios>
#include <istream>
#include "../Utils/Event.h"
#include "../Utils/RD53Event.h"
#include "../Utils/D19cCbc3Event.h"
#include "../Utils/D19cCbc3EventZS.h"
#include "../Utils/easylogging++.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Definition.h"


using namespace Ph2_HwDescription;
namespace Ph2_HwInterface {

    /*!
     * \class Data
     * \brief Data buffer class for CBC data
     */
    class Data
    {
      private:
        uint32_t fNevents;              /*! Number of Events<*/
        uint32_t fCurrentEvent;         /*! Current EventNumber in use <*/
        uint32_t fNCbc;                 /*! Number of CBCs in the setup <*/
        uint32_t fEventSize;            /*! Size of 1 Event <*/

        //to look up if an index is first or last word valid for up to 8 CBCs per AMC
        const std::set<uint32_t> fChannelFirstRows {5, 14, 23, 32, 41, 50, 59, 68};
        const std::set<uint32_t> fChannelLastRows {13, 22, 31, 40, 49, 58, 67, 76};

        std::vector<Event*> fEventList;
        std::future<void> fFuture;

      private:

        uint32_t swap_bytes ( uint32_t& n)
        {
            return __builtin_bswap32 (n);
        }

        uint32_t reverse_bits ( uint32_t& n)
        {
            n = ( (n >> 1) & 0x55555555) | ( (n << 1) & 0xaaaaaaaa) ;
            n = ( (n >> 2) & 0x33333333) | ( (n << 2) & 0xcccccccc) ;
            n = ( (n >> 4) & 0x0f0f0f0f) | ( (n << 4) & 0xf0f0f0f0) ;
            n = ( (n >> 8) & 0x00ff00ff) | ( (n << 8) & 0xff00ff00) ;
            n = ( (n >> 16) & 0x0000ffff) | ( (n << 16) & 0xffff0000) ;
            return n;
        }

        bool is_channel_data (uint32_t pIndex)
        {
            // return true if the word is channel data and not the first or last row of a CBC block, false if not!
            if (pIndex > 4 && pIndex < (EVENT_HEADER_SIZE_32 + CBC_EVENT_SIZE_32 * fNCbc) ) return true;
            else return false;
        }

        bool is_channel_first_row (uint32_t pIndex)
        {
            //return true if it is the first word of any CBC block containing channel data
            return (fChannelFirstRows.find (pIndex) != std::end (fChannelFirstRows) && pIndex < fEventSize - 1);
        }

        bool is_channel_last_row (uint32_t pIndex)
        {
            //return true if it is the last word of any CBC block containing channel data
            return fChannelLastRows.find (pIndex) != std::end (fChannelLastRows);
        }

        //private methods to be used in set according to the BoardType enum
        void setIC (uint32_t& pWord, uint32_t pWordIndex, uint32_t pSwapIndex);
        void setCbc3Fc7 (uint32_t& pWord);
        void setStrasbourgSupervisor (uint32_t& pWord);

      public:
        /*!
         * \brief Constructor of the Data class
         * \param pNbCbc
         */
        Data( ) :  fCurrentEvent ( 0 ), fEventSize ( 0 )
        {
        }
        /*!
         * \brief Copy Constructor of the Data class
         */
        Data ( const Data& pData );
        /*!
         * \brief Destructor of the Data class
         */
        ~Data()
        {
            Reset();
        }
        /*!
         * \brief Set the data in the data map
         * \param *pBoard : pointer to Boat
         * \param *pData : Data from the Cbc
         * \param pNevents : The number of events in this acquisiton
         * \param pType : the board type according to the Enum defined in Definitions.h
         */
        void Set ( const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType);
        void privateSet ( const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType);

        /*!
         * \brief Reset the data structure
         */
        void Reset();
        /*!
         * \brief Get the next Event
         * \param pBoard: pointer to BeBoard
         * \return Next Event
         */
        // cannot be const as fCurrentEvent is incremented
        const Event* GetNextEvent ( const BeBoard* pBoard )
        {
            //fFuture.wait();
            fFuture.get();
            return ( ( fCurrentEvent >= fEventList.size() ) ? nullptr : fEventList.at ( fCurrentEvent++ ) );
        }
        const Event* GetEvent ( const BeBoard* pBoard, int i )
        {
            //fFuture.wait();
            fFuture.get();
            return ( ( i >= (int) fEventList.size() ) ? nullptr : fEventList.at ( i ) );
        }
        const std::vector<Event*>& GetEvents ( const BeBoard* pBoard )
        {
            //fFuture.wait();
            fFuture.get();
            return fEventList;
        }
    };

}
#endif
