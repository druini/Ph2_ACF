/*!

        \file                           GlibFWInterface.h
        \brief                          GlibFWInterface init/config of the Glib and its Cbc's
        \author                         Lorenzo BIDEGAIN, Nicolas PIERRE
        \version            1.0
        \date                           28/07/14
        Support :                       mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#ifndef __GLIBFWINTERFACE_H__
#define __GLIBFWINTERFACE_H__

#include <string>
#include <map>
#include <vector>
#include <limits.h>
#include <stdint.h>
#include "BeBoardFWInterface.h"
#include "../HWDescription/Module.h"
#include "../Utils/Visitor.h"
#include "../Utils/easylogging++.h"

using namespace Ph2_HwDescription;

/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {
    class FpgaConfig;
    /*!
     * \class GlibFWInterface
     * \brief init/config of the Glib and its Cbc's
     */
    class GlibFWInterface : public BeBoardFWInterface
    {

      private:
        struct timeval fStartVeto;
        std::string fStrSram, fStrSramUserLogic, fStrFull, fStrReadout, fStrOtherSram, fStrOtherSramUserLogic;
        //std::string fCbcStubLat, fCbcI2CCmdAck, fCbcI2CCmdRq, fCbcHardReset, fCbcFastReset;
        FpgaConfig* fpgaConfig;
        FileHandler* fFileHandler ;
        uint32_t fNthAcq, fNpackets;

      private:
        /*!
         * \brief SRAM selection for DAQ
         */
        void SelectDaqSRAM();
        static bool cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2);

      public:
        /*!
         * \brief Constructor of the GlibFWInterface class
         * \param puHalConfigFileName : path of the uHal Config File
         * \param pBoardId
         */
        GlibFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId );
        GlibFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler );
        /*!
        * \brief Constructor of the GlibFWInterface class
        * \param pId : ID string
        * \param pUri: URI string
        * \param pAddressTable: address tabel string
        */
        GlibFWInterface ( const char* pId, const char* pUri, const char* pAddressTable );
        GlibFWInterface ( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler );
        void setFileHandler (FileHandler* pHandler);

        /*!
         * \brief Destructor of the GlibFWInterface class
         */
        virtual ~GlibFWInterface()
        {
            if (fFileHandler) delete fFileHandler;
        }

        ///////////////////////////////////////////////////////
        //      Glib Methods                                //
        /////////////////////////////////////////////////////

        /*! \brief Read a block of a given size
         * \param pRegNode Param Node name
         * \param pBlocksize Number of 32-bit words to read
         * \return Vector of validated 32-bit values
         */
        std::vector<uint32_t> ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize ) override;

        bool WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues ) override;

        /*!
         * \brief Get the FW info
         */
        uint32_t getBoardInfo();

        BoardType getBoardType() const
        {
            return BoardType::GLIB;
        }
        /*!
         * \brief Configure the board with its Config File
         * \param pBoard
         */
        void ConfigureBoard ( const BeBoard* pBoard ) override;
        //not supported but for compatability
        void FindPhase () override {}
        /*!
         * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
         */
        //void SelectFEId();
        /*!
         * \brief Start a DAQ
         */
        void Start() override;
        /*!
         * \brief Stop a DAQ
         */
        void Stop() override;
        /*!
         * \brief Pause a DAQ
         */
        void Pause() override;
        /*!
         * \brief Unpause a DAQ
         */
        void Resume() override;
        /*!
         * \brief Read data from DAQ
         * \param pBreakTrigger : if true, enable the break trigger
         * \return fNpackets: the number of packets read
         */
        uint32_t ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait=true) override;
        /*!
         * \brief Read data for pNEvents
         * \param pBoard : the pointer to the BeBoard
         * \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
         */
        void ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait=true);

      private:

        //I2C Methods

        /*!
         * \brief Wait for the I2C command acknowledgement
         * \param pAckVal : Expected status of acknowledgement, 1/0 -> true/false
         * \param pNcount : Number of registers at stake
         * \return boolean confirming the acknowledgement
         */
        bool I2cCmdAckWait ( uint32_t pAckVal, uint8_t pNcount = 1 );
        void EnableI2c ( bool pEnable );
        /*!
         * \brief Send request to r/w blocks via I2C
         * \param pVecReq : Block of words to send
         * \param pWrite : 1/0 -> Write/Read
         */
        void WriteI2C ( std::vector<uint32_t>& pVecReq, bool pWrite );
        /*!
         * \brief Read blocks from SRAM via I2C
         * \param pVecReq : Vector to stack the read words
         */
        void ReadI2C ( std::vector<uint32_t>& pVecReq );
        /*!
         * \brief Enable I2C communications
         * \param pEnable : 1/0 -> Enable/Disable
         */
        //void EnableI2c( bool pEnable );

        //void SelectFeSRAM( uint32_t pFe );

        /*! Compute the size of an acquisition data block
         * \return Number of 32-bit words to be read at each iteration */
        uint32_t computeBlockSize (BeBoard* pBoard);


      public:
        ///////////////////////////////////////////////////////
        //      CBC Methods                                 //
        /////////////////////////////////////////////////////

        //Encode/Decode Cbc values
        /*!
        * \brief Encode a/several word(s) readable for a Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Cbc to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        void EncodeReg ( const CbcRegItem& pRegItem,
                         uint8_t pCbcId, std::vector<uint32_t>& pVecReq,
                         bool pRead = false,
                         bool pWrite = false ) override; /*!< Encode a/several word(s) readable for a Cbc*/
        /*!
        * \brief Encode a/several word(s) readable for a Broadcast command
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pVecReq : Vector to stack the encoded words
        */
        void BCEncodeReg ( const CbcRegItem& pRegItem,
                           uint8_t pNCbc,
                           std::vector<uint32_t>& pVecReq,
                           bool pRead = false,
                           bool pWrite = false ) override; /*!< Encode a/several word(s) readable for a Cbc*/
        /*!
        * \brief Encode a/several word(s) readable for a Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Cbc to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        void EncodeReg ( const CbcRegItem& pRegItem,
                         uint8_t pFeId,
                         uint8_t pCbcId,
                         std::vector<uint32_t>& pVecReq,
                         bool pRead = false,
                         bool pWrite = false ) override; /* Encode a/several word(s) readable for a Cbc */
        /*!
        * \brief Decode a word from a read of a register of the Cbc
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to read
        * \param pCbcId : Id of the Cbc to work with
        * \param pWord : variable to put the decoded word
        */
        void DecodeReg ( CbcRegItem& pRegItem,
                         uint8_t& pCbcId,
                         uint32_t pWord,
                         bool& pRead,
                         bool& pFailed ) override; /*!< Decode a word from a read of a register of the Cbc*/
        //r/w the Cbc registers
        /*!
         * \brief Read register blocks of a Cbc
         * \param pFeId : FrontEnd to work with
         * \param pVecReq : Vector to stack the read words
         */
        bool WriteCbcBlockReg (  std::vector<uint32_t>& pVecReq, uint8_t& pWriteAttempts,  bool pReadback ) override;
        /*!
         * \brief Read register blocks of a Cbc
         * \param pFeId : FrontEnd to work with
         * \param pVecReq : Vector to stack the read words
         */
        bool BCWriteCbcBlockReg (  std::vector<uint32_t>& pVecReq, bool pReadback ) override;
        /*! \brief Read register blocks of a Cbc
         * \param pFeId : FrontEnd to work with
         * \param pVecReq : Vector to stack the read words
         */
        void ReadCbcBlockReg (  std::vector<uint32_t>& pVecReq );

        void CbcHardReset();

        void CbcFastReset();

        void CbcTrigger() {}

        void CbcTestPulse() {}

        ///////////////////////////////////////////////////////
        //      FPGA CONFIG                                 //
        /////////////////////////////////////////////////////

        /*! \brief Upload a firmware (FPGA configuration) from a file in MCS format into a given configuration
         * \param numConfig FPGA configuration number (1 or 2)
         * \param pstrFile path to MCS file
         */
        void FlashProm ( const std::string& strConfig, const char* pstrFile );
        /*! \brief Jump to an FPGA configuration */
        void JumpToFpgaConfig ( const std::string& strConfig);
        /*! \brief Is the FPGA being configured ?
         * \return FPGA configuring process or NULL if configuration occurs */
        const FpgaConfig* getConfiguringFpga()
        {
            return fpgaConfig;
        }
        /*! \brief Reboot the board */
        void RebootBoard();
        /*! \brief Set or reset the start signal */
        void SetForceStart ( bool bStart);

        void ReadVer() override;
        void PowerOn() override;
        void PowerOff() override;

    };
}

#endif
