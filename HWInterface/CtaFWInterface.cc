/*

        FileName :                    CtaFWInterface.h
        Content :                     CtaFWInterface init/config of the CTA and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "CtaFWInterface.h"
#include "CtaFpgaConfig.h"

using namespace std;

namespace Ph2_HwInterface {

    CtaFWInterface::CtaFWInterface ( const char* puHalConfigFileName,
                                     uint32_t pBoardId ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId )
        //fpgaConfig ( nullptr ),
        //fData ( nullptr ),
        //fNthAcq (0)
    {
        fpgaConfig = nullptr;
        fData = nullptr;
        fNthAcq = 0;
    }


    CtaFWInterface::CtaFWInterface ( const char* puHalConfigFileName,
                                     uint32_t pBoardId,
                                     FileHandler* pFileHandler ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fFileHandler ( pFileHandler ),
        fNthAcq (0)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    CtaFWInterface::CtaFWInterface ( const char* pId,
                                     const char* pUri,
                                     const char* pAddressTable ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fNthAcq (0)
    {
    }


    CtaFWInterface::CtaFWInterface ( const char* pId,
                                     const char* pUri,
                                     const char* pAddressTable,
                                     FileHandler* pFileHandler ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fFileHandler ( pFileHandler ),
        fNthAcq (0)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }


    void CtaFWInterface::setFileHandler (FileHandler* pHandler)
    {
        if (pHandler != nullptr )
        {
            fFileHandler = pHandler;
            fSaveToFile = true;
        }
        else LOG (INFO) << "Error, can not set NULL FileHandler" ;
    }

    uint32_t CtaFWInterface::getBoardInfo()
    {
        //LOG(INFO) << "FMC1 present : " << ReadReg ( "status.fmc1_present" ) ;
        //LOG(INFO) << "FMC2 present : " << ReadReg ( "status.fmc2_present" ) ;
        uint32_t cVersionMajor = ReadReg ( "firm_id.firmware_major" );
        uint32_t cVersionMinor = ReadReg ( "firm_id.firmware_minor" );
        LOG (INFO) << "FW version : " << cVersionMajor << "." << cVersionMinor << "." << std::to_string (ReadReg ( "firm_id.firmware_build" ) ) ;

        uhal::ValWord<uint32_t> cBoardType = ReadReg ( "board_id" );

        LOG (INFO) << "BoardType : ";

        char cChar = ( ( cBoardType & cMask4 ) >> 24 );
        LOG (INFO) << cChar;

        cChar = ( ( cBoardType & cMask3 ) >> 16 );
        LOG (INFO) << cChar;

        cChar = ( ( cBoardType & cMask2 ) >> 8 );
        LOG (INFO) << cChar;

        cChar = ( cBoardType & cMask1 );
        LOG (INFO) << cChar ;

        //LOG(INFO) << "FMC User Board ID : " << ReadReg ( "user_wb_ttc_fmc_regs.user_board_id" ) ;
        //LOG(INFO) << "FMC User System ID : " << ReadReg ( "user_wb_ttc_fmc_regs.user_sys_id" ) ;
        //LOG(INFO) << "FMC User Version : " << ReadReg ( "user_wb_ttc_fmc_regs.user_version" ) ;

        uint32_t cVersionWord = ( (cVersionMajor & 0x0000FFFF) << 16 || (cVersionMinor & 0x0000FFFF) );
        return cVersionWord;
    }


    void CtaFWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        std::chrono::milliseconds cPause ( 200 );

        //Primary Configuration
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );
        cVecReg.push_back ( {"pc_commands.SRAM1_end_readout", 0} );
        cVecReg.push_back ( {"pc_commands.SRAM2_end_readout", 0} );

        // iterate the BeBoardRegMap to get the user configuration
        BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

        for ( auto const& it : cGlibRegMap )
            cVecReg.push_back ( {it.first, it.second} );

        cVecReg.push_back ( {"pc_commands.force_BG0_start", 0} );
        WriteStackReg ( cVecReg );
        cVecReg.clear();

        this->CbcHardReset();
        usleep (10);
        //cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        //WriteStackReg ( cVecReg );
        //cVecReg.clear();
    }


    void CtaFWInterface::Start()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        //Starting the DAQ
        WriteReg ( "break_trigger", 0 );
        WriteReg ( "pc_commands.PC_config_ok", 1 );
        WriteReg ( "pc_commands.force_BG0_start", 1 );

        //WriteStackReg ( cVecReg );
        cVecReg.clear();

        fNthAcq = 0;
        // Since the Number of  Packets is a FW register, it should be read from the Settings Table which is one less than is actually read
        fNpackets = ReadReg ( "pc_commands.CBC_DATA_PACKET_NUMBER" ) + 1 ;
        //fBlockSize = 0;
        //Wait for start acknowledge
        uhal::ValWord<uint32_t> cVal;
        std::chrono::milliseconds cWait ( 100 );

        /*do
        {
            cVal = ReadReg ( "status_flags.CMD_START_VALID" );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );*/
    }

    void CtaFWInterface::Stop()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        uhal::ValWord<uint32_t> cVal;

        //Select SRAM
        SelectDaqSRAM();
        //Stop the DAQ
        //sure this should not be 1? Like GlibFWInterface
        cVecReg.push_back ( {"break_trigger", 0} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );
        cVecReg.push_back ( {"pc_commands.force_BG0_start", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        std::chrono::milliseconds cWait ( 100 );

        /*//Wait for the selected SRAM to be full then empty it
        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 1 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 1 );
        */
        //WriteReg ( fStrReadout, 0 );
        WriteReg ("pc_commands.SRAM1_end_readout", 0);
        WriteReg ("pc_commands.SRAM2_end_readout", 0);
        //if (fFileHandler && fFileHandler->file_open() ) fFileHandler->closeFile();
    }

    void CtaFWInterface::SafeStop (BeBoard* pBoard)
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        uhal::ValWord<uint32_t> cVal;

        //Select SRAM
        SelectDaqSRAM();
        //Stop the DAQ
        cVecReg.push_back ( {"break_trigger", 1} );
        cVecReg.push_back ( {"pc_commands.force_BG0_start", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        std::chrono::milliseconds cWait ( 100 );

        //FIFO goes to write_data state
        //Select SRAM
        SelectDaqSRAM();

        if ( pBoard )
            fBlockSize = computeBlockSize ( pBoard );

        do  //Wait for the SRAM full condition.
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        uint32_t nbEvtPacket = fNpackets;
        uint32_t nbBlockSize = fBlockSize;
        std::vector<uint32_t> cData;

        nbEvtPacket = fNpackets - ReadReg (fStrEvtCounter);
        nbBlockSize = fBlockSize / fNpackets * nbEvtPacket;

        //Read SRAM
        if (nbBlockSize > 0)
            cData =  ReadBlockRegValue ( fStrSram, nbBlockSize );

        std::this_thread::sleep_for ( 10 * cWait );
        WriteReg ( fStrReadout, 1 );
        std::this_thread::sleep_for ( 10 * cWait );
        WriteReg ("pc_commands.PC_config_ok", 0 );

        //now I did an acquistion, so I need to increment the counter
        fNthAcq++;

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        if (nbEvtPacket > 0)       // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        {
            fData->Set ( pBoard, cData , nbEvtPacket, false );

            if ( fSaveToFile )
            {
                fFileHandler->set ( cData );
                fFileHandler->writeFile();
            }
        }

        //WriteReg ( fStrReadout, 0 );
        WriteReg ("pc_commands.SRAM1_end_readout", 0);
        WriteReg ("pc_commands.SRAM2_end_readout", 0);

    }


    void CtaFWInterface::Pause()
    {
        fJustPaused = true;
        WriteReg ( "break_trigger", 1 );
        //std::this_thread::sleep_for ( std::chrono::milliseconds(10) );

        //uhal::ValWord<uint32_t> cVal= ReadReg("EVENT_COUNTER_CLEARED");
        //cout<<"Event counter cleared: "<<cVal.value()<<endl;
    }


    void CtaFWInterface::Resume()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        cVecReg.push_back ( {"break_trigger", 0} );
        WriteStackReg ( cVecReg );
    }

    uint32_t CtaFWInterface::ReadData ( BeBoard* pBoard,  bool pBreakTrigger )
    {
    	std::vector<uint32_t> cData;
    	return ReadData ( pBoard,  pBreakTrigger, cData);
    }

    uint32_t CtaFWInterface::ReadData ( BeBoard* pBoard,  bool pBreakTrigger, std::vector<uint32_t>& cData)
    {
        //Readout settings
        std::chrono::milliseconds cWait ( 1 );

        uhal::ValWord<uint32_t> cVal;

        if ( pBoard )
            fBlockSize = computeBlockSize ( pBoard );

        //FIFO goes to write_data state
        //Select SRAM
        SelectDaqSRAM();


        do  //Wait for the SRAM full condition.
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        //break trigger
        if ( pBreakTrigger ) WriteReg ( "break_trigger", 1 );

        uint32_t nbEvtPacket = fNpackets;
        uint32_t nbBlockSize = fBlockSize;
        //std::vector<uint32_t> cData;

        if (fJustPaused)
        {
            fJustPaused = false;
            nbEvtPacket = fNpackets - ReadReg (fStrEvtCounter);
            nbBlockSize = fBlockSize / fNpackets * nbEvtPacket;
        }

        //Set read mode to SRAM
        //WriteReg ( fStrSramUserLogic, 0 );

        //Read SRAM
        if (nbBlockSize > 0)
            cData =  ReadBlockRegValue ( fStrSram, nbBlockSize );

        std::this_thread::sleep_for ( 10 * cWait );
        //WriteReg ( fStrSramUserLogic, 1 );
        WriteReg ( fStrReadout, 1 );
        std::this_thread::sleep_for ( 10 * cWait );

        //now I did an acquistion, so I need to increment the counter
        fNthAcq++;

        //Wait for the non SRAM full condition starts,
        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 1 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 1 );

        //Wait for the non SRAM full condition ends.

        WriteReg ( fStrReadout, 0 );
        WriteReg ( "pc_commands.force_BG0_start", 0 );

        if ( pBreakTrigger ) WriteReg ( "break_trigger", 0 );

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        if (nbEvtPacket > 0)       // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        {
            fData->Set ( pBoard, cData , nbEvtPacket, false );

            if ( fSaveToFile )
            {
                fFileHandler->set ( cData );
                fFileHandler->writeFile();
            }
        }

        return nbEvtPacket;
    }

    void CtaFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        fNpackets = pNEvents;
        //Starting the DAQ
        cVecReg.push_back ( {"pc_commands.CBC_DATA_PACKET_NUMBER", pNEvents - 1} );
        cVecReg.push_back ( {"break_trigger", 0} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        cVecReg.push_back ( {"pc_commands.force_BG0_start", 1} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();


        //Wait for start acknowledge
        uhal::ValWord<uint32_t> cVal;
        std::chrono::milliseconds cWait ( 100 );

        do
        {
            cVal = ReadReg ( "status_flags.CMD_START_VALID" );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        if ( pBoard )
            fBlockSize = computeBlockSize ( pBoard );

        //Select SRAM
        SelectDaqSRAM();

        //Wait for the SRAM full condition.
        cVal = ReadReg ( fStrFull );

        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 0 )
                std::this_thread::sleep_for ( cWait );
        }
        while ( cVal == 0 );

        //break trigger
        cVecReg.push_back ({ "break_trigger", 0 } );
        cVecReg.push_back ( {"pc_commands.force_BG0_start", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //Set read mode to SRAM
        //WriteReg ( fStrSramUserLogic, 0 );

        //Read SRAM
        std::vector<uint32_t> cData =  ReadBlockRegValue ( fStrSram, fBlockSize );

        //WriteReg ( fStrSramUserLogic, 1 );

        //need to increment the internal Acquisition counter
        fNthAcq++;

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, cData , fNpackets, false );

        if ( fSaveToFile )
        {
            fFileHandler->set ( cData );
            fFileHandler->writeFile();
        }

        WriteReg ( "pc_commands.PC_config_ok", 0 );
    }


    /** compute the block size according to the number of CBC's on this board
     * this will have to change with a more generic FW */
    uint32_t CtaFWInterface::computeBlockSize ( BeBoard* pBoard )
    {
        //use a counting visitor to find out the number of CBCs
        struct CbcCounter : public HwDescriptionVisitor
        {
            uint32_t fNCbc = 0;

            void visit ( Cbc& pCbc )
            {
                fNCbc++;
            }
            uint32_t getNCbc()
            {
                if ( fNCbc == 2 )
                    // since the 2 CBC FW outputs data for 4 CBCs (beamtest heritage, might have to change in the future)
                    return 2 * fNCbc;
                else return fNCbc;
            }
        };

        CbcCounter cCounter;
        pBoard->accept ( cCounter );


        uint32_t cEvtSize = 0;

        if ( pBoard->getNCbcDataSize() != 0 )
            cEvtSize = std::max (pBoard->getNCbcDataSize(), (uint16_t) 4) * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ;
        else
            cEvtSize = std::max (cCounter.getNCbc()   , (uint32_t) 4) * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ; // in 32 bit words

        return cEvtSize * fNpackets;
    }

    std::vector<uint32_t> CtaFWInterface::ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();

        return vBlock;
    }

    bool CtaFWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );

        return cWriteCorr;
    }

    void CtaFWInterface::SelectDaqSRAM()
    {
        fStrSram  = ( fNthAcq % 2 == 0 ? "sram1" : "sram2" );
        //fStrSramUserLogic = ( ( fNthAcq % 2 + 1 ) == 1 ? "ctrl_sram.sram1_user_logic" : "ctrl_sram.sram2_user_logic" );
        fStrFull = ( fNthAcq % 2  == 0 ? "flags.SRAM1_full" : "flags.SRAM2_full" );
        fStrReadout = ( fNthAcq % 2  == 0 ? "pc_commands.SRAM1_end_readout" : "pc_commands.SRAM2_end_readout" );
        fStrEvtCounter  = ( fNthAcq % 2 == 0 ? "event_counter_SRAM1" : "event_counter_SRAM2" );
    }



    //Methods for Cbc's:

    //void CtaFWInterface::StartThread ( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor )
    //{
    //if ( runningAcquisition ) return;

    //runningAcquisition = true;
    //numAcq = 0;
    //nbMaxAcq = uNbAcq;

    //thrAcq = boost::thread ( &Ph2_HwInterface::CtaFWInterface::threadAcquisitionLoop, this, pBoard, visitor );
    //}

    //void CtaFWInterface::threadAcquisitionLoop ( BeBoard* pBoard, HwInterfaceVisitor* visitor )
    //{
    //Start( );
    //fBlockSize = computeBlockSize ( pBoard );

    //while ( runningAcquisition && ( nbMaxAcq == 0 || numAcq < nbMaxAcq ) )
    //{
    //ReadData ( pBoard, true );

    //for ( const Ph2_HwInterface::Event* cEvent = GetNextEvent ( pBoard ); cEvent; cEvent = GetNextEvent ( pBoard ) )
    //visitor->visit ( *cEvent );

    //if ( runningAcquisition )
    //numAcq++;

    //}

    //Stop ( );
    //runningAcquisition = false;
    //};

    ///////////////////////////////////////////////////////
    //      CBC Methods                                 //
    /////////////////////////////////////////////////////

    void CtaFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                     uint8_t pCbcId,
                                     std::vector<uint32_t>& pVecReq,
                                     bool pRead,
                                     bool pWrite )
    {
        uint8_t uValue = pRegItem.fAddress == 0 ? pRegItem.fValue & 0x7F : pRegItem.fValue;
        // temporary for 16CBC readout FW  (Beamtest NOV 15)
        // will have to be corrected if we want to read two modules from the same GLIB
        // (pCbcId >> 3) becomes FE ID and is encoded starting from bit21 (not used so far)
        // (pCbcId & 7) restarts CbcIDs from 0 for FE 1 (if CbcID > 7)
        pVecReq.push_back ( ( pCbcId + 0x41 ) << 21 | ( pCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | uValue );
    }

    void CtaFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                     uint8_t pFeId,
                                     uint8_t pCbcId,
                                     std::vector<uint32_t>& pVecReq,
                                     bool pRead,
                                     bool pWrite )
    {
        // (pCbcId & 7) restarts CbcIDs from 0 for FE 1 (if CbcID > 7)
        uint8_t uValue = pRegItem.fAddress == 0 ? pRegItem.fValue & 0x7F : pRegItem.fValue;
        pVecReq.push_back ( ( pCbcId + 0x41 ) << 21 | pCbcId << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | uValue );
    }

    void CtaFWInterface::BCEncodeReg ( const CbcRegItem& pRegItem,
                                       uint8_t pNCbc,
                                       std::vector<uint32_t>& pVecReq,
                                       bool pRead,
                                       bool pWrite )
    {
        uint8_t uValue = pRegItem.fAddress == 0 ? pRegItem.fValue & 0x7F : pRegItem.fValue;

        // here I need to loop over all CBCs somehow...
        for (uint8_t cCbcId = 0; cCbcId < pNCbc; cCbcId++)
            pVecReq.push_back ( ( cCbcId + 0x41 ) << 21 | ( cCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | uValue );
    }

    void CtaFWInterface::DecodeReg ( CbcRegItem& pRegItem,
                                     uint8_t& pCbcId,
                                     uint32_t pWord,
                                     bool& pRead,
                                     bool& pFailed )
    {
        // temporary for 16CBC readout FW  (Beamtest NOV 15)
        // will have to be corrected if we want to read two modules from the same GLIB
        uint8_t cbcAddr = ( pWord & cMask7 ) >> 21;
        pCbcId = ( ( pWord & cMask5 )  ) >> 17;
        pRegItem.fPage = ( pWord & cMask6 ) >> 16;
        pRegItem.fAddress = ( pWord & cMask2 ) >> 8;
        pRegItem.fValue = pWord & cMask1;
        //LOG(INFO) << "FEID " << +(cbcAddr) << " pCbcID " << +(pCbcId) ;
    }

    bool CtaFWInterface::I2cCmdAckWait (  bool bZero, uint8_t pNcount )
    {
        unsigned int cWait ( 100 );

        if ( bZero )
            cWait = pNcount * 500;

        usleep ( cWait );

        uhal::ValWord<uint32_t> cVal;
        uint32_t cLoop = 0;

        do
        {
            cVal = ReadReg ( "cbc_i2c_cmd_ack" );

            if ( bZero )
            {
                if (cVal == 0 )
                    usleep ( cWait );
                else if (cVal == 0b01)
                    return true;
                else
                    throw Exception ( " CbcInterface::I2cCmdAckWait bad acknowledge value" );
            }
            else
            {
                if (cVal != 0)
                    usleep ( cWait );
                else
                    return true;
            }
        }
        while ( cVal == 0 && ++cLoop < 70 );

        return false;
    }

    void CtaFWInterface::WriteI2C ( std::vector<uint32_t>& pVecReq, bool pWrite )
    {
        //pVecReq.push_back ( 0xFFFFFFFF );

        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        //WriteReg ( "ctrl_sram.sram1_user_logic", 0 );
        WriteBlockReg ( "cbc_config_fifo_tx_FE0", pVecReq );

        //cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"cbc_i2c_cmd_rq", pWrite ? 1 : 3} );
        WriteStackReg ( cVecReg );

        if ( I2cCmdAckWait ( true, pVecReq.size() ) == false)
            throw Exception ( " CbcInterface::I2cCmdAckWait not 0 failed." );

        WriteReg ( "cbc_i2c_cmd_rq", 0 );

        if ( I2cCmdAckWait ( false, pVecReq.size() ) == false)
            throw Exception ( " CbcInterface::I2cCmdAckWait 0 failed." );
    }

    void CtaFWInterface::ReadI2C ( std::vector<uint32_t>& pVecReq )
    {
        //WriteReg ( "ctrl_sram.sram1_user_logic", 0 );
        pVecReq = ReadBlockRegValue ( "cbc_config_fifo_rx_FE0", pVecReq.size() );
        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        //cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"cbc_i2c_cmd_rq", 0} );
        WriteStackReg ( cVecReg );
    }


    bool CtaFWInterface::WriteCbcBlockReg (  std::vector<uint32_t>& pVecReq, uint8_t& pWriteAttempts , bool pReadback)
    {
        int cMaxWriteAttempts = 5;
        bool cSuccess = false;
        std::vector<uint32_t> cWriteVec = pVecReq;

        try
        {
            WriteI2C ( pVecReq, true );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        // now read back
        // not sure if I should clear the last 8 bits of the vector, let's assume it is safe to not do that
        if (pReadback)
        {
            try
            {
                WriteI2C ( pVecReq, false );
            }
            catch ( Exception& e )
            {
                throw e;
            }

            try
            {
                ReadI2C ( pVecReq );
            }
            catch (Exception& e )
            {
                throw e;
            }

            // now I need to make sure that the written and the read-back vector are the same
            std::vector<uint32_t> cWriteAgain = get_mismatches (cWriteVec.begin(), cWriteVec.end(), pVecReq.begin(), CtaFWInterface::cmd_reply_comp);

            
            // now check the size of the WriteAgain vector and assert Success or not
            // also check that the number of write attempts does not exceed cMaxWriteAttempts
            if (cWriteAgain.empty() ) cSuccess = true;
            else
            {
                cSuccess = false;

                // if the number of errors is greater than 100, give up
                if (cWriteAgain.size() < 100 && pWriteAttempts < cMaxWriteAttempts )
                {
                    if (pReadback)  LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string(pWriteAttempts) << ") There were " << cWriteAgain.size() << " Readback Errors -trying again!" << RESET ;
                    
                    pWriteAttempts++;
                    this->WriteCbcBlockReg ( cWriteAgain, pWriteAttempts, true);
                }
                else if ( pWriteAttempts >= cMaxWriteAttempts )
                {
                    cSuccess = false; 
                    pWriteAttempts = 0 ;   
                }
                //else std::cout << "There were too many errors " << cWriteAgain.size() << " (>120 Registers). Something is wrong - aborting!" << std::endl;
                else throw Exception ( "Too many CBC readback errors - no functional I2C communication. Check the Setup" );
            }
        }
        else cSuccess = true;

        return cSuccess;
    }


    bool CtaFWInterface::BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReq, bool pReadback)
    {
        //use the method above for that!
        bool cSuccess = false;
        std::vector<uint32_t> cWriteVec = pVecReq;

        try
        {
            WriteI2C ( pVecReq, true );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        return cSuccess;
    }

    void CtaFWInterface::ReadCbcBlockReg (  std::vector<uint32_t>& pVecReq )
    {
        try
        {
            WriteI2C ( pVecReq, false );
        }
        catch ( Exception& e )
        {
            throw e;
        }

        try
        {
            ReadI2C ( pVecReq );
        }
        catch (Exception& e )
        {
            throw e;
        }
    }

    void CtaFWInterface::CbcFastReset()
    {
        //WriteReg ( "cbc_fast_reset", 1 );
        //usleep ( 2000 );
        //WriteReg ( "cbc_fast_reset", 0 );
    }

    void CtaFWInterface::CbcHardReset()
    {
        WriteReg ( "cbc_hard_reset", 1 );

        usleep ( 2000 );

        WriteReg ( "cbc_hard_reset", 0 );

        usleep ( 20 );
    }

    void CtaFWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
    {
        checkIfUploading();

        fpgaConfig->runUpload ( strConfig, pstrFile );
    }

    void CtaFWInterface::JumpToFpgaConfig ( const std::string& strConfig)
    {
        checkIfUploading();

        fpgaConfig->jumpToImage ( strConfig);
    }

    void CtaFWInterface::DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest)
    {
        checkIfUploading();
        fpgaConfig->runDownload ( strConfig, strDest.c_str() );
    }

    std::vector<std::string> CtaFWInterface::getFpgaConfigList()
    {
        checkIfUploading();
        return fpgaConfig->getFirmwareImageNames( );
    }

    void CtaFWInterface::DeleteFpgaConfig ( const std::string& strId)
    {
        checkIfUploading();
        fpgaConfig->deleteFirmwareImage ( strId);
    }

    void CtaFWInterface::checkIfUploading()
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new CtaFpgaConfig ( this );
    }
    bool CtaFWInterface::cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2)
    {
        //if (cWord1 != cWord2)
        //LOG(INFO)  << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 21) & 0xF) << " CbcId " << + ( (cWord1 >> 17) & 0xF) <<  " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF)   << " ## " << std::bitset<32> (cWord2) << " ### FMCId: " << ( (cWord2 >> 21) & 0xF) << " CbcId " << + ( (cWord2 >> 17) & 0xF) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  ;
        //LOG(INFO) << "Readback error" ;
        return ( cWord1  == cWord2 );
    }

    /*! \brief Reboot the board */
    void CtaFWInterface::RebootBoard()
    {
        checkIfUploading();
        fpgaConfig->resetBoard();
    }
    /*! \brief Set or reset the start signal */
    void CtaFWInterface::SetForceStart ( bool bStart)
    {
        WriteReg ( "pc_commands.force_BG0_start", bStart ? 1 : 0);
    }

}
