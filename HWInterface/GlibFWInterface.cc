/*

        FileName :                    GlibFWInterface.h
        Content :                     GlibFWInterface init/config of the Glib and its Cbc's
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            28/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "GlibFWInterface.h"
#include "GlibFpgaConfig.h"

namespace Ph2_HwInterface {

    GlibFWInterface::GlibFWInterface ( const char* puHalConfigFileName,
                                       uint32_t pBoardId ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig ( nullptr ),
        fNthAcq (0)
    {
    }


    GlibFWInterface::GlibFWInterface ( const char* puHalConfigFileName,
                                       uint32_t pBoardId,
                                       FileHandler* pFileHandler ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig ( nullptr ),
        fFileHandler ( pFileHandler ),
        fNthAcq (0)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    GlibFWInterface::GlibFWInterface ( const char* pId,
                                       const char* pUri,
                                       const char* pAddressTable ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fNthAcq (0)
    {
    }


    GlibFWInterface::GlibFWInterface ( const char* pId,
                                       const char* pUri,
                                       const char* pAddressTable,
                                       FileHandler* pFileHandler ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fFileHandler ( pFileHandler ),
        fNthAcq (0)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }


    void GlibFWInterface::setFileHandler (FileHandler* pHandler)
    {
        if (pHandler != nullptr )
        {
            fFileHandler = pHandler;
            fSaveToFile = true;
        }
        else LOG (INFO) << "Error, can not set NULL FileHandler" ;
    }

    uint32_t GlibFWInterface::getBoardInfo()
    {
        LOG (INFO) << "FMC1 present : " << std::to_string (ReadReg ( "status.fmc1_present" ) ) ;
        LOG (INFO) << "FMC2 present : " << std::to_string (ReadReg ( "status.fmc2_present" ) ) ;
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
        uint32_t cVersionWord = ( (cVersionMajor & 0x0000FFFF) << 16 | (cVersionMinor & 0x0000FFFF) );
        return cVersionWord;
    }


    void GlibFWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        std::chrono::milliseconds cPause ( 200 );

        //Primary Configuration
        //TODO: the PC_config_ok toggle was reverted here originally (first to 1 and then to 0)
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );
        cVecReg.push_back ( {"pc_commands.SRAM1_end_readout", 0} );
        cVecReg.push_back ( {"pc_commands.SRAM2_end_readout", 0} );
        cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"ctrl_sram.sram2_user_logic", 1} );

        // iterate the BeBoardRegMap to get the user configuration
        BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

        for ( auto const& it : cGlibRegMap )
            cVecReg.push_back ( {it.first, it.second} );

        cVecReg.push_back ( {"pc_commands.SPURIOUS_FRAME", 0} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 0} );
        cVecReg.push_back ( {"cbc_acquisition.CBC_TRIGGER_ONE_SHOT", 0} );
        cVecReg.push_back ({"cbc_reg_i2c_settings", 0x000009F4});

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        this->CbcHardReset();
        usleep (10);
        //cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );

        //WriteStackReg ( cVecReg );
        //cVecReg.clear();
    }


    void GlibFWInterface::Start()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        //Starting the DAQ
        cVecReg.push_back ( {"break_trigger", 0} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 1} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        fNthAcq = 0;
        // Since the Number of  Packets is a FW register, it should be read from the Settings Table which is one less than is actually read
        fNpackets = ReadReg ( "pc_commands.CBC_DATA_PACKET_NUMBER" ) + 1 ;

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
    }

    void GlibFWInterface::Stop()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        uhal::ValWord<uint32_t> cVal;

        //Select SRAM
        SelectDaqSRAM();
        //Stop the DAQ
        cVecReg.push_back ( {"break_trigger", 1} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 0} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 0} );

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
        WriteReg ( fStrReadout, 0 );
    }


    void GlibFWInterface::Pause()
    {
        WriteReg ( "break_trigger", 1 );
    }


    void GlibFWInterface::Resume()
    {
        WriteReg ( "break_trigger", 0 );
    }

    uint32_t GlibFWInterface::ReadData ( BeBoard* pBoard,  bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
    {
        //Readout settings
        std::chrono::milliseconds cWait ( 1 );

        uhal::ValWord<uint32_t> cVal;

        if ( pBoard )
            fBlockSize = computeBlockSize ( pBoard );

        //FIFO goes to write_data state
        //Select SRAM
        SelectDaqSRAM();

        //Wait for the SRAM full condition.
        cVal = ReadReg ( fStrFull );

        do
        {
            cVal = ReadReg ( fStrFull );

            if ( cVal == 0 ){
                if (!pWait){
                    return 0;
                }
                std::this_thread::sleep_for ( cWait );
            }
        }
        while ( cVal == 0 );

        //break trigger
        if ( pBreakTrigger ) WriteReg ( "break_trigger", 1 );

        //Set read mode to SRAM
        WriteReg ( fStrSramUserLogic, 0 );

        //Read SRAM
        pData =  ReadBlockRegValue ( fStrSram, fBlockSize );

        WriteReg ( fStrSramUserLogic, 1 );
        WriteReg ( fStrReadout, 1 );

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

        if ( pBreakTrigger ) WriteReg ( "break_trigger", 0 );

        if ( fSaveToFile )
        {
            fFileHandler->set ( pData );
            //fFileHandler->writeFile();
        }

        return fNpackets;
    }

    void GlibFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData , bool pWait)
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        fNpackets = pNEvents;
        //Starting the DAQ
        cVecReg.push_back ( {"pc_commands.CBC_DATA_PACKET_NUMBER", pNEvents - 1} );
        cVecReg.push_back ( {"break_trigger", 0} );
        cVecReg.push_back ( {"pc_commands.PC_config_ok", 1} );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 1} );

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
        cVecReg.push_back ({ "break_trigger", 1 } );
        cVecReg.push_back ( {"pc_commands2.force_BG0_start", 0} );

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //Set read mode to SRAM
        WriteReg ( fStrSramUserLogic, 0 );

        //Read SRAM
        pData =  ReadBlockRegValue ( fStrSram, fBlockSize );

        WriteReg ( fStrSramUserLogic, 1 );

        //need to increment the internal Acquisition counter
        fNthAcq++;

        if ( fSaveToFile )
        {
            fFileHandler->set ( pData );
            //fFileHandler->writeFile();
        }

        WriteReg ( "pc_commands.PC_config_ok", 0 );
    }


    /** compute the block size according to the number of CBC's on this board
     * this will have to change with a more generic FW */
    uint32_t GlibFWInterface::computeBlockSize ( BeBoard* pBoard )
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

        if ( pBoard->getNCbcDataSize() != 0 ) return fNpackets * ( pBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 );
        else return fNpackets * ( cCounter.getNCbc() * CBC_EVENT_SIZE_32 + EVENT_HEADER_TDC_SIZE_32 ); // in 32 bit words
    }

    std::vector<uint32_t> GlibFWInterface::ReadBlockRegValue ( const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();

        // To avoid the IPBUS bug
        // need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
        if ( pBlocksize > 255 )
        {
            std::string fSram_256 = pRegNode + "_256";
            uhal::ValWord<uint32_t> cWord = ReadReg ( fSram_256 );
            vBlock[255] = cWord.value();
        }

        return vBlock;
    }

    bool GlibFWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );

        if ( pValues.size() > 255 )
            WriteReg ( pRegNode + "_256", pValues[255] );

        return cWriteCorr;
    }

    void GlibFWInterface::SelectDaqSRAM()
    {
        fStrSram  = ( ( fNthAcq % 2 + 1 ) == 1 ? "sram1" : "sram2" );
        fStrSramUserLogic = ( ( fNthAcq % 2 + 1 ) == 1 ? "ctrl_sram.sram1_user_logic" : "ctrl_sram.sram2_user_logic" );
        fStrFull = ( ( fNthAcq % 2 + 1 ) == 1 ? "flags.SRAM1_full" : "flags.SRAM2_full" );
        fStrReadout = ( ( fNthAcq % 2 + 1 ) == 1 ? "pc_commands.SRAM1_end_readout" : "pc_commands.SRAM2_end_readout" );
    }



    //Methods for Cbc's:

    //void GlibFWInterface::StartThread ( BeBoard* pBoard, uint32_t uNbAcq, HwInterfaceVisitor* visitor )
    //{
    //if ( runningAcquisition ) return;

    //runningAcquisition = true;
    //numAcq = 0;
    //nbMaxAcq = uNbAcq;

    //thrAcq = boost::thread ( &Ph2_HwInterface::GlibFWInterface::threadAcquisitionLoop, this, pBoard, visitor );
    //}

    //void GlibFWInterface::threadAcquisitionLoop ( BeBoard* pBoard, HwInterfaceVisitor* visitor )
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

    void GlibFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
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
        pVecReq.push_back ( ( pCbcId >> 3 ) << 21 | ( pCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | uValue );
    }

    void GlibFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                      uint8_t pFeId,
                                      uint8_t pCbcId,
                                      std::vector<uint32_t>& pVecReq,
                                      bool pRead,
                                      bool pWrite )
    {
        uint8_t uValue = pRegItem.fAddress == 0 ? pRegItem.fValue & 0x7F : pRegItem.fValue;
        // (pCbcId & 7) restarts CbcIDs from 0 for FE 1 (if CbcID > 7)
        pVecReq.push_back ( pFeId  << 21 | pCbcId << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | uValue );
    }

    void GlibFWInterface::BCEncodeReg ( const CbcRegItem& pRegItem,
                                        uint8_t pNCbc,
                                        std::vector<uint32_t>& pVecReq,
                                        bool pRead,
                                        bool pWrite )
    {
        uint8_t uValue = pRegItem.fAddress == 0 ? pRegItem.fValue & 0x7F : pRegItem.fValue;

        // here I need to loop over all CBCs somehow...
        for (uint8_t cCbcId = 0; cCbcId < pNCbc; cCbcId++)
            pVecReq.push_back ( ( cCbcId >> 3 ) << 21 | ( cCbcId & 7 ) << 17 | pRegItem.fPage << 16 | pRegItem.fAddress << 8 | uValue );
    }

    void GlibFWInterface::DecodeReg ( CbcRegItem& pRegItem,
                                      uint8_t& pCbcId,
                                      uint32_t pWord,
                                      bool& pRead,
                                      bool& pFailed )
    {
        // temporary for 16CBC readout FW  (Beamtest NOV 15)
        // will have to be corrected if we want to read two modules from the same GLIB
        uint8_t cFeId = ( pWord & cMask7 ) >> 21;
        pCbcId = ( ( pWord & cMask5 ) | ( cFeId << 3 ) ) >> 17;
        pRegItem.fPage = ( pWord & cMask6 ) >> 16;
        pRegItem.fAddress = ( pWord & cMask2 ) >> 8;
        pRegItem.fValue = pWord & cMask1;
    }

    bool GlibFWInterface::I2cCmdAckWait ( uint32_t pAckVal, uint8_t pNcount )
    {
        unsigned int cWait ( 100 );

        if ( pAckVal )
            cWait = pNcount * 500;

        usleep ( cWait );

        uhal::ValWord<uint32_t> cVal;
        uint32_t cLoop = 0;
        bool cAckReceived = false;

        do
        {
            cVal = ReadReg ( "cbc_i2c_cmd_ack" );
            if ( cVal != pAckVal )
                usleep ( cWait );
            else cAckReceived = true;
        }
        while ( cVal != pAckVal && ++cLoop < 70 );

        return cAckReceived;
    }

    void GlibFWInterface::EnableI2c ( bool pEnable )
    {
        uint32_t cValue = pEnable ? 0x000009F4 : 0 ;
        WriteReg ( "cbc_reg_i2c_settings", cValue );
        //if ( pEnable )
        //usleep ( 100000 );
    }

    void GlibFWInterface::WriteI2C ( std::vector<uint32_t>& pVecReq, bool pWrite )
    {
        pVecReq.push_back ( 0xFFFFFFFF );

        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        WriteReg ( "ctrl_sram.sram1_user_logic", 0 );
        WriteBlockReg ( "sram1", pVecReq );

        cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        //TODO
        cVecReg.push_back ( {"cbc_i2c_cmd_rq", pWrite ? 3 : 1} );
        //cVecReg.push_back ( {"cbc_i2c_cmd_rq", pWrite ? 1 : 3} );
        WriteStackReg ( cVecReg );

        //now remove the 0xFFFFFFFF word again so I can re-use the vector
        pVecReq.pop_back();
        //std::cout << __PRETTY_FUNCTION__ << "Timing out here! size: " << pVecReq.size() << std::endl;
        if ( I2cCmdAckWait ( ( uint32_t ) 1, pVecReq.size() ) == 0 )
            throw Exception ( "CbcInterface: Command Request Timed out." );

        WriteReg ( "cbc_i2c_cmd_rq", 0 );

        if ( I2cCmdAckWait ( ( uint32_t ) 0, pVecReq.size() ) == 0 )
            throw Exception ( "CbcInterface: I2C realease timed out." );
    }

    void GlibFWInterface::ReadI2C ( std::vector<uint32_t>& pVecReq )
    {
        //Read Size + 1 to have the ffffffff word
        uint32_t pVecReqSize = pVecReq.size() + 1;
        pVecReq.clear();
        WriteReg ( "ctrl_sram.sram1_user_logic", 0 );
        pVecReq = ReadBlockRegValue ( "sram1", pVecReqSize );

        if (pVecReq.back() != 0xFFFFFFFF) LOG (INFO) << "ERROR, the last word read was not 0xFFFFFFFF - not sure if I read all required words!" ;
        else pVecReq.pop_back();

        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        cVecReg.push_back ( {"ctrl_sram.sram1_user_logic", 1} );
        cVecReg.push_back ( {"cbc_i2c_cmd_rq", 0} );
        WriteStackReg ( cVecReg );
    }


    bool GlibFWInterface::WriteCbcBlockReg (  std::vector<uint32_t>& pVecReq, uint8_t& pWriteAttempts ,  bool pReadback)
    {
        int cMaxWriteAttempts = 5;
        bool cSuccess = false;
        std::vector<uint32_t> cWriteVec = pVecReq;

        try
        {
            WriteI2C ( cWriteVec, true );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        // now read back
        if (pReadback)
        {
            //to use exactly the same algorithm, I should copy the vector and mask out the last 8 bits
            for (auto& cWord : pVecReq)
                cWord = (cWord & 0xFFFFFF00);

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
            std::vector<uint32_t> cWriteAgain = get_mismatches (cWriteVec.begin(), cWriteVec.end(), pVecReq.begin(), GlibFWInterface::cmd_reply_comp);

            // now check the size of the WriteAgain vector and assert Success or not
            // also check that the number of write attempts does not exceed cMaxWriteAttempts
            if (cWriteAgain.empty() ) cSuccess = true;
            else
            {
                cSuccess = false;

                // if the number of errors is greater than 100, give up
                if (cWriteAgain.size() < 100 && pWriteAttempts < cMaxWriteAttempts )
                {
                    if (pReadback)  LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string (pWriteAttempts) << ") There were " << cWriteAgain.size() << " Readback Errors -trying again!" << RESET ;

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


    bool GlibFWInterface::BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReq, bool pReadback)
    {
        //use the method above for that!
        bool cSuccess = true;
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

    void GlibFWInterface::ReadCbcBlockReg (  std::vector<uint32_t>& pVecReq )
    {
        try
        {
            WriteI2C ( pVecReq, false );
        }
        catch ( Exception& e )
        {
            throw e;
        }

        ReadI2C ( pVecReq );
    }

    void GlibFWInterface::CbcFastReset()
    {
        WriteReg ( "cbc_fast_reset", 1 );
        usleep ( 2000 );
        WriteReg ( "cbc_fast_reset", 0 );
        usleep ( 20 );
    }

    void GlibFWInterface::CbcHardReset()
    {
        WriteReg ( "cbc_hard_reset", 1 );
        usleep ( 2000 );
        WriteReg ( "cbc_hard_reset", 0 );
        usleep ( 20 );
    }

    void GlibFWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is already uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->runUpload ( strConfig, pstrFile );
    }

    void GlibFWInterface::JumpToFpgaConfig ( const std::string& strConfig )
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->jumpToImage ( strConfig );
    }

    bool GlibFWInterface::cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2)
    {
        //if (cWord1 != cWord2)
        //LOG(INFO)  << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 21) & 0xF) << " CbcId " << + ( (cWord1 >> 17) & 0xF) <<  " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF)   << " ## " << std::bitset<32> (cWord2) << " ### FMCId: " << ( (cWord2 >> 21) & 0xF) << " CbcId " << + ( (cWord2 >> 17) & 0xF) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  ;
        //LOG(INFO) << "Readback error" ;

        return ( cWord1  == cWord2 );
    }
    /*! \brief Reboot the board */
    void GlibFWInterface::RebootBoard()
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->resetBoard();
    }
    /*! \brief Set or reset the start signal */
    void GlibFWInterface::SetForceStart ( bool bStart)
    {
        WriteReg ( "pc_commands2.force_BG0_start", bStart ? 1 : 0);
    }

    void GlibFWInterface::PowerOn()
    {
        ;
    }

    void GlibFWInterface::PowerOff()
    {
        ;
    }

    void GlibFWInterface::ReadVer()
    {
        ;
    }
}
