
/*!

        \file                           D19cFWInterface.h
        \brief                          D19cFWInterface init/config of the FC7 and its Chip's
        \author                         G. Auzinger, K. Uchida, M. Haranko
        \version            1.0
        \date                           24.03.2017
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch
                                                  mykyta.haranko@SPAMNOT.cern.ch

 */

#ifndef _D19CFWINTERFACE_H__
#define _D19CFWINTERFACE_H__

#include <string>
#include <map>
#include <vector>
#include <limits.h>
#include <stdint.h>
#include "BeBoardFWInterface.h"


/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface
{
    class D19cFpgaConfig;
    /*!
     * \class Cbc3Fc7FWInterface
     *
     * \brief init/config of the Fc7 and its Chip's
     */
    class D19cFWInterface : public BeBoardFWInterface
    {
      private:
        D19cFpgaConfig* fpgaConfig;
        FileHandler* fFileHandler ;
        uint32_t fBroadcastCbcId;
        uint32_t fNCbc;
        uint32_t fNMPA;
        uint32_t fNSSA;
        uint32_t fFMCId;

        uint32_t fFWNHybrids;
        uint32_t fFWNChips;
        FrontEndType fFirmwareFrontEndType;
        bool fCBC3Emulator;
        bool fIsDDR3Readout;
        bool fDDR3Calibrated;
        uint32_t fDDR3Offset;
        uint32_t fI2CVersion;

        const uint32_t SINGLE_I2C_WAIT = 200; //used for 1MHz I2C

        // some useful stuff 
        int fResetAttempts; 
      public:
        /*!
         *
         * \brief Constructor of the Cbc3Fc7FWInterface class
         * \param puHalConfigFileName : path of the uHal Config File
         * \param pBoardId
         */

        D19cFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId );
        D19cFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId, FileHandler* pFileHandler );
        /*!
         *
        * \brief Constructor of the Cbc3Fc7FWInterface class
        * \param pId : ID string
        * \param pUri: URI string
        * \param pAddressTable: address tabel string
        */

        D19cFWInterface ( const char* pId, const char* pUri, const char* pAddressTable );
        D19cFWInterface ( const char* pId, const char* pUri, const char* pAddressTable, FileHandler* pFileHandler );
        void setFileHandler (FileHandler* pHandler);

        /*!
         *
         * \brief Destructor of the Cbc3Fc7FWInterface class
         */

        ~D19cFWInterface()
        {
            if (fFileHandler) delete fFileHandler;
        }

        ///////////////////////////////////////////////////////
        //      d19c Methods                                //
        /////////////////////////////////////////////////////

        /*! \brief Read a block of a given size
         * \param pRegNode Param Node name
         * \param pBlocksize Number of 32-bit words to read
         * \return Vector of validated 32-bit values
         */
        std::vector<uint32_t> ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize ) override;

        /*! \brief Read a block of a given size
         * \param pRegNode Param Node name
         * \param pBlocksize Number of 32-bit words to read
         * \param pBlockOffset Offset of the block
         * \return Vector of validated 32-bit values
         */
        std::vector<uint32_t> ReadBlockRegOffsetValue ( const std::string& pRegNode, const uint32_t& pBlocksize, const uint32_t& pBlockOffset );

        bool WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues ) override;

        /*!
         * \brief Get the FW info
         */
        uint32_t getBoardInfo();

        BoardType getBoardType() const
        {
            return BoardType::D19C;
        }
        /*!
         * \brief Configure the board with its Config File
         * \param pBoard
         */
        void ConfigureBoard ( const Ph2_HwDescription::BeBoard* pBoard ) override;
        /*!
         * \brief Detect the right FE Id to write the right registers (not working with the latest Firmware)
         */
        void SelectFEId();
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
         * \brief Reset Readout
         */
        void ResetReadout();

        /*!
         * \brief DDR3 Self-test
         */
        void DDR3SelfTest();

        /*!
          * \brief Tune the 320MHz buses phase shift
          */
        void PhaseTuning(const Ph2_HwDescription::BeBoard *pBoard);

        /*!
         * \brief Read data from DAQ
         * \param pBreakTrigger : if true, enable the break trigger
         * \return fNpackets: the number of packets read
         */

        uint32_t ReadData ( Ph2_HwDescription::BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait = true ) override;
        /*!
         * \brief Read data for pNEvents
         * \param pBoard : the pointer to the BeBoard
         * \param pNEvents :  the 1 indexed number of Events to read - this will set the packet size to this value -1
         */
        void ReadNEvents (Ph2_HwDescription::BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait = true);

      private:
        uint32_t computeEventSize ( Ph2_HwDescription::BeBoard* pBoard );
        //I2C command sending implementation
        bool WriteI2C (  std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pWriteRead, bool pBroadcast );
        bool ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies);

        //binary predicate for comparing sent I2C commands with replies using std::mismatch
        static bool cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2);
        static bool cmd_reply_ack (const uint32_t& cWord1, const uint32_t& cWord2);

        void PowerOnDIO5();
        std::string getFMCCardName (uint32_t id);
        std::string getChipName(uint32_t pChipCode);
        FrontEndType getFrontEndType(uint32_t pChipCode);
        void SetI2CAddressTable();
        void Align_out();

        //template to copy every nth element out of a vector to another vector
        template<class in_it, class out_it>
        out_it copy_every_n ( in_it b, in_it e, out_it r, size_t n)
        {
            for (size_t i = std::distance (b, e) / n; i--; std::advance (b, n) )
                *r++ = *b;

            return r;
        }

        //method to split a vector in vectors that contain elements from even and odd indices
        void splitVectorEvenOdd (std::vector<uint32_t> pInputVector, std::vector<uint32_t>& pEvenVector, std::vector<uint32_t>& pOddVector)
        {
            bool ctoggle = false;
            std::partition_copy (pInputVector.begin(),
                                 pInputVector.end(),
                                 std::back_inserter (pEvenVector),
                                 std::back_inserter (pOddVector),
                                 [&ctoggle] (int)
            {
                return ctoggle = !ctoggle;
            });
        }

        void getOddElements (std::vector<uint32_t> pInputVector, std::vector<uint32_t>& pOddVector)
        {
            bool ctoggle = true;
            std::copy_if (pInputVector.begin(),
                          pInputVector.end(),
                          std::back_inserter (pOddVector),
                          [&ctoggle] (int)
            {
                return ctoggle = !ctoggle;
            });
        }

        void ReadErrors();

      public:
        ///////////////////////////////////////////////////////
        //      CBC Methods                                 //
        /////////////////////////////////////////////////////

        //Encode/Decode Chip values
        /*!
        * \brief Encode a/several word(s) readable for a Chip
        * \param pRegItem : ChipRegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Chip to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        void EncodeReg (const ChipRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pReadBack, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Chip*/
        void EncodeReg (const ChipRegItem& pRegItem, uint8_t pFeId, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pReadBack, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Chip*/

        void BCEncodeReg (const ChipRegItem& pRegItem, uint8_t pNCbc, std::vector<uint32_t>& pVecReq, bool pReadBack, bool pWrite ) override;
        void DecodeReg ( ChipRegItem& pRegItem, uint8_t& pCbcId, uint32_t pWord, bool& pRead, bool& pFailed ) override;

        bool WriteChipBlockReg   ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback) override;
        bool BCWriteChipBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) override;
        void ReadChipBlockReg (  std::vector<uint32_t>& pVecReg );

        void ChipReSync() override;

        void ChipReset() override;

        void ChipI2CRefresh();

        void ChipTestPulse();

        void ChipTrigger();

        // phase tuning coomands - d19c
        void PhaseTuningGetLineStatus(uint8_t pHybrid, uint8_t pChip, uint8_t pLine);
        void PhaseTuningGetDefaultFSMState();
        void PhaseTuningParseStatus();

        // measures the occupancy of the 2S chips
        bool Measure2SOccupancy(uint32_t pNEvents, uint8_t **&pErrorCounters, uint8_t ***&pChannelCounters);
        void Manage2SCountersMemory(uint8_t **&pErrorCounters, uint8_t ***&pChannelCounters, bool pAllocate);

        ///////////////////////////////////////////////////////
        //      MPA/SSA Methods                             //
        /////////////////////////////////////////////////////

        // Coms
        void PSInterfaceBoard_SetSlaveMap();
        void PSInterfaceBoard_ConfigureI2CMaster(uint32_t pEnabled, uint32_t pFrequency);
        void PSInterfaceBoard_SendI2CCommand(uint32_t slave_id,uint32_t board_id,uint32_t read,uint32_t register_address, uint32_t data);
        uint32_t PSInterfaceBoard_SendI2CCommand_READ(uint32_t slave_id,uint32_t board_id,uint32_t read,uint32_t register_address, uint32_t data);

        // Main Power:
        void PSInterfaceBoard_PowerOn(uint8_t mpaid = 0 , uint8_t ssaid = 0  );
        void PSInterfaceBoard_PowerOff();

        // MPA power on
        void PSInterfaceBoard_PowerOn_MPA(float VDDPST = 1.25, float DVDD = 1.2, float AVDD = 1.25, float VBG = 0.3, uint8_t mpaid = 0 , uint8_t ssaid = 0);
        void PSInterfaceBoard_PowerOff_MPA(uint8_t mpaid = 0 , uint8_t ssaid = 0 );
        /// SSA power on
        void PSInterfaceBoard_PowerOn_SSA_v1(float VDDPST = 1.25, float DVDD = 1.25, float AVDD = 1.25, float VBF = 0.3, float BG = 0.0, uint8_t mpaid = 0 , uint8_t ssaid = 0);
        void PSInterfaceBoard_PowerOff_SSA_v1(uint8_t mpaid = 0 , uint8_t ssaid = 0 );
        void PSInterfaceBoard_PowerOn_SSA_v2(float VDDPST = 1.2, float DVDD = 1.0, float AVDD = 1.2, float VBG = 0.3, uint8_t mpaid = 0 , uint8_t ssaid = 0);
        void PSInterfaceBoard_PowerOff_SSA_v2(uint8_t mpaid = 0 , uint8_t ssaid = 0 );
        void ReadPower_SSA(uint8_t mpaid = 0 , uint8_t ssaid = 0);
        void KillI2C();
        ///

        void Pix_write_MPA(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel,uint32_t data);
        uint32_t Pix_read_MPA(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel);
        std::vector<uint16_t> ReadoutCounters_MPA(uint32_t raw_mode_en = 0);

        void Compose_fast_command(uint32_t duration = 0,uint32_t resync_en = 0,uint32_t l1a_en = 0,uint32_t cal_pulse_en = 0,uint32_t bc0_en = 0);
        void PS_Open_shutter(uint32_t duration = 0);
        void PS_Close_shutter(uint32_t duration = 0);
        void PS_Clear_counters(uint32_t duration = 0);
        void PS_Start_counters_read(uint32_t duration = 0);

        ///////////////////////////////////////////////////////
        //      FPGA CONFIG                                 //
        /////////////////////////////////////////////////////

        void checkIfUploading();
        /*! \brief Upload a firmware (FPGA configuration) from a file in MCS format into a given configuration
         * \param strConfig FPGA configuration name
         * \param pstrFile path to MCS file
         */
        void FlashProm ( const std::string& strConfig, const char* pstrFile );
        /*! \brief Jump to an FPGA configuration */
        void JumpToFpgaConfig ( const std::string& strConfig);

        void DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest );
        /*! \brief Is the FPGA being configured ?
         * \return FPGA configuring process or NULL if configuration occurs */
        const FpgaConfig* GetConfiguringFpga()
        {
            return (const FpgaConfig*) fpgaConfig;
        }
        /*! \brief Get the list of available FPGA configuration (or firmware images)*/
        std::vector<std::string> getFpgaConfigList( );
        /*! \brief Delete one Fpga configuration (or firmware image)*/
        void DeleteFpgaConfig ( const std::string& strId);
        /*! \brief Reboot the board */
        void RebootBoard();
        /*! \brief Set or reset the start signal */
        void SetForceStart ( bool bStart) {}
    };
}

#endif
