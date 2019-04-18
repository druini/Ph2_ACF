/*!
  \file                  FC7FWInterface.h
  \brief                 FC7FWInterface init/config of the FC7 and its RD53's
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "FC7FWInterface.h"

namespace Ph2_HwInterface
{
  FC7FWInterface::FC7FWInterface (const char* pId, const char* pUri, const char* pAddressTable) :
    BeBoardFWInterface (pId, pUri, pAddressTable) {}

  void FC7FWInterface::setFileHandler (FileHandler* pHandler)
  {
    if (pHandler != nullptr)
      {
	fFileHandler = pHandler;
	fSaveToFile  = true;
      }
    else LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << "\tError, can not set NULL FileHandler" << RESET;
  }

  uint32_t FC7FWInterface::getBoardInfo()
  {
    uint32_t cIDuserLogic1 = ReadReg ("user.stat_regs.usr_id.usr_id_char1");
    uint32_t cIDuserLogic2 = ReadReg ("user.stat_regs.usr_id.usr_id_char2");
    uint32_t cIDuserLogic3 = ReadReg ("user.stat_regs.usr_id.usr_id_char3");
    uint32_t cIDuserLogic4 = ReadReg ("user.stat_regs.usr_id.usr_id_char4");
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tID user logic 1st : " << cIDuserLogic1;
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tID user logic 2nd : " << cIDuserLogic2;
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tID user logic 3rd : " << cIDuserLogic3;
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tID user logic 4th : " << cIDuserLogic4;

    uint32_t cVersionMajor = ReadReg ("user.stat_regs.usr_ver.usr_ver_major");
    uint32_t cVersionMinor = ReadReg ("user.stat_regs.usr_ver.usr_ver_minor");
    uint32_t cVersionBuild = ReadReg ("user.stat_regs.usr_ver.usr_ver_build");
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tFW version : " << cVersionMajor << "." << cVersionMinor;
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tBuild version : " << cVersionBuild;

    uint32_t cFWyear  = ReadReg ("user.stat_regs.usr_ver.usr_firmware_yy");
    uint32_t cFWmonth = ReadReg ("user.stat_regs.usr_ver.usr_firmware_mm");
    uint32_t cFWday   = ReadReg ("user.stat_regs.usr_ver.usr_firmware_dd");
    LOG (INFO) << __PRETTY_FUNCTION__ << "\tFirmware date (yyyy/mm/dd) : " << cFWyear << "/" << cFWmonth << "/" << cFWday;

    uint32_t cVersionWord = ((cVersionMajor << NBIT_FWVER) | cVersionMinor);
    return cVersionWord;
  }

  void FC7FWInterface::ConfigureBoard (const BeBoard* pBoard)
  {
    // @TMP@
    // this->TurnOffFMC();
    // this->TurnOnFMC();
    this->ResetReadout();
    // this->ResetBoard();
    // this->ChipReset();
    // this->ChipReSync();

    // Wait for user to reset power to the chip
    // LOG (INFO) << BOLDMAGENTA << "Powercycle SCC and press any key to continue:" << RESET;

    // system("read");

    std::vector< std::pair<std::string, uint32_t> > cVecReg;

    BeBoardRegMap cFC7RegMap = pBoard->getBeBoardRegMap();
    LOG (INFO) << BOLDYELLOW << "Initializing board's registers" << RESET;

    for (const auto& it : cFC7RegMap)
      {
	LOG (INFO) << BOLDBLUE << "\t--> " << it.first << " = " << BOLDYELLOW << it.second << RESET;
	cVecReg.push_back ({it.first, it.second});
      }
    if (cVecReg.size() != 0) WriteStackReg (cVecReg);
  }

  void FC7FWInterface::SerializeSymbols (std::vector<std::vector<uint16_t> > & data,
					 std::vector<uint32_t>               & serialData)
  {
    for (unsigned int i = 0; i < data.size(); i++)
      for (unsigned int j = 0; j < data[i].size(); j++)
	serialData.push_back(data[i][j]);
  }

  void FC7FWInterface::WriteChipCommand (std::vector<uint32_t> & data, unsigned int repetition)
  {
    std::vector< std::pair<std::string, uint32_t> > stackRegisters;

    if (ReadReg ("user.stat_regs.cmd_proc.fifo_empty") == false)
      LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: command processor FIFO NOT empty before sending new commands" << RESET;

    if (ReadReg ("user.stat_regs.cmd_proc.fifo_full") == true)
      LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << "\tError: command processor FIFO full" << RESET;

    switch (data.size())
      {
      case 1:
	{
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.ctrl_reg", data[0]));
	  break;
	}
      case 2:
	{
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.ctrl_reg", data[0]));
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.data0_reg",data[1]));
	  break;
	}
      case 3:
	{
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.ctrl_reg", data[0]));
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.data0_reg",data[1]));
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.data1_reg",data[2]));
	  break;
	}
      case 4:
	{
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.ctrl_reg", data[0]));
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.data0_reg",data[1]));
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.data1_reg",data[2]));
	  stackRegisters.push_back(std::pair<std::string, uint32_t>("user.cmd_regs.data2_reg",data[3]));
	  break;
	}
      }

    for (unsigned int i = 0; i < repetition; i++) WriteStackReg (stackRegisters);
  }

  std::pair< std::vector<uint16_t>,std::vector<uint16_t> > FC7FWInterface::ReadChipRegisters (std::vector<uint32_t> & data, unsigned int nBlocks2Read)
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

    for (unsigned int i = 0; i < nActiveChns; i++)
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

    for (unsigned int i = 0; i < nActiveChns; i++)
      {
	myString.clear(); myString.str("");
	myString << "user.readout" << i << ".reg_read";

	if (nBlocks2Read <= nodeBlocks) regFIFO = ReadBlockRegValue(myString.str().c_str(), nBlocks2Read);
	else LOG (INFO) << BOLDRED << "Number of register blocks to read (" << nBlocks2Read << ") exceds FIFO lenght " << nodeBlocks << RESET;

	for (unsigned int i = 0; i < regFIFO.size(); i++)
	  {
	    outputDecoded.first .push_back((regFIFO[i] >> NBIT_VALUE) & static_cast<uint32_t>(pow(2,NBIT_ADDRESS)-1));
	    outputDecoded.second.push_back(regFIFO[i] & static_cast<uint32_t>(pow(2,NBIT_VALUE)-1));
	    uint8_t status = (regFIFO[i] >> (NBIT_VALUE + NBIT_ADDRESS))               & static_cast<uint32_t>(pow(2,NBIT_STATUS)-1);
	    uint8_t id     = (regFIFO[i] >> (NBIT_VALUE + NBIT_ADDRESS + NBIT_STATUS)) & static_cast<uint32_t>(pow(2,NBIT_ID)-1);
	  }
      }

    return outputDecoded;
  }

  bool FC7FWInterface::InitChipCommunication()
  {
    LOG (INFO) << BOLDYELLOW << "Checking status GLOBAL REGISTERS" << RESET;

    
    // #################################
    // # Check clock generator locking #
    // #################################
    if (ReadReg ("user.stat_regs.global_reg.clk_gen_lock") == 1)
      LOG (INFO) << BOLDGREEN << "\t--> Clock generator is locked" << RESET;
    else
      LOG (INFO) << BOLDRED << "\t--> Clock generator is not locked" << RESET;


    // ############################
    // # Check I2C initialization #
    // ############################
    if (ReadReg ("user.stat_regs.global_reg.i2c_init") == 1)
      LOG (INFO) << BOLDGREEN << "\t--> I2C initialized" << RESET;
    else
      {
	LOG (INFO) << BOLDRED << "I2C not initialized" << RESET;
	unsigned int status = ReadReg ("user.stat_regs.global_reg.i2c_init_err");
	LOG (INFO) << BOLDRED << "\t--> I2C initialization status: " << BOLDYELLOW << status << RESET;
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
    LOG (INFO) << BOLDBLUE << "Clock rate 1: " << BOLDYELLOW << clkRate << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_2");
    LOG (INFO) << BOLDBLUE << "Clock rate 2: " << BOLDYELLOW << clkRate << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_3");
    LOG (INFO) << BOLDBLUE << "Clock rate 3: " << BOLDYELLOW << clkRate << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_4");
    LOG (INFO) << BOLDBLUE << "Clock rate 4: " << BOLDYELLOW << clkRate << RESET;

    clkRate = ReadReg ("user.stat_regs.clk_rate_5");
    LOG (INFO) << BOLDBLUE << "Clock rate 5: " << BOLDYELLOW << clkRate << RESET;


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
    LOG (INFO) << BOLDRED << "\t--> Aurora channels up number less than expected: " << BOLDYELLOW << bitReg.count() << RESET;
    return false;
  }

  void FC7FWInterface::Start()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.start_trigger");
  }

  void FC7FWInterface::Stop()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.stop_trigger");
  }

  void FC7FWInterface::Pause()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.stop_trigger");
  }

  void FC7FWInterface::Resume()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.start_trigger");
  }

  uint32_t FC7FWInterface::ReadData (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
  {
    uint32_t cNWords    = ReadReg("user.stat_regs.words_to_read").value(),
             handshake  = ReadReg("user.ctrl_regs.readout_block.data_handshake_en").value(),
             cNtriggers = ReadReg("user.stat_regs.trigger_cntr").value();

    // @TMP@
    LOG (INFO) << GREEN << "cNWords = " << cNWords << RESET;
    LOG (INFO) << GREEN << "handshake = " << handshake << RESET;
    LOG (INFO) << GREEN << "cNtriggers = " << cNtriggers << RESET;

    if (!pWait && !cNWords) return 0;

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
            LOG(INFO) << BOLDRED << "Waiting for readout request, FSM status: " << fsm_status << RESET;
            
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

  std::vector<uint32_t> FC7FWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize)
  {
    uhal::ValVector<uint32_t> valBlock = RegManager::ReadBlockReg (pRegNode, pBlocksize);
    return valBlock.value();
  }

  void FC7FWInterface::TurnOffFMC()
  {
    WriteStackReg({{"system.ctrl_2.fmc_pg_c2m",0},
	           {"system.ctrl_2.fmc_l8_pwr_en",0},
		   {"system.ctrl_2.fmc_l12_pwr_en",0}});
  }

  void FC7FWInterface::TurnOnFMC()
  {
    WriteStackReg({{"system.ctrl_2.fmc_l12_pwr_en",1},
	           {"system.ctrl_2.fmc_l8_pwr_en",1},
		   {"system.ctrl_2.fmc_pg_c2m",1}});
    usleep(DEEPSLEEP);
  }

  void FC7FWInterface::ResetBoard()
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
  }
  
  void FC7FWInterface::ResetReadout()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.ipb_reset");
    
    WriteReg ("user.ctrl_regs.reset_reg.readout_block_rst",1);
    WriteReg ("user.ctrl_regs.reset_reg.readout_block_rst",0);

    while (!ReadReg("user.stat_regs.readout1.ddr3_initial_calibration_done").value())
      {
        LOG (INFO) << YELLOW << "Waiting for DDR3 calibration" << RESET;
        usleep(DEEPSLEEP);
      }

    LOG (INFO) << BOLDGREEN << "\t--> DDR3 calibration done" << RESET;
  }

  void FC7FWInterface::ChipReset()
  {
    WriteStackReg({
	{"user.ctrl_regs.reset_reg.scc_rst",1},
	{"user.ctrl_regs.reset_reg.scc_rst",0}});
    usleep(DEEPSLEEP);

    WriteStackReg({
	{"user.ctrl_regs.fast_cmd_reg_1.ipb_ecr",1},
	{"user.ctrl_regs.fast_cmd_reg_1.ipb_ecr",0}});
    usleep(DEEPSLEEP);
  }

  void FC7FWInterface::ChipReSync()
  {
    WriteStackReg({
	{"user.ctrl_regs.fast_cmd_reg_1.ipb_bcr",1},
	{"user.ctrl_regs.fast_cmd_reg_1.ipb_bcr",0}});
    usleep(DEEPSLEEP);
  }

  std::vector<FC7FWInterface::Event> FC7FWInterface::DecodeEvents(const std::vector<uint32_t>& data)
  {
    std::vector<size_t> event_start;
    for (size_t i = 0; i < data.size(); i += 4)
      {
	if (data[i] >> NBIT_BLOCKSIZE == EVT_HEADER) event_start.push_back(i);
      }

    std::vector<FC7FWInterface::Event> events;
    events.reserve(event_start.size());

    for (size_t i = 0; i < event_start.size(); i++)
      {
	const size_t start = event_start[i];
	const size_t end = (i == event_start.size() - 1) ? data.size() : event_start[i + 1];
	events.emplace_back(data.begin() + start, end - start);
      }

    return events;
  }

  template <class It>
  FC7FWInterface::Event::Event(const It& data, size_t n)
  {
    std::tie(block_size) = unpack_bits<NBIT_BLOCKSIZE>(data[0]);
    
    if (block_size * 4 != n) LOG (ERROR) << BOLDRED << "Invalid event block size: " << block_size << " instead of " << (n / 4) << RESET;
    
    std::tie(tlu_trigger_id, data_format_ver, std::ignore) = unpack_bits<NBIT_TRIGGID, NBIT_FMTVER, NBIT_DUMMY>(data[1]);
    std::tie(tdc, l1a_counter) = unpack_bits<NBIT_TDC, NBIT_L1ACNT>(data[2]);
    bx_counter = data[3];
    
    std::vector<size_t> chip_start;
    for (size_t i = 4; i < n; i += 4)
      {
	if (data[i] >> (NBIT_ERR + NBIT_HYBRID + NBIT_CHIPID + NBIT_L1ASIZE) == CHIP_HEADER) chip_start.push_back(i);
      }

    chip_data.reserve(chip_start.size());
    for (size_t i = 0; i < chip_start.size(); i++)
      {
	const size_t start = chip_start[i];
	const size_t end = (i == chip_start.size() - 1) ? n : chip_start[i + 1];
	chip_data.emplace_back(data + start, end - start);
      }
  }

  template <class It>
  FC7FWInterface::ChipData::ChipData(const It& data, size_t n) : chip_event_header(data[2])
  {
    uint16_t header;
    std::tie(header, error_code, hybrid_id, chip_id, l1a_data_size) = unpack_bits<NBIT_CHIPHEAD, NBIT_ERR, NBIT_HYBRID, NBIT_CHIPID, NBIT_L1ASIZE>(data[0]);

    if (l1a_data_size * 4 != n) LOG (ERROR) << BOLDRED << "Invalid chip L1A data size: " << l1a_data_size << " instead of " << (n / 4) << RESET;

    std::tie(chip_type, frame_delay) = unpack_bits<NBIT_CHIPTYPE, NBIT_FRAME>(data[1]);

    for (size_t i = 3; i < n; i++)
      {
	if (data[i]) hit_data.emplace_back(data[i]);
      }
  }
  
  void FC7FWInterface::SendBoardCommand(const std::string& cmd_reg)
  {
    WriteStackReg({}); // Dispatch any previous commands @TMP@

    WriteStackReg({
	{cmd_reg, 1},
	{"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 1},
	{"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 0},
	{cmd_reg, 0}
      });
  }
  
  void FC7FWInterface::ConfigureFastCommands(const FastCommandsConfig& config)
  {
    WriteStackReg({
	  // ############################
	  // # General data for trigger #
	  // ############################
	{"user.ctrl_regs.fast_cmd_reg_2.trigger_source",           (uint32_t)config.trigger_source},
	{"user.ctrl_regs.fast_cmd_reg_2.backpressure_en",          (uint32_t)config.backpressure_en},
	{"user.ctrl_regs.fast_cmd_reg_2.init_ecr_en",              (uint32_t)config.initial_ecr_en},
	{"user.ctrl_regs.fast_cmd_reg_2.veto_en",                  (uint32_t)config.veto_en},
	{"user.ctrl_regs.fast_cmd_reg_2.ext_trig_delay",           (uint32_t)config.ext_trigger_delay},
	{"user.ctrl_regs.fast_cmd_reg_3.triggers_to_accept",       (uint32_t)config.n_triggers},

	  // ##############################
	  // # Fast command configuration #
	  // ##############################
	{"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_ecr_en",            (uint32_t)config.fast_cmd_fsm.ecr_en},
	{"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_test_pulse_en",     (uint32_t)config.fast_cmd_fsm.first_cal_en},
	{"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_inject_pulse_en",   (uint32_t)config.fast_cmd_fsm.second_cal_en},
	{"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_trigger_en",        (uint32_t)config.fast_cmd_fsm.trigger_en},

	{"user.ctrl_regs.fast_cmd_reg_3.delay_after_ecr",          (uint32_t)config.fast_cmd_fsm.delay_after_ecr},
	{"user.ctrl_regs.fast_cmd_reg_4.cal_data_prime",           (uint32_t)config.fast_cmd_fsm.first_cal_data},
	{"user.ctrl_regs.fast_cmd_reg_4.delay_after_prime_pulse",  (uint32_t)config.fast_cmd_fsm.delay_after_first_cal},
	{"user.ctrl_regs.fast_cmd_reg_5.cal_data_inject",          (uint32_t)config.fast_cmd_fsm.second_cal_data},
	{"user.ctrl_regs.fast_cmd_reg_5.delay_after_inject_pulse", (uint32_t)config.fast_cmd_fsm.delay_after_second_cal},
	{"user.ctrl_regs.fast_cmd_reg_6.delay_after_autozero",     (uint32_t)config.fast_cmd_fsm.delay_after_autozero},
	{"user.ctrl_regs.fast_cmd_reg_6.delay_before_next_pulse",  (uint32_t)config.fast_cmd_fsm.delay_loop},

	  // ##########################
	  // # Autozero configuration #
	  // ##########################
        {"user.ctrl_regs.fast_cmd_reg_2.autozero_source",          (uint32_t)config.autozero.autozero_source},
	{"user.ctrl_regs.fast_cmd_reg_7.glb_pulse_data",           (uint32_t)config.autozero.glb_pulse_data},
	{"user.ctrl_regs.fast_cmd_reg_7.autozero_freq",            (uint32_t)config.autozero.autozero_freq},
	{"user.ctrl_regs.fast_cmd_reg_7.veto_after_autozero",      (uint32_t)config.autozero.veto_after_autozero}
      });
    
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.load_config");
      
    WriteStackReg({
	{"user.ctrl_regs.readout_block.data_handshake_en", HANDSHAKE_EN},
        {"user.ctrl_regs.readout_block.l1a_timeout_value", L1A_TIMEOUT},
	{"user.ctrl_regs.Hybrid1.Hybrid_en",               HYBRID_EN},
	{"user.ctrl_regs.Hybrid1.Chips_en",                READOUT_CHIP_MASK}
      });

    usleep(DEEPSLEEP);
  }

  void FC7FWInterface::ConfigureDIO5 (const DIO5Config& config)
  {
    bool ext_clk_en;
    std::tie(ext_clk_en, std::ignore, std::ignore, std::ignore, std::ignore) = unpack_bits<1,1,1,1,1>(config.ch_out_en);

    WriteStackReg({
	{"user.ctrl_regs.ext_tlu_reg1.dio5_en",            (uint32_t)config.enable},
	{"user.ctrl_regs.ext_tlu_reg1.dio5_ch_out_en",     (uint32_t)config.ch_out_en},
	{"user.ctrl_regs.ext_tlu_reg1.dio5_ch1_thr",       (uint32_t)config.ch1_thr},
	{"user.ctrl_regs.ext_tlu_reg1.dio5_ch2_thr",       (uint32_t)config.ch2_thr},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_ch3_thr",       (uint32_t)config.ch3_thr},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_ch4_thr",       (uint32_t)config.ch4_thr},
	{"user.ctrl_regs.ext_tlu_reg2.dio5_ch5_thr",       (uint32_t)config.ch5_thr},
	{"user.ctrl_regs.ext_tlu_reg2.tlu_en",             (uint32_t)config.tlu_en},
	{"user.ctrl_regs.ext_tlu_reg2.tlu_handshake_mode", (uint32_t)config.tlu_handshake_mode},

	{"user.ctrl_regs.ext_tlu_reg2.dio5_load_config",   1},
        {"user.ctrl_regs.ext_tlu_reg2.dio5_load_config",   0}

	// {"user.ctrl_regs.ext_tlu_reg2.ext_clk_en",         (uint32_t)ext_clk_en}
      });
    
    usleep(DEEPSLEEP);
  }

  void FC7FWInterface::SendTriggers(unsigned int n)
  {
    for (unsigned int i = 0; i < n; i++)
      {
	WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_trigger", 1);
	usleep(1000);
      }
  }
}
