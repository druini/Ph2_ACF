/*!

        \file                           ICICICGlibFWInterface.h
        \brief                          ICICICGlibFWInterface init/config of the Glib and its Cbc's
        \author                         G. Auzinger, K. Uchida
        \version            1.0
        \date                           25.02.2016
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch

 */

#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "ICGlibFWInterface.h"
#include "GlibFpgaConfig.h"
//#include "CbcInterface.h"


namespace Ph2_HwInterface {

    ICGlibFWInterface::ICGlibFWInterface ( const char* puHalConfigFileName,
                                           uint32_t pBoardId ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig (nullptr),
        fData ( nullptr ),
        fBroadcastCbcId (0),
        fReplyBufferSize (1024),
        fFMCId (1)
    {}


    ICGlibFWInterface::ICGlibFWInterface ( const char* puHalConfigFileName,
                                           uint32_t pBoardId,
                                           FileHandler* pFileHandler ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig (nullptr),
        fData ( nullptr ),
        fBroadcastCbcId (0),
        fReplyBufferSize (1024),
        fFileHandler ( pFileHandler ),
        fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    ICGlibFWInterface::ICGlibFWInterface ( const char* pId,
                                           const char* pUri,
                                           const char* pAddressTable ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fBroadcastCbcId (0),
        fReplyBufferSize (1024),
        fFMCId (1)
    {}


    ICGlibFWInterface::ICGlibFWInterface ( const char* pId,
                                           const char* pUri,
                                           const char* pAddressTable,
                                           FileHandler* pFileHandler ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fData ( nullptr ),
        fBroadcastCbcId (0),
        fReplyBufferSize (1024),
        fFileHandler ( pFileHandler ),
        fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    void ICGlibFWInterface::setFileHandler (FileHandler* pHandler)
    {
        if (pHandler != nullptr )
        {
            fFileHandler = pHandler;
            fSaveToFile = true;
        }
        else std::cout << "Error, can not set NULL FileHandler" << std::endl;
    }

    uint32_t ICGlibFWInterface::getBoardInfo()
    {
        //std::cout << "FMC1 present : " << ReadReg ( "user_stat.current_fec_fmc2_cbc0" ) << std::endl;
        //std::cout << "FMC2 present : " << ReadReg ( "user_stat.current_fec_fmc2_cbc1" ) << std::endl;
        uint32_t cVersionMajor, cVersionMinor;
        cVersionMajor = ReadReg ( "user_stat.version.ver_major" );
        cVersionMinor = ReadReg ( "user_stat.version.ver_minor" );
        std::cout << "FW version : " << cVersionMajor << "." << cVersionMinor << "." << ReadReg ( "user_stat.version.ver_build" ) << std::endl;

        uhal::ValWord<uint32_t> cBoardType = ReadReg ( "sys_regs.board_id" );

        std::cout << "BoardType : ";

        char cChar = ( ( cBoardType & cMask4 ) >> 24 );
        std::cout << cChar;

        cChar = ( ( cBoardType & cMask3 ) >> 16 );
        std::cout << cChar;

        cChar = ( ( cBoardType & cMask2 ) >> 8 );
        std::cout << cChar;

        cChar = ( cBoardType & cMask1 );
        std::cout << cChar << std::endl;

        uint32_t cVersionWord = ( (cVersionMajor & 0x0000FFFF) << 16 || (cVersionMinor & 0x0000FFFF) );
        return cVersionWord;
    }

    void ICGlibFWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;
        //here i want to first configure the FW according to the HW structure attached - since this method is aware of pBoard, I can loop the HW structure and thus count CBCs, set i2c addresses and FMC config

        char tmpChar[64];
        //read a couple of useful data
        fDataSizeperEvent32 = ReadReg ("user_stat.fw_cnfg.data_size32.evt_total");
        bool cfmc1_en = ReadReg ("user_stat.fw_cnfg.fmc_cnfg.fmc1_cbc_en");
        bool cfmc2_en = ReadReg ("user_stat.fw_cnfg.fmc_cnfg.fmc2_cbc_en");
        bool cDIO5_en = ReadReg ("user_stat.fw_cnfg.dio5ch_xtrig_en");
        fBroadcastCbcId = ReadReg ("user_stat.fw_cnfg.fmc_cnfg.ncbc_per_fmc");
        std::cout << "FMC1 en: " << cfmc1_en << std::endl << "FMC2 en: " << cfmc2_en << std::endl << "Number of CBCs: " << fBroadcastCbcId << std::endl;
        std::cout << "DIO5 en: " << cDIO5_en << std::endl;
        std::cout << "Number of 32-bit words/Event in this configuration: " << fDataSizeperEvent32 << std::endl;
        //this does not work because BeBoard* is const
        //pBoard->setNCbcDataSize(uint16_t(cNCbcperFMC));

        //get the # of CBCs on the first FE and use that as default, write the CBC i2c addresses etc
        for (Module* cFe : pBoard->fModuleVector)
        {
            // need to find the correct FMC Id for each module
            // better to get the fmc_cbc enable and use that since I assume only one module per board for the moment
            fFMCId = cFe->getFMCId();

            if (cfmc1_en) fFMCId = 1;
            else if (cfmc2_en) fFMCId = 2;

            for ( Cbc* cCbc : cFe->fCbcVector)
            {
                uint8_t cCbcId = cCbc->getCbcId();
                sprintf (tmpChar, "cbc_daq_ctrl.cbc_i2c_addr_fmc%d.cbc%d", fFMCId, cCbcId);
                std::string cRegString (tmpChar);
                uint32_t cAddress = 0x41 + cCbcId;
                uint32_t cVal = (1 << 28) | (cCbcId << 24) | (cAddress & 0x7F);
                cVecReg.push_back ({cRegString, cVal });
            }
        }

        bool cVal = (fBroadcastCbcId == 2) ? 0 : 1;
        cVecReg.push_back ({"cbc_daq_ctrl.general.fmc_wrong_pol", static_cast<uint32_t> (cVal) });
        cVecReg.push_back ({"cbc_daq_ctrl.general.fmc_pc045c_4hybrid", static_cast<uint32_t> (cVal) });

        //last, loop over the variable registers from the HWDescription.xml file
        BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

        for ( auto const& it : cGlibRegMap )
        {
            cVecReg.push_back ( {it.first, it.second} );
        }

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //hard reset CBC according to Kirika however, this violates our paradigm....sort of
        this->CbcHardReset();
        usleep (10);
        //before I'm done I need to reset all the state machines which loads the configuration
        //all the daq_ctrl registers have to be used with hex values and not with the sub-masks but they are auto clearing after 1 has been written
        //0x1 reset, 0x2 start, 0x4 stop, 0x8000 counter reset,
        cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", CTR_RESET });
        cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", RESET_ALL });
        cVecReg.push_back ({"cbc_daq_ctrl.cbc_i2c_ctrl", RESET_ALL });
        cVecReg.push_back ({"cbc_daq_ctrl.fmcdio5ch_thr_dac_ctrl", 0x1 });
        WriteStackReg ( cVecReg );
        cVecReg.clear();
        std::vector<uint32_t> pReplies;
        ReadI2C (  fBroadcastCbcId, pReplies);

        bool cSuccess = false;

        for (auto& cWord : pReplies)
            cSuccess = ( ( (cWord >> 20) & 0x1) == 0 && ( (cWord) & 0x000000FF) != 0 ) ? true : false;

        if (cSuccess) std::cout << "Successfully received *Pings* from " << fBroadcastCbcId << " Cbcs on FMC " << +fFMCId << std::endl;
        else std::cout << "Error, did not receive the correct number of *Pings*; expected: " << fBroadcastCbcId << ", received: " << pReplies.size() << std::endl;

        cVecReg.push_back ({"cbc_daq_ctrl.cbc_i2c_ctrl", 0x2});
        WriteStackReg ( cVecReg );
        cVecReg.clear();
    }


    void ICGlibFWInterface::Start()
    {
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        //first reset all the counters and state machines
        cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", RESET_ALL });
        cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", CTR_RESET });

        //according to Kirika, this is not necessary to set explicitly any more
        //cVecReg.push_back ({"commissioning_cycle_ctrl", 0x1 });
        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //now issue start
        //cVecReg.push_back ({"commissioning_cycle_ctrl", 0x2 });
        cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", START });
        WriteStackReg ( cVecReg );
        cVecReg.clear();
    }

    void ICGlibFWInterface::Stop()
    {
        WriteReg ( "cbc_daq_ctrl.daq_ctrl", STOP );
    }


    void ICGlibFWInterface::Pause()
    {
        //this should just brake triggers
        WriteReg ( "cbc_daq_ctrl.daq_ctrl", 0x4000 );
    }


    void ICGlibFWInterface::Resume()
    {
        WriteReg ( "cbc_daq_ctrl.daq_ctrl", 0x2000 );
    }

    uint32_t ICGlibFWInterface::ReadData ( BeBoard* pBoard, bool pBreakTrigger )
    {
        std::chrono::milliseconds cWait ( 1 );
        //first, read how many Events per Acquisition
        fNEventsperAcquistion = ReadReg ("cbc_daq_ctrl.nevents_per_pcdaq");
        //the size of the packet to read then is fNEventsperAcquistion * fDataSizeperEvent32

        //first, poll if the packet is ready
        uint32_t cVal = 0;

        while (cVal == 0)
        {
            cVal = ReadReg ("cbc_daq_ctrl.event_data_buf_status.data_ready" ) & 0x1;
            std::this_thread::sleep_for ( cWait );
        }

        //ok, packet complete, now let's read
        std::vector<uint32_t> cData =  ReadBlockRegValue ( "data_buf", fNEventsperAcquistion * fDataSizeperEvent32 );

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, cData , fNEventsperAcquistion, true );

        if ( fSaveToFile )
        {
            fFileHandler->set ( cData );
            fFileHandler->writeFile();
        }

        return fNEventsperAcquistion;
    }

    void ICGlibFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents )
    {
        // I need a check if pNEvents is grater than 2000 - in this case I have to split in packets of 2000
        uint32_t cNCycles = 1;
        uint32_t cNEvents = pNEvents;

        if ( pNEvents > 2000 )
        {
            if (pNEvents % 2000 == 0)
            {
                //total number of events is divisible by 2000 so lets just get the number of cycles
                cNCycles = pNEvents / 2000;
                cNEvents = 2000;
            }
            else
            {
                // if not, let's take the remainder of the division as packet size
                cNEvents = pNEvents % 2000;
                cNCycles = ceil (pNEvents / double (cNEvents) );
            }

            std::cout << "Packet Size larger than 2000, splitting in Acquisitions of " << cNEvents << " in " << cNCycles << " cycles!" << std::endl;
        }

        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        // probably no need to reset everything since I am calling this a lot during commissioning
        cVecReg.push_back ({"cbc_daq_ctrl.nevents_per_pcdaq", cNEvents});
        cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", RESET_ALL });
        //cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", CTR_RESET });
        WriteStackReg ( cVecReg );
        cVecReg.clear();

        //here I optimize for speed during calibration, so I explicitly set the nevents_per_pcdaq to the event number I desire
        fNEventsperAcquistion = cNEvents;
        std::vector<uint32_t> cData;

        // I have to do cNCycles with cNEvents
        for (uint32_t cIndex = 0; cIndex < cNCycles; cIndex++)
        {
            //now issue start
            cVecReg.push_back ({"cbc_daq_ctrl.daq_ctrl", START} );

            WriteStackReg ( cVecReg );
            cVecReg.clear();
            //now poll for data to be ready
            std::chrono::milliseconds cWait ( 1 );

            //first, poll if the packet is ready
            uint32_t cVal = 0;

            while (cVal == 0)
            {
                cVal = ReadReg ("cbc_daq_ctrl.event_data_buf_status.data_ready" ) & 0x1;
                std::this_thread::sleep_for ( cWait );
            }

            //now stop triggers & DAQ
            WriteReg ( "cbc_daq_ctrl.daq_ctrl", STOP );

            //ok, packet complete, now let's read and append to cData
            std::vector<uint32_t> cTmpData =  ReadBlockRegValue ( "data_buf", fNEventsperAcquistion * fDataSizeperEvent32 );
            cData.insert (cData.end(), std::make_move_iterator (cTmpData.begin() ), std::make_move_iterator (cTmpData.end() ) );
        }

        // just creates a new Data object, setting the pointers and getting the correct sizes happens in Set()
        if ( fData ) delete fData;

        fData = new Data();

        // set the vector<uint32_t> as event buffer and let him know how many packets it contains
        fData->Set ( pBoard, cData , cNCycles * fNEventsperAcquistion, true );

        if ( fSaveToFile )
        {
            fFileHandler->set ( cData );
            fFileHandler->writeFile();
        }
    }

    std::vector<uint32_t> ICGlibFWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();

        // To avoid the IPBUS bug
        // need to convert uHal::ValVector to vector<uint32_t> so we can replace the 256th word
        //if ( pBlocksize > 255 )
        //{
        //std::string fSram_256 = pRegNode + "_256";
        //uhal::ValWord<uint32_t> cWord = ReadReg ( fSram_256 );
        //vBlock[255] = cWord.value();
        //}

        return vBlock;
    }

    bool ICGlibFWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );

        //if ( pValues.size() > 255 )
        //WriteReg ( pRegNode + "_256", pValues[255] );

        return cWriteCorr;
    }

    ///////////////////////////////////////////////////////
    //      CBC Methods                                 //
    /////////////////////////////////////////////////////

    // this is clearly for addressing individual CBCs, have to see how to deal with broadcast commands
    void ICGlibFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                        uint8_t pCbcId,
                                        std::vector<uint32_t>& pVecReq,
                                        bool pRead,
                                        bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        pVecReq.push_back ( ( fFMCId << 28 ) | ( pCbcId << 24 ) | (  pRead << 21 ) | (  pWrite << 20 ) | ( pRegItem.fPage << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }
    void ICGlibFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                        uint8_t pFeId,
                                        uint8_t pCbcId,
                                        std::vector<uint32_t>& pVecReq,
                                        bool pRead,
                                        bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        pVecReq.push_back ( ( pFeId << 28 ) | ( pCbcId << 24 ) | (  pRead << 21 ) | (  pWrite << 20 ) | ( pRegItem.fPage << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }

    void ICGlibFWInterface::BCEncodeReg ( const CbcRegItem& pRegItem,
                                          uint8_t pNCbc,
                                          std::vector<uint32_t>& pVecReq,
                                          bool pRead,
                                          bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        pVecReq.push_back ( ( fFMCId << 28 ) | ( fBroadcastCbcId << 24 ) | (  pRead << 21 ) | (  pWrite << 20 )  | ( pRegItem.fPage << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }

    void ICGlibFWInterface::DecodeReg ( CbcRegItem& pRegItem,
                                        uint8_t& pCbcId,
                                        uint32_t pWord,
                                        bool& pRead,
                                        bool& pFailed )
    {
        pCbcId   =  ( pWord & 0x07000000 ) >> 24;
        pFailed  =  ( pWord & 0x00100000 ) >> 20;
        //pRead is 1 for read transaction, 0 for a write transaction
        pRead    =  ( pWord & 0x00020000 ) >> 17;
        pRegItem.fPage    =  ( pWord & 0x00010000 ) >> 16;
        pRegItem.fAddress =  ( pWord & 0x0000FF00 ) >> 8;
        pRegItem.fValue   =  ( pWord & 0x000000FF );
    }

    bool ICGlibFWInterface::ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies)
    {
        usleep (SINGLE_I2C_WAIT * pNReplies );

        bool cFailed (false);

        //read the number of received replies from nwdata and use this number to compare with the number of expected replies and to read this number 32-bit words from the reply FIFO
        char tmp[256];
        sprintf ( tmp, "cbc_daq_ctrl.i2c_reply_fifo_fmc%d_status.nwdata", fFMCId );
        std::string cNode (tmp);
        uint32_t cNReplies = ReadReg (cNode);


        if (cNReplies != pNReplies)
        {
            std::cout << "Error: Read " << cNReplies << " I2C replies whereas " << pNReplies << " are expected!" << std::endl;
            cFailed = true;
        }

        sprintf ( tmp, "cbc_i2c_reply.fmc%d", fFMCId );
        cNode = tmp;

        //not sure if necessary
        try
        {
            pReplies = ReadBlockRegValue ( cNode, cNReplies );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        //here i create a dummy reg item for decoding so I can find if 1 cFailed
        CbcRegItem cItem;
        uint8_t cCbcId;
        bool cRead;

        //explicitly reset the nwdata word
        WriteReg ("cbc_daq_ctrl.cbc_i2c_ctrl", 0x2);

        return cFailed;
    }

    bool ICGlibFWInterface::WriteI2C ( std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pReadback, bool pBroadcast )
    {
        //This one's recursive, beware!
        // figure out how to best determine if this is boradcast or not? (decode the 1st word of the Vector and decide based on the CBC address?)
        //cN is the size of the vector to send
        uint32_t cN ( pVecSend.size() );
        //cM is the replybuffersize (1024) / 2 if i write & read, else divided by 1 / broadcastCbcId(8) or 1
        uint32_t cM ( fReplyBufferSize / ( pReadback ? 2 : 1 ) / ( pBroadcast ? fBroadcastCbcId  : 1 ) );
        //division of cN/cM and remainder
        uint32_t cDivNM ( cN / cM );
        uint32_t cRemNM ( cN % cM );
        bool cFailed ( false );

        if ( ( cDivNM == 1 && cRemNM == 0 ) || ( cDivNM == 0 && cRemNM != 0 ) )
        {
            try
            {
                WriteBlockReg ( "cbc_i2c_command", pVecSend );
            }
            catch ( Exception& except )
            {
                throw except;
            }

            uint32_t cNReplies = pVecSend.size() * ( pReadback ? 2 : 1 ) * ( pBroadcast ? fBroadcastCbcId : 1 );

            //if ( ReadI2C (  cNReplies, pReplies) ) cFailed = true;
            cFailed = ReadI2C (  cNReplies, pReplies) ;
        }
        else
        {
            for ( uint32_t cIndex = 0; cIndex < cDivNM; cIndex++ )
            {
                std::vector<uint32_t> cCommandBlock ( pVecSend.begin() + cIndex * cM, pVecSend.begin() + ( cIndex + 1 ) * cM );

                //if ( WriteI2C (  cCommandBlock, pReplies, pReadback, pBroadcast ) ) cFailed = true;
                cFailed = WriteI2C (  cCommandBlock, pReplies, pReadback, pBroadcast );
            }

            if ( cRemNM )
            {
                std::vector<uint32_t> cCommandBlock ( pVecSend.begin() + cDivNM * cM, pVecSend.end() );

                //if ( WriteI2C (  cCommandBlock, pReplies, pReadback, pBroadcast ) ) cFailed = true;
                cFailed = WriteI2C (  cCommandBlock, pReplies, pReadback, pBroadcast );
            }
        }

        return cFailed;
    }



    bool ICGlibFWInterface::WriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback)
    {
        // the actual write & readback command is in the vector
        std::vector<uint32_t> cReplies;
        bool cSuccess = !WriteI2C ( pVecReg, cReplies, pReadback, false );
        // the reply format is different from the sent format, therefore a binary predicate is necessary to compare
        // fValue is in the 8 lsb, then address is in 15 downto 8, page is in 16, CBCId is in 24
        // could use a mask 0x0F01FFFF

        //for (int index = 0; index < pVecReg.size(); index++)
        //{
        //uint32_t cWord1 = pVecReg.at (index);
        //uint32_t cWord2 = cReplies.at (2 * index);
        //uint32_t cWord3 = cReplies.at ( (2 * index) + 1);
        //std::cout << std::endl << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 28) & 0xF) << " CbcId " << + ( (cWord1 >> 24) & 0xF) << " Read " << + ( (cWord1 >> 21) & 0x1) << " Write " << + ( (cWord1 >> 20) & 0x1) << " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF)  << std::endl << " ## " << std::bitset<32> (cWord2) << " ### Read:           CbcId " << + ( (cWord2 >> 24) & 0xF) << " Info " << + ( (cWord2 >> 20) & 0x1) << " Read? " << + ( (cWord2 >> 17) & 0x1) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  << std::endl << " ## " << std::bitset<32> (cWord3) << " ### Read:           CbcId " << + ( (cWord3 >> 24) & 0xF) << " Info " << + ( (cWord3 >> 20) & 0x1) << " Read? " << + ( (cWord3 >> 17) & 0x1) << " Page  " << + ( (cWord3 >> 16) & 0x1) << " Address " << + ( (cWord3 >> 8) & 0xFF) << " Value " << + ( (cWord3) & 0xFF)  << std::endl;
        //;
        //}

        //here make a distinction: if pReadback is true, compare only the read replies using the binary predicate
        //else, just check that info is 0 and thus the CBC acqnowledged the command if the writeread is 0
        std::vector<uint32_t> cWriteAgain;

        if (pReadback)
        {
            //split the reply vector in even and odd replies
            //even is the write reply, odd is the read reply
            //since I am already reading back, might as well forget about the CMD acknowledge from the CBC and directly look at the read back value
            std::vector<uint32_t> cOdd;
            getOddElements (cReplies, cOdd);

            //now use the Template from BeBoardFWInterface to return a vector with all written words that have been read back incorrectly
            cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cOdd.begin(), ICGlibFWInterface::cmd_reply_comp);

            // now clear the initial cmd Vec and set the read-back
            pVecReg.clear();
            pVecReg = cOdd;
        }
        else
        {
            //since I do not read back, I can safely just check that the info bit of the reply is 0 and that it was an actual write reply
            //then i put the replies in pVecReg so I can decode later in CBCInterface
            cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), ICGlibFWInterface::cmd_reply_ack);
            pVecReg.clear();
            pVecReg = cReplies;
        }

        // now check the size of the WriteAgain vector and assert Success or not
        if (cWriteAgain.empty() ) cSuccess = true;
        else
        {
            cSuccess = false;

            // if the number of errors is greater than 100, give up
            if (cWriteAgain.size() < 100)
            {
                if (pReadback) std::cout << "There were " << cWriteAgain.size() << " Readback Errors -trying again!" << std::endl;
                else std::cout << "There were " << cWriteAgain.size() << " CBC CMD acknowledge bits missing -trying again!" << std::endl;

                this->WriteCbcBlockReg ( cWriteAgain, true);
            }
            //else std::cout << "There were too many errors (>100 Registers). Something is wrong - aborting!" << std::endl;
            else throw Exception ( "Too many CBC readback errors - no functional I2C communication. Check the Setup" );
        }

        return cSuccess;
    }

    bool ICGlibFWInterface::BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback)
    {
        std::vector<uint32_t> cReplies;
        bool cSuccess = !WriteI2C ( pVecReg, cReplies, false, true );

        //just as above, I can check the replies - there will be NCbc * pVecReg.size() write replies and also read replies if I chose to enable readback
        //this needs to be adapted
        if (pReadback)
        {
            //TODO: actually, i just need to check the read write and the info bit in each reply - if all info bits are 0, this is as good as it gets, else collect the replies that faild for decoding - potentially no iterative retrying
            //TODO: maybe I can do something with readback here - think about it
            for (auto& cWord : cReplies)
            {
                //it was a write transaction!
                if ( ( (cWord >> 17) & 0x1) == 0)
                {
                    // infor bit is 0 which means that the transaction was acknowledged by the CBC
                    if ( ( (cWord >> 20) & 0x1) == 0)
                        cSuccess = true;
                    else cSuccess == false;
                }
                else
                    cSuccess = false;

                //std::cout << std::bitset<32>(cWord) << std::endl;
            }

            //cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), ICGlibFWInterface::cmd_reply_ack);
            pVecReg.clear();
            pVecReg = cReplies;

        }

        return cSuccess;
    }

    void ICGlibFWInterface::ReadCbcBlockReg (  std::vector<uint32_t>& pVecReg )
    {
        std::vector<uint32_t> cReplies;
        //it sounds weird, but ReadI2C is called inside writeI2c, therefore here I have to write and disable the readback. The actual read command is in the words of the vector, no broadcast, maybe I can get rid of it
        WriteI2C ( pVecReg, cReplies, false, false);
        pVecReg.clear();
        pVecReg = cReplies;
    }

    void ICGlibFWInterface::CbcFastReset()
    {
        WriteReg ( "cbc_daq_ctrl.cbc_ctrl", FAST_RESET );
    }

    void ICGlibFWInterface::CbcHardReset()
    {
        WriteReg ( "cbc_daq_ctrl.cbc_ctrl", HARD_RESET );
    }

    void ICGlibFWInterface::CbcI2CRefresh()
    {
        WriteReg ("cbc_daq_ctrl.cbc_ctrl", I2C_REFRESH);
    }

    void ICGlibFWInterface::CbcTestPulse()
    {
        WriteReg ("cbc_daq_ctrl.cbc_ctrl", TEST_PULSE);
    }

    void ICGlibFWInterface::CbcTrigger()
    {
        WriteReg ("cbc_daq_ctrl.cbc_ctrl", L1A);
    }

    void ICGlibFWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is already uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->runUpload ( strConfig, pstrFile );
    }

    void ICGlibFWInterface::JumpToFpgaConfig ( const std::string& strConfig )
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new GlibFpgaConfig ( this );

        fpgaConfig->jumpToImage ( strConfig );
    }

    bool ICGlibFWInterface::cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2)
    {
        //TODO: cleanup
        if ( (cWord1 & 0x0F00FFFF) != (cWord2 & 0x0F00FFFF) )
            std::cout << std::endl << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 28) & 0xF) << " CbcId " << + ( (cWord1 >> 24) & 0xF) << " Read " << + ( (cWord1 >> 21) & 0x1) << " Write " << + ( (cWord1 >> 20) & 0x1) << " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF)  << std::endl << " ## " << std::bitset<32> (cWord2) << " ### Read:           CbcId " << + ( (cWord2 >> 24) & 0xF) << " Info " << + ( (cWord2 >> 20) & 0x1) << " Read? " << + ( (cWord2 >> 17) & 0x1) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  << std::endl;

        //if the Register is FrontEndControl at p0 addr0, page is not defined and therefore I ignore it!
        if ( ( (cWord1 >> 16) & 0x1) == 0 && ( (cWord1 >> 8 ) & 0xFF) == 0) return ( (cWord1 & 0x0F00FFFF) == (cWord2 & 0x0F00FFFF) );
        else return ( (cWord1 & 0x0F01FFFF) == (cWord2 & 0x0F01FFFF) );
    }

    bool ICGlibFWInterface::cmd_reply_ack (const uint32_t& cWord1, const
                                           uint32_t& cWord2)
    {
        // if Info (>>20) is 0 and  it was a write transaction (>>17 == 0) and
        // the CBC id matches it is false
        if ( ( (cWord2 >> 20) & 0x1) == 0 && ( (cWord2 >> 17) & 0x1 ) == 0 &&
                (cWord1 & 0x0F000000) == (cWord2 & 0x0F000000) ) return true;
        else
            return false;
    }


}
