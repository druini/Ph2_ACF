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

#include <uhal/uhal.hpp>
#include "RegManager.h"
#include "../Utils/Event.h"
#include "../Utils/FileHandler.h"
#include "../Utils/Data.h"
#include "../Utils/Utilities.h"
#include "../Utils/Exception.h"
#include "../Utils/FileHandler.h"
#include "../Utils/easylogging++.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/ChipRegItem.h"
#include "../HWDescription/Chip.h"
#include "../HWDescription/ReadoutChip.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/SSA.h"
#include "../HWDescription/MPA.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/RD53.h"
#include "../HWDescription/RegItem.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

using namespace Ph2_HwDescription;


//enum class BoardType {GLIB, ICGLIB, CTA, ICFC7, Chip3FC7};

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
* \brief enable the file handler temporarily
*/
        void enableFileHandler()
        {
            fSaveToFile = true;
        }
/*!
* \brief disable the file handler temporarily
*/
        void disableFileHandler()
        {
            fSaveToFile = false;
        }
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

//Encode/Decode Chip values
/*!
* \brief Encode a/several word(s) readable for a Chip
* \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
* \param pChipId : Id of the Chip to work with
* \param pVecReq : Vector to stack the encoded words
*/
        virtual void EncodeReg ( const ChipRegItem& pRegItem, uint8_t pChipId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite )
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
        /*!< Encode a/several word(s) readable for a Chip*/
        /*!
        * \brief Encode a/several word(s) readable for a Chip
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pChipId : Id of the Chip to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        virtual void EncodeReg ( const ChipRegItem& pRegItem, uint8_t pFeId, uint8_t pChipId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Encode a/several word(s) readable for a Chip*/
/*!
* \brief Encode a/several word(s) for Broadcast write to Chips
* \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
* \param pNChip : number of Chips to write to
* \param pVecReq : Vector to stack the encoded words
*/
        virtual void BCEncodeReg ( const ChipRegItem& pRegItem, uint8_t pNChip, std::vector<uint32_t>& pVecReq, bool pRead = false, bool pWrite = false ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Encode a/several word(s) readable for a Chip*/
/*!
* \brief Decode a word from a read of a register of the Chip
* \param pRegItem : RegItem containing infos (name, adress, value...) about the register to read
* \param pChipId : Id of the Chip to work with
* \param pWord : variable to put the decoded word
*/
        virtual void DecodeReg ( ChipRegItem& pRegItem, uint8_t& pChipId, uint32_t pWord, bool& pRead, bool& pFailed ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Decode a word from a read of a register of the Chip*/

//Encode/Decode MPA/SSA values
        virtual void EncodeReg ( const RegItem& pRegItem, uint8_t pChipId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Encode a/several word(s) readable for a Chip*/\
        virtual void EncodeReg ( const RegItem& pRegItem, uint8_t pFeId, uint8_t pChipId, std::vector<uint32_t>& pVecReq, bool pRead, bool pWrite ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Encode a/several word(s) readable for a Chip*/	
        virtual void BCEncodeReg ( const RegItem& pRegItem, uint8_t pNChip, std::vector<uint32_t>& pVecReq, bool pRead = false, bool pWrite = false ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Encode a/several word(s) readable for a Chip*/
        virtual void DecodeReg ( RegItem& pRegItem, uint8_t& pChipId, uint32_t pWord, bool& pRead, bool& pFailed ) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        } 
/*!< Decode a word from a read of a register of the Chip*/


//virtual pure methods which are defined in the proper BoardFWInterface class
//r/w the Chip registers
/*!
* \brief Write register blocks of a Chip
* \param pFeId : FrontEnd to work with
* \param pVecReq : Block of words to write
* \param pWriteAttempt : number of tries write was attempted
*/
// virtual bool WriteChipBlockReg (  std::vector<uint32_t>& pVecReq, uint8_t& pWriteAttempts, bool pReadback ) = 0;
//r/w the Chip registers
/*!
* \brief Write register blocks of a Chip
* \param pFeId : FrontEnd to work with
* \param pVecReq : Block of words to write
*/
// virtual bool BCWriteChipBlockReg (  std::vector<uint32_t>& pVecReq, bool pReadback ) = 0;
/*!
* \brief Read register blocks of a Chip
* \param pFeId : FrontEnd to work with
* \param pVecReq : Vector to stack the read words
*/
// virtual void ReadChipBlockReg (  std::vector<uint32_t>& pVecReq ) = 0;
/*!
* \brief Configure the board with its Config File
* \param pBoard
*/
        virtual void ConfigureBoard ( const BeBoard* pBoard ) = 0;

/*!
* \brief Send a Chip reset
*/
        virtual void ChipReset() = 0;

/*!
* \brief Send a Chip re-sync
*/
        virtual void ChipReSync() = 0;

/*!
* \brief Send a Chip trigger
*/
        virtual void ChipTrigger() 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        }
/*!
* \brief Send a Chip trigger
*/
        virtual void ChipTestPulse() 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        }
/*!
* \brief Start an acquisition in a separate thread
* \param pBoard Board running the acquisition
* \param uNbAcq Number of acquisition iterations (each iteration will get Chip_DATA_PACKET_NUMBER + 1 events)
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
        virtual uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait = true ) = 0;
/*!
* \brief Read data for pNEvents
* \param pBoard : the pointer to the BeBoard
* \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
*/
        virtual void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = true) = 0;

        virtual std::vector<uint32_t> ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize ) = 0;


//LORE
        virtual bool WriteChipBlockReg   ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
            return false;
        }
        virtual bool BCWriteChipBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
            return false;
        }
        virtual void ReadChipBlockReg (  std::vector<uint32_t>& pVecReq )
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        }
//LORE


/*!
* Activate power on and off sequence
*/

        virtual void PowerOn();
        virtual void PowerOff();

/*!
* Read the firmware version
*/

        virtual void ReadVer();


        virtual BoardType getBoardType() const = 0;
/*! \brief Reboot the board */
        virtual void RebootBoard() 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        }
/*! \brief Set or reset the start signal */
        virtual void SetForceStart ( bool bStart) 
        {
            LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        }


	// ################################################################################################
	// # Virtual methods which are defined in the concrete implementation of BeBoardFWInterface class #
	// ################################################################################################
	virtual void WriteChipCommand (std::vector<uint32_t> & data, unsigned int nCmd = 1)
        {
	  LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
        }
	
        virtual std::vector<std::pair<uint16_t,uint16_t>> ReadChipRegisters (std::vector<uint32_t> & data, uint8_t chipID, uint8_t filter = 0)
	{
	  LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
	  return std::vector<std::pair<uint16_t,uint16_t>>();
	}
	
        virtual bool InitChipCommunication()
        {
	  LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: implementation of virtual member function is absent" << RESET;
	  return false;
        }
	// ################################################################################################
	

    protected:

//bool runningAcquisition;
        uint32_t fBlockSize, fNPackets, numAcq, nbMaxAcq;
//boost::thread thrAcq;

//template to return a vector of all mismatched elements in two vectors using std::mismatch for readback value comparison

template<typename T, class BinaryPredicate>
        std::vector<typename std::iterator_traits<T>::value_type>
        get_mismatches (T pWriteVector_begin, T pWriteVector_end, T pReadVector_begin, BinaryPredicate p)
        {
            std::vector<typename std::iterator_traits<T>::value_type> pMismatchedWriteVector;

            for (std::pair<T, T> cPair = std::make_pair (pWriteVector_begin, pReadVector_begin);
                (cPair = std::mismatch (cPair.first, pWriteVector_end, cPair.second, p) ).first != pWriteVector_end;
                ++cPair.first, ++cPair.second
                )
                pMismatchedWriteVector.push_back (*cPair.first);

            return pMismatchedWriteVector;
        }
    };
}

#endif
