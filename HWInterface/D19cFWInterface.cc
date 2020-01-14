/*!

        \file                           D19cFWInterface.h
        \brief                          D19cFWInterface init/config of the FC7 and its Chip's
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
#include "D19cFpgaConfig.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/OuterTrackerModule.h"

#pragma GCC diagnostic ignored "-Wpedantic"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
    D19cFWInterface::D19cFWInterface ( const char* puHalConfigFileName,
     uint32_t pBoardId ) 
    : BeBoardFWInterface ( puHalConfigFileName, pBoardId )
    , fpgaConfig (nullptr)
    , fFileHandler (nullptr)
    , fBroadcastCbcId (0)
    , fNCbc (0)
    , fNMPA (0)
    , fNSSA (0)
    , fFMCId (1)
    {fResetAttempts = 0 ; }


    D19cFWInterface::D19cFWInterface ( const char* puHalConfigFileName,
     uint32_t pBoardId,
     FileHandler* pFileHandler ) 
    : BeBoardFWInterface ( puHalConfigFileName, pBoardId )
    , fpgaConfig (nullptr)
    , fFileHandler ( pFileHandler )
    , fBroadcastCbcId (0)
    , fNCbc (0)
    , fNMPA (0)
    , fNSSA (0)
    , fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
        fResetAttempts = 0 ; 
    }

    D19cFWInterface::D19cFWInterface ( const char* pId,
     const char* pUri,
     const char* pAddressTable ) 
    : BeBoardFWInterface ( pId, pUri, pAddressTable )
    , fpgaConfig (nullptr)
    , fFileHandler (nullptr)
    , fBroadcastCbcId (0)
    , fNCbc (0)
    , fNMPA (0)
    , fNSSA (0)
    , fFMCId (1)
    {fResetAttempts = 0 ; }

    D19cFWInterface::D19cFWInterface ( const char* pId,
     const char* pUri,
     const char* pAddressTable,
     FileHandler* pFileHandler ) 
    : BeBoardFWInterface ( pId, pUri, pAddressTable )
    , fpgaConfig (nullptr)
    , fFileHandler ( pFileHandler )
    , fBroadcastCbcId (0)
    , fNCbc (0)
    , fNMPA (0)
    , fNSSA (0)
    , fFMCId (1)
    {
        if ( fFileHandler == nullptr ) fSaveToFile = false;
        else fSaveToFile = true;
        fResetAttempts = 0 ; 
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

    void D19cFWInterface::ReadErrors()
    {
        int error_counter = ReadReg ("fc7_daq_stat.general.global_error.counter");

        if (error_counter == 0)
            LOG (INFO) << "No Errors detected";
        else
        {
            std::vector<uint32_t> pErrors = ReadBlockRegValue ("fc7_daq_stat.general.global_error.full_error", error_counter);

            for (auto& cError : pErrors)
            {
                int error_block_id = (cError & 0x0000000f);
                int error_code = ( (cError & 0x00000ff0) >> 4);
                LOG (ERROR) << "Block: " << BOLDRED << error_block_id << RESET << ", Code: " << BOLDRED << error_code << RESET;
            }
        }
    }

    std::string D19cFWInterface::getFMCCardName (uint32_t id)
    {
        std::string name = "";

        switch (id)
        {
            case 0x0:
            name = "None";
            break;

            case 0x1:
            name = "DIO5";
            break;

            case 0x2:
            name = "2xCBC2";
            break;

            case 0x3:
            name = "8xCBC2";
            break;

            case 0x4:
            name = "2xCBC3";
            break;

            case 0x5:
            name = "8xCBC3_FMC1";
            break;

            case 0x6:
            name = "8xCBC3_FMC2";
            break;

            case 0x7:
            name = "FMC_1CBC3";
            break;

            case 0x8:
            name = "FMC_MPA_SSA_BOARD";
            break;

            case 0x9:
            name = "FMC_FERMI_TRIGGER_BOARD";
            break;

            case 0xe:
            name = "OPTO_QUAD";
            break;

            case 0xf:
            name = "UNKNOWN";
            break;
        }

        return name;
    }

    std::string D19cFWInterface::getChipName (uint32_t pChipCode)
    {
        std::string name = "UNKNOWN";

        switch (pChipCode)
        {
            case 0x0:
            name = "CBC2";
            break;

            case 0x1:
            name = "CBC3";
            break;

            case 0x2:
            name = "MPA";
            break;

            case 0x3:
            name = "SSA";
            break;
        }

        return name;
    }

    FrontEndType D19cFWInterface::getFrontEndType (uint32_t pChipCode)
    {
        FrontEndType chip_type = FrontEndType::UNDEFINED;

        switch (pChipCode)
        {
            case 0x1:
            chip_type = FrontEndType::CBC3;
            break;

            case 0x2:
            chip_type = FrontEndType::MPA;
            break;

            case 0x3:
            chip_type = FrontEndType::SSA;
            break;
            
            case 0x4:
            chip_type = FrontEndType::CIC;
            break;

        }

        return chip_type;
    }

    uint32_t D19cFWInterface::getBoardInfo()
    {
        // firmware info
        LOG (INFO) << GREEN << "============================" << RESET;
        LOG (INFO) << BOLDGREEN << "General Firmware Info" << RESET;

        int implementation = ReadReg ("fc7_daq_stat.general.info.implementation");
        int chip_code = ReadReg ("fc7_daq_stat.general.info.chip_type");
        int num_hybrids = ReadReg ("fc7_daq_stat.general.info.num_hybrids");
        int num_chips = ReadReg ("fc7_daq_stat.general.info.num_chips");
        uint32_t fmc1_card_type = ReadReg ("fc7_daq_stat.general.info.fmc1_card_type");
        uint32_t fmc2_card_type = ReadReg ("fc7_daq_stat.general.info.fmc2_card_type");
        int firmware_timestamp = ReadReg("fc7_daq_stat.general.firmware_timestamp");

        LOG(INFO) << "Compiled on: " << BOLDGREEN << ((firmware_timestamp >> 27) & 0x1F) << "." << ((firmware_timestamp >> 23) & 0xF) << "." << ((firmware_timestamp >> 17) & 0x3F) << " " << ((firmware_timestamp >> 12) & 0x1F) << ":" << ((firmware_timestamp >> 6) & 0x3F) << ":" << ((firmware_timestamp >> 0) & 0x3F) << " (dd.mm.yy hh:mm:ss)" << RESET;

        if (implementation == 0)
            LOG (INFO) << "Implementation: " << BOLDGREEN << "Optical" << RESET;
        else if (implementation == 1)
            LOG (INFO) << "Implementation: " << BOLDGREEN << "Electrical" << RESET;
        else if (implementation == 2)
            LOG (INFO) << "Implementation: " << BOLDGREEN << "CBC3 Emulation" << RESET;
        else
            LOG (WARNING) << "Implementation: " << BOLDRED << "Unknown" << RESET;

        LOG (INFO) << BOLDYELLOW << "FMC1 Card: " << RESET << getFMCCardName (fmc1_card_type);
        LOG (INFO) << BOLDYELLOW << "FMC2 Card: " << RESET << getFMCCardName (fmc2_card_type);

        LOG (INFO) << "Chip Type: " << BOLDGREEN << getChipName (chip_code) << RESET;
        LOG (INFO) << "Number of Hybrids: " << BOLDGREEN << num_hybrids << RESET;
        LOG (INFO) << "Number of Chips per Hybrid: " << BOLDGREEN << num_chips << RESET;

        // temporary used for board status printing
        LOG (INFO) << YELLOW << "============================" << RESET;
        LOG (INFO) << BOLDYELLOW << "Current Status" << RESET;

        ReadErrors();

        int source_id = ReadReg ("fc7_daq_stat.fast_command_block.general.source");
        double user_frequency = ReadReg ("fc7_daq_cnfg.fast_command_block.user_trigger_frequency");

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

        int state_id = ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state");

        if (state_id == 0)
            LOG (INFO) << "Trigger State: " << BOLDGREEN << "Idle" << RESET;
        else if (state_id == 1)
            LOG (INFO) << "Trigger State: " << BOLDGREEN << "Running" << RESET;
        else if (state_id == 2)
            LOG (INFO) << "Trigger State: " << BOLDGREEN << "Paused. Waiting for readout" << RESET;
        else
            LOG (WARNING) << " Trigger State: " << BOLDRED << "Unknown" << RESET;

        int i2c_replies_empty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");

        if (i2c_replies_empty == 0)
            LOG (INFO) << "I2C Replies Available: " << BOLDGREEN << "Yes" << RESET;
        else LOG (INFO) << "I2C Replies Available: " << BOLDGREEN << "No" << RESET;

        LOG (INFO) << YELLOW << "============================" << RESET;

        uint32_t cVersionWord = 0;
        return cVersionWord;
    }

    void D19cFWInterface::ConfigureBoard ( const BeBoard* pBoard )
    {
        // after firmware loading it seems that CBC3 is not super stable
        // and it needs fast reset after, so let's be secure and do also the hard one..
        this->ChipReset();
        this->ChipReSync();
        usleep (1);

        WriteReg ("fc7_daq_ctrl.command_processor_block.global.reset", 0x1);

        usleep (500);

    // read info about current firmware
        uint32_t cFrontEndTypeCode = ReadReg ("fc7_daq_stat.general.info.chip_type");
        std::cout << __PRETTY_FUNCTION__ << "\t" << cFrontEndTypeCode << std::endl;

        std::string cChipName = getChipName (cFrontEndTypeCode);
        fFirmwareFrontEndType = getFrontEndType (cFrontEndTypeCode);
        fFWNHybrids = ReadReg ("fc7_daq_stat.general.info.num_hybrids");
        fFWNChips = ReadReg ("fc7_daq_stat.general.info.num_chips");
        fCBC3Emulator = (ReadReg ("fc7_daq_stat.general.info.implementation") == 2);
        fIsDDR3Readout = (ReadReg("fc7_daq_stat.ddr3_block.is_ddr3_type") == 1);
        fI2CVersion = (ReadReg("fc7_daq_stat.command_processor_block.i2c.master_version"));
        if(fI2CVersion >= 1) this->SetI2CAddressTable();

        fNCbc = 0;
        std::vector< std::pair<std::string, uint32_t> > cVecReg;

        LOG (INFO) << BOLDGREEN << "According to the Firmware status registers, it was compiled for: " << fFWNHybrids << " hybrid(s), " << fFWNChips << " " << cChipName << " chip(s) per hybrid" << RESET;
        if( fFirmwareFrontEndType == FrontEndType::CIC ) 
        {
            LOG (INFO) << BOLDBLUE << "Enabling CIC clock" << RESET; 
            // make sure CIC is receiving clock 
            WriteReg( "fc7_daq_cnfg.physical_interface_block.cic.clock_enable" , 1 ) ;
            std::this_thread::sleep_for (std::chrono::milliseconds (100) );
        }
        else
        {
            LOG (INFO) << BOLDBLUE << "Firmware NOT configured for a CIC" << RESET;
        }

        int fNHybrids = 0;
        uint16_t hybrid_enable = 0;
        uint8_t* chips_enable = new uint8_t[16];

        for (int i = 0; i < 16; i++) chips_enable[i] = 0;
    //then loop the HWDescription and find out about our Connected CBCs
            for (Module* cFe : pBoard->fModuleVector)
            {
                fNHybrids++;
                LOG (INFO) << "Enabling Hybrid " << (int) cFe->getFeId();
                hybrid_enable |= 1 << cFe->getFeId();

                if (fFirmwareFrontEndType == FrontEndType::CBC3) {
                    for ( Chip* cCbc : cFe->fReadoutChipVector)
                    {
                        LOG (INFO) << "     Enabling CBC3 Chip " << (int) cCbc->getChipId();
                        chips_enable[cFe->getFeId()] |= 1 << cCbc->getChipId();
                //need to increment the NCbc counter for I2C controller
                        fNCbc++;
                    }
                } else if (fFirmwareFrontEndType == FrontEndType::MPA) {
                    for ( MPA* cMPA : static_cast<OuterTrackerModule*>(cFe)->fMPAVector)
                    {
                        LOG (INFO) << "     Enabling MPA Chip " << (int) cMPA->getMPAId();
                        chips_enable[cFe->getFeId()] |= 1 << cMPA->getMPAId();
                //need to increment the counter for I2C controller
                        fNMPA++;
                    }
                } else if (fFirmwareFrontEndType == FrontEndType::SSA) {
                    for (SSA* cSSA : static_cast<OuterTrackerModule*>(cFe)->fSSAVector)
                    {
                        LOG (INFO) << "     Enabling SSA Chip " << (int) cSSA->getSSAId();
                        chips_enable[cFe->getFeId()] |= 1 << cSSA->getSSAId();
                //need to increment the counter for I2C controller
                        fNSSA++;
                    }
                }

            }

    // hybrid / chips enabling part
            cVecReg.push_back ({"fc7_daq_cnfg.global.hybrid_enable", hybrid_enable});

            for (uint32_t i = 0; i < 16; i++)
            {
                char name[50];
                std::sprintf (name, "fc7_daq_cnfg.global.chips_enable_hyb_%02d", i);
                std::string name_str (name);
                cVecReg.push_back ({name_str, chips_enable[i]});
            }

            delete chips_enable;
            LOG (INFO) << BOLDGREEN << fNHybrids << " hybrid(s) was(were) enabled with the total amount of " << (fNCbc+fNMPA+fNSSA) << " chip(s)!" << RESET;

    //last, loop over the variable registers from the HWDescription.xml file
    //this is where I should get all the clocking and FastCommandInterface settings
            BeBoardRegMap cGlibRegMap = pBoard->getBeBoardRegMap();

            bool dio5_enabled = false;

            for ( auto const& it : cGlibRegMap )
            {
                cVecReg.push_back ( {it.first, it.second} );

                if (it.first == "fc7_daq_cnfg.dio5_block.dio5_en") dio5_enabled = (bool) it.second;
            }

            WriteStackReg ( cVecReg );
            cVecReg.clear();

    // load trigger configuration
            WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);

    // load dio5 configuration
            if (dio5_enabled)
            {
                PowerOnDIO5();
                WriteReg ("fc7_daq_ctrl.dio5_block.control.load_config", 0x1);
            }

    // now set event type (ZS or VR)
            if (pBoard->getEventType() == EventType::ZS) WriteReg ("fc7_daq_cnfg.readout_block.global.zero_suppression_enable", 0x1);
            else WriteReg ("fc7_daq_cnfg.readout_block.global.zero_suppression_enable", 0x0);

    // resetting hard
            this->ChipReset();
        for (Module* cFe : pBoard->fModuleVector)
        {
            std::vector<uint32_t> cVec; 
            for ( Chip* cCbc : cFe->fReadoutChipVector)
            {
                ChipRegItem cRegItem = cCbc->getRegItem ( "FeCtrl&TrgLat2" ); 
                this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getChipId(), cVec, true, false);
            }
            
            std::vector<uint32_t> cReplies;
            //it sounds weird, but ReadI2C is called inside writeI2c, therefore here I have to write and disable the readback. The actual read command is in the words of the vector, no broadcast, maybe I can get rid of it
            bool cSuccess = !WriteI2C ( cVec, cReplies, false, false);
            if( cSuccess ) 
            {
                LOG (INFO) << BOLDGREEN << " Successfully read back values from " << +cFe->fReadoutChipVector.size() << " CBCs." << RESET;
                cVec.clear();
                if( static_cast<OuterTrackerModule*>(cFe)->fCic != NULL )
                {
                    int cFeId = static_cast<OuterTrackerModule*>(cFe)->fCic->getFeId();
                    int cChipId = static_cast<OuterTrackerModule*>(cFe)->fCic->getChipId();
                    ChipRegItem cRegItem = static_cast<OuterTrackerModule*>(cFe)->fCic->getRegItem ( "FE_ENABLE" ); 
                    this->EncodeReg (cRegItem, cFeId, cChipId, cVec, true, false);
                    LOG (INFO) << BOLDBLUE << "Going to try and read FE_ENABLE register from CIC.." << RESET;
                }
                cReplies.clear();
                cSuccess = !WriteI2C ( cVec, cReplies, false, false);
                if( cSuccess ) 
                    LOG (INFO) << BOLDGREEN << " Successfully read back values from CIC" << RESET;
                else
                    LOG (INFO) << BOLDRED << " FAILED to read back values from CIC." << RESET;
            }
            else
                LOG (INFO) << BOLDRED << " FAILED to read back values from " << +cFe->fReadoutChipVector.size() << " CBCs." << RESET;
        }

            this->PhaseTuning (pBoard);

            this->ResetReadout();

            //adding an Orbit reset to align CBC L1A counters
            this->WriteReg("fc7_daq_ctrl.fast_command_block.control.fast_orbit_reset",0x1);
        }

        void D19cFWInterface::PowerOnDIO5()
        {
            LOG (INFO) << BOLDGREEN << "Powering on DIO5" << RESET;

            uint32_t fmc1_card_type = ReadReg ("fc7_daq_stat.general.info.fmc1_card_type");
            uint32_t fmc2_card_type = ReadReg ("fc7_daq_stat.general.info.fmc2_card_type");

        //define constants
            uint8_t i2c_slv   = 0x2f;
            uint8_t wr = 1;
        //uint8_t rd = 0;

            uint8_t sel_fmc_l8  = 0;
            uint8_t sel_fmc_l12 = 1;

        //uint8_t p3v3 = 0xff - 0x09;
            uint8_t p2v5 = 0xff - 0x2b;
        //uint8_t p1v8 = 0xff - 0x67;

            if (fmc1_card_type == 0x1)
            {
                LOG (INFO) << "Found DIO5 at L12. Configuring";

            // disable power
                WriteReg ("sysreg.fmc_pwr.l12_pwr_en", 0x0);

            // enable i2c
                WriteReg ("sysreg.i2c_settings.i2c_bus_select", 0x0);
                WriteReg ("sysreg.i2c_settings.i2c_prescaler", 1000);
                WriteReg ("sysreg.i2c_settings.i2c_enable", 0x1);
            //uint32_t i2c_settings_reg_command = (0x1 << 15) | (0x0 << 10) | 1000;
            //WriteReg("sysreg.i2c_settings", i2c_settings_reg_command);

            // set value
                uint8_t reg_addr = (sel_fmc_l12 << 7) + 0x08;
                uint8_t wrdata = p2v5;
                uint32_t sys_i2c_command = ( (1 << 24) | (wr << 23) | (i2c_slv << 16) | (reg_addr << 8) | (wrdata) );

                WriteReg ("sysreg.i2c_command", sys_i2c_command | 0x80000000);
                WriteReg ("sysreg.i2c_command", sys_i2c_command);

            int status   = 0; // 0 - busy, 1 -done, 2 - error
            int attempts = 0;
            int max_attempts = 1000;
            usleep (1000);

            while (status == 0 && attempts < max_attempts)
            {
                uint32_t i2c_status = ReadReg ("sysreg.i2c_reply.status");
                attempts = attempts + 1;

                //
                if ( (int) i2c_status == 1)
                    status = 1;
                else if ( (int) i2c_status == 0)
                    status = 0;
                else
                    status = 2;
            }

            // disable i2c
            WriteReg ("sysreg.i2c_settings.i2c_enable", 0x0);

            usleep (1000);
            WriteReg ("sysreg.fmc_pwr.l12_pwr_en", 0x1);
        }

        if (fmc2_card_type == 0x1)
        {
            LOG (INFO) << "Found DIO5 at L8. Configuring";

            // disable power
            WriteReg ("sysreg.fmc_pwr.l8_pwr_en", 0x0);

            // enable i2c
            WriteReg ("sysreg.i2c_settings.i2c_bus_select", 0x0);
            WriteReg ("sysreg.i2c_settings.i2c_prescaler", 1000);
            WriteReg ("sysreg.i2c_settings.i2c_enable", 0x1);
            //uint32_t i2c_settings_reg_command = (0x1 << 15) | (0x0 << 10) | 1000;
            //WriteReg("sysreg.i2c_settings", i2c_settings_reg_command);

            // set value
            uint8_t reg_addr = (sel_fmc_l8 << 7) + 0x08;
            uint8_t wrdata = p2v5;
            uint32_t sys_i2c_command = ( (1 << 24) | (wr << 23) | (i2c_slv << 16) | (reg_addr << 8) | (wrdata) );

            WriteReg ("sysreg.i2c_command", sys_i2c_command | 0x80000000);
            WriteReg ("sysreg.i2c_command", sys_i2c_command);

            int status   = 0; // 0 - busy, 1 -done, 2 - error
            int attempts = 0;
            int max_attempts = 1000;
            usleep (1000);

            while (status == 0 && attempts < max_attempts)
            {
                uint32_t i2c_status = ReadReg ("sysreg.i2c_reply.status");
                attempts = attempts + 1;

                //
                if ( (int) i2c_status == 1)
                    status = 1;
                else if ( (int) i2c_status == 0)
                    status = 0;
                else
                    status = 2;
            }

            // disable i2c
            WriteReg ("sysreg.i2c_settings.i2c_enable", 0x0);

            usleep (1000);
            WriteReg ("sysreg.fmc_pwr.l8_pwr_en", 0x1);
        }

        if (fmc1_card_type != 0x1 && fmc2_card_type != 0x1)
            LOG (ERROR) << "No DIO5 found, you should disable it in the config file..";
    }

    // set i2c address table depending on the hybrid
    void D19cFWInterface::SetI2CAddressTable() 
    {

        LOG (INFO) << BOLDGREEN << "Setting the I2C address table" << RESET;

    // creating the map
        std::vector< std::vector<uint32_t> > i2c_slave_map;

    // setting the map for different chip types
        if (fFirmwareFrontEndType == FrontEndType::CBC3) {
        // nothing to de done here default addresses are set for CBC
        // actually FIXME
            return;
        } else if (fFirmwareFrontEndType == FrontEndType::MPA) {
            for (unsigned int id = 0; id < fFWNChips; id++) {
            // for chip emulator register width is 8 bits, not 16 as for MPA
                if(!fCBC3Emulator) {
                    i2c_slave_map.push_back({0x40 + id, 2, 1, 1, 1, 0});
                } else {
                    i2c_slave_map.push_back({0x40 + id, 1, 1, 1, 1, 0});
                }
            }
        }
        else if (fFirmwareFrontEndType == FrontEndType::SSA) // MUST BE IN ORDER! CANNOT DO 0, 1, 4
        {
            LOG (INFO) << BOLDRED << "WE ARE HERE!!! WE ARE HERE!!! WE ARE HERE!!!  " << fFWNChips << RESET;
            for (unsigned int id = 0; id < fFWNChips; id++) 
            {
                i2c_slave_map.push_back({0x20 + id, 2, 1, 1, 1, 0}); // FIXME SSA ??
            }
        }
        for (unsigned int ism = 0; ism < i2c_slave_map.size(); ism++) {
        // setting the params
            uint32_t shifted_i2c_address = i2c_slave_map[ism][0]<<25;
            uint32_t shifted_register_address_nbytes = i2c_slave_map[ism][1]<<10;
            uint32_t shifted_data_wr_nbytes = i2c_slave_map[ism][2]<<5;
            uint32_t shifted_data_rd_nbytes = i2c_slave_map[ism][3]<<0;
            uint32_t shifted_stop_for_rd_en = i2c_slave_map[ism][4]<<24;
            uint32_t shifted_nack_en = i2c_slave_map[ism][5]<<23;

        // writing the item to the firmware
            uint32_t final_item = shifted_i2c_address + shifted_register_address_nbytes + shifted_data_wr_nbytes + shifted_data_rd_nbytes + shifted_stop_for_rd_en + shifted_nack_en;
            std::string curreg = "fc7_daq_cnfg.command_processor_block.i2c_address_table.slave_" + std::to_string(ism) + "_config";
            WriteReg(curreg, final_item);
        }
    }

  void D19cFWInterface::Start()
  {
    this->ChipReSync();
    this->ResetReadout();
    
        //here open the shutter for the stub counter block (for some reason self clear doesn't work, that why we have to clear the register manually)
        WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_open", 0x1);
        WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_open", 0x0);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );

        WriteReg ("fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );
    }

    void D19cFWInterface::Stop()
    {
        //here close the shutter for the stub counter block
        WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_close", 0x1);
        WriteReg ("fc7_daq_ctrl.stub_counter_block.general.shutter_close", 0x0);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );

        WriteReg ("fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );

        //here read the stub counters
        /*
    uint32_t cBXCounter1s = ReadReg ("fc7_daq_stat.stub_counter_block.bx_counter_ls");
        uint32_t cBXCounterms = ReadReg ("fc7_daq_stat.stub_counter_block.bx_counter_ms");
        uint32_t cStubCounter0 = ReadReg ("fc7_daq_stat.stub_counter_block.counters_hyb0_chip0");
        uint32_t cStubCounter1 = ReadReg ("fc7_daq_stat.stub_counter_block.counters_hyb0_chip1");
    */
        /*
    LOG (INFO) << BOLDGREEN << "Reading FW Stub and Error counters at the end of the run: " << RESET;
        LOG (INFO) << BOLDBLUE << "BX Counter 1s: " << RED << cBXCounter1s << RESET;
        LOG (INFO) << BOLDBLUE << "BX Counter ms: " << RED << cBXCounterms << RESET;
        LOG (INFO) << BOLDGREEN << "FE 0 CBC 0:" << RESET;
        LOG (INFO) << BOLDBLUE << " Stub Counter: " << RED << (cStubCounter0 & 0x0000FFFF) << RESET;
        LOG (INFO) << BOLDBLUE << "Error Counter: " << RED << ( (cStubCounter0 & 0xFFFF0000) >> 16 ) << RESET;
        LOG (INFO) << BOLDGREEN << "FE 0 CBC 1:" << RESET;
        LOG (INFO) << BOLDBLUE << " Stub Counter: " << RED << (cStubCounter1 & 0x0000FFFF) << RESET;
        LOG (INFO) << BOLDBLUE << "Error Counter: " << RED << ( (cStubCounter1 & 0xFFFF0000) >> 16) << RESET;
        */
    }


    void D19cFWInterface::Pause()
    {
        LOG (INFO) << BOLDBLUE << "................................ Pausing run ... " << RESET ; 
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.stop_trigger", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );
    }


    void D19cFWInterface::Resume()
    {
        LOG (INFO) << BOLDBLUE << "Reseting readout before resuming run ... " << RESET ; 
        this->ResetReadout();

        LOG (INFO) << BOLDBLUE << "................................ Resuming run ... " << RESET ; 
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.start_trigger", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );
    }

    void D19cFWInterface::ResetReadout()
    {
        WriteReg ("fc7_daq_ctrl.readout_block.control.readout_reset", 0x1);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );

        WriteReg ("fc7_daq_ctrl.readout_block.control.readout_reset", 0x0);
        std::this_thread::sleep_for (std::chrono::microseconds (10) );

        if (fIsDDR3Readout) {
            fDDR3Offset = 0;
            fDDR3Calibrated = (ReadReg("fc7_daq_stat.ddr3_block.init_calib_done") == 1);
            bool i=false;
            while(!fDDR3Calibrated) {
                if(i==false) LOG(INFO) << "Waiting for DDR3 to finish initial calibration";
                i=true;
                std::this_thread::sleep_for (std::chrono::milliseconds (100) );
                fDDR3Calibrated = (ReadReg("fc7_daq_stat.ddr3_block.init_calib_done") == 1);
            }
        }
    }

    void D19cFWInterface::DDR3SelfTest()
    {
    //opened issue: without this time delay the self-test doesn't examine entire 4Gb address space of the chip(reason not obvious) 
        std::this_thread::sleep_for (std::chrono::seconds (1) );
        if (fIsDDR3Readout && fDDR3Calibrated) {
                // trigger the self check
            WriteReg ("fc7_daq_ctrl.ddr3_block.control.traffic_str", 0x1);

            bool cDDR3Checked = (ReadReg("fc7_daq_stat.ddr3_block.self_check_done") == 1);
            bool j=false;
            LOG (INFO) << GREEN << "============================" << RESET;
            LOG (INFO) << BOLDGREEN << "DDR3 Self-Test" << RESET;

            while(!cDDR3Checked) {
                if(j==false) LOG(INFO) << "Waiting for DDR3 to finish self-test";
                j=true;
                std::this_thread::sleep_for (std::chrono::milliseconds (100) );
                cDDR3Checked = (ReadReg("fc7_daq_stat.ddr3_block.self_check_done") == 1);
            }

            if(cDDR3Checked) {
                int num_errors = ReadReg("fc7_daq_stat.ddr3_block.num_errors");
                int num_words = ReadReg("fc7_daq_stat.ddr3_block.num_words");
                LOG(DEBUG) << "Number of checked words " << num_words;
                LOG(DEBUG) << "Number of errors " << num_errors;
                if(num_errors == 0){
                    LOG(INFO) << "DDR3 self-test ->" << BOLDGREEN << " PASSED" << RESET;
                }
                else LOG(ERROR) << "DDR3 self-test ->" << BOLDRED << " FAILED" << RESET;
            }
            LOG (INFO) << GREEN << "============================" << RESET;
        }
    }


    void D19cFWInterface::PhaseTuning (const BeBoard* pBoard)
    {
        if (fFirmwareFrontEndType == FrontEndType::CBC3)
        {
            if (!fCBC3Emulator)
            {
                bool cDoAuto = true;

                    // automatic mode
                if (cDoAuto)
                {
                    std::map<Chip*, uint8_t> cStubLogictInputMap;
                    std::map<Chip*, uint8_t> cHipRegMap;
                    std::vector<uint32_t> cVecReq;

                    cVecReq.clear();

                    for (auto cFe : pBoard->fModuleVector)
                    {
                        for (auto cCbc : cFe->fReadoutChipVector)
                        {

                            uint8_t cOriginalStubLogicInput = cCbc->getReg ("Pipe&StubInpSel&Ptwidth");
                            uint8_t cOriginalHipReg = cCbc->getReg ("HIP&TestMode");
                            cStubLogictInputMap[cCbc] = cOriginalStubLogicInput;
                            cHipRegMap[cCbc] = cOriginalHipReg;


                            ChipRegItem cRegItem = cCbc->getRegItem ( "Pipe&StubInpSel&Ptwidth" );
                            cRegItem.fValue = (cOriginalStubLogicInput & 0xCF) | (0x20 & 0x30);
                            this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getChipId(), cVecReq, true, true);

                            cRegItem = cCbc->getRegItem ( "HIP&TestMode" );
                            cRegItem.fValue = (cOriginalHipReg & ~ (0x1 << 4) );
                            this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getChipId(), cVecReq, true, true);

                        }
                    }

                    uint8_t cWriteAttempts = 0;
                    this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
                    std::this_thread::sleep_for (std::chrono::milliseconds (10) );

                    int cCounter = 0;
                    int cMaxAttempts = 10;

                    uint32_t hardware_ready = 0;
                    while (hardware_ready < 1)
                    {    
                        if (cCounter++ > cMaxAttempts)
                        {
                            LOG(ERROR) << BOLDRED << "Failed phase tuning, debug information: " << RESET;
                            // print statuses
                            for (auto cFe : pBoard->fModuleVector)
                            {
                                for (auto cCbc : cFe->fReadoutChipVector)
                                {
                                    PhaseTuningGetLineStatus(cFe->getFeId(), cCbc->getChipId(), 5);
                                }
                            }
                            exit (1);
                        }

			this->ChipReSync();
			usleep (10);
                        // reset  the timing tuning
                        WriteReg ("fc7_daq_ctrl.physical_interface_block.control.cbc3_tune_again", 0x1);

                        std::this_thread::sleep_for (std::chrono::milliseconds (100) );
                        hardware_ready = ReadReg ("fc7_daq_stat.physical_interface_block.hardware_ready");
                    }

                        //re-enable the stub logic
                    cVecReq.clear();
                    for (auto cFe : pBoard->fModuleVector)
                    {
                        for (auto cCbc : cFe->fReadoutChipVector)
                        {

                            ChipRegItem cRegItem = cCbc->getRegItem ( "Pipe&StubInpSel&Ptwidth" );
                            cRegItem.fValue = cStubLogictInputMap[cCbc];
                            //this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getChipId(), cVecReq, true, true);

                            cRegItem = cCbc->getRegItem ( "HIP&TestMode" );
                            cRegItem.fValue = cHipRegMap[cCbc];
                            this->EncodeReg (cRegItem, cCbc->getFeId(), cCbc->getChipId(), cVecReq, true, true);

                        }
                    }

                    cWriteAttempts = 0;
                    this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);

                    LOG (INFO) << GREEN << "CBC3 Phase tuning finished succesfully" << RESET;

                } 
                else 
                {
                    // manual mode apply
                    uint8_t phase_cbc0[2] = {15, 3}; // delay, bitslip
                    uint8_t phase_cbc1[2] = {15, 3}; // delay, bitslip

                    // cbc0
                    for(uint8_t line = 0; line < 6; line++) 
                    {
                            // const
                        uint32_t hybrid_raw = (0 & 0xF) << 28;
                        uint32_t chip_raw = (0 & 0xF) << 24;
                        uint32_t line_raw = (line & 0xF) << 20;
                        uint32_t command_raw = (2 & 0xF) << 16;

                            // manual mode
                        uint32_t mode_raw = (2 & 0x3) << 12;
                        uint32_t delay_raw = (phase_cbc0[0] & 0x1F) << 3;
                        uint32_t bitslip_raw = (phase_cbc0[1] & 0x7) << 0;

                            // write
                        uint32_t command_final = command_raw + hybrid_raw + chip_raw + line_raw + mode_raw + delay_raw + bitslip_raw;
                        WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
                    }

                        // cbc1
                    for(uint8_t line = 0; line < 6; line++) 
                    {
                            // const
                        uint32_t hybrid_raw = (0 & 0xF) << 28;
                        uint32_t chip_raw = (1 & 0xF) << 24;
                        uint32_t line_raw = (line & 0xF) << 20;
                        uint32_t command_raw = (2 & 0xF) << 16;

                            // manual mode
                        uint32_t mode_raw = (2 & 0x3) << 12;
                        uint32_t delay_raw = (phase_cbc1[0] & 0x1F) << 3;
                        uint32_t bitslip_raw = (phase_cbc1[1] & 0x7) << 0;

                            // write
                        uint32_t command_final = command_raw + hybrid_raw + chip_raw + line_raw + mode_raw + delay_raw + bitslip_raw;
                        WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
                    }

                    LOG (INFO) << GREEN << "CBC3 Phase tuning " << RESET << RED << "APPLIED" << RESET << GREEN <<" succesfully" << RESET;
                }

                    // print statuses
                for (auto cFe : pBoard->fModuleVector)
                {
                    for (auto cCbc : cFe->fReadoutChipVector)
                    {
                        PhaseTuningGetLineStatus(cFe->getFeId(), cCbc->getChipId(), 5);
                    }
                }

            }
        }
        
        else if (fFirmwareFrontEndType == FrontEndType::MPA)
        {
                // first need to set the proper i2c settings of the chip for the phase alignment
            std::map<MPA*, uint8_t> cReadoutModeMap;
            std::map<MPA*, uint8_t> cStubModeMap;
            std::vector<uint32_t> cVecReq;

            cVecReq.clear();

            for (auto cFe : pBoard->fModuleVector)
            {
                for (auto cMpa : static_cast<OuterTrackerModule*>(cFe)->fMPAVector)
                {

                    uint8_t cOriginalReadoutMode = cMpa->getReg ("ReadoutMode");
                    uint8_t cOriginalStubMode = cMpa->getReg ("ECM");
                    cReadoutModeMap[cMpa] = cOriginalReadoutMode;
                    cStubModeMap[cMpa] = cOriginalStubMode;

                        // sync mode
                    ChipRegItem cRegItem = cMpa->getRegItem ( "ReadoutMode" );
                    cRegItem.fValue = 0x00;
                    this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                    uint8_t cWriteAttempts = 0;
                    this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
                    cVecReq.clear();

                        // ps stub mode
                    cRegItem = cMpa->getRegItem ( "ECM" );
                    cRegItem.fValue = 0x08;
                    this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                    cWriteAttempts = 0;
                    this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
                    cVecReq.clear();

                }
            }

            uint8_t cWriteAttempts = 0;
                //this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
            std::this_thread::sleep_for (std::chrono::milliseconds (10) );

                // now do phase tuning
            Align_out();

                //re-enable everything back
            cVecReq.clear();
            for (auto cFe : pBoard->fModuleVector)
            {
                for (auto cMpa : static_cast<OuterTrackerModule*>(cFe)->fMPAVector)
                {

                    ChipRegItem cRegItem = cMpa->getRegItem ( "ReadoutMode" );
                    cRegItem.fValue = cReadoutModeMap[cMpa];
                    this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                    cWriteAttempts = 0;
                    this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
                    cVecReq.clear();

                    cRegItem = cMpa->getRegItem ( "ECM" );
                    cRegItem.fValue = cStubModeMap[cMpa];
                    this->EncodeReg (cRegItem, cMpa->getFeId(), cMpa->getMPAId(), cVecReq, true, true);

                    cWriteAttempts = 0;
                    this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);
                    cVecReq.clear();

                }
            }

            cWriteAttempts = 0;
                //this->WriteChipBlockReg (cVecReq, cWriteAttempts, true);

            LOG (INFO) << GREEN << "MPA Phase tuning finished succesfully" << RESET;
        }

        else
        {
            LOG (INFO) << "No tuning procedure implemented for this chip type.";
            exit (1);
        }

    }

    uint32_t D19cFWInterface::ReadData ( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
    {
        uint32_t cEventSize = computeEventSize (pBoard);
        uint32_t cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
        uint32_t data_handshake = ReadReg ("fc7_daq_cnfg.readout_block.global.data_handshake_enable");
        uint32_t cPackageSize = ReadReg ("fc7_daq_cnfg.readout_block.packet_nbr") + 1;

        bool pFailed = false; 
        int cCounter = 0 ; 
        while (cNWords == 0 && !pFailed )
        {
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            if(cCounter % 100 == 0 && cCounter > 0) {
                LOG(INFO) << BOLDRED << "Zero events in FIFO, waiting for the triggers" << RESET;
            }
            cCounter++;

            if (!pWait) 
                return 0;
            else
                std::this_thread::sleep_for (std::chrono::microseconds (10) );
        }

        uint32_t cNEvents = 0;
        uint32_t cNtriggers = 0; 
        uint32_t cNtriggers_prev = cNtriggers;

        if (data_handshake == 1 && !pFailed )
        {
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            cNtriggers = ReadReg ("fc7_daq_stat.fast_command_block.trigger_in_counter"); 
            cNtriggers_prev = cNtriggers;
            // uint32_t cNWords_prev = cNWords;
            uint32_t cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");

            cCounter = 0 ; 
            while (cReadoutReq == 0 && !pFailed )
            {
                if (!pWait) {
                    return 0;
                }

                // cNWords_prev = cNWords;
                cNtriggers_prev = cNtriggers;

                cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");
                cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
                cNtriggers = ReadReg ("fc7_daq_stat.fast_command_block.trigger_in_counter");

                /*if( cNWords == cNWords_prev && cCounter > 100 && cNtriggers != cNtriggers_prev )
                {
                    pFailed = true; 
                    LOG (INFO) << BOLDRED << "Warning!! Read-out has stopped responding after receiving " << +cNtriggers << " triggers!! Read back " << +cNWords << " from FC7." << RESET ; 
               
                }
                else*/
                if( cNtriggers == cNtriggers_prev && cCounter > 0 )
                {
                    if( cCounter % 100 == 0 )
                        LOG (INFO) << BOLDRED << " ..... waiting for more triggers .... got " << +cNtriggers << " so far." << RESET ;

                }
                cCounter++;
                std::this_thread::sleep_for (std::chrono::microseconds (10) );
            }

            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            if (pBoard->getEventType() == EventType::VR)
            {
                cNEvents = cNWords / computeEventSize (pBoard);
                if ( (cNWords % computeEventSize (pBoard) ) != 0) {
                    pFailed = true;
                    LOG (ERROR) << "Data amount (in words) is not multiple to EventSize! (" << cNWords << ")";
                }
            }
            else
            {
                // for zs it's impossible to check, so it'll count later during event assignment
                cNEvents = cPackageSize;
            }

            // read all the words
            if (fIsDDR3Readout) {
                pData = ReadBlockRegOffsetValue ("fc7_daq_ddr3", cNWords, fDDR3Offset);
                //in the handshake mode offset is cleared after each handshake
                fDDR3Offset = 0;
            }
            else
                pData = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cNWords);

        }
        else if(!pFailed)
        {
            if (pBoard->getEventType() == EventType::ZS)
            {
                LOG (ERROR) << "ZS Event only with handshake!!! Exiting...";
                exit (1);
            }
            cNEvents = 0;
            //while (cNEvents < cPackageSize)
            //{
            cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            uint32_t cNEventsAvailable = (uint32_t) cNWords / cEventSize;

            while (cNEventsAvailable < 1)
            {
                if(!pWait) {
                    return 0;
                }
                std::this_thread::sleep_for (std::chrono::milliseconds (10) );
                cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
                cNEventsAvailable = (uint32_t) cNWords / cEventSize;                    
            }                

            std::vector<uint32_t> event_data;
            if (fIsDDR3Readout) 
            {                    
                // read
                event_data = ReadBlockRegOffsetValue ("fc7_daq_ddr3", cNEventsAvailable*cEventSize, fDDR3Offset);
                // in the no handshake mode the, wr counter is reset when it reaches the maximal value
                // therefore offset here also has to be reset
                // by coincidence when no hanndshake it also rises the readout_req before resetting the address
                uint32_t cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");
                if(cReadoutReq == 1) fDDR3Offset = 0;
            } 
            else 
            {
                event_data = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cNEventsAvailable*cEventSize);
            }

            pData.insert (pData.end(), event_data.begin(), event_data.end() );
            cNEvents += cNEventsAvailable;

            //}
        }

        if( pFailed )
        {
            pData.clear();

            LOG(INFO) << BOLDRED << "Re-starting the run and resetting the readout" << RESET; 

            this->Stop();
            std::this_thread::sleep_for (std::chrono::milliseconds (500) );
            LOG(INFO) << BOLDGREEN << " ... Run Stopped, current trigger FSM state: " << +ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state") << RESET;

            this->Start();
            std::this_thread::sleep_for (std::chrono::milliseconds (500) );
            LOG(INFO) << BOLDGREEN << " ... Run Started, current trigger FSM state: " << +ReadReg ("fc7_daq_stat.fast_command_block.general.fsm_state") << RESET;

            LOG (INFO) << BOLDRED << " ... trying to read data again .... " << RESET ; 
            cNEvents = this->ReadData(pBoard,  pBreakTrigger,  pData, pWait);
        }
        if (fSaveToFile)
            fFileHandler->set (pData);

        //need to return the number of events read
        return cNEvents;
    }


    void D19cFWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait )
    {
        // data hadnshake has to be enabled in that mode
        WriteReg ("fc7_daq_cnfg.readout_block.packet_nbr", pNEvents-1);
        WriteReg ("fc7_daq_cnfg.readout_block.global.data_handshake_enable", 0x1);

        // write the amount of the test pulses to be sent
        WriteReg ("fc7_daq_cnfg.fast_command_block.triggers_to_accept", pNEvents);
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);
        usleep (1);

        // start triggering machine which will collect N events
        this->Start();

        // sta
        bool pFailed = false;
        uint32_t cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");
        uint32_t cNtriggers = ReadReg ("fc7_daq_stat.fast_command_block.trigger_in_counter");
        uint32_t cTimeoutCounter = 0 ;
        uint32_t cTimeoutValue = 1000;
        while (cReadoutReq == 0 && !pFailed )
        {
            cReadoutReq = ReadReg ("fc7_daq_stat.readout_block.general.readout_req");
            cNtriggers = ReadReg ("fc7_daq_stat.fast_command_block.trigger_in_counter");

            if( cNtriggers == pNEvents )
            {
                if( cTimeoutCounter >= cTimeoutValue ) {
                    pFailed = true;
                    LOG(INFO) << "No data in the readout after receiving all triggers. Re-trying the point";
                }
                cTimeoutCounter++;
                std::this_thread::sleep_for (std::chrono::microseconds (10) );
            }
        }

        if (!pFailed) {

            // check the amount of words
            uint32_t cNWords = ReadReg ("fc7_daq_stat.readout_block.general.words_cnt");
            if (pBoard->getEventType() == EventType::VR)
            {
                if ( (cNWords % computeEventSize (pBoard) ) != 0) {
                    pFailed = true;
                    LOG (ERROR) << "Data amount (in words) is not multiple to EventSize! (" << cNWords << ")";
                }
            }
            else
            {
                // for zs it's impossible to check, so it'll count later during event assignment
            }

            // read all the words
            if (fIsDDR3Readout) {
                pData = ReadBlockRegOffsetValue ("fc7_daq_ddr3", cNWords, fDDR3Offset);
                //in the handshake mode offset is cleared after each handshake
                fDDR3Offset = 0;
            }
            else
                pData = ReadBlockRegValue ("fc7_daq_ctrl.readout_block.readout_fifo", cNWords);

        }

        // again check if failed to re-run in case
        if (pFailed)
        {
            pData.clear();
            this->Stop();

            this->ResetReadout();

            this->ReadNEvents (pBoard, pNEvents, pData);
        }        

        if (fSaveToFile)
            fFileHandler->set (pData);
    }

/** compute the block size according to the number of CBC's on this board
 * this will have to change with a more generic FW */
    uint32_t D19cFWInterface::computeEventSize ( BeBoard* pBoard )
    {
        uint32_t cNFe = pBoard->getNFe();
        uint32_t cNCbc = 0;
        uint32_t cNMPA = 0;
        uint32_t cNSSA = 0;

        uint32_t cNEventSize32 = 0;

        for (const auto& cFe : pBoard->fModuleVector)
        {
            cNCbc += cFe->getNChip();
            cNMPA += static_cast<OuterTrackerModule*>(cFe)->getNMPA();
            cNSSA += static_cast<OuterTrackerModule*>(cFe)->getNSSA();
        }
        if (cNCbc>0) cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32_CBC3 + cNCbc * D19C_EVENT_SIZE_32_CBC3;
        if (cNMPA>0) cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32 + cNFe * D19C_EVENT_HEADER2_SIZE_32 + cNMPA * D19C_EVENT_SIZE_32_MPA;
        if (cNSSA>0) cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32 + cNFe * D19C_EVENT_HEADER2_SIZE_32 + cNSSA * D19C_EVENT_SIZE_32_SSA;
        if (cNCbc>0 && cNMPA>0)
        {
            LOG(INFO) << "Not configurable for multiple chips";
            exit (1);
        }
        if (fIsDDR3Readout) {
            uint32_t cNEventSize32_divided_by_8 = ((cNEventSize32 >> 3) << 3);
            if (!(cNEventSize32_divided_by_8 == cNEventSize32)) {
                cNEventSize32 = cNEventSize32_divided_by_8 + 8;
            }
        }

        return cNEventSize32;
    }

    std::vector<uint32_t> D19cFWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockReg ( pRegNode, pBlocksize );
        std::vector<uint32_t> vBlock = valBlock.value();
        return vBlock;
    }

    std::vector<uint32_t> D19cFWInterface::ReadBlockRegOffsetValue ( const std::string& pRegNode, const uint32_t& pBlocksize, const uint32_t& pBlockOffset )
    {
        uhal::ValVector<uint32_t> valBlock = ReadBlockRegOffset( pRegNode, pBlocksize, pBlockOffset );
        std::vector<uint32_t> vBlock = valBlock.value();
        if (fIsDDR3Readout) {
            fDDR3Offset += pBlocksize;
        }
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

  void D19cFWInterface::EncodeReg ( const ChipRegItem& pRegItem,
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

    void D19cFWInterface::EncodeReg (const ChipRegItem& pRegItem,
       uint8_t pFeId,
       uint8_t pCbcId,
       std::vector<uint32_t>& pVecReq,
       bool pReadBack,
       bool pWrite )
    {
        //use fBroadcastCBCId for broadcast commands
        bool pUseMask = false;
        if (fI2CVersion >= 1) {
        // new command consists of one word if its read command, and of two words if its write. first word is always the same
            pVecReq.push_back( (0 << 28) | (0 << 27) | (pFeId << 23) | (pCbcId << 18) | (pReadBack << 17) | ((!pWrite) << 16) | (pRegItem.fPage << 8) | (pRegItem.fAddress << 0) );
        // only for write commands
            if (pWrite) pVecReq.push_back( (0 << 28) | (1 << 27) | (pRegItem.fValue << 0) );
        } else {
            pVecReq.push_back ( ( 0 << 28 ) | ( pFeId << 24 ) | ( pCbcId << 20 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
        }
    }

    void D19cFWInterface::DecodeReg ( ChipRegItem& pRegItem,
      uint8_t& pCbcId,
      uint32_t pWord,
      bool& pRead,
      bool& pFailed )
    {
        if (fI2CVersion >= 1) {
        //pFeId    =  ( ( pWord & 0x07800000 ) >> 27) ;
            pCbcId   =  ( ( pWord & 0x007c0000 ) >> 22) ;
            pFailed  =  0 ;
            pRegItem.fPage    =  0 ;
            pRead    =  true ;
            pRegItem.fAddress =  ( pWord & 0x0000FF00 ) >> 8;
            pRegItem.fValue   =  ( pWord & 0x000000FF );
        } else {
        //pFeId    =  ( ( pWord & 0x00f00000 ) >> 24) ;
            pCbcId   =  ( ( pWord & 0x00f00000 ) >> 20) ;
            pFailed  =  0 ;
            pRegItem.fPage    =  ( (pWord & 0x00020000 ) >> 17);
            pRead    =  (pWord & 0x00010000) >> 16;
            pRegItem.fAddress =  ( pWord & 0x0000FF00 ) >> 8;
            pRegItem.fValue   =  ( pWord & 0x000000FF );
        }

    }

    void D19cFWInterface::BCEncodeReg ( const ChipRegItem& pRegItem,
        uint8_t pNCbc,
        std::vector<uint32_t>& pVecReq,
        bool pReadBack,
        bool pWrite )
    {
    //use fBroadcastCBCId for broadcast commands
        bool pUseMask = false;
        pVecReq.push_back ( ( 2 << 28 ) | ( pReadBack << 19 ) | (  pUseMask << 18 )  | ( (pRegItem.fPage ) << 17 ) | ( ( !pWrite ) << 16 ) | ( pRegItem.fAddress << 8 ) | pRegItem.fValue );
    }

    bool D19cFWInterface::ReadI2C (  uint32_t pNReplies, std::vector<uint32_t>& pReplies)
    {
        bool cFailed (false);

        uint32_t single_WaitingTime = SINGLE_I2C_WAIT * pNReplies;
        uint32_t max_Attempts = 100;
        uint32_t counter_Attempts = 0;

        //read the number of received replies from ndata and use this number to compare with the number of expected replies and to read this number 32-bit words from the reply FIFO
        usleep (single_WaitingTime);
        uint32_t cNReplies = ReadReg ("fc7_daq_stat.command_processor_block.i2c.nreplies");

        while (cNReplies != pNReplies)
        {
            if (counter_Attempts > max_Attempts)
            {
                LOG (INFO) << "Error: Read " << cNReplies << " I2C replies whereas " << pNReplies << " are expected!" ;
                ReadErrors();
                cFailed = true;
                break;
            }

            usleep (single_WaitingTime);
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
        usleep (10);

        try
        {
            WriteBlockReg ( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", pVecSend );
        }
        catch ( Exception& except )
        {
            throw except;
        }

        uint32_t cNReplies = 0;

        for (auto word : pVecSend)
        {
            // if read or readback for write == 1, then count
            if (fI2CVersion >= 1) {
                if ( (((word & 0x08000000) >> 27) == 0) && (( ( (word & 0x00010000) >> 16) == 1) or ( ( (word & 0x00020000) >> 17) == 1)) )
                {
                    if (pBroadcast) cNReplies += fNCbc;
                    else cNReplies += 1;
                }
            } else {
                if ( ( ( (word & 0x00010000) >> 16) == 1) or ( ( (word & 0x00080000) >> 19) == 1) )
                {
                    if (pBroadcast) cNReplies += fNCbc;
                    else cNReplies += 1;
                }
            }
        }
        usleep (20);

        cFailed = ReadI2C (  cNReplies, pReplies) ;

        return cFailed;
    }


    bool D19cFWInterface::WriteChipBlockReg ( std::vector<uint32_t>& pVecReg, uint8_t& pWriteAttempts, bool pReadback)
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
                this->WriteChipBlockReg ( cWriteAgain, pWriteAttempts, true);
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

    bool D19cFWInterface::BCWriteChipBlockReg ( std::vector<uint32_t>& pVecReg, bool pReadback)
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

    void D19cFWInterface::ReadChipBlockReg (  std::vector<uint32_t>& pVecReg )
    {
        std::vector<uint32_t> cReplies;
        //it sounds weird, but ReadI2C is called inside writeI2c, therefore here I have to write and disable the readback. The actual read command is in the words of the vector, no broadcast, maybe I can get rid of it
        WriteI2C ( pVecReg, cReplies, false, false);
        pVecReg.clear();
        pVecReg = cReplies;
    }

  void D19cFWInterface::ChipReSync()
  {
    WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_reset", 0x1 );
  }
  
  void D19cFWInterface::ChipI2CRefresh()
  {
    WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_i2c_refresh", 0x1 );
  }
  
  void D19cFWInterface::ChipReset()
  {
    //for CBCs
    WriteReg ( "fc7_daq_ctrl.physical_interface_block", (0x1) << 0 );
    std::this_thread::sleep_for (std::chrono::milliseconds (100) );

    //for CICs 
    WriteReg ( "fc7_daq_ctrl.physical_interface_block", (0x1) << 2 );
    std::this_thread::sleep_for (std::chrono::milliseconds (100) );

    //WriteReg ( "fc7_daq_ctrl.physical_interface_block.control.chip_hard_reset", 0x1 );
    usleep (10);
  }

  void D19cFWInterface::ChipTestPulse()
  {
    ;
  }

  void D19cFWInterface::ChipTrigger()
  {
    WriteReg ( "fc7_daq_ctrl.fast_command_block.control.fast_trigger", 0x1 );
  }

    // line status phase tuning
    void D19cFWInterface::PhaseTuningGetLineStatus(uint8_t pHybrid, uint8_t pChip, uint8_t pLine)
    {
        // encode the line id's
        uint32_t hybrid_raw = (pHybrid & 0xF) << 28;
        uint32_t chip_raw = (pChip & 0xF) << 24;
        uint32_t line_raw = (pLine & 0xF) << 20;

        // print header
        LOG(INFO) << BOLDBLACK << "\t Hybrid: " << RESET << +pHybrid << BOLDBLACK << ", Chip: " << RESET << +pChip << BOLDBLACK << ", Line: " << RESET << +pLine;

        // encode command type
        uint32_t command_raw = (0 & 0xF) << 16;
        uint32_t command_final = hybrid_raw + chip_raw + line_raw + command_raw;
        WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
        // sleep a bi
        usleep(100);
        // get the status back
        PhaseTuningParseStatus();

        // encode command type
        command_raw = (1 & 0xF) << 16;
        command_final = hybrid_raw + chip_raw + line_raw + command_raw;
        WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
        // sleep a bi
        usleep(100);
        // get the status back
        PhaseTuningParseStatus();


    }

    void D19cFWInterface::PhaseTuningGetDefaultFSMState() {
        // encode command type
        uint32_t command_raw = (0 & 0xF) << 16;
        uint32_t command_final = command_raw;
        WriteReg( "fc7_daq_ctrl.physical_interface_block.phase_tuning_ctrl", command_final );
        // sleep a bi
        usleep(100);
        // get the status back
        PhaseTuningParseStatus();
    }

    void D19cFWInterface::PhaseTuningParseStatus() {
        // map of the phase tuning statuses
        std::map<int, std::string> cPhaseFSMStateMap = {{0, "IdlePHASE"},
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
        std::map<int, std::string> cWordFSMStateMap = {{0, "IdleWORD or WaitIserdese"},
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

        // read status
        uint32_t reply = ReadReg( "fc7_daq_stat.physical_interface_block.phase_tuning_reply" );
        uint8_t output_type = (reply >> 24) & 0xF;

        if (output_type == 0) {
            uint8_t mode = (reply & 0x00003000) >> 12;
            uint8_t delay = (reply & 0x000000F8) >> 3;
            uint8_t bitslip = (reply & 0x00000007) >> 0;

            LOG(INFO) << "\t\t Mode: " << +mode;
            LOG(INFO) << "\t\t Manual Delay: " << +delay << ", Manual Bitslip: " << +bitslip;

        } else if (output_type == 1) {
            uint8_t delay = (reply & 0x00F80000) >> 19;
            uint8_t bitslip = (reply & 0x00070000) >> 16;
            uint8_t done = (reply & 0x00008000) >> 15;
            int wa_fsm_state = (reply & 0x00000F00) >> 8;
            int pa_fsm_state = (reply & 0x0000000F) >> 0;

            LOG(INFO) << "\t\t Done: " << +done << ", PA FSM: " << BOLDGREEN << cPhaseFSMStateMap[pa_fsm_state] << RESET << ", WA FSM: " << BOLDGREEN << cWordFSMStateMap[wa_fsm_state] << RESET;
            LOG(INFO) << "\t\t Delay: " << +delay << ", Bitslip: " << +bitslip;
        } else if (output_type == 6) {
            uint8_t default_fsm_state = (reply & 0x000000FF) >> 0;
            LOG(INFO) << "\t\t Default FSM State: " << +default_fsm_state;
        }
    }

    // measures the occupancy of the 2S chips
    bool D19cFWInterface::Measure2SOccupancy(uint32_t pNEvents, uint8_t **&pErrorCounters, uint8_t ***&pChannelCounters )
    {
        // this will anyway be constant
        const int COUNTER_WIDTH_BITS = 8; // we have 8bit counters currently
        const int BIT_MASK = 0xFF; // for counter widht 8

        // check the amount of events
        if (pNEvents > pow(2,COUNTER_WIDTH_BITS)-1) {
            LOG(ERROR) << "Requested more events, that counters could fit";
            return false;
        }

        // set the configuration of the fast command (number of events)
        WriteReg ("fc7_daq_cnfg.fast_command_block.triggers_to_accept", pNEvents);
        WriteReg ("fc7_daq_ctrl.fast_command_block.control.load_config", 0x1);

        // disable the readout backpressure (no one cares about readout)
        uint32_t cBackpressureOldValue = ReadReg("fc7_daq_cnfg.fast_command_block.misc.backpressure_enable");
        WriteReg ("fc7_daq_cnfg.fast_command_block.misc.backpressure_enable", 0x0);

        // reset the counters fsm
        //WriteReg ("fc7_daq_ctrl.calibration_2s_block.control.reset_fsm", 0x1); // self reset
        //usleep (1);

        // finally start the loop
        WriteReg ("fc7_daq_ctrl.calibration_2s_block.control.start", 0x1);

        // now loop till the machine is not done
        bool cLastPackage = false;
        while (!cLastPackage) {

            // loop waiting for the counters
            while (ReadReg ("fc7_daq_stat.calibration_2s_block.general.counters_ready") == 0) {
                // just wait
                //uint32_t cFIFOEmpty = ReadReg ("fc7_daq_stat.calibration_2s_block.general.fifo_empty");
                //LOG(INFO) << "FIFO Empty: " << cFIFOEmpty;
                usleep (1);
            }
            cLastPackage = ((ReadReg ("fc7_daq_stat.calibration_2s_block.general.fsm_done") == 1) && (ReadReg ("fc7_daq_stat.calibration_2s_block.general.counters_ready") == 1));

            // so the counters are ready let's read the fifo
            uint32_t header = ReadReg("fc7_daq_ctrl.calibration_2s_block.counter_fifo");
            if (((header >> 16) & 0xFFFF) != 0xFFFF) {
                LOG(ERROR) << "Something bad with counters header";
                return false;
            }
            uint32_t cEventSize = (header & 0x0000FFFF);
            //LOG(INFO) << "Stub Counters Event size is: " << cEventSize;

            std::vector<uint32_t> counters_data = ReadBlockRegValue ("fc7_daq_ctrl.calibration_2s_block.counter_fifo", cEventSize - 1);
            //for(auto word : counters_data) std::cout << std::hex << word << std::dec << std::endl;

            uint32_t cParserOffset = 0;
            while(cParserOffset < counters_data.size()) {
                // get chip header
                uint32_t chipHeader = counters_data.at(cParserOffset);
                // check it
                if (((chipHeader >> 28) & 0xF) != 0xA) {
                    LOG(ERROR) << "Something bad with chip header";
                    return false;
                }
                // get hybrid chip id
                uint8_t cHybridId = (chipHeader >> 20) & 0xFF;
                uint8_t cChipId = (chipHeader >> 16) & 0xF;
                uint8_t cErrorCounter = (chipHeader >> 8) & 0xFF;
                uint8_t cTriggerCounter = (chipHeader >> 0) & 0xFF;
                //LOG(INFO) << "\tHybrid: " << +cHybridId << ", Chip: " << +cChipId << ", Error Counter: " << +cErrorCounter << ", Trigger Counter: " << +cTriggerCounter;
                if (cTriggerCounter != pNEvents) {
                    LOG(ERROR) << "Number of triggers does not match the requested amount";
                    return false;
                }

                // now parse the counters
                pErrorCounters[cHybridId][cChipId] = cErrorCounter;
                for(uint8_t ch = 0; ch < NCHANNELS; ch++) {
                    uint8_t cWordId = cParserOffset + 1 + (uint8_t)ch/(32/COUNTER_WIDTH_BITS); // 1 for header, ch/4 because we have 4 counters per word
                    uint8_t cBitOffset = ch%(32/COUNTER_WIDTH_BITS) * COUNTER_WIDTH_BITS;
                    pChannelCounters[cHybridId][cChipId][ch] = (counters_data.at(cWordId) >> cBitOffset) & BIT_MASK;
                }

                // increment the offset
                cParserOffset += (1 + (NCHANNELS + (4-NCHANNELS%4))/4);
            }
        }

        // debug out
        //for(uint8_t ch = 0; ch < NCHANNELS; ch++) std::cout << "Ch: " << +ch << ", Counter: " << +pChannelCounters[0][0][ch] << std::endl;

        // just in case write back the old backrepssure valie
        WriteReg ("fc7_daq_cnfg.fast_command_block.misc.backpressure_enable", cBackpressureOldValue);

        // return
        return true;
    }

    // method to remove the arrays
    void D19cFWInterface::Manage2SCountersMemory(uint8_t **&pErrorCounters, uint8_t ***&pChannelCounters, bool pAllocate)
    {
        // this will anyway be constant
        const unsigned int NCHIPS_PER_HYBRID_COUNTERS = 8; // data from one CIC
        const unsigned int HYBRIDS_TOTAL = fFWNHybrids; // for allocation

        if (pAllocate) {
            // allocating the array
            if (pChannelCounters == nullptr && pErrorCounters == nullptr) {
                // allocate
                pChannelCounters = new uint8_t**[HYBRIDS_TOTAL];
                pErrorCounters = new uint8_t*[HYBRIDS_TOTAL];
                for(uint32_t h = 0; h < HYBRIDS_TOTAL; h++) {
                    pChannelCounters[h] = new uint8_t*[NCHIPS_PER_HYBRID_COUNTERS];
                    pErrorCounters[h] = new uint8_t[NCHIPS_PER_HYBRID_COUNTERS];
                    for(uint32_t c = 0; c < NCHIPS_PER_HYBRID_COUNTERS; c++) {
                        pChannelCounters[h][c] = new uint8_t[NCHANNELS];
                    }
                }

                // set to zero
                for(uint32_t h = 0; h < HYBRIDS_TOTAL; h++) {
                    for(uint32_t c = 0; c < NCHIPS_PER_HYBRID_COUNTERS; c++) {
                        for(int32_t ch = 0; ch < NCHANNELS; ch++) {
                            pChannelCounters[h][c][ch] = 0;
                        }
                    }
                }
            }
        } else {
            // deleting all the array
            for(uint32_t h = 0; h < HYBRIDS_TOTAL; h++) {
                for(uint32_t c = 0; c < NCHIPS_PER_HYBRID_COUNTERS; c++) delete pChannelCounters[h][c];
                    delete pChannelCounters[h];
                delete pErrorCounters[h];
            }
            delete pChannelCounters;
            delete pErrorCounters;
        }
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
            fpgaConfig = new D19cFpgaConfig ( this );
    }

    void D19cFWInterface::RebootBoard()
    {
        if ( !fpgaConfig )
            fpgaConfig = new D19cFpgaConfig ( this );

        fpgaConfig->resetBoard();
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

    //TODO: cleanup here the version
    //if (fI2CVersion >= 1) {
        return true;
    //} else {
    //  return ( (cWord1 & 0x00F2FFFF) == (cWord2 & 0x00F2FFFF) );
    //}
    }

    bool D19cFWInterface::cmd_reply_ack (const uint32_t& cWord1, const
       uint32_t& cWord2)
    {
        // if it was a write transaction (>>17 == 0) and
        // the CBC id matches it is false
        if (  ( (cWord2 >> 16) & 0x1 ) == 0 && (cWord1 & 0x00F00000) == (cWord2 & 0x00F00000) ) return true;
        else return false;
    }

    void D19cFWInterface::PSInterfaceBoard_PowerOn_SSA_v1(float VDDPST , float DVDD , float AVDD , float VBF, float BG, uint8_t mpaid  , uint8_t ssaid  )
    {

        uint32_t read = 1;
        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t pcf8574 = 1;
        uint32_t dac7678 = 4;
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );

        PSInterfaceBoard_SetSlaveMap();
        PSInterfaceBoard_ConfigureI2CMaster(1,SLOW);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );

        float Vc = 0.0003632813;

        LOG(INFO) << "ssa vdd on" ;

        float Vlimit = 1.32;
        if (VDDPST > Vlimit) VDDPST = Vlimit;
        float diffvoltage = 1.5 - VDDPST;
        uint32_t setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;

        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x33, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        LOG(INFO) << "ssa vddD on";
        Vlimit = 1.32;
        if (DVDD > Vlimit) DVDD = Vlimit;
        diffvoltage = 1.5 - DVDD;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x31, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        LOG(INFO) << "ssa vddA on";
        Vlimit = 1.32;
        if (AVDD > Vlimit) AVDD = Vlimit;
        diffvoltage = 1.5 - AVDD;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01) ; // to SCO on PCA9646
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x35, setvoltage) ; // tx to DAC C
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        LOG(INFO) << "ssa BG on";
        Vlimit = 1.32;
        if (BG > Vlimit) BG = Vlimit;
        float Vc2 = 4095/1.5;
        setvoltage = int(round(BG * Vc2));
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        LOG(INFO) << "ssa VBF on";
        Vlimit = 0.5;
        if (VBF > Vlimit) VBF = Vlimit;
        setvoltage = int(round(VBF * Vc2));
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x37, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        std::string DONE = "no";
        while (not (DONE == "yes" or DONE == "Yes" or DONE == "YES"))
        {
            LOG(INFO) << BOLDBLUE << "Write or read? (W/R)" << RESET;
            std::string RW;
            std::cin >> RW;
            if (RW == "W" or RW == "w")
            {
                std::string VALSTRING = "0";
                LOG (INFO) << BOLDBLUE << "What value are you writing?" << RESET;
                std::cin >> VALSTRING;
                uint32_t VAL = std::stoi(VALSTRING);
                LOG (INFO) << BOLDRED << "writing SSA/MPA byte: " << std::bitset<32>(VAL) << RESET;
                PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);
                std::this_thread::sleep_for (std::chrono::milliseconds (500) );
                PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, VAL);  // set reset bit
            }
            if (RW == "R" or RW == "r")
            {
                uint32_t BYTE;
                BYTE = PSInterfaceBoard_SendI2CCommand_READ(pcf8574, 0, read, 0, 0);
                LOG(INFO) << BOLDRED << "reading SSA/MPA byte: " << std::bitset<32>(BYTE) << RESET;
            }
            LOG( INFO) << BOLDBLUE << "Are you done?" << RESET;
            std::cin >> DONE;
        }

    /*  LOG (INFO) << "ssa set address"; FIXME Temporarily switched to user input for probe-tests with Ed B.
        uint32_t val = (mpaid << 5) + (ssaid << 1);
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // to SCO on PCA9646
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val);  // tx to DAC C
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        LOG(INFO) << "ssa enable";
        uint32_t val2 = (mpaid << 5) + (ssaid << 1) + 1; // reset bit for MPA
        LOG (INFO) << RED << val2 << RESET;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        LOG(INFO) << BOLDBLUE << "ssa enable bit: " << std::bitset<32>(val2) << RESET;
        PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val2);  // set reset bit
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
    */
        // disable the i2c master at the end (first set the mux to the chip)
     //   PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x04);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_ConfigureI2CMaster(0, SLOW);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
    }

    void D19cFWInterface::PSInterfaceBoard_PowerOn_SSA_v2(float VDDPST , float DVDD , float AVDD , float VBG , uint8_t mpaid  , uint8_t ssaid  )
    {

        std::chrono::milliseconds cWait( 1500 );
    }

////// MPA/SSA Methods:


// COMS:
    void D19cFWInterface::PSInterfaceBoard_SetSlaveMap()
    {

        std::vector< std::vector<uint32_t> >  i2c_slave_map;
        i2c_slave_map.push_back({0x70, 0, 1, 1, 0, 1}); //0  PCA9646
        i2c_slave_map.push_back({0x20, 0, 1, 1, 0, 1}); //1  PCF8574
        i2c_slave_map.push_back({0x24, 0, 1, 1, 0, 1}); //2  PCF8574
        i2c_slave_map.push_back({0x14, 0, 2, 3, 0, 1}); //3  LTC2487
        i2c_slave_map.push_back({0x48, 1, 2, 2, 0, 0}); //4  DAC7678
        i2c_slave_map.push_back({0x40, 1, 2, 2, 0, 1}); //5  INA226
        i2c_slave_map.push_back({0x41, 1, 2, 2, 0, 1}); //6  INA226
        i2c_slave_map.push_back({0x42, 1, 2, 2, 0, 1}); //7  INA226
        i2c_slave_map.push_back({0x44, 1, 2, 2, 0, 1}); //8  INA226
        i2c_slave_map.push_back({0x45, 1, 2, 2, 0, 1}); //9  INA226
        i2c_slave_map.push_back({0x46, 1, 2, 2, 0, 1}); //10  INA226
        i2c_slave_map.push_back({0x40, 2, 1, 1, 1, 0}); //11  ????
        i2c_slave_map.push_back({0x20, 2, 1, 1, 1, 0}); //12  ????
        i2c_slave_map.push_back({ 0x0, 0, 1, 1, 0, 0}); //13  ????
        i2c_slave_map.push_back({ 0x0, 0, 1, 1, 0, 0}); //14  ????
        i2c_slave_map.push_back({0x5F, 1, 1, 1, 1, 0}); //15  CBC3


        LOG(INFO) << "Updating the Slave ID Map (mpa ssa board) ";

        for (int ism = 0; ism < 16; ism++)
        {
            uint32_t shifted_i2c_address            = i2c_slave_map[ism][0]<<25;
            uint32_t shifted_register_address_nbytes    = i2c_slave_map[ism][1]<<6;
            uint32_t shifted_data_wr_nbytes         = i2c_slave_map[ism][2]<<4;
            uint32_t shifted_data_rd_nbytes         = i2c_slave_map[ism][3]<<2;
            uint32_t shifted_stop_for_rd_en         = i2c_slave_map[ism][4]<<1;
            uint32_t shifted_nack_en            = i2c_slave_map[ism][5]<<0;
            uint32_t final_command              = shifted_i2c_address + shifted_register_address_nbytes + shifted_data_wr_nbytes + shifted_data_rd_nbytes + shifted_stop_for_rd_en + shifted_nack_en;

            std::string curreg = "fc7_daq_cnfg.mpa_ssa_board_block.slave_"+std::to_string(ism)+"_config";
            WriteReg(curreg, final_command);
        }

    }

    void D19cFWInterface::PSInterfaceBoard_ConfigureI2CMaster(uint32_t pEnabled = 1, uint32_t pFrequency = 4)
    {
    // wait for all commands to be executed
        std::chrono::milliseconds cWait( 100 );
        while (!ReadReg("fc7_daq_stat.command_processor_block.i2c.command_fifo.empty")) {
            std::this_thread::sleep_for( cWait );
        }

        if( pEnabled > 0) LOG (INFO) << "Enabling the MPA SSA Board I2C master";
        else LOG (INFO) << "Disabling the MPA SSA Board I2C master";

    // setting the values
        WriteReg( "fc7_daq_cnfg.physical_interface_block.i2c.master_en", int(not pEnabled) );
        WriteReg( "fc7_daq_cnfg.mpa_ssa_board_block.i2c_master_en", pEnabled);
        WriteReg( "fc7_daq_cnfg.mpa_ssa_board_block.i2c_freq", pFrequency);

        std::this_thread::sleep_for( cWait );

    // resetting the fifos and the board
        WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.control.reset", 1);
        WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.control.reset_fifos", 1);
        WriteReg( "fc7_daq_ctrl.mpa_ssa_board_block.reset", 1);
        std::this_thread::sleep_for( cWait );
    }

    void D19cFWInterface::PSInterfaceBoard_SendI2CCommand(uint32_t slave_id,uint32_t board_id,uint32_t read,uint32_t register_address, uint32_t data)
    {

        std::chrono::milliseconds cWait( 10 );
        std::chrono::milliseconds cShort( 1 );

        uint32_t shifted_command_type   = 1 << 31;
        uint32_t shifted_word_id_0  = 0;
        uint32_t shifted_slave_id   = slave_id << 21;
        uint32_t shifted_board_id   = board_id << 20;
        uint32_t shifted_read       = read << 16;
        uint32_t shifted_register_address = register_address;

        uint32_t shifted_word_id_1  = 1<<26;
        uint32_t shifted_data       = data;


        uint32_t word_0 = shifted_command_type + shifted_word_id_0 + shifted_slave_id + shifted_board_id + shifted_read + shifted_register_address;
        uint32_t word_1 = shifted_command_type + shifted_word_id_1 + shifted_data;


        WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", word_0);
        std::this_thread::sleep_for( cShort );
        WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", word_1);
        std::this_thread::sleep_for( cShort );

        int readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
        while (readempty > 0)
        {
            std::this_thread::sleep_for( cShort );
            readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
        }

        // int reply = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply");
        int reply_err = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.err");
        int reply_data = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.data");
    //LOG(INFO) << BOLDGREEN << "reply: "<< std::hex << reply << std::dec <<RESET;
    //LOG(INFO) << BOLDGREEN << "reply err: "<< std::hex << reply_err << std::dec <<RESET;
    //LOG(INFO) << BOLDGREEN << "reply data: "<< std::hex << reply_data << std::dec <<RESET;

        if (reply_err == 1) LOG(ERROR) << "Error code: "<< std::hex << reply_data << std::dec;
    //  print "ERROR! Error flag is set to 1. The data is treated as the error code."
    //elif reply_slave_id != slave_id:
    //  print "ERROR! Slave ID doesn't correspond to the one sent"
    //elif reply_board_id != board_id:
    //  print "ERROR! Board ID doesn't correspond to the one sent"

        else
        {
            if (read == 1) LOG (INFO) << BOLDBLUE <<  "Data that was read is: "<< reply_data << RESET;
            else LOG (DEBUG) << BOLDBLUE << "Successful write transaction" <<RESET;
        }
    }

    uint32_t D19cFWInterface::PSInterfaceBoard_SendI2CCommand_READ(uint32_t slave_id,uint32_t board_id,uint32_t read,uint32_t register_address, uint32_t data)
    {

        std::chrono::milliseconds cWait( 10 );
        std::chrono::milliseconds cShort( 1 );

        uint32_t shifted_command_type   = 1 << 31;
        uint32_t shifted_word_id_0  = 0;
        uint32_t shifted_slave_id   = slave_id << 21;
        uint32_t shifted_board_id   = board_id << 20;
        uint32_t shifted_read       = read << 16;
        uint32_t shifted_register_address = register_address;

        uint32_t shifted_word_id_1  = 1<<26;
        uint32_t shifted_data       = data;

        uint32_t word_0 = shifted_command_type + shifted_word_id_0 + shifted_slave_id + shifted_board_id + shifted_read + shifted_register_address;
        uint32_t word_1 = shifted_command_type + shifted_word_id_1 + shifted_data;


        WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", word_0);
        std::this_thread::sleep_for( cWait );
        WriteReg( "fc7_daq_ctrl.command_processor_block.i2c.command_fifo", word_1);
        std::this_thread::sleep_for( cWait );

        int readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
        LOG (INFO) << BOLDYELLOW << readempty << RESET;
        while (readempty > 0)
        {
            std::cout << ".";
            std::this_thread::sleep_for( cShort );
            readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
        }
        std::cout<<std::endl;

        uint32_t reply = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply");
    //LOG (INFO) << BOLDRED << std::hex << reply << std::dec << RESET;
        uint32_t reply_err = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.err");
        uint32_t reply_data = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.data");

        if (reply_err == 1) LOG(ERROR) << "Error code: "<< std::hex << reply_data << std::dec;
    //  print "ERROR! Error flag is set to 1. The data is treated as the error code."
    //elif reply_slave_id != slave_id:
    //  print "ERROR! Slave ID doesn't correspond to the one sent"
    //elif reply_board_id != board_id:
    //  print "ERROR! Board ID doesn't correspond to the one sent"

        else
        {
            if (read == 1){
                LOG (INFO) << BOLDBLUE <<  "Data that was read is: "<< std::hex << reply_data << std::dec << "   ecode: " << reply_err << RESET;
                return reply & 0xFFFFFF;        
            }
            else LOG (DEBUG) << BOLDBLUE << "Successful write transaction" <<RESET;
        }

        return 0;
    }


    void D19cFWInterface::Pix_write_MPA(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel,uint32_t data)
    {
        uint8_t cWriteAttempts = 0;

        ChipRegItem rowreg =cRegItem;
        rowreg.fAddress  = ((row & 0x0001f) << 11 ) | ((cRegItem.fAddress & 0x000f) << 7 ) | (pixel & 0xfffffff);
        rowreg.fValue  = data;
        std::vector<uint32_t> cVecReq;
        cVecReq.clear();
        this->EncodeReg (rowreg, cMPA->getFeId(), cMPA->getMPAId(), cVecReq, false, true);
        this->WriteChipBlockReg (cVecReq, cWriteAttempts, false);
    }

    uint32_t D19cFWInterface::Pix_read_MPA(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel)
    {
        uint8_t cWriteAttempts = 0;
        uint32_t rep;

        std::vector<uint32_t> cVecReq;
        cVecReq.clear();
        this->EncodeReg (cRegItem, cMPA->getFeId(), cMPA->getMPAId(), cVecReq, false, false);
        this->WriteChipBlockReg (cVecReq,cWriteAttempts, false);
        std::chrono::milliseconds cShort( 1 );
    //uint32_t readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
    //while (readempty == 0)
    //  {
    //  std::cout<<"RE:"<<readempty<<std::endl;
    //  //ReadStatus()
    //  std::this_thread::sleep_for( cShort );
    //  readempty = ReadReg ("fc7_daq_stat.command_processor_block.i2c.reply_fifo.empty");
    //  }
    //uint32_t forcedreply = ReadReg("fc7_daq_ctrl.command_processor_block.i2c.reply_fifo");
        rep = ReadReg ("fc7_daq_ctrl.command_processor_block.i2c.mpa_ssa_i2c_reply.data");

        return rep;
    }



    std::vector<uint16_t> D19cFWInterface::ReadoutCounters_MPA(uint32_t raw_mode_en)
    {
        WriteReg("fc7_daq_cnfg.physical_interface_block.raw_mode_en", raw_mode_en);
        uint32_t mpa_counters_ready = ReadReg("fc7_daq_stat.physical_interface_block.stat_slvs_debug.mpa_counters_ready");
        std::chrono::milliseconds cWait( 10 );
        std::vector<uint16_t> count(2040, 0);
    //std::cout<<"MCR  "<<mpa_counters_ready<<std::endl;
        PS_Start_counters_read();
        uint32_t  timeout = 0;
        while ((mpa_counters_ready == 0) & (timeout < 50))
        {
            std::this_thread::sleep_for( cWait );
            mpa_counters_ready = ReadReg("fc7_daq_stat.physical_interface_block.stat_slvs_debug.mpa_counters_ready");
        //std::cout<<"MCR iwh"<<mpa_counters_ready<<std::endl;
            timeout += 1;
        }
        if (timeout >= 50)
        {
            std::cout<<"fail"<<std::endl;
            return count;
        }

        if (raw_mode_en == 1)
        {
            uint32_t cycle = 0;
            for (int i=0; i<20000;i++)
            {
                uint32_t fifo1_word = ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo1_data");
                uint32_t fifo2_word = ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo2_data");

                uint32_t line1 = (fifo1_word&0x0000FF)>>0; //to_number(fifo1_word,8,0)
                uint32_t line2 = (fifo1_word&0x00FF00)>>8; // to_number(fifo1_word,16,8)
                uint32_t line3 = (fifo1_word&0xFF0000)>>16; //  to_number(fifo1_word,24,16)

                uint32_t line4 = (fifo2_word&0x0000FF)>>0; //to_number(fifo2_word,8,0)
                uint32_t line5 = (fifo2_word&0x00FF00)>>8; // to_number(fifo2_word,16,8)

                if (((line1 & 0x80) == 128) && ((line4 & 0x80) == 128))
                {
                    uint32_t temp = ((line2 & 0x20) << 9) | ((line3 & 0x20) << 8) | ((line4 & 0x20) << 7) | ((line5 & 0x20) << 6) | ((line1 & 0x10) << 6) | ((line2 & 0x10) << 5) | ((line3 & 0x10) << 4) | ((line4 & 0x10) << 3) | ((line5 & 0x80) >> 1) | ((line1 & 0x40) >> 1) | ((line2 & 0x40) >> 2) | ((line3 & 0x40) >> 3) | ((line4 & 0x40) >> 4) | ((line5 & 0x40) >> 5) | ((line1 & 0x20) >> 5);
                    if (temp != 0) 
                    {
                        count[cycle] = temp - 1;
                        cycle += 1;
                    }
                }
            }
        } 
        else    {
            ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo2_data");
            for (int i=0; i<2040;i++)
            {
            //std::chrono::milliseconds cWait( 100 );
                count[i] = ReadReg("fc7_daq_ctrl.physical_interface_block.ctrl_slvs_debug_fifo2_data") - 1;
            //std::cout<<i<<"     "<<count[i]<<std::endl;
            }
        }

        std::this_thread::sleep_for( cWait );
        mpa_counters_ready = ReadReg("fc7_daq_stat.physical_interface_block.stat_slvs_debug.mpa_counters_ready");
        return count;
    }

    void D19cFWInterface::Compose_fast_command(uint32_t duration ,uint32_t resync_en ,uint32_t l1a_en ,uint32_t cal_pulse_en ,uint32_t bc0_en )
    {
        uint32_t encode_resync = resync_en<<16;
        uint32_t encode_cal_pulse = cal_pulse_en<<17;
        uint32_t encode_l1a = l1a_en<<18;
        uint32_t encode_bc0 = bc0_en<<19;
        uint32_t encode_duration = duration<<28;

        uint32_t final_command = encode_resync + encode_l1a + encode_cal_pulse + encode_bc0 + encode_duration;

        WriteReg("fc7_daq_ctrl.fast_command_block.control", final_command);

    }

    void D19cFWInterface::PS_Open_shutter(uint32_t duration )
    {
        Compose_fast_command(duration,0,1,0,0);
    }

    void D19cFWInterface::PS_Close_shutter(uint32_t duration )
    {
        Compose_fast_command(duration,0,0,0,1);
    }

    void D19cFWInterface::PS_Clear_counters(uint32_t duration )
    {
        Compose_fast_command(duration,0,1,0,1);
    }
    void D19cFWInterface::PS_Start_counters_read(uint32_t duration )
    {
        Compose_fast_command(duration,1,0,0,1);
    }

    void D19cFWInterface::KillI2C()
    {
        PSInterfaceBoard_SendI2CCommand(0, 0, 0, 0, 0x04);
        PSInterfaceBoard_ConfigureI2CMaster(0);
    }

// POWER:
    void D19cFWInterface::PSInterfaceBoard_PowerOn( uint8_t mpaid  , uint8_t ssaid  )
    {

        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t powerenable = 2;

        PSInterfaceBoard_SetSlaveMap();

        LOG(INFO) << "Interface Board Power ON";

        PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(powerenable, 0, write, 0, 0x00); // There is an inverter! Be Careful!
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_ConfigureI2CMaster(0, SLOW);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

    }

    void D19cFWInterface::PSInterfaceBoard_PowerOff()
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );

        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t powerenable = 2;

        PSInterfaceBoard_SetSlaveMap();

        LOG(INFO) << "Interface Board Power OFF";

        PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_SendI2CCommand(powerenable, 0, write, 0, 0x01);
        std::this_thread::sleep_for (std::chrono::milliseconds (1000) );
        PSInterfaceBoard_ConfigureI2CMaster(0, SLOW);

    }

    void D19cFWInterface::ReadPower_SSA(uint8_t mpaid , uint8_t ssaid)
    {

        uint32_t read = 1;
        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t ina226_7 = 7;
        uint32_t ina226_6 = 6;
        uint32_t ina226_5 = 5;

        LOG (INFO) << BOLDBLUE << "power information:" << RESET;
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        PSInterfaceBoard_SetSlaveMap();
        PSInterfaceBoard_ConfigureI2CMaster(1,SLOW);

        LOG (INFO) << BOLDBLUE << " - - - VDD:" << RESET;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x08);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        uint32_t dread2 = PSInterfaceBoard_SendI2CCommand_READ(ina226_7, 0, read, 0x02, 0);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        float vret = float(dread2) * 0.00125;
        uint32_t dread1 = PSInterfaceBoard_SendI2CCommand_READ(ina226_7, 0, read, 0x01, 0);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        float iret = float(dread1) * 0.00250 / 0.1;
        float pret = vret * iret;
        LOG (INFO) << BOLDGREEN << "V = " << vret << "V, I = " << iret << "mA, P = " << pret << "mW" << RESET;

        LOG (INFO) << BOLDBLUE << " - - - Digital:" << RESET;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x08);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        dread2 = PSInterfaceBoard_SendI2CCommand_READ(ina226_6, 0, read, 0x02, 0);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        vret = float(dread2) * 0.00125;
        dread1 = PSInterfaceBoard_SendI2CCommand_READ(ina226_6, 0, read, 0x01, 0);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        iret = float(dread1) * 0.00250 / 0.1;
        pret = vret * iret;
        LOG (INFO) << BOLDGREEN << "V = " << vret << "V, I = " << iret << "mA, P = " << pret << "mW" << RESET;

        LOG (INFO) << BOLDBLUE << " - - - Analog:" << RESET;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x08);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        dread2 = PSInterfaceBoard_SendI2CCommand_READ(ina226_5, 0, read, 0x02, 0);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        vret = float(dread2) * 0.00125;
        dread1 = PSInterfaceBoard_SendI2CCommand_READ(ina226_5, 0, read, 0x01, 0);
        std::this_thread::sleep_for (std::chrono::milliseconds (750) );
        iret = float(dread1) * 0.00250 / 0.1;
        pret = vret * iret;
        LOG (INFO) << BOLDGREEN << "V = " << vret << "V, I = " << iret << "mA, P = " << pret << "mW" << RESET;

        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x04);
        PSInterfaceBoard_ConfigureI2CMaster(0);

    }

    void D19cFWInterface::PSInterfaceBoard_PowerOn_MPA(float VDDPST , float DVDD , float AVDD , float VBG , uint8_t mpaid  , uint8_t ssaid  )
    {

        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t pcf8574 = 1;
        uint32_t dac7678 = 4;
        std::chrono::milliseconds cWait( 1500 );

        PSInterfaceBoard_SetSlaveMap();
        PSInterfaceBoard_ConfigureI2CMaster(1,SLOW);

        float Vc = 0.0003632813;

        LOG(INFO) << "mpa vdd on" ;

        float Vlimit = 1.32;
        if (VDDPST > Vlimit) VDDPST = Vlimit;
        float diffvoltage = 1.5 - VDDPST;
        uint32_t setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;

        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x33, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa vddD on";
        Vlimit = 1.2;
        if (DVDD > Vlimit) DVDD = Vlimit;
        diffvoltage = 1.5 - DVDD;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x31, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa vddA on";
        Vlimit = 1.32;
        if (AVDD > Vlimit) AVDD = Vlimit;
        diffvoltage = 1.5 - AVDD;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01) ; // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x35, setvoltage) ; // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa VBG on";
        Vlimit = 0.5;
        if (VBG > Vlimit) VBG = Vlimit;
        float Vc2 = 4095/1.5;
        setvoltage = int(round(VBG * Vc2));
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );


        LOG(INFO) << "mpa enable";
        uint32_t val2 = (mpaid << 5) + (ssaid << 1) + 1; // reset bit for MPA
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
        PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val2);  // set reset bit
        std::this_thread::sleep_for( cWait );

        // disable the i2c master at the end (first set the mux to the chip
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x04);
        PSInterfaceBoard_ConfigureI2CMaster(0, SLOW);
    }

    void D19cFWInterface::PSInterfaceBoard_PowerOff_SSA_v1(uint8_t mpaid , uint8_t ssaid )
    {
        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t pcf8574 = 1; // MPA and SSA address and reset 8 bit port
        uint32_t dac7678 = 4;
        float Vc = 0.0003632813; // V/Dac step
        std::chrono::milliseconds cWait( 1500 );

        PSInterfaceBoard_SetSlaveMap();
        PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);

        LOG(INFO) << "ssa disable";
        uint32_t val = (mpaid << 5) + (ssaid << 1); // reset bit for MPA
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
        PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val);  // set reset bit
        std::this_thread::sleep_for( cWait );


        LOG(INFO) << "ssa VBF off";
        uint32_t setvoltage = 0;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x37, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );


        LOG(INFO) << "ssa vddA off";
        float diffvoltage = 1.5;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x35, 0);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "ssa vddD off";
        diffvoltage = 1.5;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x31, 0);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "ssa vdd off";
        diffvoltage = 1.5;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x33, 0);  // tx to DAC C
        std::this_thread::sleep_for( cWait );
    }


    void D19cFWInterface::PSInterfaceBoard_PowerOff_SSA_v2(uint8_t mpaid , uint8_t ssaid )
    {
        std::chrono::milliseconds cWait( 1500 );
    }

    void D19cFWInterface::PSInterfaceBoard_PowerOff_MPA(uint8_t mpaid , uint8_t ssaid )
    {
        uint32_t write = 0;
        uint32_t SLOW = 2;
        uint32_t i2cmux = 0;
        uint32_t pcf8574 = 1; // MPA and SSA address and reset 8 bit port
        uint32_t dac7678 = 4;
        float Vc = 0.0003632813; // V/Dac step
        std::chrono::milliseconds cWait( 1000 );

        PSInterfaceBoard_SetSlaveMap();
        PSInterfaceBoard_ConfigureI2CMaster(1, SLOW);

        LOG(INFO) << "mpa disable";
        uint32_t val = (mpaid << 5) + (ssaid << 1); // reset bit for MPA
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x02);  // route to 2nd PCF8574
        PSInterfaceBoard_SendI2CCommand(pcf8574, 0, write, 0, val);  // set reset bit
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa VBG off";
        uint32_t setvoltage = 0;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x36, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa vddA off";
        float diffvoltage = 1.5;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x32, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa vddA off";
        diffvoltage = 1.5;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x30, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

        LOG(INFO) << "mpa vdd off";
        diffvoltage = 1.5;
        setvoltage = int(round(diffvoltage / Vc));
        if (setvoltage > 4095) setvoltage = 4095;
        setvoltage = setvoltage << 4;
        PSInterfaceBoard_SendI2CCommand(i2cmux, 0, write, 0, 0x01);  // to SCO on PCA9646
        PSInterfaceBoard_SendI2CCommand(dac7678, 0, write, 0x34, setvoltage);  // tx to DAC C
        std::this_thread::sleep_for( cWait );

    }


    void D19cFWInterface::Align_out()
    {
        int cCounter = 0;
        int cMaxAttempts = 10;

        uint32_t hardware_ready = 0;

        while (hardware_ready < 1)
        {
            if (cCounter++ > cMaxAttempts)
            {
                uint32_t delay5_done_cbc0 = ReadReg ("fc7_daq_stat.physical_interface_block.delay5_done_cbc0");
                uint32_t serializer_done_cbc0 = ReadReg ("fc7_daq_stat.physical_interface_block.serializer_done_cbc0");
                uint32_t bitslip_done_cbc0 = ReadReg ("fc7_daq_stat.physical_interface_block.bitslip_done_cbc0");

                uint32_t delay5_done_cbc1 = ReadReg ("fc7_daq_stat.physical_interface_block.delay5_done_cbc1");
                uint32_t serializer_done_cbc1 = ReadReg ("fc7_daq_stat.physical_interface_block.serializer_done_cbc1");
                uint32_t bitslip_done_cbc1 = ReadReg ("fc7_daq_stat.physical_interface_block.bitslip_done_cbc1");
                LOG (INFO) << "Clock Data Timing tuning failed after " << cMaxAttempts << " attempts with value - aborting!";
                LOG (INFO) << "Debug Info CBC0: delay5 done: " << delay5_done_cbc0 << ", serializer_done: " << serializer_done_cbc0 << ", bitslip_done: " << bitslip_done_cbc0;
                LOG (INFO) << "Debug Info CBC1: delay5 done: " << delay5_done_cbc1 << ", serializer_done: " << serializer_done_cbc1 << ", bitslip_done: " << bitslip_done_cbc1;
                uint32_t tuning_state_cbc0 = ReadReg("fc7_daq_stat.physical_interface_block.state_tuning_cbc0");
                uint32_t tuning_state_cbc1 = ReadReg("fc7_daq_stat.physical_interface_block.state_tuning_cbc1");
                LOG(INFO) << "tuning state cbc0: " << tuning_state_cbc0 << ", cbc1: " << tuning_state_cbc1;
                exit (1);
            }

        this->ChipReSync();
        usleep (10);
        // reset  the timing tuning
        WriteReg("fc7_daq_ctrl.physical_interface_block.control.cbc3_tune_again", 0x1);
        std::this_thread::sleep_for (std::chrono::milliseconds (100) );
        hardware_ready = ReadReg ("fc7_daq_stat.physical_interface_block.hardware_ready");
        }
    }
}
