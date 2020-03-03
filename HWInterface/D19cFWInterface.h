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
#include "../Utils/easylogging++.h"
#include "../Utils/Event.h"
#include "../Utils/DataContainer.h"
//#include "../Utils/OccupancyAndPh.h"
//#include "../Utils/GenericDataVector.h"
#include <uhal/uhal.hpp>


namespace D19cFWEvtEncoder
{
  // ################
  // # Event header #
  // ################
  const uint16_t EVT_HEADER     = 0xFFFF;

  const uint16_t IWORD_L1_HEADER = 4;
  const uint16_t SBIT_L1_HEADER       = 28;
  const uint16_t SBIT_L1_STATUS       = 24;
  const uint16_t SBIT_HYBRID_ID       = 16;
  const uint16_t SBIT_CHIP_ID       = 12;
  
 
  // ################
  // # Event status #
  // ################
  const uint16_t GOOD             = 0x0000; // Event status Good
  const uint16_t EMPTY            = 0x0002; // Event status Empty event
  const uint16_t BADHEADER        = 0x0004; // Bad header
  const uint16_t BADL1HEADER      = 0x0006; // Bad L1 header
  const uint16_t BADSTUBHEADER    = 0x0008; // Bad L1 header
  /*const uint16_t INCOMPLETE = 0x0004; // Event status Incomplete event header
  const uint16_t L1A        = 0x0008; // Event status L1A counter mismatch
  const uint16_t FWERR      = 0x0010; // Event status Firmware error
  const uint16_t FRSIZE     = 0x0020; // Event status Invalid frame size
  const uint16_t MISSCHIP   = 0x0040; // Event status Chip data are missing*/
  const uint16_t NODECODER  = 0xFFFF; // Event decoding not implemented 

  const uint16_t CLUSTER_2S = 14; 
  const uint16_t SCLUSTER_PS = 14;
  const uint16_t PCLUSTER_PS = 17;
  const uint16_t SCLUSTER_MPA = 0;
  const uint16_t PCLUSTER_MPA = 0;
  const uint16_t HITS_2S = 274;
  const uint16_t HITS_SSA = 120; 
  const uint16_t HITS_CBC = 254; 
}


/*!
 * \namespace Ph2_HwInterface
 * \brief Namespace regrouping all the interfaces to the hardware
 */
namespace Ph2_HwInterface {
    class D19cFpgaConfig;
    class D19cSSAEvent;
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
        uint32_t fNReadoutChip;
        uint32_t fNCic;
        uint32_t fFMCId;

        // number of chips and hybrids defined in firmware (compiled for)
        uint32_t fFWNHybrids;
        uint32_t fFWNChips;
        FrontEndType fFirmwareFrontEndType;
        bool fCBC3Emulator;
        bool fIsDDR3Readout;
        bool fDDR3Calibrated;
        uint32_t fDDR3Offset;
    	  // i2c version of master
        uint32_t fI2CVersion;	
        // optical readout
        bool fOptical=false;
        bool fConfigureCDCE=false;
        std::map<uint8_t,uint8_t> fRxPolarity;
        std::map<uint8_t,uint8_t> fTxPolarity;
        
        uint32_t fGBTphase;

        const uint32_t SINGLE_I2C_WAIT = 200; //used for 1MHz I2C

        // some useful stuff 
        int fResetAttempts; 
      public:
        // struct Event : public Ph2_HwInterface::Event
        // {
        //   Event (const uint32_t* data, size_t n);

        //   void fillDataContainer          (BoardDataContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup) override;
        //   static void addBoardInfo2Events (const Ph2_HwDescription::BeBoard* pBoard, std::vector<D19cFWInterface::Event>& decodedEvents);

        //   uint16_t block_size;
        //   uint16_t tlu_trigger_id;
        //   uint16_t data_format_ver;
        //   uint16_t tdc;
        //   uint32_t l1a_counter;
        //   uint32_t bx_counter;
        //   uint16_t evtStatus;


        //   protected:

        // };

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

        // uint16_t ParseEvents(const std::vector<uint32_t>& pData) override;
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
        * \breif Disconnect Setup with Multiplexing Backplane
        */
        void DisconnectMultiplexingSetup() ;
 
        /*!
        * \breif Scan Setup with Multiplexing Backplane
        */
        uint32_t ScanMultiplexingSetup(uint8_t pWait_ms=100) ;
   
        /*!
        * \breif Configure Setup with Multiplexing Backplane
        * \param BackplaneNum
        * \param CardNum
        */
        uint32_t ConfigureMultiplexingSetup(int BackplaneNum, int CardNum) ;
        
        /*!
          * \brief Tune the 320MHz buses phase shift
          */
        bool PhaseTuning(Ph2_HwDescription::BeBoard *pBoard , uint8_t pFeId, uint8_t pChipId, uint8_t pLineId, uint16_t pPattern, uint16_t pPatternPeriod);
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
        // FMCs 
        void InitFMCPower();
        
        // ########################################
        // # Vector containing the decoded events #
        // ########################################
        //static std::vector<D19cFWInterface::Event> decodedEvents;

        static void DecodeSSAEvents (const std::vector<uint32_t>& data, std::vector<D19cSSAEvent*>& events, uint32_t fEventSize, uint32_t fNFe);

      private:
        uint32_t computeEventSize ( Ph2_HwDescription::BeBoard* pBoard );
        //I2C command sending implementation
        bool WriteI2C (  std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pWriteRead, bool pBroadcast );
        bool ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies);

        //binary predicate for comparing sent I2C commands with replies using std::mismatch
        static bool cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2);
        static bool cmd_reply_ack (const uint32_t& cWord1, const uint32_t& cWord2);

        // ########################################
        // # FMC powering/control/configuration  #
        // ########################################
        void powerAllFMCs(bool pEnable=false);
        // dedicated method to power on dio5
        void PowerOnDIO5();
        // get fmc card name
        std::string getFMCCardName (uint32_t id);
        // convert code of the chip from firmware
        std::string getChipName(uint32_t pChipCode);
        FrontEndType getFrontEndType(uint32_t pChipCode);
      	// set i2c address table depending on the hybrid
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
        * \param pRegItem : RegItem containing infos (name, adress, value...) about the register to write
        * \param pCbcId : Id of the Chip to work with
        * \param pVecReq : Vector to stack the encoded words
        */
        void EncodeReg (const Ph2_HwDescription::ChipRegItem& pRegItem, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pReadBack, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Chip*/
        void EncodeReg (const Ph2_HwDescription::ChipRegItem& pRegItem, uint8_t pFeId, uint8_t pCbcId, std::vector<uint32_t>& pVecReq, bool pReadBack, bool pWrite ) override; /*!< Encode a/several word(s) readable for a Chip*/
        
        void BCEncodeReg (const Ph2_HwDescription::ChipRegItem& pRegItem, uint8_t pNCbc, std::vector<uint32_t>& pVecReq, bool pReadBack, bool pWrite ) override;
        void DecodeReg ( Ph2_HwDescription::ChipRegItem& pRegItem, uint8_t& pCbcId, uint32_t pWord, bool& pRead, bool& pFailed ) override;
        


        bool WriteChipBlockReg   ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback) override;
        bool BCWriteChipBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback) override;
        void ReadChipBlockReg (  std::vector<uint32_t>& pVecReg );

        void ChipReSync() override;

        void ChipReset() override;

        void ChipI2CRefresh();

        void ChipTestPulse();

        void ChipTrigger();
        void Trigger(uint8_t pDuration=1);
        // Readout chip specific stuff 
        void ReadoutChipReset(); 
        // CIC BE stuff 
        bool Bx0Alignment();
        // TP FSM 
        void ConfigureTestPulseFSM(uint16_t pDelayAfterFastReset=1, uint16_t pDelayAfterTP=200, uint16_t pDelayBeforeNextTP=400, uint8_t pEnableFastReset=1, uint8_t pEnableTP=1, uint8_t pEnableL1A =1) ;
        // trigger FSM 
        void ConfigureTriggerFSM( uint16_t pNtriggers=100, uint16_t pTriggerRate=100, uint8_t pSource=3, uint8_t pStubsMask=0, uint8_t pStubLatency=50) ;
        // consecutive triggers FSM 
        void ConfigureConsecutiveTriggerFSM( uint16_t pNtriggers=32, uint16_t pDelayBetweenTriggers=1, uint16_t pDelayToNext=1 ) ;
        // back-end tuning for CIC data 
        void ConfigureFastCommandBlock(const Ph2_HwDescription::BeBoard* pBoard);

        void L1ADebug();
        void StubDebug(bool pWithTestPulse=true, uint8_t pNlines=5);
        bool L1PhaseTuning(const Ph2_HwDescription::BeBoard* pBoard , bool pScope=false);
        bool L1WordAlignment(const Ph2_HwDescription::BeBoard* pBoard , bool pScope=false);
        bool L1Tuning(const Ph2_HwDescription::BeBoard* pBoard , bool pScope=false);
        bool StubTuning(const Ph2_HwDescription::BeBoard* pBoard , bool pScope=false);
        //bool BackEndTuning(const BeBoard* pBoard, bool pDoL1A=true);
        
        // Optical readout specific functions - d19c [temporary]
        void setGBTxPhase(uint32_t pPhase){fGBTphase=pPhase;}
        void configureLink(const Ph2_HwDescription::BeBoard* pBoard );
        bool GBTLock( const Ph2_HwDescription::BeBoard* pBoard);
        std::pair<uint16_t,float> readADC( std::string pValueToRead="AMUX_L" , bool pApplyCorrection=false );
        void setRxPolarity( uint8_t pLinkId , uint8_t pPolarity=1){ fRxPolarity.insert({pLinkId,pPolarity}); };
        void setTxPolarity( uint8_t pLinkId , uint8_t pPolarity=1){ fTxPolarity.insert({pLinkId,pPolarity}); };

        // CDCE 
        void configureCDCE_old(uint16_t pClockRate=120 );
        void configureCDCE( uint16_t pClockRate=120, std::pair<std::string,float> pCDCEselect=std::make_pair("sec",40) ); 
        void syncCDCE();
        void epromCDCE();

        
        // struct GBTx
        // {
        //     uint8_t pGBTxAddress=0x01;
        //     uint16_t pSCAMaster = 14+3;
        //     std::map <std::string,uint8_t> fScaAdcChnMap = {{"AMUX_L", 0}, {"AMUX_R",30}, {"VMIN", 14}, {"VM1V5", 21}, {"VM2V5", 27}, {"VRSSI", 24}, {"EXT_TEMP", 25}, {"INT_TEMP",31} };
        //     std::map <std::string,std::pair<int,int>> fScaAdcVoltageDeviderMap = { {"AMUX_L", std::make_pair( 0, 1)}, {"AMUX_R", std::make_pair(0,1)}, {"VMIN", std::make_pair( 91000,   4700)},
        //                             {"VM1V5"     , std::make_pair(100000, 110000)},
        //                             {"VM2V5"     , std::make_pair(   100,     47)},
        //                             {"VRSSI"     , std::make_pair(     0,      1)},
        //                             {"EXT_TEMP"  , std::make_pair(     0,  10000)},
        //                             {"INT_TEMP"  , std::make_pair(     0,      1)}
        //                         };

        //     // FMCs
        //     void powerFMCs(BeBoardFWInterface* pInterface, bool pEnable=false)
        //     {
        //         pInterface->WriteReg ("sysreg.fmc_pwr.pg_c2m", (int)pEnable );
        //         pInterface->WriteReg ("sysreg.fmc_pwr.l12_pwr_en", (int)pEnable );
        //         pInterface->WriteReg ("sysreg.fmc_pwr.l8_pwr_en", (int)pEnable);
        //     };
        //     // Slow contron mux 
        //     void selectMux(BeBoardFWInterface* pInterface, uint8_t pLinkId  , uint32_t cWait_ms = 100 )
        //     {
        //       pInterface->WriteReg("fc7_daq_cnfg.optical_block.mux",pLinkId);
        //       std::this_thread::sleep_for (std::chrono::milliseconds (cWait_ms) );
        //       //pInterface->WriteReg("fc7_daq_ctrl.optical_block.sca.reset",0x1);
        //       //std::this_thread::sleep_for (std::chrono::milliseconds (100) );
        //     }
        //     // GBTX ec 
        //     void ecReset(BeBoardFWInterface* pInterface )
        //     {
        //         std::vector< std::pair<std::string, uint32_t> > cVecReg;
        //         cVecReg.push_back ({"fc7_daq_ctrl.optical_block.sca.start",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.gbtx",0x00});
        //         //cVecReg.push_back ({"fc7_daq_ctrl.optical_block.sca.reset",0x1});
        //         pInterface->WriteStackReg( cVecReg ); 
        //     };
        //     uint32_t ecWrite(BeBoardFWInterface* pInterface, uint16_t pI2Cmaster, uint32_t pCommand , uint32_t pData=0x00 ) 
        //     {
        //         std::vector< std::pair<std::string, uint32_t> > cVecReg;
        //         cVecReg.push_back ({"fc7_daq_ctrl.optical_block.sca.start",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.gbtx",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.address", 0x01}); 
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.id",0x01}) ; 
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.channel", pI2Cmaster});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.cmd", pCommand});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.data", pData});
        //         pInterface->WriteStackReg( cVecReg );
        //         LOG (DEBUG) << BOLDBLUE << "GBTx EC write to I2C master " << +pI2Cmaster <<  " - data field : " << +pData << " [ command 0x" << std::hex << pCommand << std::dec << "]." << RESET; 
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.sca.start",0x1); 
        //         // check for error 
        //         uint32_t cErrorCode = pInterface->ReadReg("fc7_daq_stat.optical_block.sca.error");
        //         // reset 
        //         //ecReset(pInterface);
        //         return cErrorCode;
        //     };
        //     uint32_t ecRead(BeBoardFWInterface* pInterface, uint16_t pI2Cmaster, uint32_t pCommand , uint32_t pData=0x00)
        //     {
        //         std::vector< std::pair<std::string, uint32_t> > cVecReg;
        //         cVecReg.push_back ({"fc7_daq_ctrl.optical_block.sca.start",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.gbtx",0x00});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.address", 0x01}); 
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.id",0x02}) ; 
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.channel", pI2Cmaster});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.cmd", pCommand});
        //         cVecReg.push_back ({"fc7_daq_cnfg.optical_block.sca.data", pData});
        //         pInterface->WriteStackReg( cVecReg );
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.sca.start",0x1);
        //         uint32_t cRead = pInterface->ReadReg("fc7_daq_stat.optical_block.sca.data");
        //         LOG (DEBUG) << BOLDBLUE << "GBTx EC read returns : " << std::bitset<32>(cRead) << RESET;
        //         //ecReset(pInterface);
        //         return cRead;
        //     };
        //     // GBTx ic 
        //     void icReset(BeBoardFWInterface* pInterface ) 
        //     {
        //         std::vector< std::pair<std::string, uint32_t> > cVecReg;
        //         cVecReg.push_back ( {"fc7_daq_ctrl.optical_block.ic",0x00} );
        //         cVecReg.push_back ( {"fc7_daq_cnfg.optical_block.ic",0x00} );
        //         cVecReg.push_back ( {"fc7_daq_cnfg.optical_block.gbtx",0x00} );
        //         pInterface->WriteStackReg( cVecReg );
        //      };
        //     void icWrite(BeBoardFWInterface* pInterface, uint32_t pAddress, uint32_t pData ) 
        //     {
        //         //config
        //         pInterface->WriteReg("fc7_daq_cnfg.optical_block.gbtx.address",pGBTxAddress);
        //         pInterface->WriteReg("fc7_daq_cnfg.optical_block.gbtx.data", pData); 
        //         pInterface->WriteReg("fc7_daq_cnfg.optical_block.ic.register", pAddress); 
        //         //perform operation 
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.write",0x01);
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.write",0x00);
        //         //
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.start_write",0x01);
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.start_write",0x00);
        //         icReset(pInterface);
        //     };
        //     uint32_t icRead(BeBoardFWInterface* pInterface, uint32_t pAddress, uint32_t pNwords ) 
        //     {
        //         //config
        //         pInterface->WriteReg("fc7_daq_cnfg.optical_block.gbtx.address",pGBTxAddress);
        //         pInterface->WriteReg("fc7_daq_cnfg.optical_block.ic.register", pAddress); 
        //         pInterface->WriteReg("fc7_daq_cnfg.optical_block.ic.nwords", pNwords); 
        //         //perform operation 
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.start_read",0x01);
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.start_read",0x00);
        //         //
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.read",0x01);
        //         pInterface->WriteReg("fc7_daq_ctrl.optical_block.ic.read",0x00);
        //         // 
        //         uint32_t cRead = pInterface->ReadReg("fc7_daq_stat.optical_block.ic.data");
        //         //reset 
        //         icReset(pInterface);
        //         return cRead;
        //     };
        //     // SCA - enable I2C master interfaces, GPIO, ADC 
        //     uint8_t scaEnable(BeBoardFWInterface* pInterface, uint16_t cI2Cmaster=0x00) 
        //     {
        //         uint32_t cErrorCode = ecWrite(pInterface, cI2Cmaster, 0x02 , 0x04000000 );
        //         if( cErrorCode != 0 ) 
        //         {
        //             LOG (INFO) << BOLDBLUE << "SCA Error code : " << +cErrorCode << RESET;
        //             return 0; 
        //         }
        //         cErrorCode = ecWrite(pInterface, cI2Cmaster, 0x04, 0x00000000) ;
        //         if( cErrorCode != 0 ) 
        //         {
        //             LOG (INFO) << BOLDBLUE << "SCA Error code : " << +cErrorCode << RESET;
        //             return 0; 
        //         }
        //         cErrorCode = ecWrite(pInterface, cI2Cmaster, 0x06, 0x16000000) ;
        //         if( cErrorCode != 0 ) 
        //         {
        //             LOG (INFO) << BOLDBLUE << "SCA Error code : " << +cErrorCode << RESET;
        //             return 0; 
        //         }

        //         return (cErrorCode == 0 );
        //     };
        //     void scaConfigure( BeBoardFWInterface* pInterface) 
        //     {
        //         LOG (INFO) << BOLDBLUE << "Set all registers involved in GBT-SCA communication, as instructed on page 66 of gbtx manual" << RESET;
        //         // dll 
        //         icWrite( pInterface, 231, 0x00dd); 
        //         icWrite( pInterface, 232, 0x000d); 
        //         icWrite( pInterface, 233, 0x0070); 
        //         for( uint16_t cRegister = 237 ; cRegister < 246; cRegister += 4 ) 
        //             icWrite( pInterface, cRegister, 0x0000); 
        //         icWrite( pInterface, 248, 0x0007);
        //         icWrite( pInterface, 251, 0x0000);
        //         icWrite( pInterface, 254, 0x0070);
        //         icWrite( pInterface, 257, 0x0000);
        //         icWrite( pInterface, 273, 0x0020);
        //     }
        //     bool scaSetGPIO( BeBoardFWInterface* pInterface, uint8_t cChannel , uint8_t cLevel ) 
        //     {
        //         uint32_t cMask = (1 << cChannel ) ;
        //         cMask = (~cMask & 0xFFFFFFFF); 
        //         uint8_t cSCAchannel =0x02;
        //         if( cChannel < 31 ) 
        //         {
        //             uint32_t cValue = ecRead( pInterface, cSCAchannel , 0x11);  
        //             uint8_t cErrorCode = ecWrite( pInterface, cSCAchannel, 0x10, (cLevel << cChannel) | (cValue & cMask) ); 
        //             return (cErrorCode == 0 );
        //         }
        //         return false;
        //     };
        //     // configure gpio [sca]
        //     void scaConfigureGPIO(BeBoardFWInterface* pInterface)
        //     {
        //         uint32_t cMask = (1 << 31 )  | (1 << 30) | (1<<3) | (1 << 2 );
        //         cMask = (~cMask & 0xFFFFFFFF);
        //         uint8_t  cMaster=0x02;
        //         uint32_t cData = (0 << 31)  | (1 << 30) | (0 << 3) | (1<<2) ;
        //         uint32_t cErrorCode = ecWrite(pInterface, cMaster , 0x10 , 0x40000004 );
        //         if( cErrorCode != 0 )
        //             exit(0);
        //         //
        //         uint32_t cValue = ecRead( pInterface, cMaster, 0x21); 
        //         cData = ( (1 << 31)  | (1 << 30) | (1 << 3) | (1<<2) ) ;
        //         ecWrite(pInterface, cMaster, 0x20, cData | (cValue&cMask) );
        //         //
        //         cValue = ecRead( pInterface, cMaster, 0x31) ; 
        //         cData = ((0 << 31)  | (0 << 30) | (0 << 3) | (0<<2))  ;
        //         ecWrite(pInterface, cMaster, 0x30, cData | (cValue&cMask) );
        //         //
        //         cValue = ecRead( pInterface, cMaster, 0x11) ;
        //         cData = (1 << 31)  | (0 << 30) | (1 << 3) | (0<<2) ;
        //         ecWrite(pInterface, cMaster, 0x10, cData | (cValue&cMask) );
        //     };
        //     uint16_t readAdcChn ( BeBoardFWInterface* pInterface, std::string pValueToRead , bool pConvertRawReading=false) 
        //     {
        //         uint32_t cADCslave = fScaAdcChnMap[pValueToRead]; 
        //         LOG (DEBUG) << BOLDBLUE << "Using ADC on GBTx to read " << pValueToRead << " -- which is ADC slave " << cADCslave << RESET;
        //         uint32_t  cMaster = 0x14;
        //         uint32_t cErrorMux, cErrorGo;
        //         if( pValueToRead == "EXT_TEMP") // turn on current source for external temperature sensor 
        //         {
        //             cErrorMux = ecWrite(pInterface,  cMaster, 0x60 , cADCslave );
        //             if( cErrorMux != 0 ) 
        //                 LOG (INFO) << BOLDYELLOW << "Error setting SCA AdcMuxSelect" << RESET;
        //         }
                
        //         cErrorMux = ecWrite(pInterface,  cMaster, 0x50 , cADCslave );
        //         if( cErrorMux != 0 ) 
        //           LOG (INFO) << BOLDYELLOW << "Error setting SCA AdcMuxSelect" << RESET;
        //         cErrorGo  = ecWrite(pInterface,  cMaster, 0x02 , 0x00000001 );
        //         if( cErrorGo != 0 ) 
        //           LOG (INFO) << BOLDYELLOW << "Error asking SCA AdcGo for starting conversion" << RESET;
                
        //         uint32_t cAdcValue = ecRead(pInterface,  cMaster, ((pConvertRawReading) ? 0x21 : 0x31) );
        //         LOG (DEBUG) << BLUE << "SCA ADC chn: "<< cADCslave << " reads"<< cAdcValue <<" for pConvertRawReading="<< pConvertRawReading << RESET;

        //         if( pValueToRead == "EXT_TEMP") // turn off current source for external temperature sensor 
        //         {
        //             cErrorMux = ecWrite(pInterface,  cMaster, 0x60 , 0x00 );
        //             if( cErrorMux != 0 ) 
        //                 LOG (INFO) << BOLDYELLOW << "Error setting SCA AdcMuxSelect" << RESET;
        //         }
        //         return (uint16_t)(cAdcValue);
        //     };
        //     uint32_t readAdcCalibration ( BeBoardFWInterface* pInterface)
        //     {
        //         uint32_t cCmdCalib = 0x11;
        //         uint16_t cAdcChnId = 0x14;
        //         uint32_t cAdcValue = ecRead(pInterface, cAdcChnId , cCmdCalib );
        //         return cAdcValue;
        //     };
        //     float convertAdcReading ( uint16_t pReading , std::string pValueToRead  ) 
        //     {
        //         /*!
        //         * \brief Transform ADC reading from 0-1 V range to real value based on resistances in voltage divider  
        //         * \param pReading : SCA ADC raw/ temp. corrected value uint16_t
        //         * \param pR1 : resistor 1 int
        //         * \param pR2 : resistor 2 int
        //         */ 
        //         auto cResistances = fScaAdcVoltageDeviderMap[pValueToRead]; 
        //         return float(pReading)/(std::pow(2,12)-1)*(cResistances.first+cResistances.second)/float(cResistances.second);
        //     };
        //     // GBTx configuration 
        //     void gbtxSelectEdgeTx(BeBoardFWInterface* pInterface, bool pRising=true)
        //     {
        //       uint32_t cReadBack = icRead( pInterface, 244 , 1);
        //       bool pValue = (pRising) ? 7 : 0 ;
        //       uint32_t cRegValue = (cReadBack & 0xC7 ) | ( pValue << 3 ); 
        //       icWrite(pInterface, 244 , cRegValue ) ;
        //     }

        //     void gbtxSelectEdge(BeBoardFWInterface* pInterface, bool pRising=true)
        //     {
        //       uint32_t cReadBack = icRead( pInterface, 244 , 1);
        //       uint32_t cRegValue = (cReadBack & 0xC0 ); 
        //       for( size_t cIndex = 0 ; cIndex < 6; cIndex ++ ) 
        //       {
        //         cRegValue = cRegValue | ( (uint8_t)pRising << cIndex );
        //       }
        //       LOG (INFO) << BOLDBLUE << "GBTx default configuration " << std::bitset<8>(cReadBack) << " -- will be set to " << std::bitset<8>(cRegValue) << RESET;    
        //       //icWrite(pInterface, 244 , cRegValue ) ;
        //     }
        //     void gbtxSetPhase(BeBoardFWInterface* pInterface, uint8_t pPhase=11)
        //     {
        //         uint16_t cReg = 62;
        //         // set phase mode to static 
        //         uint8_t cPhaseSelectMode = 0x00;
        //         uint32_t cReadBack = icRead(pInterface,  cReg , 1);
        //         uint32_t cWrite = (cReadBack & 0xc0) | ( (cPhaseSelectMode << 4) | (cPhaseSelectMode << 2) | (cPhaseSelectMode << 0)) ; 
        //         icWrite(pInterface, cReg, cWrite ) ;
        //         // reset phase
        //         for( size_t cIndex=0; cIndex < 7 ; cIndex++) 
        //         {
        //             uint16_t cChannelReg = 84 + 24*cIndex; 
        //             LOG (DEBUG) << BOLDBLUE << "Setting register " << cChannelReg << " to 0xFF" << RESET;
        //             icWrite(pInterface, cChannelReg , 0xFF ) ;
        //             icWrite(pInterface, cChannelReg+1 , 0xFF ) ;
        //             icWrite(pInterface, cChannelReg+2 , 0xFF ) ;
        //             cReadBack = icRead(pInterface,  cChannelReg , 1);
        //             LOG (DEBUG) << BOLDBLUE << "\t...register set to " << cChannelReg << " to 0x" << std::hex << cReadBack << std::dec << RESET;
        //             std::this_thread::sleep_for (std::chrono::milliseconds (100) );
        //             LOG (DEBUG) << BOLDBLUE << "Setting register " << cChannelReg << " to 0x00" << RESET;
        //             icWrite(pInterface, cChannelReg , 0x00 ) ;
        //             icWrite(pInterface, cChannelReg+1 , 0x00 ) ;
        //             icWrite(pInterface, cChannelReg+2 , 0x00 ) ;
        //         }
        //         // set phase 
        //         std::vector<uint16_t> cRegisters = { 66, 90, 114 , 138, 143 , 162 , 186, 210}; 
        //         for( auto cChannelReg : cRegisters ) 
        //         {
        //             for( size_t cIndex=0; cIndex < 12 ; cIndex++)
        //             {
        //                 icWrite(pInterface, cChannelReg+cIndex , (pPhase << 4) | ( pPhase << 0 ) ) ;
        //             }
        //         }
        //     };
        //     void gbtxConfigureChargePumps(BeBoardFWInterface* pInterface, uint8_t pStrength = 0x04) 
        //     {
        //         LOG (INFO) << BOLDBLUE << "Setting DLLs charge-pump control registers to " << std::dec << +pStrength << RESET;
        //         for( uint16_t cRegister = 16 ; cRegister < 24 ; cRegister ++ )
        //         {
        //             uint32_t cReadBack = icRead( pInterface, cRegister , 1); 
        //             icWrite(pInterface, cRegister, (cReadBack & 0xF0) |  (pStrength << 0) ) ;
        //         }
        //         LOG (INFO) << BOLDBLUE << "Configuring PLL ..." << RESET; 
        //         // Programing the phase-shifter channels’ frequency
		      //       uint32_t cReadBack = icRead( pInterface, 16 , 1); 
        //         icWrite(pInterface, 16, (cReadBack & 0xF0) |  (0x0F << 0) ) ;
		      //       // charge current + resistor for PLL
        //         cReadBack = icRead( pInterface, 26 , 1); 
        //         //icWrite(pInterface, 26, (cReadBack & 0x8F) | 0xF ) ;
        //         icWrite(pInterface, 26, (0x7 << 4 ) | 0xF ) ;
        //         // configure the power-up state machine for the pll watchdog 
        //         //cReadBack = icRead( pInterface, 52 , 1); //watchdog 
        //         //icWrite(pInterface, 52, (cReadBack & 0xC0) | (0x7 << 3) |  (0x7 << 0) ) ;
        //         //icWrite(pInterface, 52, (cReadBack & 0xC0) | (0x7 << 3) |  (0x0 << 0) ) ;
        //         cReadBack = icRead( pInterface, 52 , 1); //watchdog 
		      //       LOG (INFO) << BOLDBLUE << "Watchdog timeout set to " << std::bitset<8>(cReadBack) << RESET;  
        //     };
        //     // set clocks 
        //     void gbtxSetClocks(BeBoardFWInterface* pInterface , uint8_t pFrequency = 3 , uint8_t cDriveStrength = 0x0a , uint8_t cCoarsePhase = 0 , uint8_t cFinePhase = 0 )
        //     {
        //         LOG (INFO) << BOLDBLUE << "Set clock frequency on GBTx channels 0 to 7 to : " << 40*std::pow(2, (int)pFrequency) << " MHz." << RESET; 
        //         // Set frequencies to 320 MHz
        //         // was 0x30 here .. but is 0x34 in Christian's
        //         for( uint16_t cRegister = 16 ; cRegister < 24 ; cRegister ++ )
        //         {
        //             uint32_t cReadBack = icRead( pInterface, cRegister , 1); 
        //             icWrite(pInterface, cRegister, (cReadBack & 0xCF) |  (pFrequency << 4) ) ;
        //         }
        //         // set drive strength
        //         for( uint16_t cRegister = 269 ; cRegister  < 274; cRegister ++ ) 
        //         {
        //             if( cRegister < 273 )
        //               icWrite(pInterface, cRegister , (cDriveStrength << 4) | (cDriveStrength << 0) ) ; // set all channels to the same value ... each register holds two channels 
        //             else
        //             {
        //               // for ec channel 
        //               uint32_t cReadBack = icRead(pInterface,  cRegister , 1);
        //               icWrite(pInterface, cRegister , (cReadBack & 0xF0 ) | (cDriveStrength << 0) | ( 1 << 5 ) ) ; 
        //             }
        //         }
        //         // read back drive strength
        //         for( uint16_t cRegister = 269 ; cRegister  < 273; cRegister ++ ) 
        //         {
        //             uint32_t cReadBack = icRead(pInterface,  cRegister , 1);
        //             LOG (DEBUG) << BOLDBLUE << "Drive strength register " << std::hex << cRegister << std::dec << " set to 0x" << std::hex << +cReadBack << " in GBTx" << RESET;
        //         }
        //         // configure coarse+fine phase 
        //         // coarse phase 
        //         for( uint16_t cRegister = 8 ; cRegister < 16; cRegister++)
        //         {
        //             uint32_t cReadBack = icRead(pInterface,  cRegister , 1);
        //             icWrite( pInterface, cRegister , (cCoarsePhase << 5) | (cReadBack & 0xE0) );
        //         }
        //         // fine phase 
        //         for( uint16_t cRegister = 4 ; cRegister < 8; cRegister++)
        //         {
        //             icWrite( pInterface, cRegister , (cFinePhase << 4) | (cFinePhase << 0 ) );
        //         }
        //     };
        //     // read clocks 
        //     void gbtxReadClocks(BeBoardFWInterface* pInterface) 
        //     {
        //         std::vector<uint16_t> cRegisters = {16, 17, 18, 19, 20, 21, 22, 23, 26, 52};
        //         for(auto cRegister : cRegisters) 
        //         {
        //             uint32_t cReadBack = icRead(pInterface,  cRegister , 1);
        //             LOG (DEBUG) << BOLDBLUE << "Clock Register 0x" << std::hex << cRegister << std::dec << " set to 0x" << std::hex << +cReadBack << " in GBTx" << RESET;
        //         }
        //     };
        //     // reset clock plls
        //     void gbtxResetPhaseShifterClocks(BeBoardFWInterface* pInterface )
        //     {
        //         // first .. reset PLLs
        //         LOG (INFO) << BOLDBLUE << "Resetting PLLs on GBTx.." << RESET;
        //         icWrite( pInterface, 25, 0x00); 
        //         icWrite( pInterface, 25, 0x01);
        //         // wait for 10 us 
        //         std::this_thread::sleep_for (std::chrono::microseconds (10) );
        //         // reset DLLs
        //         LOG (INFO) << BOLDBLUE << "Resetting DLLs on GBTx.." << RESET;
        //         icWrite( pInterface, 24, 0x00); 
        //         icWrite( pInterface, 24, 0xFF); 
        //         // wait for more than 50 us
        //         std::this_thread::sleep_for (std::chrono::microseconds (100) );
        //     };
        //     // configure e-links 
        //     void gbtxConfigureTxMode(BeBoardFWInterface* pInterface, std::vector<uint8_t> cGroups , uint8_t pDataRate = 0 ) // 0 -- disable, 1 -- 80 Mb/s , 2 -- 160 Mbps , 3 -- 320 Mbps
        //     {
        //         // Registers holding the clock rates for the e-link tx
        //         for(auto cGroup : cGroups)// uint8_t cGroup = 0 ; cGroup < 5 ; cGroup++)
        //         {
        //             uint16_t cRegister = 254 + cGroup*3;
        //             LOG (DEBUG) << BOLDBLUE << "Setting all output e-links clock rates for group " << +cGroup << " to 0x" << std::hex << +pDataRate << std::dec << RESET;
        //             std::vector<uint16_t> cOffsets = { 0 , 78 , 15 }; 
        //             for( size_t cIndex2 = 0 ; cIndex2 < cOffsets.size(); cIndex2++)
        //             {
        //                 cRegister += cOffsets[cIndex2]; 
        //                 // 0 - disable, 1 - 80, 2 - 160, 3 = 320
        //                 uint32_t cReadBack = icRead(pInterface,  cRegister , 1);
        //                 uint8_t cValue = (cReadBack & 0xFC) | ( pDataRate << 0);
        //                 icWrite(pInterface, cRegister , cValue);
        //             }
        //         }
        //     };
        //     void gbtxConfigureTxClocks(BeBoardFWInterface* pInterface, std::vector<uint8_t> cGroups , uint8_t pDataRate = 0 ) // 0 -- disable, 1 -- 80 Mb/s , 2 -- 160 Mbps , 3 -- 320 Mbps
        //     {
        //         // Registers holding the clock rates for the e-link tx
        //         for( auto cGroup : cGroups)
        //         {
        //             uint16_t cRegister = 254 + cGroup*3;
        //             LOG (DEBUG) << BOLDBLUE << "Setting all output e-links clock rates for group " << +cGroup << " to 0x" << std::hex << +pDataRate << std::dec << RESET;
        //             std::vector<uint16_t> cOffsets = { 0 , 78 , 15 }; 
        //             for( size_t cIndex = 0 ; cIndex < cOffsets.size(); cIndex++)
        //             {
        //                 cRegister += cOffsets[cIndex]; 
        //                 // 0 - 40, 1 - 80, 2 - 160, 3 = 320
        //                 uint32_t cReadBack = icRead(pInterface,  cRegister , 1);
        //                 uint8_t cValue = (cReadBack & 0xF3) | ( pDataRate << 2);
        //                 icWrite(pInterface, cRegister , cValue);
        //             }
        //         }
        //     };
        //     void gbtxConfigureRxClocks(BeBoardFWInterface* pInterface, std::vector<uint8_t> cGroups, uint8_t pDataRate = 0 ) // 0 -- disable, 1 -- 80 Mb/s , 2 -- 160 Mbps , 3 -- 320 Mbps
        //     {
        //         // Registers holding the clock rate for the e-link rx
        //         for( auto cGroup : cGroups)
        //         {
        //             uint16_t cRegister = 63 + cGroup*24;
        //             LOG (DEBUG) << BOLDBLUE << "Setting all input e-link clock rates for group " << +cGroup << " [register " << +cRegister << " ] to 0x00" << RESET;
        //             icWrite(pInterface, cRegister , (pDataRate << 4) | ( pDataRate << 2 ) | (pDataRate << 0 ) ); 
        //         }
        //     };
        //     void gbtxEnableTxChannel( BeBoardFWInterface* pInterface, uint8_t pGroup , std::vector<uint8_t> pChannels ) 
        //     {
        //         uint16_t cRegister = 256 + pGroup*3;
        //         LOG (DEBUG) << BOLDBLUE << "Enabling  output e-links for group " << +pGroup << RESET;
        //         std::vector<uint16_t> cOffsets = { 0 , 78 , 15 }; 
        //         for( size_t cIndex2 = 0 ; cIndex2 < cOffsets.size(); cIndex2++)
        //         {
        //             cRegister += cOffsets[cIndex2]; 
        //             uint8_t cValue=0; 
        //             for(auto cChannel : pChannels ) 
        //             {
        //                 LOG (DEBUG) << BOLDBLUE << "\t.. Register " << cRegister << " used to enable Tx channel " << +cChannel << RESET;
        //                 cValue += (1 << cChannel);
        //             }
        //             icWrite(pInterface, cRegister, cValue );
        //         }
        //     };
        //     void gbtxEnableRxChannel( BeBoardFWInterface* pInterface, uint8_t pGroup, std::vector<uint8_t> pChannels ) 
        //     {
        //         LOG (DEBUG) << BOLDBLUE << "Enabling input e-links for group " << +pGroup << RESET;
        //         for( size_t cIndex=0; cIndex < 3 ; cIndex++)
        //         {
        //             uint16_t cRegister = 81 + pGroup*24 + cIndex;
        //             LOG (DEBUG) << BOLDBLUE << "\t... Register " << cRegister << RESET;
        //             uint8_t cValue=0; 
        //             for(auto cChannel : pChannels )
        //             {
        //                 cValue += (1 << cChannel);
        //                 LOG (DEBUG) << BOLDBLUE << "\t.. Register " << cRegister << " used to enable Rx channel " << +cChannel << RESET;
        //             }
        //             icWrite(pInterface, cRegister, cValue );
        //         }
        //     };
        //     void gbtxConfigureLinks(BeBoardFWInterface* pInterface )
        //     {
        //         // set all clocks to 40 MHz [ Tx] 
        //         std::vector<uint8_t> cTxGroups = { 0, 1 ,2, 3, 4};
        //         gbtxConfigureTxClocks( pInterface, cTxGroups, 3) ; 
        //         // enable Fast command links [Tx] for left and right hand side
        //         // hybrids 
        //         std::vector<uint8_t> cTxHybrid = { 2, 3 }; 
        //         gbtxConfigureTxMode( pInterface, cTxHybrid, 3);
        //         gbtxEnableTxChannel( pInterface, 2 , {4} ); 
        //         gbtxEnableTxChannel( pInterface, 3 , {0} ); 
        //         std::vector<uint8_t> cRxGroups  = { 0, 1 ,2 , 3 , 4, 5, 6, 7};
        //         gbtxConfigureRxClocks(pInterface, cRxGroups, 3 ); // 0 -- disable, 1 -- 80 Mb/s , 2 -- 160 Mbps , 3 -- 320 Mbps   
        //         gbtxEnableRxChannel( pInterface, 0 , {0, 4} ); 
        //         gbtxEnableRxChannel( pInterface, 1 , {4} ); 
        //         gbtxEnableRxChannel( pInterface, 2 , {0, 4} ); 
        //         gbtxEnableRxChannel( pInterface, 3 , {0, 4} ); 
        //         gbtxEnableRxChannel( pInterface, 4 , {0, 4} ); 
        //         gbtxEnableRxChannel( pInterface, 5 , {1, 4} ); 
        //         gbtxEnableRxChannel( pInterface, 6 , {0, 4} );  
        //     };
        //     void gbtxDisableAllLinks(BeBoardFWInterface* pInterface)
        //     {
        //         LOG (INFO) << BOLDBLUE << "Disabling all input/output groups on GBTx.." << RESET;
        //         // Registers holding the clock rates for the e-link tx
        //         std::vector<uint8_t> cTxGroups = { 0  , 1 , 2, 3, 4 };
        //         gbtxConfigureTxMode(pInterface, cTxGroups, 0 ); // 0 -- disable, 1 -- 80 Mb/s , 2 -- 160 Mbps , 3 -- 320 Mbps
        //         std::vector<uint8_t> cRxGroups  = { 0, 1 ,2 , 3 , 4, 5, 6, 7};
        //         gbtxConfigureRxClocks(pInterface, cRxGroups, 0 ); // 0 -- disable, 1 -- 80 Mb/s , 2 -- 160 Mbps , 3 -- 320 Mbps   
        //     };
        //     void gbtxResetFrameAlignerDLL(BeBoardFWInterface* pInterface, std::vector<uint8_t> pGroups ) 
        //     {
        //         // for e-links 
        //         for(auto cGroup : pGroups ) 
        //         {
        //             uint16_t cRegister = 65 + cGroup*24;
        //             uint32_t cReadBack = icRead(pInterface, cRegister , 1 );
        //             icWrite( pInterface, cRegister , ( cReadBack & 0x8F) | (0x7 << 4) );
        //             icWrite( pInterface, cRegister,  ( cReadBack & 0x8F) | (0x0 << 4) );
        //         }
        //         // then for EC port 
        //         uint16_t cRegister = 232;
        //         uint32_t cReadBack = icRead(pInterface, cRegister , 1 );
        //         icWrite( pInterface, cRegister , ( cReadBack & 0x8F) | (0x7 << 4) );
        //         icWrite( pInterface, cRegister , ( cReadBack & 0x8F) | (0x0 << 4) );
        //     };
        //     void gbtxFrameAlignerDLL(BeBoardFWInterface* pInterface, std::vector<uint8_t> pGroups, uint8_t pDLLcurrent = 11, uint8_t pLockMode = 7)
        //     {
        //         for(auto cGroup : pGroups ) 
        //         {
        //             uint16_t cRegister = 64 + cGroup*24;
        //             // registers A + B 
        //             icWrite(pInterface, cRegister , (pDLLcurrent << 4 ) | (pDLLcurrent << 0) );
        //             // register C 
        //             uint32_t cReadBack = icRead(pInterface, cRegister+1, 1);
        //             icWrite( pInterface, cRegister+1, (cReadBack & 0xF0) | pDLLcurrent );
        //         }
        //         uint32_t cReadBack = icRead(pInterface, 233 , 1);
        //         icWrite( pInterface, 233 , (cReadBack & 0x8F) | (pLockMode << 4) );
        //     };
        //     void gbtxConfigure(BeBoardFWInterface* pInterface, uint8_t pDLLcurrent=11 , uint8_t pDLLlockMode=7)
        //     {
        //         //disable everything
        //         gbtxDisableAllLinks(pInterface);
        //         gbtxConfigureLinks(pInterface);
        //         // Configure input frame aligner's DLLs
        //         gbtxFrameAlignerDLL(pInterface, {0,1,2,3,4,5,6}, pDLLcurrent, pDLLlockMode);
        //         // reset input frame aligner DLLs
        //         gbtxResetFrameAlignerDLL(pInterface, {0,1,2,3,4,5,6} );
        //     };
            
        //     // Temp replacement for new CPB 
        //     uint8_t configI2C( BeBoardFWInterface* pInterface, uint16_t  pMaster, uint8_t pNBytes=2, uint8_t pSclMode=0, int pFrequency=1000 ) 
        //     {
        //         LOG (DEBUG) << BOLDBLUE << "Configuring I2C [SCA] to read " << +pNBytes << " in SCL mode [ " << +pSclMode << "] at a frequency of " << pFrequency << RESET; 
        //         uint32_t cRegister = ( std::min(3, static_cast<int>(pFrequency/200)) << 0 | pNBytes<<2 | pSclMode<<7 )  << 3*8;
        //         uint32_t cErrorCode = ecWrite(pInterface, pMaster, 0x30, cRegister); 
        //         if( cErrorCode != 0 ) 
        //         {
        //             LOG (INFO) << BOLDBLUE << "SCA Error code : " << +cErrorCode << RESET;
        //             return cErrorCode; 
        //         }
        //         return cErrorCode;
        //     };
        //     uint32_t readI2C( BeBoardFWInterface* pInterface, uint16_t pMaster, uint8_t pSlave , uint8_t pNBytes)
        //     {
        //         configI2C( pInterface , pMaster, pNBytes);
        //         uint32_t pData = ecRead( pInterface, pMaster, (pNBytes==1) ? 0x86: 0xDE , (pSlave << 3*8) );
        //         return ( (pData & 0x00FF0000) >> 2*8 );
        //     };
        //     uint8_t writeI2C( BeBoardFWInterface* pInterface, uint16_t pMaster, uint8_t pSlave , uint32_t pData, uint8_t pNBytes)
        //     {
        //         configI2C( pInterface , pMaster, pNBytes);
        //         if( pNBytes == 1 )
        //         {
        //             uint32_t cData = (pSlave << 3*8) | (pData << 2*8);
        //             return ecWrite( pInterface, pMaster, 0x82 , cData) ;
        //         }
        //         else
        //         {
        //             //upload data bytes to send in the DATA register
        //             uint32_t cErrorCode = ecWrite( pInterface, pMaster , 0x40 , pData  );
        //             if( cErrorCode != 0 ) 
        //                 return cErrorCode;
        //             return ecWrite(pInterface, pMaster , 0xDA , (pSlave << 3*8) );
        //         }
        //     };
        //     uint8_t cbcGetPageRegister(BeBoardFWInterface* pInterface, uint8_t pFeId, uint8_t pChipId ) 
        //     {
        //         uint8_t cAddress = (0x40 | (pChipId+1) );
        //         uint8_t cErrorCode = writeI2C(pInterface, pSCAMaster + pFeId, cAddress  , 0x00 , 1); 
        //         if( cErrorCode == 0 )
        //         {
        //             uint8_t cValue =  readI2C( pInterface , pSCAMaster + pFeId , cAddress , 1);
        //             LOG (DEBUG) << BOLDGREEN << "\t... page is currently set to " << std::hex << +cValue << std::dec << RESET;
        //             return cValue;
        //         }
        //         LOG (INFO) << BOLDYELLOW << "Error reading CBC page register." << RESET;
        //         return cErrorCode;
        //     };
        //     uint8_t cbcSetPage(BeBoardFWInterface* pInterface, uint8_t pFeId, uint8_t pChipId, uint8_t pPage ) 
        //     {
        //         LOG (DEBUG) << BOLDGREEN << "\t... setting page to " << +pPage << RESET;
        //         uint8_t cPageRegister = cbcGetPageRegister(pInterface, pFeId, pChipId);
        //         uint8_t cNewRegValue = ( (~pPage & 0x01) << 7) | ( cPageRegister & 0x7F);
        //         LOG (DEBUG) << BOLDGREEN << "\t... setting page register to 0x" << std::hex << std::bitset<8>(+cNewRegValue) << std::dec << RESET;
        //         //uint32_t cValue  = (0x00 << 8*3) | (cNewRegValue << 8*2)  ;
        //         return writeI2C(pInterface, pSCAMaster + pFeId, 0x40 | (1+pChipId) , (0x00 << 8*3) | ( cNewRegValue << 8*2) , 2); 
        //     };
        //     uint32_t cbcRead(BeBoardFWInterface* pInterface, uint8_t pFeId, uint8_t pChipId, uint8_t pPage , uint8_t pRegisterAddress ) 
        //     {
        //         uint8_t cErrorCode = cbcSetPage(pInterface, pFeId, pChipId, pPage) ; 
        //         if( cErrorCode != 0 )
        //         { 
        //             LOG (INFO) << BOLDYELLOW << "Error setting CBC page register." << RESET;
        //             return cErrorCode; 
        //         }
        //         cErrorCode = writeI2C(pInterface, pSCAMaster + pFeId, 0x40 | (1 + pChipId), pRegisterAddress, 1);
        //         if( cErrorCode != 0 ) 
        //             return cErrorCode; 
        //         uint32_t cValue =  readI2C( pInterface, pSCAMaster + pFeId, 0x40 | (1+ pChipId), 1); 
        //         LOG (DEBUG) << BOLDBLUE << "Read back 0x" << std::hex << cValue << std::dec << " when reading register 0x" << std::hex << +pRegisterAddress << std::dec << " from page " << +pPage << " on CBC" << +pChipId << RESET;
        //         return cValue;  
        //     };
        //     bool cbcWrite(BeBoardFWInterface* pInterface, uint8_t pFeId, uint8_t pChipId, uint8_t pPage , uint8_t pRegisterAddress , uint8_t pRegisterValue , bool pReadBack=false) 
        //     {
        //         uint8_t cErrorCode = cbcSetPage(pInterface, pFeId, pChipId, pPage) ; 
        //         if( cErrorCode != 0 )
        //         { 
        //             LOG (INFO) << BOLDYELLOW << "Error setting CBC page register." << RESET;
        //             return cErrorCode; 
        //         }
        //         cErrorCode = writeI2C(pInterface, pSCAMaster + pFeId ,  0x40 | (1 + pChipId),  (pRegisterAddress << 8*3) | (pRegisterValue << 8*2) , 2 );
        //         if( pReadBack )
        //             return (cbcRead(pInterface, pFeId, pChipId, pPage , pRegisterAddress) == pRegisterValue);
        //         else 
        //             return (cErrorCode == 0 );
        //     };
        //     uint32_t cicRead(BeBoardFWInterface* pInterface , uint8_t pFeId, uint8_t pRegisterAddress) 
        //     {
        //         writeI2C(pInterface, pSCAMaster + pFeId, 0x60, (pRegisterAddress << 16) , 2 );
        //         uint32_t cValue =  readI2C( pInterface, pSCAMaster + pFeId , 0x60, 1);
        //         LOG (DEBUG) << BOLDBLUE << "Read back 0x" << std::hex << cValue << std::dec << " when reading register 0x" << std::hex << +pRegisterAddress << std::dec << " on CIC on FE" << +pFeId << RESET;
        //         return cValue;
        //     };
        //     bool cicWrite(BeBoardFWInterface* pInterface, uint8_t pFeId, uint8_t pRegisterAddress, uint8_t pRegisterValue , bool pReadBack=false)
        //     {
        //         uint8_t cWrite = writeI2C(pInterface, pSCAMaster +pFeId, 0x60, (pRegisterAddress << 16) | ( pRegisterValue << 8) , 3 );
        //         if( pReadBack ) 
        //             return (cicRead(pInterface, pFeId, pRegisterAddress) == pRegisterValue);
        //         else 
        //             return (cWrite==0);
        //     };
            
        // }; 

        // phase tuning commands - d19c
        struct PhaseTuner
        {
            uint8_t fType;
            uint8_t fMode; 
            uint8_t fDelay;
            uint8_t fBitslip;
            uint8_t fDone;
            uint8_t fWordAlignmentFSMstate;
            uint8_t fPhaseAlignmentFSMstate;
            uint8_t fFSMstate;

            void ParseResult(uint32_t pReply )
            {
                fType = (pReply >> 24) & 0xF;
                if (fType == 0) 
                {
                    fMode = (pReply & 0x00003000) >> 12;
                    fDelay = (pReply & 0x000000F8) >> 3;
                    fBitslip = (pReply & 0x00000007) >> 0;
                }
                else if( fType == 1 )
                {
                    fDelay = (pReply & 0x00F80000) >> 19;
                    fBitslip = (pReply & 0x00070000) >> 16;
                    fDone = (pReply & 0x00008000) >> 15;
                    fWordAlignmentFSMstate = (pReply & 0x00000F00) >> 8;
                    fPhaseAlignmentFSMstate = (pReply & 0x0000000F) >> 0;
                }
                else if( fType == 6 ) 
                {
                    fFSMstate = (pReply & 0x000000FF) >> 0;
                }
            };
            uint8_t ParseStatus(BeBoardFWInterface* pInterface)
            {
                uint8_t cStatus=0;
                // read status
                uint32_t cReply = pInterface->ReadReg( "fc7_daq_stat.physical_interface_block.phase_tuning_reply" );
                ParseResult(cReply);

                if (fType == 0) 
                {
                    LOG(INFO) << "\t\t Mode: " << +fMode;
                    LOG(INFO) << "\t\t Manual Delay: " << +fDelay << ", Manual Bitslip: " << +fBitslip;
                    cStatus=1;
                }
                else if( fType == 1 )
                {
                    LOG(INFO) << "\t\t Done: " << +fDone << ", PA FSM: " << BOLDGREEN << fPhaseFSMStateMap[fPhaseAlignmentFSMstate] << RESET << ", WA FSM: " << BOLDGREEN << fWordFSMStateMap[fWordAlignmentFSMstate] << RESET;
                    LOG(INFO) << "\t\t Delay: " << +fDelay << ", Bitslip: " << +fBitslip;
                    cStatus=1;
                }
                else if( fType == 6 ) 
                {
                    LOG(INFO) << "\t\t Default FSM State: " << +fFSMstate;
                    cStatus=1;
                }
                else 
                    cStatus=0;
                return cStatus;
            };
            uint32_t fHybrid;
            uint32_t fChip; 
            uint32_t fLine; 
            void ConfigureInput(uint8_t pHybrid, uint8_t pChip, uint8_t pLine )
            {
                fHybrid = (pHybrid & 0xF) << 28;
                fChip = (pChip & 0xF) << 24;
                fLine = (pLine & 0xF) << 20;
            };
            uint32_t fCommand;
            void ConfigureCommandType( uint8_t pType )
            {
                fCommand = (pType & 0xF) << 16;
            };
            void SetLineMode(BeBoardFWInterface* pInterface, uint8_t pHybrid, uint8_t pChip, uint8_t pLine, uint8_t pMode=0, uint8_t pDelay = 0, uint8_t pBitSlip = 0, uint8_t pEnableL1 = 0, uint8_t pMasterLine = 0 )
            {
                // select FE
                ConfigureInput( pHybrid, pChip, pLine); 
                //command 
                uint32_t command_type = 2;
                ConfigureCommandType( command_type) ;
                // shift payload 
                uint32_t mode_raw = (pMode & 0x3) << 12;
                // set defaults
                uint32_t l1a_en_raw = (pMode == 0 ) ? ( (pEnableL1 & 0x1) << 11) : 0;
                uint32_t master_line_id_raw = (pMode == 1 ) ? ( (pMasterLine & 0xF) << 8 ) : 0 ;
                uint32_t delay_raw = (pMode == 2 ) ? ((pDelay & 0x1F) << 3) : 0;
                uint32_t bitslip_raw = (pMode == 2) ? ((pBitSlip & 0x7) << 0) : 0;
                // form command 
                uint32_t  command_final = fHybrid + fChip + fLine + fCommand  + mode_raw + l1a_en_raw + master_line_id_raw + delay_raw + bitslip_raw;
                LOG (DEBUG) << BOLDBLUE << "Line " << +pLine << " setting line mode to " << std::hex << command_final << std::dec << RESET;
                pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
            };
            void SetLinePattern(BeBoardFWInterface* pInterface,  uint8_t pHybrid, uint8_t pChip , uint8_t pLine, uint16_t pPattern, uint16_t pPatternPeriod)
            {
                // select FE
                ConfigureInput( pHybrid, pChip, pLine); 
                // set the pattern size 
                uint8_t command_type = 3;
                ConfigureCommandType( command_type) ;
                uint32_t len_raw = (0xFF & pPatternPeriod) << 0; 
                uint32_t command_final = fHybrid + fChip + fLine + fCommand  + len_raw; 
                LOG (DEBUG) << BOLDBLUE << "Setting line pattern size to " << std::hex << command_final << std::dec << RESET;
                pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
                // set the pattern 
                command_type = 4;
                ConfigureCommandType( command_type) ;
                uint8_t byte_id_raw = (0xFF & 0) << 8; 
                uint8_t pattern_raw = (0xFF & pPattern) << 0;
                command_final = fHybrid +  fChip + fLine + fCommand + byte_id_raw + pattern_raw;
                LOG (DEBUG) << BOLDBLUE << "Setting line pattern  to " << std::hex << command_final << std::dec << RESET;
                pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
            };
            void SendControl(BeBoardFWInterface* pInterface,  uint8_t pHybrid, uint8_t pChip , uint8_t pLine, std::string pCommand)
            {
                // select FE
                ConfigureInput( pHybrid, pChip, pLine); 
                // set the pattern size 
                uint8_t command_type = 5;
                ConfigureCommandType( command_type) ; 
                uint32_t command_final = fHybrid + fChip + fLine + fCommand ;             
                if( pCommand == "Apply" ) 
                    command_final += 4;
                else if(pCommand == "WordAlignment" )
                    command_final += 2; 
                else if( pCommand == "PhaseAlignment" ) 
                    command_final +=1;
                LOG (DEBUG) << BOLDBLUE << pCommand << ": sending "  << std::hex << command_final << std::dec << RESET;
                pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
            };
            uint8_t GetLineStatus( BeBoardFWInterface* pInterface, uint8_t pHybrid, uint8_t pChip , uint8_t pLine ) 
            {
                // select FE
                ConfigureInput( pHybrid, pChip, pLine); 
                // print header
                LOG(INFO) << BOLDBLUE << "\t Hybrid: " << RESET << +pHybrid << BOLDBLUE << ", Chip: " << RESET << +pChip << BOLDBLUE << ", Line: " << RESET << +pLine;
                uint8_t command_type = 0;
                ConfigureCommandType( command_type) ;
                uint32_t command_final = fHybrid + fChip + fLine + fCommand ;             
                pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
                std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                uint8_t cStatus = ParseStatus(pInterface);
                // 
                command_type = 1;
                ConfigureCommandType( command_type) ;
                command_final = fHybrid + fChip + fLine + fCommand ;
                pInterface->WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
                std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                cStatus = ParseStatus(pInterface);
                return cStatus;
            };
            bool TuneLine( BeBoardFWInterface* pInterface, uint8_t pHybrid, uint8_t pChip, uint8_t pLine , uint8_t pPattern, uint8_t pPatternPeriod, bool pChangePattern)
            {
                LOG (INFO) << BOLDBLUE << "Tuning line " << +pLine << RESET;
                if( pChangePattern ) 
                {
                    SetLineMode( pInterface, pHybrid, pChip, pLine ); 
                    std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                    SetLinePattern( pInterface, pHybrid, pChip, pLine , pPattern, pPatternPeriod);
                    std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                }
                // perform phase alignment 
                //LOG (INFO) << BOLDBLUE << "\t..... running phase alignment...." << RESET;
                SendControl(pInterface, pHybrid, pChip, pLine, "PhaseAlignment");
                std::this_thread::sleep_for (std::chrono::milliseconds (100) );
                // perform word alignment
                //LOG (INFO) << BOLDBLUE << "\t..... running word alignment...." << RESET;
                SendControl(pInterface, pHybrid, pChip, pLine, "WordAlignment"); 
                std::this_thread::sleep_for (std::chrono::milliseconds (100) );
                uint8_t cLineStatus = GetLineStatus(pInterface, pHybrid, pChip, pLine);
                return ( cLineStatus == 1);
            };


            // maps to decode status of word and phase alignment FSM 
            std::map<int, std::string> fPhaseFSMStateMap = {{0, "IdlePHASE"},
                                                        {1, "ResetIDELAYE"},
                                                        {2, "WaitResetIDELAYE"},
                                                        {3, "ApplyInitialDelay"},
                                                        {4, "CheckInitialDelay"},
                                                        {5, "InitialSampling"},
                                                        {6, "ProcessInitialSampling"},
                                                        {7, "ApplyDelay"},
                                                        {8, "CheckDelay"},
                                                        {9, "Sampling"},
                                                        {10, "ProcessSampling"},
                                                        {11, "WaitGoodDelay"},
                                                        {12, "FailedInitial"},
                                                        {13, "FailedToApplyDelay"},
                                                        {14, "TunedPHASE"},
                                                        {15, "Unknown"}
                                                       };
            std::map<int, std::string> fWordFSMStateMap = {{0, "IdleWORD or WaitIserdese"},
                                                        {1, "WaitFrame"},
                                                        {2, "ApplyBitslip"},
                                                        {3, "WaitBitslip"},
                                                        {4, "PatternVerification"},
                                                        {5, "Not Defined"},
                                                        {6, "Not Defined"},
                                                        {7, "Not Defined"},
                                                        {8, "Not Defined"},
                                                        {9, "Not Defined"},
                                                        {10, "Not Defined"},
                                                        {11, "Not Defined"},
                                                        {12, "FailedFrame"},
                                                        {13, "FailedVerification"},
                                                        {14, "TunedWORD"},
                                                        {15, "Unknown"}
                                                       };
        };
        
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
        void PSInterfaceBoard_PowerOn_SSA(float VDDPST = 1.25, float DVDD = 1.25, float AVDD = 1.25, float VBF = 0.3, float BG = 0.0, uint8_t ENABLE = 0);
        void PSInterfaceBoard_PowerOff_SSA(uint8_t mpaid = 0 , uint8_t ssaid = 0 );
        void ReadPower_SSA(uint8_t mpaid = 0 , uint8_t ssaid = 0);
	void SSAEqualizeDACs(uint8_t pChipId);
        void KillI2C();
        ///

        void Pix_write_MPA(Ph2_HwDescription::MPA* cMPA,Ph2_HwDescription::ChipRegItem cRegItem,uint32_t row,uint32_t pixel,uint32_t data);
        uint32_t Pix_read_MPA(Ph2_HwDescription::MPA* cMPA,Ph2_HwDescription::ChipRegItem cRegItem,uint32_t row,uint32_t pixel);
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

        ///////////////////////////////////////////////////////
        //      Optical readout                                 //
        /////////////////////////////////////////////////////
        void selectLink(uint8_t pLinkId=0 , uint32_t cWait_ms = 100) override;

    };
}

#endif
