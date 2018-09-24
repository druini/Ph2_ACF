/*!

        \file                                            BeBoardInterface.h
        \brief                                           User Interface to the Boards
        \author                                          Lorenzo BIDEGAIN, Nicolas PIERRE
        \version                                         1.0
        \date                        31/07/14
        Support :                    mail to : lorenzo.bidegain@gmail.com, nicolas.pierre@cern.ch


\mainpage Acquisition& Control Framework
*
* \section intro_sec Introduction
*
* The ACF middleware users should only use a BeBoardInterface( and a CbcInterface ) object and pass to its functions Hardware Description object pointers( from HWDescription namespace ) as arguments.
* \image html uml.png
* \section structure_sec Project structure
* The project compiles into the 5 following dynamic libraries
*
* \subsection lib1 HWDescription
* Hardware description of the boards, containing the modules, containing the CBC chips.\n
* The objects are:
FrontEndDescription, BeBoard, Cbc, Module

* \subsection lib2 HWInterface
* Hardware interface seen by the ACF middleware users. \n
* Classes:
RegManager, BeBoardFWInterface, GlibFWInterface, CbcInterface, BeBoardInterface, FpgaConfig
*
* \subsection lib3 System
* Base class derivated by all command line tools from the src directory.\n
    * Class: SystemController
    *
    * \subsection lib4 Utils
    * Utility functions used by other libraries.\n
    * Classes: Exception, Utilities, Event, Data, argvparser

    * \subsection lib5 tools
    * Library using ROOT functions to perform calibration and other data processing.\n
    * Classes: Calibration, FastCalibration, Channel, HybridTester, CMTester
    * /

*/

#ifndef __BEBOARDINTERFACE_H__
#define __BEBOARDINTERFACE_H__

#include "BeBoardFWInterface.h"

using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {

    using BeBoardFWMap = std::map<uint16_t, BeBoardFWInterface*>;    /*!< Map of Board connected */

    /*!
     * \class BeBoardInterface
     * \brief Class representing the User Interface to the different boards
     */
    class BeBoardInterface
    {

      private:
        BeBoardFWMap fBoardMap;                     /*!< Map of Board connected */
        BeBoardFWInterface* fBoardFW;                     /*!< Board loaded */
        uint16_t prevBoardIdentifier;                     /*!< Id of the previous board */

      public:
        /*!
         * \brief Set the board to talk with
         * \param pBoardId
         */
        void setBoard ( uint16_t pBoardIdentifier );

        /*!
         * \brief Constructor of the BeBoardInterface class
         * \param pBoardMap Reference to the BoardFWInterface
         */
        BeBoardInterface ( const BeBoardFWMap& pBoardMap );
        /*!
         * \brief Destructor of the BeBoardInterface class
         */
        ~BeBoardInterface();

        /*!
         * \brief set a FileHandlerObject for the corresponding Interface
         * \param pBoard
         * \param pHandler : pointer to FileHandler object
         */
        void SetFileHandler (BeBoard* pBoard, FileHandler* pHandler);

        /*!
         * \brief enable the file handler
         * \param pBoard
         */
        void enableFileHandler (BeBoard* pBoard);

        /*!
         * \brief disable the file handler
         * \param pBoard
         */
        void disableFileHandler (BeBoard* pBoard);

        /*!
         * \brief Update both Board register and Config File
         * \param pBoard
         * \param pRegNode : Node of the register to update
         * \param pVal : Value to write
         */
        void WriteBoardReg ( BeBoard* pBoard, const std::string& pRegNode, const uint32_t& pVal );
        /*!
             * \brief Write a block of a given size into the board
            * \param pBoard
            * \param pRegNode : Node of the register to write
            * pValVec Vector of values to write
             */
        void WriteBlockBoardReg ( BeBoard* pBoard, const std::string& pRegNode, const std::vector<uint32_t>& pValVec );
        /*!
         * \brief Write: Update both Board register and Config File
         * \param pBoard
         * \param pRegVec : Vector of Register/Value pairs
         */
        void WriteBoardMultReg ( BeBoard* pBoard, const std::vector < std::pair< std::string, uint32_t > >& pRegVec );
        /*!
        * \brief Update Config File with the value in the Board register
        * \param pBoard
        * \param pRegNode : Node of the register to update
        */
        uint32_t ReadBoardReg ( BeBoard* pBoard, const std::string& pRegNode );
        /*!
             * \brief Read a block of a given size from the board
            * \param pBoard
            * \param pRegNode : Node of the register to read
            * \param pSize Number of 32-bit words in the block
             */
        std::vector<uint32_t> ReadBlockBoardReg ( BeBoard* pBoard, const std::string& pRegNode, uint32_t pSize );
        /*!
         * \brief Read a vector of Registers
         * \param pBoard
         * \param pRegVec : Vector of Register/Value pairs
         */
        void ReadBoardMultReg ( BeBoard* pBoard, std::vector < std::pair< std::string, uint32_t > >& pRegVec );
        /*!
         * \brief Get the board infos
         * \param pBoard
         */
        uint32_t getBoardInfo ( const BeBoard* pBoard );
        /*!
         * \brief Get the board infos
         * \param pBoard
         */
        BoardType getBoardType ( const BeBoard* pBoard );

        /*!
         * \brief Configure the board with its Config File
         * \param pBoard
         */
        void ConfigureBoard ( const BeBoard* pBoard );

        /*!
         * \brief Hard reset of all Cbc
         * \param pCbc
         */
        void CbcHardReset ( const BeBoard* pBoard );
        /*!
         * \brief Fast Reset of the Cbc
         * \param pCbc
         */
        void CbcFastReset ( const BeBoard* pBoard );
        /*!
         * \brief Send Cbc Trigger
         * \param pCbc
         */
        void CbcTrigger ( const BeBoard* pBoard );

        /*!
         * \brief Send Cbc TestPulse
         * \param pCbc
         */
        void CbcTestPulse ( const BeBoard* pBoard );

        /*!
         * \brief Start a DAQ
         * \param pBoard
         */
        void Start ( BeBoard* pBoard );
        /*!
         * \brief Stop a DAQ
         * \param pBoard
         */
        void Stop ( BeBoard* pBoard );
        /*!
         * \brief Pause a DAQ
         * \param pBoard
         */
        void Pause ( BeBoard* pBoard );
        /*!
         * \brief Resume a DAQ
         * \param pBoard
         */
        void Resume ( BeBoard* pBoard );
        /*!
         * \brief Read data from DAQ
         * \param pBoard
         * \param pBreakTrigger : if true, enable the break trigger
         * \return fNpackets: the number of packets read
         */
        uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait = true );
        /*!
         * \brief Read data for pNEvents
         * \param pBoard : the pointer to the BeBoard
         * \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
         */
        void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = true);

        /*! \brief Get a uHAL node object from its path in the uHAL XML address file
         * \param pBoard pointer to a board description
         * \return Reference to the uhal::node object
         */
        const uhal::Node& getUhalNode ( const BeBoard* pBoard, const std::string& pStrPath );
        /*! \brief Access to the uHAL main interface for a given board
         * \param pBoard pointer to a board description
         * \return pointer to the uhal::HwInterface object
         */
        uhal::HwInterface* getHardwareInterface ( const BeBoard* pBoard );
        /*! \brief Access to the firmware interface for a given board
         * \return pointer to the BeBoardFWInterface object
         */
        BeBoardFWInterface* getFirmwareInterface () { return fBoardFW; }
        /*! \brief Upload a configuration in a board FPGA
         * \param pBoard pointer to a board description
         * \param numConfig FPGA configuration number to be uploaded
         * \param pstrFile path to MCS file containing the FPGA configuration
         */
        void FlashProm ( BeBoard* pBoard, const std::string& strConfig, const char* pstrFile );
        /*! \brief Jump to an FPGA configuration
        * \param pBoard pointer to a board description
        * \param numConfig FPGA configuration number
        */
        void JumpToFpgaConfig ( BeBoard* pBoard, const std::string& strConfig);

        void DownloadFpgaConfig ( BeBoard* pBoard, const std::string& strConfig, const std::string& strDest);
        /*! \brief Current FPGA configuration
         * \param pBoard pointer to a board description
         * \return const pointer to an FPGA uploading process. NULL means that no upload is been processed.
         */
        const FpgaConfig* getConfiguringFpga ( BeBoard* pBoard );

        /*! \brief Get the list of available FPGA configuration (or firmware images)
        * \param pBoard pointer to a board description */
        std::vector<std::string> getFpgaConfigList ( BeBoard* pBoard);

        /*! \brief Delete one Fpga configuration (or firmware image)
        * \param pBoard pointer to a board description
         * \param strId Firmware image identifier*/
        void DeleteFpgaConfig (BeBoard* pBoard, const std::string& strId);
        /*! \brief Reboot the board */
        void RebootBoard (BeBoard* pBoard);
        /*! \brief Set or reset the start signal */
        void SetForceStart (BeBoard* pBoard, bool bStart);




        /*!
         * Activate power on and off sequence 
         */
        void PowerOn( BeBoard* pBoard );

        void PowerOff( BeBoard* pBoard );

        /*!
         * Read the firmware version
         */
        void ReadVer( BeBoard* pBoard );

        /*!
         * Returns data from buffernum and mpa.  Raw register output.
         */
        std::pair<std::vector<uint32_t>, std::vector<uint32_t>>   ReadData( BeBoard* pBoard, int buffernum, int mpa);




    };
}

#endif
