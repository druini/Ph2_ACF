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
  FC7FWInterface::FC7FWInterface (const char* pId,
				  const char* pUri,
				  const char* pAddressTable) :
    BeBoardFWInterface (pId, pUri, pAddressTable)
  {
  }

  void FC7FWInterface::setFileHandler (FileHandler* pHandler)
  {
    if (pHandler != nullptr)
      {
	fFileHandler = pHandler;
	fSaveToFile  = true;
      }
    else LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tError, can not set NULL FileHandler" << RESET;
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
    // this->ResetIPbus();
    // this->ResetBoard();
    // this->ChipReset();
    // this->ChipReSync();

    // Wait for user to reset power to the chip
    // LOG(INFO) << BOLDMAGENTA << "Powercycle SCC and press any key to continue:" << RESET;

    // system("read");

    std::vector< std::pair<std::string, uint32_t> > cVecReg;

    BeBoardRegMap cFC7RegMap = pBoard->getBeBoardRegMap();
    LOG (INFO) << BOLDYELLOW << "Initializing board's registers" << RESET;

    for (const auto& it : cFC7RegMap)
      {
	LOG (INFO) << BOLDBLUE << "\t--> " << it.first << " = " << it.second << RESET;
	cVecReg.push_back ({it.first, it.second});
      }
    if (cVecReg.size() != 0) WriteStackReg (cVecReg);

    // this->ConfigureClockSi5324();
  }

  bool FC7FWInterface::I2cCmdAckWait (unsigned int cWait, unsigned int trials)
  {
    uint32_t cLoop = 0;
    
    while (++cLoop < trials)
      {
	uint32_t status = ReadReg ("STAT.BOARD.i2c_ack");

	if      (status == 0)             usleep (cWait);
	else if (status == I2CcmdAckGOOD) return true;
	else if (status == I2CcmdAckBAD)  return false;
	else                              usleep(cWait);
      }
    return false;
  }

  void FC7FWInterface::WriteI2C (std::vector<uint32_t>& pVecReg)
  {
    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_reset",1);
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_reset",0);
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_fifo_rx_dsel",1);
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_req",I2CwriteREQ);
    usleep(WAIT);

    /* bool outcome = */ RegManager::WriteBlockReg ("CTRL.BOARD.i2c_fifo_tx", pVecReg);
    usleep(WAIT);

    if (I2cCmdAckWait (WAIT,20) == false)
      throw Exception ("[FC7FWInterface::WriteI2C]\tI2C transaction error");

    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
    usleep(WAIT);
  }

  void FC7FWInterface::ReadI2C (std::vector<uint32_t>& pVecReg)
  {
    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_reset",1);
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_reset",0);
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_fifo_rx_dsel",1);
    usleep(WAIT);
    WriteReg ("CTRL.BOARD.i2c_req",I2CreadREQ);
    usleep(WAIT);

    uint32_t sizeI2Cfifo = ReadReg("STAT.BOARD.i2c_fifo_rx_dcnt");
    usleep(WAIT);

    int size2read = 0;
    if (sizeI2Cfifo > pVecReg.size())
      {
	size2read = pVecReg.size();
	LOG (INFO) << BOLDRED << __PRETTY_FUNCTION__ << "\tWarning, I2C FIFO contains more data than the vector size" << RESET;
      }
    else
      size2read = sizeI2Cfifo;

    pVecReg = ReadBlockRegValue ("CTRL.BOARD.i2c_fifo_rx", size2read);
    usleep(WAIT);

    if (I2cCmdAckWait (WAIT,20) == false)
      throw Exception ("[FC7FWInterface::ReadI2C]\tI2C transaction error");

    WriteReg ("CTRL.BOARD.i2c_req",0); // Disable
  }
  
  void FC7FWInterface::ConfigureClockSi5324()
  {
    // ###########################################################
    // # The Si5324 chip is meant to reduce the FC7 clock jitter #
    // ###########################################################
    
    uint8_t start_wr     = 0x90;
    uint8_t stop_wr      = 0x50;
    uint8_t stop_rd_nack = 0x68;
    uint8_t rd_incr      = 0x20;
    uint8_t wr_incr      = 0x10;

    uint8_t enable_i2cmux  = 1;
    uint8_t disable_i2cmux = 0;

    uint8_t i2cmux_addr_wr = 0xe8;
    uint8_t i2cmux_addr_rd = 0xe9;
      
    uint8_t si5324_pos     = 7;
    uint8_t si5324_addr_wr = 0xd0;
    uint8_t si5324_addr_rd = 0xd1;

    uint32_t word;
    std::vector<uint32_t> data;

    // ###########################################
    // # Program Si5324 for 160MHz precise clock #
    // ###########################################
    std::vector< std::pair<uint8_t,uint8_t> > si5324Program;
    si5324Program.push_back({0x00,0x54});
    si5324Program.push_back({0x0B,0x41});
    si5324Program.push_back({0x06,0x0F});
    si5324Program.push_back({0x15,0xFE});
    si5324Program.push_back({0x03,0x55});
    si5324Program.push_back({0x02,0x22});
    si5324Program.push_back({0x19,0x80});
    si5324Program.push_back({0x1F,0x00});
    si5324Program.push_back({0x20,0x00});
    si5324Program.push_back({0x21,0x03});
    si5324Program.push_back({0x28,0xC1});
    si5324Program.push_back({0x29,0x8F});
    si5324Program.push_back({0x2A,0xFF});
    si5324Program.push_back({0x2E,0x00});
    si5324Program.push_back({0x2F,0x59});
    si5324Program.push_back({0x30,0x48});
    si5324Program.push_back({0x89,0x01});
    si5324Program.push_back({0x88,0x40});
    // ###########################################

    word = (i2cmux_addr_wr << 8) | start_wr;
    data.push_back(word);
    word = (enable_i2cmux << si5324_pos) << 8 | stop_wr;
    data.push_back(word);

    for (unsigned int i = 0; i < si5324Program.size(); i++)
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
   
    WriteI2C(data);
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
    usleep(WAIT);
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
	    // @TMP@
 	    // LOG (INFO) << BLUE << std::hex << "\t--> Readback register status: " << BOLDYELLOW << "0x" << unsigned(status)
	    // 	       << BLUE << " Readback chip ID: " << BOLDYELLOW << "0x" << unsigned(id) << std::dec << RESET;
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
    LOG (INFO) << BOLDBLUE << "Fast CMD block trigger source: " << BOLDYELLOW << fastCMDReg << RESET;

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

      // // Reset Readout
      // WriteReg ("user.ctrl_regs.reset_reg.readout_block_rst",1);
      // // usleep(WAIT);
      // WriteReg ("user.ctrl_regs.reset_reg.readout_block_rst",0);
      // // usleep(WAIT);

      // LOG (INFO) << YELLOW << "Waiting for DDR3 calibration" << RESET;
      // while (!ReadReg("user.stat_regs.readout4.ddr3_initial_calibration_done").value()) {
      //     usleep(WAIT);
      // }
      // LOG (INFO) << GREEN << "DDR3 calibration done." << RESET;

      // // ConfigureReadout
      // WriteStackReg({
      //       {"user.ctrl_regs.readout_block.data_handshake_en", HANDSHAKE_EN},
      //       {"user.ctrl_regs.readout_block.l1a_timeout_value", 4000},
      //       {"user.ctrl_regs.Hybrid1.Hybrid_en", HYBRID_EN},
      //       {"user.ctrl_regs.Hybrid1.Chips_en", READOUT_CHIP_MASK}
      //   });
        
      return true;
    }
    LOG (INFO) << BOLDRED << "\t--> Aurora channels up number less than expected: " << BOLDYELLOW << bitReg.count() << RESET;
    return false;
  }

  void FC7FWInterface::Start()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.start_trigger");

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_2.trigger_source",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_2.autozero_source",3);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_2.ext_trig_delay",0);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_2.backpressure_en",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_2.veto_en",1);
    // usleep(WAIT);

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_3.triggers_to_accept",0);
    // usleep(WAIT);

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_7.autozero_freq",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_7.veto_after_autozero",10);
    // usleep(WAIT);

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.start_trigger",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.start_trigger",0);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",0);
    // usleep(WAIT);
  }

  void FC7FWInterface::Stop()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.stop_trigger");

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.stop_trigger",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.stop_trigger",0);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",0);
    // usleep(WAIT);
  }

  void FC7FWInterface::Pause()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.stop_trigger");

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.stop_trigger",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.stop_trigger",0);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",0);
    // usleep(WAIT);
  }

  void FC7FWInterface::Resume()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.start_trigger");

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.start_trigger",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.start_trigger",0);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",0);
    // usleep(WAIT);
  }

  uint32_t FC7FWInterface::ReadData (BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait)
  {

    

    uint32_t  cNWords     = ReadReg("user.stat_regs.words_to_read").value(),
              handshake   = ReadReg("user.ctrl_regs.readout_block.data_handshake_en").value(),
              cNtriggers  = ReadReg("user.stat_regs.trigger_cntr").value();
    
    LOG (INFO) << GREEN << "cNWords = " << cNWords << RESET;
    LOG (INFO) << GREEN << "handshake = " << handshake << RESET;
    LOG (INFO) << GREEN << "cNtriggers = " << cNtriggers << RESET;

    if (!pWait && !cNWords)
        return 0;

    while (cNWords == 0) {
        usleep(10000);
        cNWords = ReadReg("user.stat_regs.words_to_read").value();
    }
    
    if (handshake) {
        // wait for handshake
        uint32_t cReadoutReq = ReadReg("user.stat_regs.readout4.readout_req").value();
        while (cReadoutReq == 0) {
            uint32_t fsm_status = ReadReg("user.stat_regs.readout4.fsm_status").value();
            LOG(INFO) << BOLDRED << "Waiting for readout request, FSM status: " << fsm_status << RESET;
            
            usleep(SLEEP);
            
            cReadoutReq = ReadReg("user.stat_regs.readout4.readout_req");
        }

        // update values
        cNWords = ReadReg("user.stat_regs.words_to_read").value();
        cNtriggers = ReadReg("user.stat_regs.trigger_cntr").value();
    }

    // read data
    uhal::ValVector<uint32_t> values = ReadBlockReg("ddr3.fc7_daq_ddr3", cNWords);
    for (const auto& val : values) {
        pData.push_back(val);
    }

    if (fSaveToFile) fFileHandler->set(pData);

    return pData.size();
  }

  void FC7FWInterface::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
  {
    unsigned int nodeBlocks = fBoard->getNode("user.readout0.dat_read").getSize();
    unsigned int dataCnt = 0;
    unsigned int it = 0;

    if (pNEvents <= nodeBlocks)
      {
	while ((dataCnt < pNEvents) && (it < pNEvents))
	  {
	    dataCnt += ReadData (pBoard, false, pData, pWait);
	    it++;
	  }
      }
    else LOG (INFO) << BOLDRED << "Number of data blocks to read (" << pNEvents << ") exceds FIFO lenght " << nodeBlocks << RESET;
  }
  
  std::vector<uint32_t> FC7FWInterface::ReadBlockRegValue (const std::string& pRegNode, const uint32_t& pBlocksize)
  {
    uhal::ValVector<uint32_t> valBlock = RegManager::ReadBlockReg (pRegNode, pBlocksize);
    return valBlock.value();
  }

  void FC7FWInterface::TurnOffFMC()
  {
    WriteReg ("system.ctrl_2.fmc_pg_c2m",0);
    WriteReg ("system.ctrl_2.fmc_l8_pwr_en",0);
    WriteReg ("system.ctrl_2.fmc_l12_pwr_en",0);
  }

  void FC7FWInterface::TurnOnFMC()
  {
    WriteReg ("system.ctrl_2.fmc_l12_pwr_en",1);
    WriteReg ("system.ctrl_2.fmc_l8_pwr_en",1);
    WriteReg ("system.ctrl_2.fmc_pg_c2m",1);

    usleep(DEEPSLEEP);
  }

  void FC7FWInterface::ResetBoard()
  {
    // #######
    // # Set #
    // #######
    WriteReg ("user.ctrl_regs.reset_reg.aurora_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.aurora_pma_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.global_rst",1);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.clk_gen_rst",1);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.fmc_pll_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",1);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.i2c_rst",1);
    usleep(SLEEP);


    // #########
    // # Reset #
    // #########
    WriteReg ("user.ctrl_regs.reset_reg.global_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.fmc_pll_rst",1);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.cmd_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.i2c_rst",0);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.aurora_pma_rst",1);
    usleep(SLEEP);

    WriteReg ("user.ctrl_regs.reset_reg.aurora_rst",1);
    usleep(SLEEP);
  }
  
  void FC7FWInterface::ResetIPbus()
  {
    SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.ipb_reset");

      // Reset Readout
    WriteReg ("user.ctrl_regs.reset_reg.readout_block_rst",1);
    // usleep(WAIT);
    WriteReg ("user.ctrl_regs.reset_reg.readout_block_rst",0);
    // usleep(WAIT);

    while (!ReadReg("user.stat_regs.readout4.ddr3_initial_calibration_done").value()) {
        LOG (INFO) << YELLOW << "Waiting for DDR3 calibration" << RESET;
        usleep(WAIT);
    }
    LOG (INFO) << GREEN << "DDR3 calibration done." << RESET;

    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_reset",1);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_reset",0);
    // usleep(WAIT);
    // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe",0);
    // usleep(WAIT);
  }

  void FC7FWInterface::ChipReset()
  {
    WriteReg ("user.ctrl_regs.reset_reg.scc_rst",1);
    usleep(WAIT);
    WriteReg ("user.ctrl_regs.reset_reg.scc_rst",0);
    usleep(WAIT);

    WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_ecr",1);
    usleep(WAIT);
    WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_ecr",0);
    usleep(WAIT);
  }

  void FC7FWInterface::ChipReSync()
  {
    WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_bcr",1);
    usleep(WAIT);
    WriteReg ("user.ctrl_regs.fast_cmd_reg_1.ipb_bcr",0);
    usleep(WAIT);
  }

  std::vector<FC7FWInterface::Event> FC7FWInterface::DecodeEvents(const std::vector<uint32_t>& data) {
    // find events
    std::vector<size_t> event_start;
    for (size_t i = 0; i < data.size(); i += 4) {
      if (data[i] >> NBIT_BLOCKSIZE == EVT_HEADER) {
        event_start.push_back(i);
      }
    }

    std::vector<FC7FWInterface::Event> events;
    events.reserve(event_start.size());

    for (size_t i = 0; i < event_start.size(); i++) {
      const size_t start = event_start[i];
      const size_t end = (i == event_start.size() - 1) ? data.size() : event_start[i + 1];
      events.emplace_back(data.begin() + start, end - start);
    }

    return events;
  }

  template <class It>
  FC7FWInterface::Event::Event(const It& data, size_t n) {
    // std::cout << "Decoding event. size = " << n << std::endl;

    std::tie(block_size) = unpack_bits<NBIT_BLOCKSIZE>(data[0]);

    if (block_size * 4 != n) {
      LOG (ERROR) << BOLDRED << "Invalid event block size: " << block_size << " instead of " << (n / 4) << RESET;
      return;
    }

    std::tie(tlu_trigger_id, data_format_ver, std::ignore) = unpack_bits<NBIT_TRIGGID, NBIT_FMTVER, NBIT_DUMMY>(data[1]);
    std::tie(tdc, l1a_counter) = unpack_bits<NBIT_TDC, NBIT_L1ACNT>(data[2]);
    bx_counter = data[3];

    std::vector<size_t> chip_start;
    for (size_t i = 4; i < n; i += 4) {
      if (data[i] >> (NBIT_ERR + NBIT_HYBRID + NBIT_CHIPID + NBIT_L1ASIZE) == CHIP_HEADER) {
        chip_start.push_back(i);
      }
    }
    // std::cout << "chips found: " << chip_start.size() << std::endl;

    chip_data.reserve(chip_start.size());
    for (size_t i = 0; i < chip_start.size(); i++) {
      const size_t start = chip_start[i];
      const size_t end = (i == chip_start.size() - 1) ? n : chip_start[i + 1];
      chip_data.emplace_back(data + start, end - start);
    }
  }

  template <class It>
  FC7FWInterface::ChipData::ChipData(const It& data, size_t n)
    : chip_event_header(data[2])
  {
    uint16_t header;
    std::tie(header, error_code, hybrid_id, chip_id, l1a_data_size) = 
      unpack_bits<NBIT_CHIPHEAD, NBIT_ERR, NBIT_HYBRID, NBIT_CHIPID, NBIT_L1ASIZE>(data[0]);

    if (l1a_data_size * 4 != n) {
      LOG (ERROR) << "Invalid chip L1A Data Size." << RESET;
      // return;
    }

    std::tie(chip_type, frame_delay) = unpack_bits<NBIT_CHIPTYPE, NBIT_FRAME>(data[1]);

    for (size_t i = 3; i < n; i++) {
      if (data[i])
        hit_data.emplace_back(data[i]);
    }
  }


  void FC7FWInterface::SendBoardCommand(const std::string& cmd_reg) {
    WriteStackReg({}); // dispatch any previous commands
    WriteStackReg({
      {cmd_reg, 1},
      {"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 1},
      {"user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 0},
      {cmd_reg, 0}
    });
  };

  void FC7FWInterface::ConfigureFastCommands(const FastCommandsConfig& config) {
      WriteStackReg({
          {"user.ctrl_regs.fast_cmd_reg_2.trigger_source", (uint32_t)config.trigger_source},
          {"user.ctrl_regs.fast_cmd_reg_2.autozero_source", (uint32_t)config.autozero_source},
          {"user.ctrl_regs.fast_cmd_reg_2.backpressure_en", (uint32_t)config.backpressure_en},
          {"user.ctrl_regs.fast_cmd_reg_2.init_ecr_en", (uint32_t)config.initial_ecr_en},
          {"user.ctrl_regs.fast_cmd_reg_2.veto_en", (uint32_t)config.veto_en},
          {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_ecr_en", (uint32_t)config.test_fsm.ecr_en},
          {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_test_pulse_en", (uint32_t)config.test_fsm.first_cal_en},
          {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_inject_pulse_en", (uint32_t)config.test_fsm.second_cal_en},
          {"user.ctrl_regs.fast_cmd_reg_2.tp_fsm_trigger_en", (uint32_t)config.test_fsm.trigger_en},
          {"user.ctrl_regs.fast_cmd_reg_3.triggers_to_accept", (uint32_t)config.n_triggers},
          {"user.ctrl_regs.fast_cmd_reg_3.delay_after_ecr", (uint32_t)config.test_fsm.delay_after_ecr},
          {"user.ctrl_regs.fast_cmd_reg_4.cal_data_prime", (uint32_t)config.test_fsm.first_cal_data},
          {"user.ctrl_regs.fast_cmd_reg_4.delay_after_prime_pulse", (uint32_t)config.test_fsm.delay_after_first_cal},
          {"user.ctrl_regs.fast_cmd_reg_5.cal_data_inject", (uint32_t)config.test_fsm.second_cal_data},
          {"user.ctrl_regs.fast_cmd_reg_5.delay_after_inject_pulse", (uint32_t)config.test_fsm.delay_after_second_cal},
          {"user.ctrl_regs.fast_cmd_reg_6.delay_after_autozero", (uint32_t)config.test_fsm.delay_after_autozero},
          {"user.ctrl_regs.fast_cmd_reg_6.delay_before_next_pulse", (uint32_t)config.test_fsm.delay_loop},
          {"user.ctrl_regs.fast_cmd_reg_7.glb_pulse_data", (uint32_t)config.test_fsm.glb_pulse_data},
          {"user.ctrl_regs.fast_cmd_reg_7.autozero_freq", (uint32_t)config.autozero_freq},
          {"user.ctrl_regs.fast_cmd_reg_7.veto_after_autozero", (uint32_t)config.veto_after_autozero}
      });

      SendBoardCommand("user.ctrl_regs.fast_cmd_reg_1.load_config");

    

    // ConfigureReadout
    WriteStackReg({
          {"user.ctrl_regs.readout_block.data_handshake_en", HANDSHAKE_EN},
          {"user.ctrl_regs.readout_block.l1a_timeout_value", 4000},
          {"user.ctrl_regs.Hybrid1.Hybrid_en", HYBRID_EN},
          {"user.ctrl_regs.Hybrid1.Chips_en", READOUT_CHIP_MASK}
    });
    usleep(WAIT);

      // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.load_config", 1);
      // usleep(WAIT);
      // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 1);
      // usleep(WAIT);

      // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.cmd_strobe", 0);
      // usleep(WAIT);
      // WriteReg ("user.ctrl_regs.fast_cmd_reg_1.load_config", 0);
      // usleep(WAIT);
    }
}
