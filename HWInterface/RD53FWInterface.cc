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
  RD53FWInterface::RD53FWInterface (const char* pId, const char* pUri, const char* pAddressTable) :
    BeBoardFWInterface (pId, pUri, pAddressTable) {}

  void RD53FWInterface::setFileHandler (FileHandler* pHandler)
  {
    if (pHandler != nullptr)
      {
	fFileHandler = pHandler;
	fSaveToFile  = true;
      }
    else LOG (ERROR) << BOLDRED << "NULL FileHandler" << RESET;
  }

  uint32_t RD53FWInterface::getBoardInfo()
  {
    uint32_t cVersionMajor = ReadReg ("user.stat_regs.usr_ver.usr_ver_major");
    uint32_t cVersionMinor = ReadReg ("user.stat_regs.usr_ver.usr_ver_minor");
    uint32_t cVersionBuild = ReadReg ("user.stat_regs.usr_ver.usr_ver_build");

    uint32_t cFWyear       = ReadReg ("user.stat_regs.usr_ver.usr_firmware_yy");
    uint32_t cFWmonth      = ReadReg ("user.stat_regs.usr_ver.usr_firmware_mm");
    uint32_t cFWday        = ReadReg ("user.stat_regs.usr_ver.usr_firmware_dd");

    LOG (INFO) << BOLDBLUE << "FW version : " << BOLDYELLOW << cVersionMajor << "." << cVersionMinor
	       << BOLDBLUE << " -- Build version : " << BOLDYELLOW << cVersionBuild
	       << BOLDBLUE << " -- Firmware date (yyyy/mm/dd) : " << BOLDYELLOW << cFWyear << "/" << cFWmonth << "/" << cFWday << RESET;

    uint32_t cVersionWord = ((cVersionMajor << NBIT_FWVER) | cVersionMinor);
    return cVersionWord;
  }

  void RD53FWInterface::ConfigureBoard (const BeBoard* pBoard)
  {
    // @TMP@
    // this->TurnOffFMC();
    // this->TurnOnFMC();
    // this->ResetBoard();
    this->ResetFastCmdBlk();
    this->ResetReadoutBlk();
    this->ConfigureFastCommands();
    this->ChipReset();
    this->ChipReSync();

    // Wait for user to reset power to the chip
    // LOG (INFO) << BOLDMAGENTA << "Powercycle SCC and press any key to continue: " << RESET;
    // system("read");

    std::vector< std::pair<std::string, uint32_t> > cVecReg;

    BeBoardRegMap cRD53FWRegMap = pBoard->getBeBoardRegMap();
    LOG (INFO) << GREEN << "Initializing board's registers:" << RESET;

    for (const auto& it : cRD53FWRegMap)
      {
	LOG (INFO) << BOLDBLUE << "\t--> " << it.first << " = " << BOLDYELLOW << it.second << RESET;
	cVecReg.push_back ({it.first, it.second});
      }
    if (cVecReg.size() != 0) WriteStackReg (cVecReg);

    this->PrintFWstatus();
  }

  void RD53FWInterface::WriteChipCommand (std::vector<uint32_t>& data, unsigned int nCmd)
  {
    std::vector< std::pair<std::string, uint32_t> > stackRegisters;

    if (ReadReg ("user.stat_regs.cmd_proc.fifo_empty") == false)
      LOG (ERROR) << BOLDRED << "Command processor FIFO NOT empty before sending new commands" << RESET;

    if (ReadReg ("user.stat_regs.cmd_proc.fifo_full") == true)
      LOG (ERROR) << BOLDRED << "Command processor FIFO full" << RESET;

    size_t size = data.size()/nCmd;
    for (auto i = 0; i < nCmd; i++)
      {
	switch (size)
	  {
	  case 1:
	    {
	      stackRegisters.push_back({"user.cmd_regs.ctrl_reg", data[size*i+0]});
	      break;
	    }
	  case 2:
	    {
	      stackRegisters.push_back({"user.cmd_regs.ctrl_reg",  data[size*i+0]});
	      stackRegisters.push_back({"user.cmd_regs.data0_reg", data[size*i+1]});
	      break;
	    }
	  case 3:
	    {
	      stackRegisters.push_back({"user.cmd_regs.ctrl_reg",  data[size*i+0]});
	      stackRegisters.push_back({"user.cmd_regs.data0_reg", data[size*i+1]});
	      stackRegisters.push_back({"user.cmd_regs.data1_reg", data[size*i+2]});
	      break;
	    }
	  case 4:
	    {
	      stackRegisters.push_back({"user.cmd_regs.ctrl_reg",  data[size*i+0]});
	      stackRegisters.push_back({"user.cmd_regs.data0_reg", data[size*i+1]});
	      stackRegisters.push_back({"user.cmd_regs.data1_reg", data[size*i+2]});
	      stackRegisters.push_back({"user.cmd_regs.data2_reg", data[size*i+3]});
	      break;
	    }
	  }

	if (nCmd != 1) stackRegisters.push_back({"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 0}); // @TMP@
      }

    WriteStackReg (stackRegisters);
  }

  std::pair< std::vector<uint16_t>,std::vector<uint16_t> > RD53FWInterface::ReadChipRegisters (std::vector<uint32_t>& data, unsigned int nBlocks2Read)
  {
    // ##############################
    // # Filter readback data:      #
    // # 0: read "cmd" only         #
    // # 1: read "auto" 2nd only    #
    // # 2: read "auto" 1st only    #
    // # 3: read "auto" 1st and 2nd #
    // ##############################
    const unsigned int filter = 0;

    std::stringstream myString;
    unsigned int nodeBlocks  = fBoard->getNode("user.readout0.reg_read").getSize();
    unsigned int nActiveChns = ReadReg ("user.stat_regs.aurora.n_ch");
    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > outputDecoded;
    std::vector<uint32_t> regFIFO;

    for (auto i = 0; i < nActiveChns; i++)
      {
	myString.clear(); myString.str("");
	myString << "user.readout" << i << ".reg_mask";
	WriteReg (myString.str().c_str(), filter);

	myString.clear(); myString.str("");
	myString << "user.readout" << i << ".sel";
	WriteReg (myString.str().c_str(), 2);

	// ##################
	// # Flush the FIFO #
	// ##################
	myString.clear(); myString.str("");
	myString << "user.readout" << i << ".reg_read";
	ReadBlockRegValue(myString.str().c_str(), nodeBlocks);
      }

    this->WriteChipCommand(data);

    for (auto i = 0; i < nActiveChns; i++)
      {
	myString.clear(); myString.str("");
	myString << "user.readout" << i << ".reg_read";

	if (nBlocks2Read <= nodeBlocks) regFIFO = ReadBlockRegValue(myString.str().c_str(), nBlocks2Read);
	else LOG (ERROR) << BOLDRED << "Number of register blocks to read (" << BOLDYELLOW << nBlocks2Read << BOLDRED << ") exceds FIFO lenght " << BOLDYELLOW << nodeBlocks << RESET;

	for (auto i = 0; i < regFIFO.size(); i++)
	  {
	    outputDecoded.first .push_back((regFIFO[i] >> RD53RegFrameEncoder::NBIT_VALUE)                                                            & static_cast<uint32_t>(RD53::SetBits<RD53RegFrameEncoder::NBIT_ADDRESS>(RD53RegFrameEncoder::NBIT_ADDRESS).to_ulong()));
	    outputDecoded.second.push_back(regFIFO[i]                                                                                                 & static_cast<uint32_t>(RD53::SetBits<RD53RegFrameEncoder::NBIT_VALUE>(RD53RegFrameEncoder::NBIT_VALUE).to_ulong()));
	    uint8_t status = (regFIFO[i] >> (RD53RegFrameEncoder::NBIT_VALUE + RD53RegFrameEncoder::NBIT_ADDRESS))                                    & static_cast<uint32_t>(RD53::SetBits<RD53RegFrameEncoder::NBIT_STATUS>(RD53RegFrameEncoder::NBIT_STATUS).to_ulong());
	    uint8_t id     = (regFIFO[i] >> (RD53RegFrameEncoder::NBIT_VALUE + RD53RegFrameEncoder::NBIT_ADDRESS + RD53RegFrameEncoder::NBIT_STATUS)) & static_cast<uint32_t>(RD53::SetBits<RD53RegFrameEncoder::NBIT_CHIPID>(RD53RegFrameEncoder::NBIT_CHIPID).to_ulong());

	    if (status != 0) LOG (ERROR) << BOLDRED << "Status error in chip register readback: " << BOLDYELLOW << std::hex << +status << std::dec << BOLDRED << " from chip ID: " << BOLDYELLOW << std::hex << +id << std::dec << RESET;
	  }
      }

    return outputDecoded;
  }

  void RD53FWInterface::PrintFWstatus()
  {
    LOG (INFO) << GREEN << "Checking Firmware status" << RESET;


    // #################################
    // # Check clock generator locking #
    // #################################
    if (ReadReg ("user.stat_regs.global_reg.clk_gen_lock") == 1)
      LOG (INFO) << BOLDGREEN << "\t--> Clock generator is locked" << RESET;
    else
      LOG (ERROR) << BOLDRED << "\t--> Clock generator is not locked" << RESET;


    // ############################
    // # Check I2C initialization #
    // ############################
    if (ReadReg ("user.stat_regs.global_reg.i2c_init") == 1)
      LOG (INFO) << BOLDGREEN << "\t--> I2C initialized" << RESET;
    else
      {
	LOG (ERROR) << BOLDRED << "I2C not initialized" << RESET;
	unsigned int status = ReadReg ("user.stat_regs.global_reg.i2c_init_err");
	LOG (ERROR) << BOLDRED << "\t--> I2C initialization status: " << BOLDYELLOW << status << RESET;
      }

    if (ReadReg ("user.stat_regs.global_reg.i2c_aqu_err") == 1)
      LOG (INFO) << BOLDGREEN << "I2C ack error during analog readout (for KSU FMC only)" << RESET;


    // ############################################################
    // # Check status registers associated wih fast command block #
    // ############################################################
    unsigned int fastCMDReg = ReadReg ("user.stat_regs.fast_cmd_1.trigger_source_o");
    LOG (INFO) << BOLDBLUE << "Fast CMD block trigger source: " << BOLDYELLOW << fastCMDReg << BOLDBLUE << " (1=IPBus, 2=Test-FSM, 3=TTC, 4=TLU, 5=External, 6=Hit-Or, 7=User-defined frequency)" << RESET;

    fastCMDReg = ReadReg ("user.stat_regs.fast_cmd_1.trigger_state");
    LOG (INFO) << BOLDBLUE << "Fast CMD block trigger state: " << BOLDYELLOW << fastCMDReg << RESET;

    fastCMDReg = ReadReg ("user.stat_regs.fast_cmd_1.if_configured");
    LOG (INFO) << BOLDBLUE << "Fast CMD block check if configuraiton registers have been set: " << BOLDYELLOW << fastCMDReg << RESET;

    fastCMDReg = ReadReg ("user.stat_regs.fast_cmd_1.error_code");
    LOG (INFO) << BOLDBLUE << "Fast CMD block error code (0 = no error): " << BOLDYELLOW << fastCMDReg << RESET;


    // ###########################
    // # Check trigger registers #
    // ###########################
    unsigned int trigReg = ReadReg ("user.stat_regs.trigger_cntr");
    LOG (INFO) << BOLDBLUE << "Trigger counter: " << BOLDYELLOW << trigReg << RESET;

    trigReg = ReadReg ("user.stat_regs.trigger_tag");
    LOG (INFO) << BOLDBLUE << "Trigger tag: " << BOLDYELLOW << trigReg << RESET;
    
    
    // ##############
    // # Clock rate #
    // ##############
    unsigned int clkRate = ReadReg ("user.stat_regs.clk_rate_1");
    LOG (INFO) << BOLDBLUE << "Clock rate 1: " << BOLDYELLOW << (double)clkRate/1000 << " MHz" << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_2");
    LOG (INFO) << BOLDBLUE << "Clock rate 2: " << BOLDYELLOW << (double)clkRate/1000 << " MHz" << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_3");
    LOG (INFO) << BOLDBLUE << "Clock rate 3: " << BOLDYELLOW << (double)clkRate/1000 << " MHz" << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_4");
    LOG (INFO) << BOLDBLUE << "Clock rate 4: " << BOLDYELLOW << (double)clkRate/1000 << " MHz" << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_5");
    LOG (INFO) << BOLDBLUE << "Clock rate 5: " << BOLDYELLOW << (double)clkRate/1000 << " MHz" << RESET;
  }

  bool RD53FWInterface::InitChipCommunication()
  {
    // ###############################
    // # Check RD53 AURORA registers #
    // ###############################
    unsigned int auroraReg = ReadReg ("user.stat_regs.aurora.gtx_lock");
    LOG (INFO) << BOLDBLUE << "Aurora PLL locking status: " << BOLDYELLOW << auroraReg << RESET;

    auroraReg = ReadReg ("user.stat_regs.aurora.speed");
    LOG (INFO) << BOLDBLUE << "Aurora speed: " << BOLDYELLOW << (auroraReg == 0 ? "1.28 Gbps" : "640 Mbps") << RESET;

    auroraReg = ReadReg ("user.stat_regs.aurora.n_ch");
    LOG (INFO) << BOLDBLUE << "Aurora number of channels: " << BOLDYELLOW << auroraReg << RESET;

    std::bitset<NBIT_AURORAREG> bitReg = static_cast<uint8_t>(ReadReg ("user.stat_regs.aurora.lane_up"));
    LOG (INFO) << BOLDBLUE << "Aurora lane up status: " << BOLDYELLOW << bitReg.count() << RESET;

    bitReg = static_cast<uint8_t>(ReadReg ("user.stat_regs.aurora.channel_up"));
    if (bitReg.count() == auroraReg)
      {
	LOG (INFO) << BOLDGREEN << "\t--> Aurora channels up number as expected: " << BOLDYELLOW << bitReg.count() << RESET;
	return true;
      }
    LOG (ERROR) << BOLDRED << "\t--> Aurora channels up number less than expected: " << BOLDYELLOW << bitReg.count() << RESET;
    return false;
  }

  void RD53FWInterface::Start()
  {
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

  uint32_t RD53FWInterface::ReadData (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
  {
    uint32_t cNWords    = ReadReg("user.stat_regs.words_to_read").value();
    uint32_t handshake  = ReadReg("user.ctrl_regs.readout_block.data_handshake_en").value();
    uint32_t cNtriggers = ReadReg("user.stat_regs.trigger_cntr").value();

    LOG (INFO) << GREEN << "--- Reading DDR3 data ---" << RESET;
    LOG (INFO) << GREEN << "n. words        = "        << cNWords    << RESET;
    LOG (INFO) << GREEN << "n. triggers     = "        << cNtriggers << RESET;
    LOG (INFO) << CYAN  << "=========================" << RESET;

    if (!cNWords) return 0;

    while (cNWords == 0)
      {
        usleep(DEEPSLEEP);
        cNWords = ReadReg("user.stat_regs.words_to_read").value();
      }
    
    if (handshake == true)
      {
        uint32_t cReadoutReq = ReadReg("user.stat_regs.readout4.readout_req").value();
        while (cReadoutReq == 0)
	  {
            uint32_t fsm_status = ReadReg("user.stat_regs.readout4.fsm_status").value();
            LOG (ERROR) << BOLDRED << "Waiting for readout request, FSM status: " << BOLDYELLOW << fsm_status << RESET;
            
            usleep(DEEPSLEEP);
            
            cReadoutReq = ReadReg("user.stat_regs.readout4.readout_req");
	  }

        cNWords    = ReadReg("user.stat_regs.words_to_read").value();
        cNtriggers = ReadReg("user.stat_regs.trigger_cntr").value();
      }

    uhal::ValVector<uint32_t> values = ReadBlockReg("ddr3.fc7_daq_ddr3", cNWords);
    for (const auto& val : values) pData.push_back(val);

    if (fSaveToFile == true) fFileHandler->set(pData);

    return pData.size();
  }

  void RD53FWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
  {
    uint8_t     status;
    bool        retry;
    
    this->localCfgFastCmd.n_triggers = pNEvents;
    this->ConfigureFastCommands();

    do
      {
	retry = false;
	pData.clear();

	this->ResetReadoutBlk();
	this->ChipReset();
	this->ChipReSync();

	this->Start();
	usleep((this->localCfgFastCmd.fast_cmd_fsm.delay_after_ecr        +
		this->localCfgFastCmd.fast_cmd_fsm.delay_after_autozero   +
		this->localCfgFastCmd.fast_cmd_fsm.delay_after_first_cal  +
		this->localCfgFastCmd.fast_cmd_fsm.delay_after_second_cal +
		this->localCfgFastCmd.fast_cmd_fsm.delay_loop) * DELAYPERIOD *
	       this->localCfgFastCmd.n_triggers + SHALLOWSLEEP);

	this->ReadData(pBoard, false, pData);
	this->Stop();


	// ##################
	// # Error checking #
	// ##################
	if (pData.size() == 0)
	  {
	    LOG (ERROR) << BOLDRED << "Sent " << this->localCfgFastCmd.n_triggers << " triggers, but no data collected " << BOLDYELLOW << "--> retry" << RESET;
	    retry = true;
	    continue;
	  }

	auto events = this->DecodeEvents(pData, status);
	if (this->EvtErrorHandler(status) == false)
	  {
	    retry = true;
	    continue;
	  }

	if (events.size() != localCfgFastCmd.n_triggers * (1 + localCfgFastCmd.trigger_duration))
	  {
	    LOG (ERROR) << BOLDRED << "Sent " << localCfgFastCmd.n_triggers * (1 + localCfgFastCmd.trigger_duration) << " triggers, but collected " << events.size() << " events" << BOLDYELLOW << " --> retry" << RESET;
	    retry = true;
	    continue;
	  }


      } while (retry == true);
  }

  std::vector<uint32_t> RD53FWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize)
  {
    uhal::ValVector<uint32_t> valBlock = RegManager::ReadBlockReg (pRegNode, pBlocksize);
    return valBlock.value();
  }

  void RD53FWInterface::TurnOffFMC()
  {
    WriteStackReg({{"system.ctrl_2.fmc_pg_c2m",    0},
	  {"system.ctrl_2.fmc_l8_pwr_en", 0},
	  {"system.ctrl_2.fmc_l12_pwr_en",0}});
  }

  void RD53FWInterface::TurnOnFMC()
  {
    WriteStackReg({{"system.ctrl_2.fmc_l12_pwr_en",1},
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
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.aurora_pma_rst",0);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.global_rst",1);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.clk_gen_rst",1);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.fmc_pll_rst",0);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",1);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.i2c_rst",1);
    usleep(DEEPSLEEP);


    // #########
    // # Reset #
    // #########
    WriteReg ("user.ctrl_regs.reset_reg.global_rst",0);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",0);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.fmc_pll_rst",1);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",0);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.i2c_rst",0);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.aurora_pma_rst",1);
    usleep(DEEPSLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.aurora_rst",1);
    usleep(DEEPSLEEP);


    // ########
    // # DDR3 #
    // ########
    while (!ReadReg("user.stat_regs.readout1.ddr3_initial_calibration_done").value())
      {
        LOG (INFO) << YELLOW << "Waiting for DDR3 calibration" << RESET;
        usleep(DEEPSLEEP);
      }
    
    LOG (INFO) << BOLDGREEN << "\t--> DDR3 calibration done" << RESET;
  }
  
  void RD53FWInterface::ResetFastCmdBlk()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.ipb_reset"); // Resets the fast command block --> which should then be reprogrammed
    
    WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_fast_duration",IPBFASTDURATION);
  }

  void RD53FWInterface::ResetReadoutBlk()
  {
    WriteStackReg({
	{"user.ctrl_regs.reset_reg.readout_block_rst",1},
	{"user.ctrl_regs.reset_reg.readout_block_rst",0}}); // Resets the readout block --> which should then be reprogrammed
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

  std::vector<RD53FWInterface::Event> RD53FWInterface::DecodeEvents (const std::vector<uint32_t>& data, uint8_t& evtStatus)
  {
    std::vector<size_t> event_start;
    size_t maxL1Counter = RD53::SetBits<RD53EvtEncoder::NBIT_TRIGID>(RD53EvtEncoder::NBIT_TRIGID).to_ulong() + 1;

    if (data.size() != 0) evtStatus = RD53FWEvtEncoder::GOOD;
    else                  evtStatus = RD53FWEvtEncoder::EMPTY;

    for (auto i = 0; i < data.size(); i++)
      if (data[i] >> RD53FWEvtEncoder::NBIT_BLOCKSIZE == RD53FWEvtEncoder::EVT_HEADER) event_start.push_back(i);

    std::vector<RD53FWInterface::Event> events;
    events.reserve(event_start.size());
    
    for (auto i = 0; i < event_start.size(); i++)
      {
	const size_t start = event_start[i];
	const size_t end   = ((i == event_start.size() - 1) ? data.size() : event_start[i + 1]);

	RD53FWInterface::Event evt(&data[start], end - start);
	events.push_back(evt);

	if (evt.evtStatus != RD53FWEvtEncoder::GOOD) evtStatus |= evt.evtStatus;
	else
	  {
	    for (auto j = 0; j < evt.chip_events.size(); j++)
	      if (evt.l1a_counter % maxL1Counter != evt.chip_events[j].trigger_id) evtStatus |= RD53FWEvtEncoder::L1A;
	  }
      }

    return events;
  }

  void RD53FWInterface::PrintEvents (const std::vector<RD53FWInterface::Event>& events)
  {
    for (auto i = 0; i < events.size(); i++)
      {
	auto& evt = events[i];
	LOG (INFO) << BOLDGREEN << "Event           = " << i                   << RESET;
	LOG (INFO) << BOLDGREEN << "block_size      = " << evt.block_size      << RESET;
	LOG (INFO) << BOLDGREEN << "trigger_id      = " << evt.tlu_trigger_id  << RESET;
	LOG (INFO) << BOLDGREEN << "data_format_ver = " << evt.data_format_ver << RESET;
	LOG (INFO) << BOLDGREEN << "tdc             = " << evt.tdc             << RESET;
	LOG (INFO) << BOLDGREEN << "l1a_counter     = " << evt.l1a_counter     << RESET;
	LOG (INFO) << BOLDGREEN << "bx_counter      = " << evt.bx_counter      << RESET;

	for (auto j = 0; j < evt.chip_events.size(); j++)
	  {
	    LOG (INFO) << CYAN << "Chip Header:"                                           << RESET;
	    LOG (INFO) << CYAN << "error_code      = " << evt.chip_frames[j].error_code    << RESET;
	    LOG (INFO) << CYAN << "hybrid_id       = " << evt.chip_frames[j].hybrid_id     << RESET;
	    LOG (INFO) << CYAN << "chip_id         = " << evt.chip_frames[j].chip_id       << RESET;
	    LOG (INFO) << CYAN << "l1a_data_size   = " << evt.chip_frames[j].l1a_data_size << RESET;
	    LOG (INFO) << CYAN << "chip_type       = " << evt.chip_frames[j].chip_type     << RESET;
	    LOG (INFO) << CYAN << "frame_delay     = " << evt.chip_frames[j].frame_delay   << RESET;
	      
	    LOG (INFO) << CYAN << "trigger_id      = " << evt.chip_events[j].trigger_id    << RESET;
	    LOG (INFO) << CYAN << "trigger_tag     = " << evt.chip_events[j].trigger_tag   << RESET;
	    LOG (INFO) << CYAN << "bc_id           = " << evt.chip_events[j].bc_id         << RESET;
	      
	    LOG (INFO) << BOLDYELLOW << "Region Data (" << evt.chip_events[j].data.size() << " words): " << RESET;

	    for (const auto& region_data : evt.chip_events[j].data)
	      {
		LOG(INFO)   << "Column: " << region_data.col 
			    << ", Row: "  << region_data.row 
			    << ", ToTs: [" << +region_data.tots[0] << "," << +region_data.tots[1] << "," << +region_data.tots[2] << "," << +region_data.tots[3] << "]"
			    << RESET;
	      }
	  }
      }

    std::cout << std::endl;
  }
  
  bool RD53FWInterface::EvtErrorHandler(uint8_t status)
  {
    bool isGood = true;

    if (status & RD53FWEvtEncoder::EVSIZE)
      {
	LOG (ERROR) << BOLDRED << "Invalid event size " << BOLDYELLOW << "--> retry" << RESET;
	isGood = false;
      }

    if (status & RD53FWEvtEncoder::EMPTY)
      {
	LOG (ERROR) << BOLDRED << "No data collected " << BOLDYELLOW << "--> retry" << RESET;
	isGood = false;
      }

    if (status & RD53FWEvtEncoder::L1A)
      {
	LOG (ERROR) << BOLDRED << "L1A counter mismatch " << BOLDYELLOW << "--> retry" << RESET;
	isGood = false;
      }

    if (status & RD53FWEvtEncoder::FRSIZE)
      {
	LOG (ERROR) << BOLDRED << "Invalid frame size " << BOLDYELLOW << "--> retry" << RESET;
	isGood = false;
      }

    if (status & RD53EvtEncoder::CHEAD)
      {
	LOG (ERROR) << BOLDRED << "Bad chip header " << BOLDYELLOW << "--> retry" << RESET;
	isGood = false;
      }

    if (status & RD53EvtEncoder::CPIX)
      {
	LOG (ERROR) << BOLDRED << "Bad pixel row or column " << BOLDYELLOW << "--> retry" << RESET;
	isGood = false;
      }

    return isGood;
  }

  RD53FWInterface::Event::Event (const uint32_t* data, size_t n)
  {
    evtStatus = RD53FWEvtEncoder::GOOD;

    std::tie(block_size) = unpack_bits<RD53FWEvtEncoder::NBIT_BLOCKSIZE>(data[0]);    
    if (block_size * 4 != n) evtStatus |= RD53FWEvtEncoder::EVSIZE;

    bool dummy_size;
    std::tie(tlu_trigger_id, data_format_ver, dummy_size) = unpack_bits<RD53FWEvtEncoder::NBIT_TRIGID, RD53FWEvtEncoder::NBIT_FMTVER, RD53FWEvtEncoder::NBIT_DUMMY>(data[1]);
    std::tie(tdc, l1a_counter) = unpack_bits<RD53FWEvtEncoder::NBIT_TDC, RD53FWEvtEncoder::NBIT_L1ACNT>(data[2]);
    bx_counter = data[3];

    std::vector<size_t> chip_start;
    for (auto i = 4; i < n; i += 4) if (data[i] >> (RD53FWEvtEncoder::NBIT_ERR + RD53FWEvtEncoder::NBIT_HYBRID + RD53FWEvtEncoder::NBIT_FRAMEHEAD + RD53FWEvtEncoder::NBIT_L1ASIZE) == RD53FWEvtEncoder::FRAME_HEADER) chip_start.push_back(i);

    chip_frames.reserve(chip_start.size());
    chip_events.reserve(chip_start.size());
    for (auto i = 0; i < chip_start.size(); i++)
      {
	const size_t start = chip_start[i];
	const size_t end   = ((i == chip_start.size() - 1) ? n : chip_start[i + 1]);
	chip_frames.emplace_back(data[start], data[start + 1]);

 	if ((chip_frames[i].l1a_data_size+dummy_size) * 4 != (end - start))
	  {
	    evtStatus |= RD53FWEvtEncoder::FRSIZE;
	    chip_frames.clear();
	    chip_events.clear();
	    return;
	  }

	const size_t size = (dummy_size ? chip_frames.back().l1a_data_size * 4 : end - start);
	chip_events.emplace_back(&data[start + 2], size - 2);

	if (chip_events[i].evtStatus != RD53EvtEncoder::CGOOD) evtStatus |= chip_events[i].evtStatus;
      }
  }

  RD53FWInterface::ChipFrame::ChipFrame (const uint32_t data0, const uint32_t data1)
  {
    std::tie(error_code, hybrid_id, chip_id, l1a_data_size) = unpack_bits<RD53FWEvtEncoder::NBIT_ERR, RD53FWEvtEncoder::NBIT_HYBRID, RD53FWEvtEncoder::NBIT_FRAMEHEAD, RD53FWEvtEncoder::NBIT_L1ASIZE>(data0);    
    std::tie(chip_type, frame_delay)                        = unpack_bits<RD53FWEvtEncoder::NBIT_CHIPTYPE, RD53FWEvtEncoder::NBIT_DELAY>(data1);
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
    if (cfg == nullptr) cfg = &localCfgFastCmd;

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

	{"user.ctrl_regs.fast_cmd_reg_3.delay_after_ecr",          (uint32_t)cfg->fast_cmd_fsm.delay_after_ecr},
	{"user.ctrl_regs.fast_cmd_reg_4.cal_data_prime",           (uint32_t)cfg->fast_cmd_fsm.first_cal_data},
	{"user.ctrl_regs.fast_cmd_reg_4.delay_after_prime_pulse",  (uint32_t)cfg->fast_cmd_fsm.delay_after_first_cal},
	{"user.ctrl_regs.fast_cmd_reg_5.cal_data_inject",          (uint32_t)cfg->fast_cmd_fsm.second_cal_data},
	{"user.ctrl_regs.fast_cmd_reg_5.delay_after_inject_pulse", (uint32_t)cfg->fast_cmd_fsm.delay_after_second_cal},
	{"user.ctrl_regs.fast_cmd_reg_6.delay_after_autozero",     (uint32_t)cfg->fast_cmd_fsm.delay_after_autozero},
	{"user.ctrl_regs.fast_cmd_reg_6.delay_before_next_pulse",  (uint32_t)cfg->fast_cmd_fsm.delay_loop},

	// ##########################
	// # Autozero configuration #
	// ##########################
	{"user.ctrl_regs.fast_cmd_reg_2.autozero_source",          (uint32_t)cfg->autozero.autozero_source},
	{"user.ctrl_regs.fast_cmd_reg_7.glb_pulse_data",           (uint32_t)cfg->autozero.glb_pulse_data},
	{"user.ctrl_regs.fast_cmd_reg_7.autozero_freq",            (uint32_t)cfg->autozero.autozero_freq},
	{"user.ctrl_regs.fast_cmd_reg_7.veto_after_autozero",      (uint32_t)cfg->autozero.veto_after_autozero}
      });
    
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.load_config");

    // #############################
    // # Configuring readout block #
    // #############################
    WriteStackReg({
	{"user.ctrl_regs.readout_block.data_handshake_en", HANDSHAKE_EN},
	{"user.ctrl_regs.readout_block.l1a_timeout_value", L1A_TIMEOUT},
	{"user.ctrl_regs.Hybrid1.Hybrid_en",               HYBRID_EN},
	{"user.ctrl_regs.Hybrid1.Chips_en",                READOUT_CHIP_MASK}
      });
  }

  void RD53FWInterface::ConfigureDIO5 (const DIO5Config* cfg)
  {
    bool ext_clk_en;
    std::tie(ext_clk_en, std::ignore) = unpack_bits<1,4>(cfg->ch_out_en);
    
    WriteStackReg({
	{"user.ctrl_regs.ext_tlu_reg1.dio5_en",            (uint32_t)cfg->enable},
	{"user.ctrl_regs.ext_tlu_reg1.dio5_ch_out_en",     (uint32_t)cfg->ch_out_en},
	{"user.ctrl_regs.ext_tlu_reg1.dio5_ch1_thr",       (uint32_t)cfg->ch1_thr},
	{"user.ctrl_regs.ext_tlu_reg1.dio5_ch2_thr",       (uint32_t)cfg->ch2_thr},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_ch3_thr",       (uint32_t)cfg->ch3_thr},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_ch4_thr",       (uint32_t)cfg->ch4_thr},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_ch5_thr",       (uint32_t)cfg->ch5_thr},
	{"user.ctrl_regs.ext_tlu_reg2.tlu_en",             (uint32_t)cfg->tlu_en},
	{"user.ctrl_regs.ext_tlu_reg2.tlu_handshake_mode", (uint32_t)cfg->tlu_handshake_mode},

	{"user.ctrl_regs.ext_tlu_reg2.dio5_load_config",   1},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_load_config",   0},

	{"user.ctrl_regs.ext_tlu_reg2.ext_clk_en",         (uint32_t)ext_clk_en}
      });
  }
}
