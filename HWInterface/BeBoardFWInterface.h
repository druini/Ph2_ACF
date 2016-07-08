/*!
        \file                BeBoardFWInterface.h
        \brief                           BeBoardFWInterface base class of all type of boards
        \author                          Lorenzo BIDEGAIN, Nicolas PIERRE
        \version             1.0
        \date                            28/07/14
        Support :                        mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __BEBOARDFWINTERFACE_H__
#define __BEBOARDFWINTERFACE_H__

#include <boost/thread.hpp>
#include <uhal/uhal.hpp>
#include "RegManager.h"
#include "../Utils/Event.h"
#include "../Utils/FileHandler.h"
#include "../Utils/Data.h"
#include "../Utils/Utilities.h"
#include "../Utils/Exception.h"
#include "../Utils/FileHandler.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/CbcRegItem.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

using namespace Ph2_HwDescription;

enum class BoardType {GLIB, ICGLIB, CTA, ICFC7};

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
class FileHandler;

namespace Ph2_HwInterface {
    class FpgaConfig;

    /*!
     * \class BeBoardFWInterface
     * \brief Class separating board system FW interface from uHal wrapper
     */
    class BeBoardFWInterface : public RegManager
    {

      public:

        FpgaConfig* fpgaConfig;
        FileHandler* fFileHandler ;
        uint32_t fNthAcq, fNpackets;
        bool fSaveToFile;

        static const uint32_t cMask1 = 0xff;
        static const uint32_t cMask2 = 0xff00;
        static const uint32_t cMask3 = 0xff0000;
        static const uint32_t cMask4 = 0xff000000;
        static const uint32_t cMask5 = 0x1e0000;
        static const uint32_t cMask6 = 0x10000;
        static const uint32_t cMask7 = 0x200000;
      public:

        /*!
        * \brief Constructor of the BeBoardFWInterface class
        * \param puHalConfigFileName : path of the uHal Config File*/
        BeBoardFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId );
        BeBoardFWInterface ( const char* pId, const char* pUri, const char* pAddressTable );
        /*!
        * \brief set a FileHandler Object and enable saving to file!
        * \param pFileHandler : pointer to file handler for saving Raw Data*/
        //void setFileHandler (FileHandler* pHandler);
        virtual void setFileHandler (FileHandler* pHandler) = 0;
        /*!
        * \brief Destructor of the BeBoardFWInterface class
        */
        virtual ~BeBoardFWInterface() {}
        /*!
        * \brief Get the board type
        */
        virtual std::string readBoardType();
        /*!
        * \brief Get the board infos
        */
        virtual uint32_t getBoardInfo() = 0;

        /*! \brief Upload a configuration in a board FPGA */
        virtual void FlashProm ( const std::string& strConfig, const char* pstrFile ) {}
        /*! \brief Jump to an FPGA configuration */
        virtual void JumpToFpgaConfig ( const std::string& strConfig ) {}

        virtual void DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest ) {}
        /*! \brief Current FPGA configuration*/
        virtual const FpgaConfig* getConfiguringFpga()
        {
            return nullptr;
        }
        virtual void ProgramCdce() {}
        /*! \brief Get the list of available FPGA configuration (or firmware images)*/
        virtual std::vector<std::string> getFpgaConfigList( )
        {
            return std::vector<std::string>();
        }
        /*! \brief Delete one Fpga configuration (or firmware image)*/
        virtual void DeleteFpgaConfig ( const std::string& strId ) {}

        //Encode/Decode Cbc values
        /*!
        * \brief Encode a/several word(s) readable for a Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Cbc to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        virtual void EncodeReg ( const CbcRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) = 0; /*!< Encode a/several word(s) readable for a Cbc*/
        /*!
        * \brief Encode a/several word(s) readable for a Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Cbc to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        virtual void EncodeReg ( const CbcRegItem& pRegItem, uint8_t pFeId, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) = 0; /*!< Encode a/several word(s) readable for a Cbc*/
        /*!
        * \brief Encode a/several word(s) for Broadcast write to CBCs
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pNCbc : number of CBCs to write to
        * \param pVecReq : Vector to stack the encoded words
        */
        virtual void BCEncodeReg ( const CbcRegItem& pRegItem, uint8_t pNCbc, std::vector<uint32_t>& pVecReq, bool pRead = false, bool pWrite = false ) = 0; /*!< Encode a/several word(s) readable for a Cbc*/
        /*!
        * \brief Decode a word from a read of a register of the Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to read
        * \param pCbcId : Id of the Cbc to work with
        * \param pWord : variable to put the decoded word
        */
        virtual void DecodeReg ( CbcRegItem& pRegItem, uint8_t& pCbcId, uint32_t pWord, bool& pRead, bool& pFailed ) = 0; /*!< Decode a word from a read of a register of the Cbc*/


        //virtual pure methods which are defined in the proper BoardFWInterface class
        //r/w the Cbc registers
        /*!
        * \brief Write register blocks of a Cbc
        * \param pFeId : FrontEnd to work with
        * \param pVecReq : Block of words to write
        */
        virtual bool WriteCbcBlockReg (  std::vector<uint32_t>& pVecReq, bool pReadback ) = 0;
        //r/w the Cbc registers
        /*!
        * \brief Write register blocks of a Cbc
        * \param pFeId : FrontEnd to work with
        * \param pVecReq : Block of words to write
        */
        virtual bool BCWriteCbcBlockReg (  std::vector<uint32_t>& pVecReq, bool pReadback ) = 0;
        /*!
        * \brief Read register blocks of a Cbc
        * \param pFeId : FrontEnd to work with
        * \param pVecReq : Vector to stack the read words
        */
        virtual void ReadCbcBlockReg (  std::vector<uint32_t>& pVecReq ) = 0;
        /*!
        * \brief Configure the board with its Config File
        * \param pBoard
        */
        virtual void ConfigureBoard ( const BeBoard* pBoard ) = 0;
        /*!
         * \brief Send a CBC fast reset
         */
        virtual void CbcHardReset() = 0;
        /*!
         * \brief Send a CBC fast reset
         */
        virtual void CbcFastReset() = 0;
        /*!
         * \brief Start an acquisition in a separate thread
         * \param pBoard Board running the acquisition
         * \param uNbAcq Number of acquisition iterations (each iteration will get CBC_DATA_PACKET_NUMBER + 1 events)
         * \param visitor override the visit() method of this object to process each event
         */
        //virtual void StartThread ( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor ) = 0;
        /*! \brief Stop a running parallel acquisition
         */
        //virtual void StopThread();
        //[>! \brief Get the parallel acquisition iteration number <]
        //int getNumAcqThread();
        //[>! \brief Is a parallel acquisition running ? <]
        //bool isRunningThread() const
        //{
        //return runningAcquisition;
        //}
        /*!
         * \brief Start a DAQ
         */
        virtual void Start() = 0;
        /*!
         * \brief Stop a DAQ
         */
        virtual void Stop() = 0;
        /*!
         * \brief Pause a DAQ
         */
        virtual void Pause() = 0;
        /*!
         * \brief Resume a DAQ
         */
        virtual void Resume() = 0;
        /*!
         * \brief Read data from DAQ
         * \param pBoard
         * \param pBreakTrigger : if true, enable the break trigger
         * \return fNpackets: the number of packets read
         */
        virtual uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger ) = 0;
        /*!
         * \brief Read data for pNEvents
         * \param pBoard : the pointer to the BeBoard
         * \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
         */
        virtual void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents) = 0;
        /*!
         * \brief Get next event from data buffer
         * \return Next event
         */
        virtual const Event* GetNextEvent ( const BeBoard* pBoard ) const = 0;
        virtual const Event* GetEvent ( const BeBoard* pBoard, int i ) const = 0;
        virtual const std::vector<Event*>& GetEvents ( const BeBoard* pBoard ) const = 0;

        virtual std::vector<uint32_t> ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize ) = 0;

        virtual BoardType getBoardType() const = 0;
        /*! \brief Reboot the board */
        virtual void RebootBoard() = 0;
        /*! \brief Set or reset the start signal */
        virtual void SetForceStart ( bool bStart) = 0;

      protected:

        //bool runningAcquisition;
        uint32_t fBlockSize, fNPackets, numAcq, nbMaxAcq;
        //boost::thread thrAcq;

        //template to return a vector of all mismatched elements in two vectors using std::mismatch for readback value comparison

        template<typename T, class BinaryPredicate>
        std::vector<typename std::iterator_traits<T>::value_type>
        get_mismatches (T pWriteVector_begin, T pWriteVector_end , T pReadVector_begin, BinaryPredicate p)
        {
            std::vector<typename std::iterator_traits<T>::value_type> pMismatchedWriteVector;

            for (std::pair<T, T> cPair = std::make_pair (pWriteVector_begin, pReadVector_begin);
                    (cPair = std::mismatch (cPair.first, pWriteVector_end, cPair.second , p) ).first != pWriteVector_end;
                    ++cPair.first, ++cPair.second
                )
                pMismatchedWriteVector.push_back (*cPair.first);

            return pMismatchedWriteVector;
        }
    };
}

#endif
