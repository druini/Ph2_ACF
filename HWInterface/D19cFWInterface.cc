/*!

        \file                           D19cFWInterface.h
        \brief                          D19cFWInterface init/config of the FC7 and its Cbc's
        \author                         G. Auzinger, K. Uchida, M. Haranko
        \version            1.0
        \date                           24.03.2017
        Support :                       mail to : georg.auzinger@SPAMNOT.cern.ch
                                                  mykyta.haranko@SPAMNOT.cern.ch

 */


#include <time.h>
#include <chrono>
#include <uhal/uhal.hpp>
#include "D19cFWInterface.h"
#include "CtaFpgaConfig.h"

//#include "CbcInterface.h"


namespace Ph2_HwInterface {

    D19cFWInterface::D19cFWInterface ( const char* puHalConfigFileName,
            uint32_t pBoardId ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig (nullptr),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFMCId (1)
    {}


    D19cFWInterface::D19cFWInterface ( const char* puHalConfigFileName,
            uint32_t pBoardId,
            FileHandler* pFileHandler ) :
        BeBoardFWInterface ( puHalConfigFileName, pBoardId ),
        fpgaConfig (nullptr),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFileHandler ( pFileHandler ),
        fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    D19cFWInterface::D19cFWInterface ( const char* pId,
            const char* pUri,
            const char* pAddressTable ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFMCId (1)
    {}


    D19cFWInterface::D19cFWInterface ( const char* pId,
            const char* pUri,
            const char* pAddressTable,
            FileHandler* pFileHandler ) :
        BeBoardFWInterface ( pId, pUri, pAddressTable ),
        fpgaConfig ( nullptr ),
        fBroadcastCbcId (0),
        fNCbc (0),
        fFileHandler ( pFileHandler ),
        fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
    }

    void D19cFWInterface::setFileHandler (FileHandler* pHandler)
    {
        if (pHandler != nullptr )
        {
            fFileHandler = pHandler;
            fSaveToFile = true;
        }
        else LOG (INFO) << "Error, can not set NULL FileHandler" ;
    }

    void D19cFWInterface::ReadErrors() {
        int error_counter = ReadReg("fc7_daq_stat.general.global_error.counter");
        if (error_counter == 0) {
            LOG (INFO) << "No Errors detected";
        }
        else {
            std::vector<uint32_t> pErrors = ReadBlockRegValue("fc7_daq_stat.general.global_error.full_error", error_counter);
            for (auto& cError : pErrors)
            {
                int error_block_id = (cError & 0x0000000f);
                int error_code = ((cError & 0x00000ff0) >> 4);
                LOG (ERROR) << "Block: " << BOLDRED << error_block_id << RESET << ", Code: " << BOLDRED << error_code << RESET;
            }
        }
    }

    uint32_t D19cFWInterface::getBoardInfo()
    {
        // firmware info
        LOG (INFO) << GREEN << "============================" << RESET;
        LOG (INFO) << BOLDGREEN << "General Firmware Info" << RESET;

        int implementation = ReadReg("fc7_daq_stat.general.info.implementation");
        int cbc_version = ReadReg("fc7_daq_stat.general.info.cbc_version");
        int num_hybrids = ReadReg("fc7_daq_stat.general.info.num_hybrids");
        int num_chips = ReadReg("fc7_daq_stat.general.info.num_chips");

        if (implementation == 0)
            LOG (INFO) << "Implementation: " << BOLDGREEN << "Optical" << RESET;
        else if (implementation == 1)
            LOG (INFO) << "Implementation: " << BOLDGREEN << "Electrical" << RESET;
        else if (implementation == 2)
            LOG (INFO) << "Implementation: " << BOLDGREEN << "CBC3 Emulation" << RESET;
        else
            LOG (WARNING) << "Implementation: " << BOLDRED << "Unknown" << RESET;

        LOG (INFO) << "CBC Version: " << BOLDGREEN << cbc_version << RESET;
        LOG (INFO) << "Number of Hybrids: " << BOLDGREEN << num_hybrids << RESET;
        LOG (INFO) << "Number of Chips per Hybrid: " << BOLDGREEN << num_chips << RESET;

        // temporary used for board status printing
        LOG (INFO) << YELLOW << "============================" << RESET;
        LOG (INFO) << BOLDYELLOW << "Current Status" << RESET;

        ReadErrors();

        int source_id = ReadReg("fc7_daq_stat.fast_command_block.general.source");
        double user_frequency = ReadReg("fc7_daq_cnfg.fast_command_block.user_trigger_frequency");
        if (source_id == 1)
            LOG (INFO) << "Trigger Source: " << BOLDGREEN << "L1-Trigger" << RESET;
        else if (source_id == 2)
            LOG (INFO) << "Trigger Source: " << BOLDGREEN << "Stubs" << RESET;
        else if (source_id == 3)
            LOG (INFO) << "Trigger Source: " << BOLDGREEN << "User Frequency (" << user_frequency << " kHz)" << RESET;
        else if (source_id == 4)
            LOG (INFO) << "Trigger Source: " << BOLDGREEN << "TLU" << RESET;
        else if (source_id == 5)
            LOG (INFO) << "Trigger Source: " << BOLDGREEN << "Ext Trigger (DIO5)" << RESET;
        else if (source_id == 6)
            LOG (INFO) << "Trigger Source: " << BOLDGREEN << "Test Pulse Trigger" << RESET;
        else
            LOG (WARNING) << " Trigger Source: " << BOLDRED << "Unknown" << RESET;

        int state_id = ReadReg("fc7_daq_stat.fast_command_block.general.fsm_state");
        if (state_id == 0)
            LOG (INFO) << "Trigger State: " << BOLDGREEN << "Idle" << RESET;
        else if (state_id == 1)
            LOG (INFO) << "Trigger State: " << BOLDGREEN << "Running" << RESET;
        else if (state_id == 2)
            LOG (INFO) << "Trigger State: " << BOLDGREEN << "Paused. Waiting for readout" << RESET;
        else
            LOG (WARNING) << " Trigger State: " << BOLDRED << "Unknown" << RESET;

        int i2c_replies_empty = ReadReg("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
        if (i2c_replies_empty==0)
            LOG (INFO) << "I2C Replies Available: " << BOLDGREEN << "Yes" << RESET;
        else LOG (INFO) << "I2C Replies Available: " << BOLDGREEN << "No" << RESET;

        LOG (INFO) << YELLOW << "============================" << RESET;
        LOG (INFO) << BOLDYELLOW << "Frequency Checker:" << RESET;
        float ipb_clk_rate = ReadReg("fc7_daq_stat.test_clock.ipb_clk_rate")/10000.0;
        float forty_mhz_clk_rate = ReadReg("fc7_daq_stat.test_clock.40mhz_clk_rate")/10000.0;
        float user_clk_rate = ReadReg("fc7_daq_stat.test_clock.trigger_rate")/10.0;
        LOG (INFO) << "IPBus Clock: " << ipb_clk_rate << "MHz";
        LOG (INFO) << "40MHz Clock: " << forty_mhz_clk_rate << "MHz";
        LOG (INFO) << "Trigger Clock: " << user_clk_rate << "kHz";

        LOG (INFO) << YELLOW << "============================" << RESET;

        uint32_t cVersionWord = 0;
        return cVersionWord;
    }

    void D19cFWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        WriteReg ("fc7_daq_ctrl.command_processor_block.global.reset", 0x1);

        usleep(500);

        // read info about current firmware
        int cbc_version = ReadReg("fc7_daq_stat.general.info.cbc_version");
        fFWNHybrids = ReadReg("fc7_daq_stat.general.info.num_hybrids");
        fFWNChips = ReadReg("fc7_daq_stat.general.info.num_chips");

        fNCbc = 0;
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        LOG (INFO) << BOLDGREEN << "According to the Firmware status registers, it was compiled for: " << fFWNHybrids << " hybrid(s), " << fFWNChips << " CBC" << cbc_version << " chip(s) per hybrid" << RESET;

        int fNHybrids = 0;
        uint16_t hybrid_enable = 0;
        uint8_t *chips_enable = new uint8_t[16];
        for(int i=0; i<16; i++) chips_enable[i] = 0;
        //then loop the HWDescription and find out about our Connected CBCs
        for (Module* cFe : pBoard->fModuleVector)
        {
            fNHybrids++;
            LOG (INFO) << "Enabling Hybrid " << (int)cFe->getFeId();
            hybrid_enable |= 1 << cFe->getFeId();
            //configure the CBCs - preliminary FW only supports 1 CBC but put the rest of the code there and comment
            for ( Cbc* cCbc : cFe->fCbcVector)
            {
                LOG (INFO) << "     Enabling Chip " << (int)cCbc->getCbcId();
                chips_enable[cFe->getFeId()] |= 1 << cCbc->getCbcId();
                //need to increment the NCbc counter for I2C controller
                fNCbc++;
            }
        }
        cVecReg.push_back({"fc7_daq_cnfg.global.hybrid_enable",hybrid_enable});
        for(uint32_t i=0; i<16; i++) {
            char name[50];
            std::sprintf(name, "fc7_daq_cnfg.global.chips_enable_hyb_%02d", i);
            std::string name_str(name);
            cVecReg.push_back({name_str, chips_enable[i]});
        }
        delete chips_enable;
        LOG (INFO) << BOLDGREEN << fNHybrids << " hybrid(s) was(were) enabled with the total amount of " << fNCbc << " chip(s)!" << RESET;

        //last, loop over the variable registers from the HWDescription.xml file
        //this is where I should get all the clocking and FastCommandInterface settings
        BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

        for ( auto const& it : cGlibRegMap )
        {
            cVecReg.push_back ( {it.first, it.second} );
        }

        WriteStackReg ( cVecReg );
        cVecReg.clear();

        // load trigger configuration
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);

        // load dio5 configuration
        WriteReg("fc7_daq_ctrl.dio5_block.control.load_config", 0x1);

        // ping all cbcs (reads data from registers #0)
        uint32_t cInit = ( ( (2) << 28 ) | (  (0) << 18 )  | ( (0) << 17 ) | ( (1) << 16 ) | (0 << 8 ) | 0);

        WriteReg("fc7_daq_ctrl.command_processor_block.i2c.command_fifo", cInit);
        //read the replies for the pings!
        std::vector<uint32_t> pReplies;
        uint32_t cWord;
        bool cReadSuccess = !ReadI2C (fNCbc, pReplies);
        bool cWordCorrect = true;
        if (cReadSuccess) {
            for (int i=0; i < pReplies.size(); i++) {
                cWord = pReplies.at(i);
                //cWordCorrect = ( (((cWord) & 0x00f00000) >> 20) == i%num_chips ) ? true : false;
                // has to be reimplemented, because now some chips can be disabled and i doesn't correspond to the actual chip id
                cWordCorrect = true;
                if (!cWordCorrect) break;
            }
        }

        if (cReadSuccess && cWordCorrect) LOG (INFO) << "Successfully received *Pings* from " << fNCbc << " Cbcs";

        if (!cReadSuccess) LOG (ERROR) << "Did not receive the correct number of *Pings*; expected: " << fNCbc << ", received: " << pReplies.size() ;
        if (!cWordCorrect) LOG (ERROR) << "CBC's ids are not correct!";
    }

    void D19cFWInterface::FindPhase()
    {
        ;
    }


    void D19cFWInterface::Start()
    {

        WriteReg ("fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
    }

    void D19cFWInterface::Stop()
    {
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
    }


    void D19cFWInterface::Pause()
    {
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
    }


    void D19cFWInterface::Resume()
    {
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
    }

    uint32_t D19cFWInterface::ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData )
    {
        uint32_t cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");

        while (cNWords == 0)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        }

        uint32_t cNEvents = 0;
        while(cNWords > 0) {
            // reading header 1
            uint32_t header1 = ReadReg ("fc7_daq_ctrl.readout_block.readout_fifo");
            uint32_t cEventSize = (0x0000FFFF & header1);

            while (cNWords < cEventSize-1)
            {
                LOG(INFO) << "Need: " << cEventSize-1 << " words for this event, Get: " << cNWords;
                std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");

            }
            pData.push_back(header1);
            std::vector<uint32_t> rest_of_data = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cEventSize-1);
            pData.insert(pData.end(), rest_of_data.begin(), rest_of_data.end());
            cNEvents++;
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        }


        if (fSaveToFile)
            fFileHandler->set (pData);

        //need to return the number of events read
        return cNEvents;
    }


    void D19cFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData )
    {

        // first write the amount of the test pulses to be sent
        WriteReg("fc7_daq_cnfg.fast_command_block.triggers_to_accept", pNEvents);        
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);
        usleep(1);

        // start triggering machine which will collect N events
        this->Start();

        bool failed = false;
        for(uint32_t event; event < pNEvents; event++) {            
            uint32_t cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");            

            int cNTries = 0;
            int cNTriesMax = 50;
            while (cNWords < 1)
            {
                if(cNTries >= cNTriesMax) {
                    uint32_t state_id = ReadReg("fc7_daq_stat.fast_command_block.general.fsm_state");
                    if (state_id == 0) {
                        LOG(INFO) << "After fsm stopped, still no data: resetting and re-trying";
                        failed = true;
                        break;
                    }
                    else cNTries = 0;
                }

                std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
                cNTries++;
            }
            if (failed) break;

            // reading header 1
            uint32_t header1 = ReadReg ("fc7_daq_ctrl.readout_block.readout_fifo");
            uint32_t cEventSize = (0x0000FFFF & header1);

            while (cNWords < cEventSize-1)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            }
            pData.push_back(header1);
            std::vector<uint32_t> rest_of_data = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cEventSize-1);
            pData.insert(pData.end(), rest_of_data.begin(), rest_of_data.end());

        }
        if(failed) {

            pData.clear();
            this->Stop();

            WriteReg("fc7_daq_ctrl.readout_block.control.readout_reset", 0x1);
            usleep(10);
            WriteReg("fc7_daq_ctrl.readout_block.control.readout_reset", 0x0);
            usleep(10);

            this->ReadNEvents(pBoard,pNEvents,pData);
        }

        if (fSaveToFile)
            fFileHandler->set (pData);
    }

    /** compute the block size according to the number of CBC's on this board
     * this will have to change with a more generic FW */
    uint32_t D19cFWInterface::computeEventSize ( BeBoard* pBoard )
    {
        return 0;
    }

    std::vector<uint32_t> D19cFWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();
        return vBlock;
    }

    bool D19cFWInterface::WriteBlockReg ( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
    {
        bool cWriteCorr = RegManager::WriteBlockReg ( pRegNode, pValues );
        return cWriteCorr;
    }

    ///////////////////////////////////////////////////////
    //      CBC Methods                                 //
    /////////////////////////////////////////////////////
    //TODO: check what to do with fFMCid and if I need it!
    // this is clearly for addressing individual CBCs, have to see how to deal with broadcast commands

    void D19cFWInterface::EncodeReg ( const CbcRegItem& pRegItem,
                                         uint8_t pCbcId,
                                         std::vector<uint32_t>& pVecReq,
                                         bool pReadBack,
                                         bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        bool pUseMask = false;
        uint8_t pFeId = 0;
        pVecReq.push_back ( ( 0 << 28 ) | ( pFeId << 24 ) | ( pCbcId << 20 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue);
    }

    void D19cFWInterface::EncodeReg (const CbcRegItem& pRegItem,
                                         uint8_t pFeId,
                                         uint8_t pCbcId,
                                         std::vector<uint32_t>& pVecReq,
                                         bool pReadBack,
                                         bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        bool pUseMask = false;
        pVecReq.push_back ( ( 0 << 28 ) | ( pFeId << 24 ) | ( pCbcId << 20 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }

    void D19cFWInterface::BCEncodeReg ( const CbcRegItem& pRegItem,
                                           uint8_t pNCbc,
                                           std::vector<uint32_t>& pVecReq,
                                           bool pReadBack,
                                           bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        bool pUseMask = false;
        pVecReq.push_back ( ( 2 << 28 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }


    void D19cFWInterface::DecodeReg ( CbcRegItem& pRegItem,
                                         uint8_t& pCbcId,
                                         uint32_t pWord,
                                         bool& pRead,
                                         bool& pFailed )
    {
        //pFeId    =  ( ( pWord & 0x0f000000 ) >> 24) ;
        pCbcId   =  ( ( pWord & 0x00f00000 ) >> 20) ;
        pFailed  =  0 ;
        pRegItem.fPage    =  ( (pWord & 0x00020000  ) >> 17) ;
        pRead    =  ( pWord & 0x00010000 ) >> 16;
        pRegItem.fAddress =  ( pWord & 0x0000FF00 ) >> 8;
        pRegItem.fValue   =  ( pWord & 0x000000FF );
    }

    bool D19cFWInterface::ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies)
    {
        bool cFailed (false);        

        uint32_t single_WaitingTime = SINGLE_I2C_WAIT*pNReplies;
        uint32_t max_Attempts = 100;
        uint32_t counter_Attempts = 0;

        //read the number of received replies from ndata and use this number to compare with the number of expected replies and to read this number 32-bit words from the reply FIFO
        usleep(single_WaitingTime);
        uint32_t cNReplies = ReadReg ("fc7_daq_stat.command_processor_block.i2c.nreplies");
        while (cNReplies != pNReplies)
        {
            if (counter_Attempts > max_Attempts) {
                LOG (INFO) << "Error: Read " << cNReplies << " I2C replies whereas " << pNReplies << " are expected!" ;
                ReadErrors();
                cFailed = true;
                break;
            }

            usleep(single_WaitingTime);
            cNReplies = ReadReg ("fc7_daq_stat.command_processor_block.i2c.nreplies");
            counter_Attempts++;
        }

        try
        {
            pReplies = ReadBlockRegValue ( "fc7_daq_ctrl.command_processor_block.i2c.reply_fifo", cNReplies );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        //reset the i2c controller here?
        return cFailed;
    }

    bool D19cFWInterface::WriteI2C ( std::vector<uint32_t>& pVecSend, std::vector<uint32_t>& pReplies, bool pReadback, bool pBroadcast )
    {
        bool cFailed ( false );
        //reset the I2C controller
        WriteReg ("fc7_daq_ctrl.command_processor_block.i2c.control.reset_fifos", 0x1);
        usleep(10);
        try
        {
            WriteBlockReg ( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", pVecSend );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        uint32_t cNReplies = 0;
        for(auto word: pVecSend) {
            // if read or readback for write == 1, then count
            if ( (((word & 0x00010000) >> 16) == 1) or (((word & 0x00080000) >> 19) == 1) ) {
                if (pBroadcast) cNReplies += fNCbc;
                else cNReplies += 1;
            }
        }

        cFailed = ReadI2C (  cNReplies, pReplies) ;

        return cFailed;
    }


    bool D19cFWInterface::WriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback)
    {

        uint8_t cMaxWriteAttempts = 5;
        // the actual write & readback command is in the vector
        std::vector<uint32_t> cReplies;
        bool cSuccess = !WriteI2C ( pVecReg, cReplies, pReadback, false );

        //for (int i = 0; i < pVecReg.size(); i++)
        //{
        //LOG (DEBUG) << std::bitset<16> ( pVecReg.at (i)  >> 16)  << " " << std::bitset<16> ( pVecReg.at (i) );
        //LOG (DEBUG) << std::bitset<16> ( cReplies.at (2 * i)  >> 16)  << " " << std::bitset<16> ( cReplies.at (2 * i) );
        //LOG (DEBUG) << std::bitset<16> ( cReplies.at (2 * i + 1 )  >> 16)  << " " << std::bitset<16> ( cReplies.at (2 * i + 1 ) );
        //LOG (DEBUG) << std::endl;
        //}

        //LOG (DEBUG) << "Command Size: " << pVecReg.size() << " Reply size " << cReplies.size();

        // the reply format is different from the sent format, therefore a binary predicate is necessary to compare
        // fValue is in the 8 lsb, then address is in 15 downto 8, page is in 16, CBCId is in 24

        //here make a distinction: if pReadback is true, compare only the read replies using the binary predicate
        //else, just check that info is 0 and thus the CBC acqnowledged the command if the writeread is 0
        std::vector<uint32_t> cWriteAgain;

        if (pReadback)
        {
            //split the reply vector in even and odd replies
            //even is the write reply, odd is the read reply
            //since I am already reading back, might as well forget about the CMD acknowledge from the CBC and directly look at the read back value
            //std::vector<uint32_t> cOdd;
            //getOddElements (cReplies, cOdd);

            //now use the Template from BeBoardFWInterface to return a vector with all written words that have been read back incorrectly
            cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), D19cFWInterface::cmd_reply_comp);

            // now clear the initial cmd Vec and set the read-back
            pVecReg.clear();
            pVecReg = cReplies;
        }
        else
        {
            //since I do not read back, I can safely just check that the info bit of the reply is 0 and that it was an actual write reply
            //then i put the replies in pVecReg so I can decode later in CBCInterface
            //cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), D19cFWInterface::cmd_reply_ack);
            pVecReg.clear();
            pVecReg = cReplies;
        }

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
                else LOG (INFO) << BOLDRED <<  "(WRITE#"  << std::to_string (pWriteAttempts) << ") There were " << cWriteAgain.size() << " CBC CMD acknowledge bits missing -trying again!" << RESET ;

                pWriteAttempts++;
                this->WriteCbcBlockReg ( cWriteAgain, pWriteAttempts, true);
            }
            else if ( pWriteAttempts >= cMaxWriteAttempts )
            {
                cSuccess = false;
                pWriteAttempts = 0 ;
            }
            else throw Exception ( "Too many CBC readback errors - no functional I2C communication. Check the Setup" );
        }


        return cSuccess;
    }

    bool D19cFWInterface::BCWriteCbcBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback)
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
                if ( ( (cWord >> 16) & 0x1) == 0)
                {
                    // infor bit is 0 which means that the transaction was acknowledged by the CBC
                    //if ( ( (cWord >> 20) & 0x1) == 0)
                        cSuccess = true;
                    //else cSuccess == false;
                }
                else
                    cSuccess = false;

                //LOG(INFO) << std::bitset<32>(cWord) ;
            }

            //cWriteAgain = get_mismatches (pVecReg.begin(), pVecReg.end(), cReplies.begin(), Cbc3Fc7FWInterface::cmd_reply_ack);
            pVecReg.clear();
            pVecReg = cReplies;

        }

        return cSuccess;
    }

    void D19cFWInterface::ReadCbcBlockReg (  std::vector<uint32_t>& pVecReg )
    {
        std::vector<uint32_t> cReplies;
        //it sounds weird, but ReadI2C is called inside writeI2c, therefore here I have to write and disable the readback. The actual read command is in the words of the vector, no broadcast, maybe I can get rid of it
        WriteI2C ( pVecReg, cReplies, false, false);
        pVecReg.clear();
        pVecReg = cReplies;
    }

    void D19cFWInterface::CbcFastReset()
    {
        WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_reset", 0x1 );
    }

    void D19cFWInterface::CbcI2CRefresh()
    {
        WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_i2c_refresh", 0x1 );
    }

    void D19cFWInterface::CbcHardReset()
    {
        ;
    }

    void D19cFWInterface::CbcTestPulse()
    {
        ;
    }

    void D19cFWInterface::CbcTrigger()
    {
        WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_trigger", 0x1 );
    }

    void D19cFWInterface::FlashProm ( const std::string& strConfig, const char* pstrFile )
    {
        checkIfUploading();

        fpgaConfig->runUpload ( strConfig, pstrFile );
    }

    void D19cFWInterface::JumpToFpgaConfig ( const std::string& strConfig)
    {
        checkIfUploading();

        fpgaConfig->jumpToImage ( strConfig);
    }

    void D19cFWInterface::DownloadFpgaConfig ( const std::string& strConfig, const std::string& strDest)
    {
        checkIfUploading();
        fpgaConfig->runDownload ( strConfig, strDest.c_str() );
    }

    std::vector<std::string> D19cFWInterface::getFpgaConfigList()
    {
        checkIfUploading();
        return fpgaConfig->getFirmwareImageNames( );
    }

    void D19cFWInterface::DeleteFpgaConfig ( const std::string& strId)
    {
        checkIfUploading();
        fpgaConfig->deleteFirmwareImage ( strId);
    }

    void D19cFWInterface::checkIfUploading()
    {
        if ( fpgaConfig && fpgaConfig->getUploadingFpga() > 0 )
            throw Exception ( "This board is uploading an FPGA configuration" );

        if ( !fpgaConfig )
            fpgaConfig = new CtaFpgaConfig ( this );
    }

    bool D19cFWInterface::cmd_reply_comp (const uint32_t& cWord1, const uint32_t& cWord2)
    {
        //TODO: cleanup
        //if ( (cWord1 & 0x0F00FFFF) != (cWord2 & 0x0F00FFFF) )
        //{
        //LOG (INFO)  << " ## " << std::bitset<32> (cWord1) << " ### Written: FMCId " <<  + ( (cWord1 >> 29) & 0xF) << " CbcId " << + ( (cWord1 >> 24) & 0xF) << " Read " << + ( (cWord1 >> 21) & 0x1) << " Write " << + ( (cWord1 >> 20) & 0x1) << " Page  " << + ( (cWord1 >> 16) & 0x1) << " Address " << + ( (cWord1 >> 8) & 0xFF) << " Value " << + ( (cWord1) & 0xFF);

        //LOG (INFO) << " ## " << std::bitset<32> (cWord2) << " ### Read:           CbcId " << + ( (cWord2 >> 24) & 0xF) << " Info " << + ( (cWord2 >> 20) & 0x1) << " Read? " << + ( (cWord2 >> 17) & 0x1) << " Page  " << + ( (cWord2 >> 16) & 0x1) << " Address " << + ( (cWord2 >> 8) & 0xFF) << " Value " << + ( (cWord2) & 0xFF)  ;
        //}

        //if the Register is FrontEndControl at p0 addr0, page is not defined and therefore I ignore it!
        //if ( ( (cWord1 >> 16) & 0x1) == 0 && ( (cWord1 >> 8 ) & 0xFF) == 0) return ( (cWord1 & 0x0F00FFFF) == (cWord2 & 0x0F00FFFF) );
        //else return ( (cWord1 & 0x0F01FFFF) == (cWord2 & 0x0F01FFFF) );

        return ( (cWord1 & 0x00F2FFFF) == (cWord2 & 0x00F2FFFF) );

    }

    bool D19cFWInterface::cmd_reply_ack (const uint32_t& cWord1, const
                                            uint32_t& cWord2)
    {
        // if it was a write transaction (>>17 == 0) and
        // the CBC id matches it is false
        if (  ( (cWord2 >> 16) & 0x1 ) == 0 &&
                (cWord1 & 0x00F00000) == (cWord2 & 0x00F00000) ) return true;
        else
            return false;
    }


}
