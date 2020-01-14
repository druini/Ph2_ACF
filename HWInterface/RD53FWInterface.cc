/*!
  \file                  RD53FWInterface.h
  \brief                 RD53FWInterface initialize and configure the FW
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53FWInterface.h"

namespace Ph2_HwInterface
{
  RD53FWInterface::RD53FWInterface (const char* pId, const char* pUri, const char* pAddressTable)
    : BeBoardFWInterface (pId, pUri, pAddressTable)
    , fpgaConfig         (nullptr)
    , ddr3Offset         (0)
  {}

  void RD53FWInterface::setFileHandler (FileHandler* pHandler)
  {
    if (pHandler != nullptr)
      {
        this->fFileHandler = pHandler;
        this->fSaveToFile  = true;
      }
    else LOG (ERROR) << BOLDRED << "NULL FileHandler" << RESET;
  }

  uint32_t RD53FWInterface::getBoardInfo()
  {
    uint32_t cVersionMajor = ReadReg ("user.stat_regs.usr_ver.usr_ver_major");
    uint32_t cVersionMinor = ReadReg ("user.stat_regs.usr_ver.usr_ver_minor");
    uint32_t cVersionWord  = ((cVersionMajor << NBIT_FWVER) | cVersionMinor);
    return cVersionWord;
  }

  void RD53FWInterface::ResetSequence()
  {
    LOG (INFO) << BOLDMAGENTA << "Resetting the backend board... it may take a while" << RESET;

    RD53FWInterface::TurnOffFMC();
    RD53FWInterface::TurnOnFMC();
    RD53FWInterface::ResetBoard();

    LOG (INFO) << BOLDMAGENTA << "Now you can start using the DAQ ... enjoy!" << RESET;
  }

  void RD53FWInterface::ConfigureBoard (const BeBoard* pBoard)
  {
    // ########################
    // # Print firmware infos #
    // ########################
    uint32_t cVersionMajor = ReadReg ("user.stat_regs.usr_ver.usr_ver_major");
    uint32_t cVersionMinor = ReadReg ("user.stat_regs.usr_ver.usr_ver_minor");

    uint32_t cFWyear       = ReadReg ("user.stat_regs.fw_date.year");
    uint32_t cFWmonth      = ReadReg ("user.stat_regs.fw_date.month");
    uint32_t cFWday        = ReadReg ("user.stat_regs.fw_date.day");
    uint32_t cFWhour       = ReadReg ("user.stat_regs.fw_date.hour");
    uint32_t cFWminute     = ReadReg ("user.stat_regs.fw_date.minute");
    uint32_t cFWseconds    = ReadReg ("user.stat_regs.fw_date.seconds");

    LOG (INFO) << BOLDBLUE << "\t--> FW version : " << BOLDYELLOW << cVersionMajor << "." << cVersionMinor
               << BOLDBLUE << " -- date (yyyy/mm/dd) : " << BOLDYELLOW << cFWyear << "/" << cFWmonth << "/" << cFWday
               << BOLDBLUE << " -- time (hour:minute:sec) : " << BOLDYELLOW << cFWhour << ":" << cFWminute << ":" << cFWseconds << RESET;


    std::stringstream myString;
    RD53FWInterface::ChipReset();
    RD53FWInterface::ChipReSync();
    RD53FWInterface::ResetFastCmdBlk();
    RD53FWInterface::ResetSlowCmdBlk();
    RD53FWInterface::ResetReadoutBlk();


    // ###############################################
    // # FW register initialization from config file #
    // ###############################################
    RD53FWInterface::DIO5Config cfgDIO5;
    std::vector< std::pair<std::string, uint32_t> > cVecReg;
    LOG (INFO) << GREEN << "Initializing board's registers:" << RESET;
    for (const auto& it : pBoard->getBeBoardRegMap())
      {
        LOG (INFO) << BOLDBLUE << "\t--> " << it.first << " = " << BOLDYELLOW << it.second << RESET;
        if (it.first.find("ext_clk_en") != std::string::npos)
          {
            cfgDIO5.enable     = true;
            cfgDIO5.ext_clk_en = it.second;
          }
        else if (it.first.find("trigger_source") != std::string::npos) RD53FWInterface::localCfgFastCmd.trigger_source = static_cast<RD53FWInterface::TriggerSource>(it.second);
        else cVecReg.push_back({it.first, it.second});
      }


    // ################################
    // # Enabling modules and chips   #
    // # Module_type hard coded in FW #
    // # 1 = single chip module       #
    // # 2 = double chip module       #
    // # 4 = quad chip module         #
    // ################################
    this->singleChip  = ReadReg("user.stat_regs.aurora_rx.Module_type") == 1;
    uint32_t chips_en = 0;
    enabledModules    = 0;
    for (const auto& cModule : pBoard->fModuleVector)
      {
        uint16_t module_id = cModule->getFeId(); // @TMP@
        enabledModules |= 1 << module_id;
        if (this->singleChip == true) chips_en = enabledModules;
        else
          {
            uint16_t mod_chips_en = 0;
            for (const auto cChip : *cModule)
              {
                uint16_t chip_lane = static_cast<RD53*>(cChip)->getChipLane();
                mod_chips_en |= 1 << chip_lane;
              }
            chips_en |= mod_chips_en << (NLANE_MODULE * module_id);
          }
      }
    cVecReg.push_back({"user.ctrl_regs.Hybrids_en", enabledModules});
    cVecReg.push_back({"user.ctrl_regs.Chips_en", chips_en});

    if (cVecReg.size() != 0) WriteStackReg(cVecReg);


    // ##################
    // # Configure DIO5 #
    // ##################
    RD53FWInterface::ConfigureDIO5(&cfgDIO5);
    LOG(INFO) << GREEN<< "DIO5 configured" << RESET;
    usleep(DEEPSLEEP);


    // ###########################
    // # Print clock measurement #
    // ###########################
    uint32_t inputClk = ReadReg ("user.stat_regs.stat_reg_22");
    uint32_t gtxClk   = ReadReg ("user.stat_regs.stat_reg_21");
    LOG (INFO) << GREEN << "Input clock frequency (could be either internal or external, should be ~40 MHz): " << BOLDYELLOW << inputClk/1000. << " MHz" << RESET;
    LOG (INFO) << GREEN << "GTX receiver clock frequency (should be ~160 MHz): " << BOLDYELLOW << gtxClk/1000. << " MHz" << RESET;


    // ##############################
    // # AURORA lock on data stream #
    // ##############################
    while (RD53FWInterface::CheckChipCommunication() == false)
      {
        RD53FWInterface::WriteChipCommand(std::vector<uint16_t>(NFRAMES_SYNC, 0), -1);
        usleep(DEEPSLEEP);
      }
  }

  void RD53FWInterface::WriteChipCommand (const std::vector<uint16_t>& data, int moduleId)
  // #############################################
  // # moduleId < 0 --> broadcast to all modules #
  // #############################################
  {
    size_t n32bitWords = (data.size() / 2) + (data.size() % 2);


    // #####################
    // # Check if all good #
    // #####################
    if (ReadReg("user.stat_regs.slow_cmd.error_flag") == true)
      LOG (ERROR) << BOLDRED << "Write-command FIFO error" << RESET;

    if (ReadReg("user.stat_regs.slow_cmd.fifo_empty") == false)
      LOG (ERROR) << BOLDRED << "Write-command FIFO not empty" << RESET;


    // #######################
    // # Load command vector #
    // #######################
    std::vector<std::pair<std::string, uint32_t>> stackRegisters;
    stackRegisters.reserve(n32bitWords + 1);

    // Header
    stackRegisters.emplace_back("user.ctrl_regs.Slow_cmd_fifo_din", bits::pack<6, 10, 4, 12>(HEADEAR_WRTCMD, (moduleId < 0 ? enabledModules : 1 << moduleId), 0, n32bitWords));

    // Commands
    for (auto i = 1u; i < data.size(); i += 2)
      stackRegisters.emplace_back("user.ctrl_regs.Slow_cmd_fifo_din", bits::pack<16, 16>(data[i - 1], data[i]));

    // If data.size() is not even, add a sync command
    if (data.size() % 2 != 0)
      stackRegisters.emplace_back("user.ctrl_regs.Slow_cmd_fifo_din", bits::pack<16, 16>(data.back(), RD53CmdEncoder::SYNC));


    // ###############################
    // # Send command(s) to the chip #
    // ###############################
    stackRegisters.emplace_back("user.ctrl_regs.Slow_cmd.dispatch_packet", 1);
    stackRegisters.emplace_back("user.ctrl_regs.Slow_cmd.dispatch_packet", 0);

    WriteStackReg(stackRegisters);

    if (ReadReg("user.stat_regs.slow_cmd.fifo_packet_dispatched") == false)
      LOG (ERROR) << BOLDRED << "Error while dispatching chip register program" << RESET;
  }

  std::vector<std::pair<uint16_t,uint16_t>> RD53FWInterface::ReadChipRegisters (Chip* pChip)
  {
    std::vector<std::pair<uint16_t,uint16_t>> regReadback;

    uint32_t chipLane;
    if (this->singleChip == true) chipLane = pChip->getFeId(); // @TMP@
    else                          chipLane = NLANE_MODULE * pChip->getFeId() + static_cast<RD53*>(pChip)->getChipLane(); // @TMP@


    // #####################
    // # Read the register #
    // #####################
    if (ReadReg("user.stat_regs.Register_Rdback.fifo_full") == true) LOG (ERROR) << BOLDRED << "Read-command FIFO full" << RESET;

    while (ReadReg("user.stat_regs.Register_Rdback.fifo_empty") == false)
      {
        uint32_t readBackData = ReadReg("user.stat_regs.Register_Rdback_fifo");

        uint16_t lane, address, value;
        std::tie(lane, address, value) = bits::unpack<6, 10, 16>(readBackData);

        if (lane == chipLane) regReadback.emplace_back(address, value);
      }

    if (regReadback.size() == 0) LOG (ERROR) << BOLDRED << "Read-command FIFO empty" << RESET;


    return regReadback;
  }

  void RD53FWInterface::PrintFWstatus()
  {
    LOG (INFO) << GREEN << "Checking Firmware status" << RESET;


    // #################################
    // # Check clock generator locking #
    // #################################
    if (ReadReg ("user.stat_regs.global_reg.clk_gen_lock") == 1)
      LOG (INFO) << BOLDBLUE << "\t--> Clock generator is " << BOLDYELLOW << "locked" << RESET;
    else
      LOG (ERROR) << BOLDRED << "\t--> Clock generator is not locked" << RESET;


    // ############################
    // # Check I2C initialization #
    // ############################
    if (ReadReg ("user.stat_regs.global_reg.i2c_init") == 1)
      LOG (INFO) << BOLDBLUE << "\t--> I2C " << BOLDYELLOW << "initialized" << RESET;
    else
      {
        LOG (ERROR) << BOLDRED << "I2C not initialized" << RESET;
        unsigned int status = ReadReg ("user.stat_regs.global_reg.i2c_init_err");
        LOG (ERROR) << BOLDRED << "\t--> I2C initialization status: " << BOLDYELLOW << status << RESET;
      }

    if (ReadReg ("user.stat_regs.global_reg.i2c_acq_err") == 1)
      LOG (INFO) << GREEN << "I2C ack error during analog readout (for KSU FMC only)" << RESET;


    // ############################################################
    // # Check status registers associated wih fast command block #
    // ############################################################
    unsigned int fastCMDReg = ReadReg ("user.stat_regs.fast_cmd.trigger_source_o");
    LOG (INFO) << GREEN << "Fast CMD block trigger source: " << BOLDYELLOW << fastCMDReg << RESET << GREEN << " (1=IPBus, 2=Test-FSM, 3=TTC, 4=TLU, 5=External, 6=Hit-Or, 7=User-defined frequency)" << RESET;

    fastCMDReg = ReadReg ("user.stat_regs.fast_cmd.trigger_state");
    LOG (INFO) << GREEN << "Fast CMD block trigger state: " << BOLDYELLOW << fastCMDReg << RESET << GREEN << " (0=idle, 2=running)" << RESET;

    fastCMDReg = ReadReg ("user.stat_regs.fast_cmd.if_configured");
    LOG (INFO) << GREEN << "Fast CMD block check if configuraiton registers have been set: " << BOLDYELLOW << fastCMDReg << RESET;

    fastCMDReg = ReadReg ("user.stat_regs.fast_cmd.error_code");
    LOG (INFO) << GREEN << "Fast CMD block error code (0=no error): " << BOLDYELLOW << fastCMDReg << RESET;


    // ###########################
    // # Check trigger registers #
    // ###########################
    unsigned int trigReg = ReadReg ("user.stat_regs.trigger_cntr");
    LOG (INFO) << GREEN << "Trigger counter: " << BOLDYELLOW << trigReg << RESET;


    // ##########################
    // # Check module registers #
    // ##########################
    unsigned int modules = ReadReg("user.stat_regs.aurora_rx.Module_type");
    LOG (INFO) << GREEN << "Module type: " << BOLDYELLOW << modules << RESET << GREEN " (1=single chip, 2=double chip, 4=quad chip)" << RESET;

    modules = ReadReg("user.stat_regs.aurora_rx.Nb_of_modules");
    LOG (INFO) << GREEN << "Number of modules which can be potentially readout: " << BOLDYELLOW << modules << RESET;
  }

  bool RD53FWInterface::CheckChipCommunication()
  {
    LOG (INFO) << GREEN << "Checking status communication FW <----> RD53" << RESET;


    // ###############################
    // # Check RD53 AURORA registers #
    // ###############################
    unsigned int speed_flag = ReadReg ("user.stat_regs.aurora_rx.speed");
    LOG (INFO) << BOLDBLUE << "\t--> Aurora speed: " << BOLDYELLOW << (speed_flag == 0 ? "1.28 Gbps" : "640 Mbps") << RESET;


    // ########################################
    // # Check communication with the chip(s) #
    // ########################################
    unsigned int chips_en = ReadReg ("user.ctrl_regs.Chips_en");
    LOG (INFO) << BOLDBLUE << "\t--> Number of required data lanes: " << BOLDYELLOW << RD53::countBitsOne(chips_en) << BOLDBLUE << " i.e. " << BOLDYELLOW << std::bitset<12>(chips_en) << RESET;

    unsigned int channel_up = ReadReg ("user.stat_regs.aurora_rx_channel_up");
    LOG (INFO) << BOLDBLUE << "\t--> Number of active data lanes:   " << BOLDYELLOW << RD53::countBitsOne(channel_up) << BOLDBLUE << " i.e. " << BOLDYELLOW << std::bitset<12>(channel_up) << RESET;

    if (chips_en & ~channel_up)
    {
      LOG (ERROR) << BOLDRED << "\t--> Some data lanes are enabled but inactive" << RESET;
      return false;
    }

    LOG (INFO) << BOLDBLUE << "\t--> All enabled data lanes are active" << RESET;
    return true;
  }

  void RD53FWInterface::Start()
  {
    RD53FWInterface::ChipReset();
    RD53FWInterface::ChipReSync();
    RD53FWInterface::ResetReadoutBlk();

    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.start_trigger");
  }

  void RD53FWInterface::Stop()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.stop_trigger");
  }

  void RD53FWInterface::Pause()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.stop_trigger");
  }

  void RD53FWInterface::Resume()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.start_trigger");
  }

  std::vector<uint32_t> RD53FWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize)
  {
    uhal::ValVector<uint32_t> valBlock = RegManager::ReadBlockReg (pRegNode, pBlocksize);
    return valBlock.value();
  }

  void RD53FWInterface::TurnOffFMC()
  {
    WriteStackReg({
        {"system.ctrl_2.fmc_pg_c2m",    0},
        {"system.ctrl_2.fmc_l8_pwr_en", 0},
        {"system.ctrl_2.fmc_l12_pwr_en",0}});
  }

  void RD53FWInterface::TurnOnFMC()
  {
    WriteStackReg({
        {"system.ctrl_2.fmc_l12_pwr_en",1},
        {"system.ctrl_2.fmc_l8_pwr_en", 1},
        {"system.ctrl_2.fmc_pg_c2m",    1}});

    usleep(DEEPSLEEP);
  }

  void RD53FWInterface::ResetBoard()
  {
    // #######
    // # Set #
    // #######
    WriteReg ("user.ctrl_regs.reset_reg.aurora_rst",0);
    WriteReg ("user.ctrl_regs.reset_reg.aurora_pma_rst",0);
    WriteReg ("user.ctrl_regs.reset_reg.global_rst",1);
    WriteReg ("user.ctrl_regs.reset_reg.clk_gen_rst",1);
    WriteReg ("user.ctrl_regs.reset_reg.fmc_pll_rst",0);
    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",1);
    WriteReg ("user.ctrl_regs.reset_reg.i2c_rst",1);


    // #########
    // # Reset #
    // #########
    WriteReg ("user.ctrl_regs.reset_reg.global_rst",0);
    WriteReg ("user.ctrl_regs.reset_reg.clk_gen_rst",0);
    WriteReg ("user.ctrl_regs.reset_reg.fmc_pll_rst",1);
    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",0);

    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.i2c_rst",0);
    WriteReg ("user.ctrl_regs.reset_reg.aurora_pma_rst",1);
    WriteReg ("user.ctrl_regs.reset_reg.aurora_rst",1);


    // ########
    // # DDR3 #
    // ########
    LOG (INFO) << YELLOW << "Waiting for DDR3 calibration..." << RESET;
    while (ReadReg("user.stat_regs.readout1.ddr3_initial_calibration_done").value() == false) usleep(DEEPSLEEP);

    LOG (INFO) << BOLDBLUE << "\t--> DDR3 calibration done" << RESET;
  }

  void RD53FWInterface::ResetFastCmdBlk()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.ipb_reset");

    WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_fast_duration",IPBUS_FASTDURATION);
  }

  void RD53FWInterface::ResetSlowCmdBlk()
  {
    WriteStackReg({
        {"user.ctrl_regs.Slow_cmd.fifo_reset", 1},
        {"user.ctrl_regs.Slow_cmd.fifo_reset", 0},
        {"user.ctrl_regs.Register_RdBack.fifo_reset", 1},
        {"user.ctrl_regs.Register_RdBack.fifo_reset", 0}});
  }

  void RD53FWInterface::ResetReadoutBlk()
  {
    ddr3Offset = 0;
    WriteStackReg({
        {"user.ctrl_regs.reset_reg.readout_block_rst",1},
        {"user.ctrl_regs.reset_reg.readout_block_rst",0}});
  }

  void RD53FWInterface::ChipReset()
  {
    WriteStackReg({
        {"user.ctrl_regs.reset_reg.scc_rst",1},
        {"user.ctrl_regs.reset_reg.scc_rst",0},
        {"user.ctrl_regs.fast_cmd_reg_1.ipb_ecr",1},
        {"user.ctrl_regs.fast_cmd_reg_1.ipb_ecr",0}});
  }

  void RD53FWInterface::ChipReSync()
  {
    WriteStackReg({
        {"user.ctrl_regs.fast_cmd_reg_1.ipb_bcr",1},
        {"user.ctrl_regs.fast_cmd_reg_1.ipb_bcr",0}});
  }

  void RD53FWInterface::PrintEvents (const std::vector<RD53FWInterface::Event>& events, const std::vector<uint32_t>& pData)
  {
    // ##################
    // # Print raw data #
    // ##################
    if (pData.size() != 0)
      for (auto j = 0u; j < pData.size(); j++)
        {
          if (j%NWORDS_DDR3 == 0) std::cout << std::dec << j << ":\t";
          std::cout << std::hex << std::setfill('0') << std::setw(8) << pData[j] << "\t";
          if (j%NWORDS_DDR3 == NWORDS_DDR3-1) std::cout << std::endl;
        }

    // ######################
    // # Print decoded data #
    // ######################
    for (auto i = 0u; i < events.size(); i++)
      {
        auto& evt = events[i];
        LOG (INFO) << BOLDGREEN << "==========================="               << RESET;
        LOG (INFO) << BOLDGREEN << "EVENT           = " << i                   << RESET;
        LOG (INFO) << BOLDGREEN << "block_size      = " << evt.block_size      << RESET;
        LOG (INFO) << BOLDGREEN << "tlu_trigger_id  = " << evt.tlu_trigger_id  << RESET;
        LOG (INFO) << BOLDGREEN << "data_format_ver = " << evt.data_format_ver << RESET;
        LOG (INFO) << BOLDGREEN << "tdc             = " << evt.tdc             << RESET;
        LOG (INFO) << BOLDGREEN << "l1a_counter     = " << evt.l1a_counter     << RESET;
        LOG (INFO) << BOLDGREEN << "bx_counter      = " << evt.bx_counter      << RESET;

        for (auto j = 0u; j < evt.chip_events.size(); j++)
          {
            LOG (INFO) << CYAN << "------- Chip Header -------"                            << RESET;
            LOG (INFO) << CYAN << "error_code      = " << evt.chip_frames[j].error_code    << RESET;
            LOG (INFO) << CYAN << "module_id       = " << evt.chip_frames[j].module_id     << RESET;
            LOG (INFO) << CYAN << "chip_lane       = " << evt.chip_frames[j].chip_lane     << RESET;
            LOG (INFO) << CYAN << "l1a_data_size   = " << evt.chip_frames[j].l1a_data_size << RESET;
            LOG (INFO) << CYAN << "chip_type       = " << evt.chip_frames[j].chip_type     << RESET;
            LOG (INFO) << CYAN << "frame_delay     = " << evt.chip_frames[j].frame_delay   << RESET;

            LOG (INFO) << CYAN << "trigger_id      = " << evt.chip_events[j].trigger_id    << RESET;
            LOG (INFO) << CYAN << "trigger_tag     = " << evt.chip_events[j].trigger_tag   << RESET;
            LOG (INFO) << CYAN << "bc_id           = " << evt.chip_events[j].bc_id         << RESET;

            LOG (INFO) << BOLDYELLOW << "--- Hit Data (" << evt.chip_events[j].hit_data.size() << " hits) ---" << RESET;

            for (const auto& hit : evt.chip_events[j].hit_data)
              {
                LOG (INFO) << BOLDYELLOW << "Column: " << std::setw(3) <<  hit.col << std::setw(-1)
                                         << ", Row: "  << std::setw(3) <<  hit.row << std::setw(-1)
                                         << ", ToT: "  << std::setw(3) << +hit.tot << std::setw(-1)
                                         << RESET;
              }
          }
      }
  }

  bool RD53FWInterface::EvtErrorHandler(uint16_t status)
  {
    bool isGood = true;

    if (status & RD53FWEvtEncoder::EVSIZE)
      {
        LOG (ERROR) << BOLDRED << "Invalid event size " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53FWEvtEncoder::EMPTY)
      {
        LOG (ERROR) << BOLDRED << "No data collected " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53FWEvtEncoder::INCOMPLETE)
      {
        LOG (ERROR) << BOLDRED << "Incomplete event header " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53FWEvtEncoder::L1A)
      {
        LOG (ERROR) << BOLDRED << "L1A counter mismatch " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53FWEvtEncoder::FWERR)
      {
        LOG (ERROR) << BOLDRED << "Firmware error " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53FWEvtEncoder::FRSIZE)
      {
        LOG (ERROR) << BOLDRED << "Invalid frame size " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53FWEvtEncoder::MISSCHIP)
      {
        LOG (ERROR) << BOLDRED << "Chip data are missing " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53EvtEncoder::CHIPHEAD)
      {
        LOG (ERROR) << BOLDRED << "Invalid chip header " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53EvtEncoder::CHIPPIX)
      {
        LOG (ERROR) << BOLDRED << "Invalid pixel row or column " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    if (status & RD53EvtEncoder::CHIPNOHIT)
      {
        LOG (ERROR) << BOLDRED << " Hit data are missing " << BOLDYELLOW << "--> retry" << std::setfill(' ') << std::setw(8) << "" << RESET;
        isGood = false;
      }

    return isGood;
  }

  uint32_t RD53FWInterface::ReadData (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
  {
    uint32_t nWordsInMemoryOld, nWordsInMemory = 0;


    // ########################################
    // # Wait until we have something in DDR3 #
    // ########################################
    if (HANDSHAKE_EN == true)
      while (ReadReg("user.stat_regs.readout4.readout_req").value() == 0)
        {
          uint32_t fsm_status = ReadReg("user.stat_regs.readout4.fsm_status").value();
          LOG (ERROR) << BOLDRED << "Waiting for readout request, FSM status: " << BOLDYELLOW << fsm_status << RESET;
          usleep(READOUTSLEEP);
        }
    nWordsInMemory = ReadReg("user.stat_regs.words_to_read").value();


    // #############################################
    // # Wait for a stable number of words to read #
    // #############################################
    do
      {
        nWordsInMemoryOld = nWordsInMemory;
        usleep(READOUTSLEEP);
      }
    while (((nWordsInMemory = ReadReg("user.stat_regs.words_to_read").value()) != nWordsInMemoryOld) && (pWait == true)); // @TMP@
    // auto nTriggersReceived = ReadReg("user.stat_regs.trigger_cntr").value();


    // #############
    // # Read DDR3 #
    // #############
    uhal::ValVector<uint32_t> values = ReadBlockRegOffset("ddr3.fc7_daq_ddr3", nWordsInMemory, ddr3Offset);
    ddr3Offset += nWordsInMemory;
    for (const auto& val : values) pData.push_back(val);


    if ((this->fSaveToFile == true) && (pData.size() != 0)) this->fFileHandler->set(pData);
    return pData.size();
  }

  void RD53FWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
  {
    uint16_t status;
    bool     retry;
    int      nAttempts = 0;

    RD53FWInterface::localCfgFastCmd.n_triggers = pNEvents;
    RD53FWInterface::ConfigureFastCommands();

    do
      {
        nAttempts++;
        retry = false;
        pData.clear();


        // ####################
        // # Readout sequence #
        // ####################
        RD53FWInterface::Start();
        while (ReadReg("user.stat_regs.trigger_cntr").value() < pNEvents*(1 + RD53FWInterface::localCfgFastCmd.trigger_duration)) usleep (READOUTSLEEP);
        RD53FWInterface::ReadData(pBoard, false, pData, pWait);
        RD53FWInterface::Stop();


        // ##################
        // # Error checking #
        // ##################
        RD53decodedEvents.clear();
        RD53FWInterface::DecodeEvents(pData, status, RD53decodedEvents);
        // RD53FWInterface::PrintEvents(RD53decodedEvents, pData); // @TMP@
        if (RD53FWInterface::EvtErrorHandler(status) == false)
          {
            retry = true;
            continue;
          }

        if (RD53decodedEvents.size() != RD53FWInterface::localCfgFastCmd.n_triggers * (1 + RD53FWInterface::localCfgFastCmd.trigger_duration))
          {
            LOG (ERROR) << BOLDRED << "Sent " << RD53FWInterface::localCfgFastCmd.n_triggers * (1 + RD53FWInterface::localCfgFastCmd.trigger_duration) << " triggers, but collected " << RD53decodedEvents.size() << " events" << BOLDYELLOW << " --> retry" << RESET;
            retry = true;
            continue;
          }

      } while ((retry == true) && (nAttempts < MAXATTEMPTS));

    if (retry == true)
      {
        LOG (ERROR) << BOLDBLUE << "[RD53FWInterface::ReadNEvent] " << BOLDRED << "reached maximum number of attempts (" << BOLDYELLOW << MAXATTEMPTS << BOLDRED << ") without success" << RESET;
        pData.clear();
      }


    // #################
    // # Show progress #
    // #################
    RD53RunProgress::update(pData.size(),true);
  }

  void RD53FWInterface::DecodeEvents (const std::vector<uint32_t>& data, uint16_t& evtStatus, std::vector<RD53FWInterface::Event>& events)
  {
    std::vector<size_t> event_start;
    const size_t maxL1Counter = RD53::setBits(RD53EvtEncoder::NBIT_TRIGID) + 1;

    evtStatus = RD53FWEvtEncoder::GOOD;


    // ######################
    // # Consistency checks #
    // ######################
    if (data.size() == 0)
      {
        evtStatus = RD53FWEvtEncoder::EMPTY;
        return;
      }


    for (auto i = 0u; i < data.size(); i++)
      if (data[i] >> RD53FWEvtEncoder::NBIT_BLOCKSIZE == RD53FWEvtEncoder::EVT_HEADER) event_start.push_back(i);

    events.reserve(events.size() + event_start.size());

    for (auto i = 0u; i < event_start.size(); i++)
      {
        const size_t start = event_start[i];
        const size_t end   = (i == event_start.size() - 1 ? data.size() : event_start[i + 1]);

        events.emplace_back(&data[start], end - start);
        if (events.back().evtStatus != RD53FWEvtEncoder::GOOD) evtStatus |= events.back().evtStatus;
        else
          {
            for (auto j = 0u; j < events.back().chip_events.size(); j++)
              if (events.back().l1a_counter % maxL1Counter != events.back().chip_events[j].trigger_id) evtStatus |= RD53FWEvtEncoder::L1A;
          }
      }
  }

  RD53FWInterface::Event::Event (const uint32_t* data, size_t n)
  {
    evtStatus = RD53FWEvtEncoder::GOOD;


    // ######################
    // # Consistency checks #
    // ######################
    if (n < 4)
      {
        evtStatus = RD53FWEvtEncoder::INCOMPLETE;
        return;
      }

    std::tie(block_size) = bits::unpack<RD53FWEvtEncoder::NBIT_BLOCKSIZE>(data[0]);
    if (block_size * NWORDS_DDR3 != n)
      {
        evtStatus = RD53FWEvtEncoder::EVSIZE;
        return;
      }


    // #########################
    // # Decoding event header #
    // #########################
    bool dummy_size;
    std::tie(tlu_trigger_id, data_format_ver, dummy_size) = bits::unpack<RD53FWEvtEncoder::NBIT_TRIGID, RD53FWEvtEncoder::NBIT_FMTVER, RD53FWEvtEncoder::NBIT_DUMMY>(data[1]);
    std::tie(tdc, l1a_counter) = bits::unpack<RD53FWEvtEncoder::NBIT_TDC, RD53FWEvtEncoder::NBIT_L1ACNT>(data[2]);
    bx_counter = data[3];


    std::vector<size_t> chip_start;
    for (auto i = 4u; i < n - dummy_size * NWORDS_DDR3; i++)
      if (data[i] >> (RD53FWEvtEncoder::NBIT_ERR + RD53FWEvtEncoder::NBIT_HYBRID + RD53FWEvtEncoder::NBIT_FRAMEHEAD + RD53FWEvtEncoder::NBIT_L1ASIZE) == RD53FWEvtEncoder::FRAME_HEADER)
        chip_start.push_back(i);


    // #################################
    // # Decoding frames and chip data #
    // #################################
    chip_frames.reserve(chip_start.size());
    chip_events.reserve(chip_start.size());
    for (auto i = 0u; i < chip_start.size(); i++)
      {
        const size_t start = chip_start[i];
        const size_t end   = (i == chip_start.size() - 1 ? n - dummy_size * NWORDS_DDR3 : chip_start[i + 1]);
        chip_frames.emplace_back(data[start], data[start + 1]);

        if (chip_frames[i].error_code != 0)                                     evtStatus |= RD53FWEvtEncoder::FWERR;
        else if ((chip_frames[i].l1a_data_size * NWORDS_DDR3) != (end - start)) evtStatus |= RD53FWEvtEncoder::FRSIZE;
        else if ((end - start) <= 2)                                            evtStatus |= RD53FWEvtEncoder::MISSCHIP;

        if ((evtStatus & RD53FWEvtEncoder::FWERR)  ||
            (evtStatus & RD53FWEvtEncoder::FRSIZE) ||
            (evtStatus & RD53FWEvtEncoder::MISSCHIP))
          {
            chip_frames.clear();
            chip_events.clear();
            return;
          }

        chip_events.emplace_back(&data[start + 2], chip_frames.back().l1a_data_size * NWORDS_DDR3 - 2);

        if (chip_events[i].evtStatus != RD53EvtEncoder::CHIPGOOD) evtStatus |= chip_events[i].evtStatus;
      }
  }

  RD53FWInterface::ChipFrame::ChipFrame (const uint32_t data0, const uint32_t data1)
  {
    std::tie(error_code, module_id, chip_lane, l1a_data_size) = bits::unpack<RD53FWEvtEncoder::NBIT_ERR, RD53FWEvtEncoder::NBIT_HYBRID, RD53FWEvtEncoder::NBIT_FRAMEHEAD, RD53FWEvtEncoder::NBIT_L1ASIZE>(data0);
    std::tie(chip_type, frame_delay)                          = bits::unpack<RD53FWEvtEncoder::NBIT_CHIPTYPE, RD53FWEvtEncoder::NBIT_DELAY>(data1);
  }

  void RD53FWInterface::SendBoardCommand (const std::string& cmd_reg)
  {
    WriteStackReg({
        {cmd_reg, 1},
        {"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 1},
        {"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 0},
        {cmd_reg, 0}
      });
  }

  void RD53FWInterface::ConfigureFastCommands (const FastCommandsConfig* cfg)
  {
    if (cfg == nullptr) cfg = &(RD53FWInterface::localCfgFastCmd);

    // ##################################
    // # Configuring fast command block #
    // ##################################
    WriteStackReg({
        // ############################
        // # General data for trigger #
        // ############################
        {"user.ctrl_regs.fast_cmd_reg_2.trigger_source",           (uint32_t)cfg->trigger_source},
        {"user.ctrl_regs.fast_cmd_reg_2.backpressure_en",          (uint32_t)cfg->backpressure_en},
        {"user.ctrl_regs.fast_cmd_reg_2.init_ecr_en",              (uint32_t)cfg->initial_ecr_en},
        {"user.ctrl_regs.fast_cmd_reg_2.veto_en",                  (uint32_t)cfg->veto_en},
        {"user.ctrl_regs.fast_cmd_reg_2.ext_trig_delay",           (uint32_t)cfg->ext_trigger_delay},
        {"user.ctrl_regs.fast_cmd_reg_2.trigger_duration",         (uint32_t)cfg->trigger_duration},
        {"user.ctrl_regs.fast_cmd_reg_3.triggers_to_accept",       (uint32_t)cfg->n_triggers},

        // ##############################
        // # Fast command configuration #
        // ##############################
        {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_ecr_en",            (uint32_t)cfg->fast_cmd_fsm.ecr_en},
        {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_test_pulse_en",     (uint32_t)cfg->fast_cmd_fsm.first_cal_en},
        {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_inject_pulse_en",   (uint32_t)cfg->fast_cmd_fsm.second_cal_en},
        {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_trigger_en",        (uint32_t)cfg->fast_cmd_fsm.trigger_en},

        {"user.ctrl_regs.fast_cmd_reg_7.delay_after_ecr",          (uint32_t)cfg->fast_cmd_fsm.delay_after_ecr},
        {"user.ctrl_regs.fast_cmd_reg_4.cal_data_prime",           (uint32_t)cfg->fast_cmd_fsm.first_cal_data},
        {"user.ctrl_regs.fast_cmd_reg_4.delay_after_prime_pulse",  (uint32_t)cfg->fast_cmd_fsm.delay_after_first_cal},
        {"user.ctrl_regs.fast_cmd_reg_5.cal_data_inject",          (uint32_t)cfg->fast_cmd_fsm.second_cal_data},
        {"user.ctrl_regs.fast_cmd_reg_5.delay_after_inject_pulse", (uint32_t)cfg->fast_cmd_fsm.delay_after_second_cal},
        {"user.ctrl_regs.fast_cmd_reg_6.delay_after_autozero",     (uint32_t)cfg->fast_cmd_fsm.delay_after_autozero}, // @TMP@
        {"user.ctrl_regs.fast_cmd_reg_6.delay_before_next_pulse",  (uint32_t)cfg->fast_cmd_fsm.delay_loop},

        // ################################
        // # @TMP@ Autozero configuration #
        // ################################
        {"user.ctrl_regs.fast_cmd_reg_2.autozero_source",2},
        {"user.ctrl_regs.fast_cmd_reg_7.glb_pulse_data", 0},
        {"user.ctrl_regs.fast_cmd_reg_7.autozero_freq",  0},
      });

    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.load_config");

    // #############################
    // # Configuring readout block #
    // #############################
    WriteStackReg({
        {"user.ctrl_regs.readout_block.data_handshake_en", HANDSHAKE_EN},
        {"user.ctrl_regs.readout_block.l1a_timeout_value", L1A_TIMEOUT},
      });
  }

  void RD53FWInterface::SetAndConfigureFastCommands (const BeBoard* pBoard, size_t nTRIGxEvent, size_t injType, uint32_t nClkDelays)
  // ############################
  // # injType == 0 --> None    #
  // # injType == 1 --> Analog  #
  // # injType == 2 --> Digital #
  // ############################
  {
    enum INJtype { None, Analog , Digital };
    enum INJdelay
    {
      FirstCal  = 32,
      SecondCal = 32,
      Loop      = 40
    };

    uint8_t chipId = RD53Constants::BROADCAST_CHIPID;


    // #############################
    // # Configuring FastCmd block #
    // #############################
    RD53FWInterface::localCfgFastCmd.n_triggers       = 0;
    RD53FWInterface::localCfgFastCmd.trigger_duration = nTRIGxEvent - 1;

    if (injType == INJtype::Digital)
      {
        // #######################################
        // # Configuration for digital injection #
        // #######################################
        RD53::CalCmd calcmd_first(1,2,8,0,0);
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.first_cal_data         = calcmd_first.getCalCmd(chipId);
        RD53::CalCmd calcmd_second(0,0,0,0,0);
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.second_cal_data        = calcmd_second.getCalCmd(chipId);

        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_after_first_cal  = INJdelay::FirstCal;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_after_second_cal = 0;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_loop             = (nClkDelays == 0 ? (uint32_t)INJdelay::Loop : nClkDelays);

        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.first_cal_en           = true;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.second_cal_en          = false;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.trigger_en             = true;
      }
    else if (injType == INJtype::Analog)
      {
        // ######################################
        // # Configuration for analog injection #
        // ######################################
        RD53::CalCmd calcmd_first(1,0,0,0,0);
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.first_cal_data         = calcmd_first.getCalCmd(chipId);
        RD53::CalCmd calcmd_second(0,0,2,0,0);
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.second_cal_data        = calcmd_second.getCalCmd(chipId);

        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_after_first_cal  = INJdelay::FirstCal;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_after_second_cal = INJdelay::SecondCal;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_loop             = (nClkDelays == 0 ? (uint32_t)INJdelay::Loop : nClkDelays);

        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.first_cal_en           = true;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.second_cal_en          = true;
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.trigger_en             = true;
      }
    else if (injType == INJtype::None)
      {
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.delay_loop             = (nClkDelays == 0 ? (uint32_t)INJdelay::Loop : nClkDelays);
        RD53FWInterface::localCfgFastCmd.fast_cmd_fsm.trigger_en             = true;
      }
    else LOG (ERROR) << BOLDRED << "Option non recognized " << injType << RESET;


    // ##############################
    // # Download the configuration #
    // ##############################
    RD53FWInterface::ConfigureFastCommands();
    RD53FWInterface::PrintFWstatus();
  }

  void RD53FWInterface::ConfigureDIO5 (const DIO5Config* cfg)
  {
    const uint8_t chnOutEnable   = 0x00;
    const uint8_t fiftyOhmEnable = 0x12;

    if (ReadReg("user.stat_regs.stat_dio5.dio5_not_ready") == true)
      LOG (ERROR) << BOLDRED << "DIO5 not ready" << RESET;

    if (ReadReg("user.stat_regs.stat_dio5.dio5_error") == true)
      LOG (ERROR) << BOLDRED << "DIO5 is in error" << RESET;

    WriteStackReg({
        {"user.ctrl_regs.ext_tlu_reg1.dio5_en",            (uint32_t)cfg->enable},
        {"user.ctrl_regs.ext_tlu_reg1.dio5_ch_out_en",     (uint32_t)chnOutEnable},
        {"user.ctrl_regs.ext_tlu_reg1.dio5_term_50ohm_en", (uint32_t)fiftyOhmEnable},
        {"user.ctrl_regs.ext_tlu_reg1.dio5_ch1_thr",       (uint32_t)cfg->ch1_thr},
        {"user.ctrl_regs.ext_tlu_reg1.dio5_ch2_thr",       (uint32_t)cfg->ch2_thr},
        {"user.ctrl_regs.ext_tlu_reg2.dio5_ch3_thr",       (uint32_t)cfg->ch3_thr},
        {"user.ctrl_regs.ext_tlu_reg2.dio5_ch4_thr",       (uint32_t)cfg->ch4_thr},
        {"user.ctrl_regs.ext_tlu_reg2.dio5_ch5_thr",       (uint32_t)cfg->ch5_thr},
        {"user.ctrl_regs.ext_tlu_reg2.tlu_en",             (uint32_t)cfg->tlu_en},
        {"user.ctrl_regs.ext_tlu_reg2.tlu_handshake_mode", (uint32_t)cfg->tlu_handshake_mode},
        {"user.ctrl_regs.ext_tlu_reg2.ext_clk_en",         (uint32_t)cfg->ext_clk_en},

        {"user.ctrl_regs.ext_tlu_reg2.dio5_load_config",   1},
        {"user.ctrl_regs.ext_tlu_reg2.dio5_load_config",   0}
      });
  }

  // ###########################################
  // # Member functions to handle the firmware #
  // ###########################################
  void RD53FWInterface::FlashProm (const std::string& strConfig, const char* fileName)
  {
    CheckIfUploading();
    fpgaConfig->runUpload(strConfig, fileName);
  }

  void RD53FWInterface::JumpToFpgaConfig (const std::string& strConfig)
  {
    CheckIfUploading();
    fpgaConfig->jumpToImage(strConfig);
  }

  void RD53FWInterface::DownloadFpgaConfig (const std::string& strConfig, const std::string& strDest)
  {
    CheckIfUploading();
    fpgaConfig->runDownload(strConfig, strDest.c_str());
  }

  std::vector<std::string> RD53FWInterface::getFpgaConfigList ()
  {
    CheckIfUploading();
    return fpgaConfig->getFirmwareImageNames();
  }

  void RD53FWInterface::DeleteFpgaConfig (const std::string& strId)
  {
    CheckIfUploading();
    fpgaConfig->deleteFirmwareImage(strId);
  }

  void RD53FWInterface::CheckIfUploading ()
  {
    if (fpgaConfig && fpgaConfig->getUploadingFpga() > 0)
      throw Exception ("This board is uploading an FPGA configuration");

    if (!fpgaConfig) fpgaConfig = new D19cFpgaConfig(this);
  }

  void RD53FWInterface::RebootBoard ()
  {
    if (!fpgaConfig) fpgaConfig = new D19cFpgaConfig(this);
    fpgaConfig->resetBoard();
  }

  const FpgaConfig* RD53FWInterface::GetConfiguringFpga ()
  {
    return (const FpgaConfig*) fpgaConfig;
  }


  // ########################################
  // # Vector containing the decoded events #
  // ########################################
  std::vector<RD53FWInterface::Event> RD53decodedEvents;


  // ################################################
  // # I2C block for programming peripheral devices #
  // ################################################
  bool RD53FWInterface::I2cCmdAckWait (unsigned int trials)
  {
    const uint16_t I2CcmdAckGOOD = 0x01;
    uint16_t status = 0x02; // 0x02 = I2CcmdAckBAD
    uint16_t cLoop  = 0;

    while (++cLoop < trials)
      {
        status = ReadReg ("user.stat_regs.global_reg.i2c_acq_err");
        if (status == I2CcmdAckGOOD) return true;
        usleep(DEEPSLEEP);
      }

    return false;
  }

  void RD53FWInterface::WriteI2C (std::vector<uint32_t>& data)
  {
    const uint16_t I2CwriteREQ = 0x01;

    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_reset",1);
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_reset",0);
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_fifo_rx_dsel",1);
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_req",I2CwriteREQ);
    usleep(DEEPSLEEP);

    /* bool outcome = */ RegManager::WriteBlockReg ("CTRL.BOARD.i2c_fifo_tx", data);
    usleep(DEEPSLEEP);

    if (I2cCmdAckWait (20) == false)
      throw Exception ("[RD53FWInterface::WriteI2C] I2C transaction error");

    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
    usleep(DEEPSLEEP);
  }

  void RD53FWInterface::ReadI2C (std::vector<uint32_t>& data)
  {
    const uint16_t I2CreadREQ = 0x03;

    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_reset",1);
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_reset",0);
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_fifo_rx_dsel",1);
    usleep(DEEPSLEEP);
    WriteReg ("CTRL.BOARD.i2c_req",I2CreadREQ);
    usleep(DEEPSLEEP);

    uint32_t sizeI2Cfifo = ReadReg("STAT.BOARD.i2c_fifo_rx_dcnt");
    usleep(DEEPSLEEP);

    int size2read = 0;
    if (sizeI2Cfifo > data.size())
      {
        size2read = data.size();
        LOG (WARNING) << BOLDRED << "[RD53FWInterface::ReadI2C] I2C FIFO contains more data than the vector size" << RESET;
      }
    else size2read = sizeI2Cfifo;

    data = ReadBlockRegValue ("CTRL.BOARD.i2c_fifo_rx", size2read);
    usleep(DEEPSLEEP);

    if (RD53FWInterface::I2cCmdAckWait(20) == false)
      throw Exception ("[RD53FWInterface::ReadI2C] I2C transaction error");

    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
  }

  void RD53FWInterface::ConfigureClockSi5324 ()
  {
    // ###################################################
    // # The Si5324 chip generates the clock for the GTX #
    // ###################################################

    uint8_t start_wr       = 0x90;
    uint8_t stop_wr        = 0x50;
    // uint8_t stop_rd_nack   = 0x68;
    // uint8_t rd_incr        = 0x20;
    uint8_t wr_incr        = 0x10;

    uint8_t enable_i2cmux  = 1;
    uint8_t disable_i2cmux = 0;

    uint8_t i2cmux_addr_wr = 0xe8;
    // uint8_t i2cmux_addr_rd = 0xe9;

    uint8_t si5324_pos     = 7;
    uint8_t si5324_addr_wr = 0xd0;
    // uint8_t si5324_addr_rd = 0xd1;

    uint32_t word;
    std::vector<uint32_t> data;


    // #############
    // # Frequency #
    // #############
    const int N1_HS  =   0;
    const int NC1_LS =  19;
    const int N2_HS  =   1;
    const int N2_LS  = 511;
    const int N32    =  31;


    // ############################################
    // # Program Si5324 for 160 MHz precise clock #
    // ############################################
    std::vector< std::pair<uint8_t,uint8_t> > si5324Program;
    si5324Program.push_back({0x00,0x54}); // Free running mode = 1, CKOUT_ALWAYS_ON = 0
    si5324Program.push_back({0x0B,0x41}); // Disable CLKIN1
    si5324Program.push_back({0x06,0x0F}); // Disable CKOUT2 (SFOUT2_REG=001), set CKOUT1 to LVDS (SFOUT1_REG=111)
    si5324Program.push_back({0x15,0xFE}); // CKSEL_PIN = 0
    si5324Program.push_back({0x03,0x55}); // CKIN2 selected, SQ_ICAL = 1

    si5324Program.push_back({0x02,0x22});

    si5324Program.push_back({0x19, N1_HS << 5});
    si5324Program.push_back({0x1F, NC1_LS >> 16});
    si5324Program.push_back({0x20, NC1_LS >> 8});
    si5324Program.push_back({0x21, NC1_LS});
    si5324Program.push_back({0x28,(N2_HS << 5) | (N2_LS >> 16)});
    si5324Program.push_back({0x29, N2_LS >> 8});
    si5324Program.push_back({0x2A, N2_LS});
    si5324Program.push_back({0x2E, N32 >> 16});
    si5324Program.push_back({0x2F, N32 >> 8});
    si5324Program.push_back({0x30, N32});

    si5324Program.push_back({0x89,0x01}); // FASTLOCK = 1
    si5324Program.push_back({0x88,0x40}); // ICAL = 1
    // ###########################################

    word = (i2cmux_addr_wr << 8) | start_wr;
    data.push_back(word);
    word = (enable_i2cmux << si5324_pos) << 8 | stop_wr;
    data.push_back(word);

    for (auto i = 0u; i < si5324Program.size(); i++)
      {
        word = (si5324_addr_wr << 8) | start_wr;
        data.push_back(word);
        word = (si5324Program[i].first << 8) | wr_incr;
        data.push_back(word);
        word = (si5324Program[i].second << 8) | stop_wr;
        data.push_back(word);
      }

    word = (i2cmux_addr_wr << 8) | start_wr;
    data.push_back(word);
    word = (disable_i2cmux << si5324_pos) << 8 | stop_wr;
    data.push_back(word);

    RD53FWInterface::WriteI2C(data);
  }
}
