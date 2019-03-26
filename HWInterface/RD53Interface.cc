/*!
  \file                  RD53Interface.cc
  \brief                 User interface to the RD53 readout chip
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Interface.h"

namespace Ph2_HwInterface
{
  RD53Interface::RD53Interface  (const BeBoardFWMap& pBoardMap) : ChipInterface (pBoardMap) {}
  RD53Interface::~RD53Interface () {}

  bool RD53Interface::ConfigureChip (const Chip* cRD53, bool pVerifLoop, uint32_t pBlockSize)
  {
    ChipRegMap cRD53RegMap = cRD53->getRegMap();

    RD53* pRD53 = dynamic_cast<RD53*>(const_cast<Chip*>(cRD53));

    for (const auto& cRegItem : cRD53RegMap)
      {
	// ###############################
	// # Programmig global registers #
	// ###############################
	if (cRegItem.second.fPrmptCfg == true) this->WriteChipReg(pRD53,cRegItem.first,cRegItem.second.fValue);
      }

    // ###############################################################
    // # Enable monitoring (needed for AutoRead register monitoring) #
    // ###############################################################
    // @TMP@
    this->WriteChipReg(pRD53,"GLOBAL_PULSE_ROUTE",0x100); // 0x100 = start monitoring
    this->WriteChipReg(pRD53,"GLOBAL_PULSE",0x4);

    // ###################################
    // # Programmig pixel cell registers #
    // ###################################

    // ##########################
    // # Disable default config #
    // ##########################
    this->WriteChipReg(pRD53,"PIX_DEFAULT_CONFIG",0x0);

    // PIX_MODE
    // bit[5]: broadcast
    // bit[4]: enable auto-col
    // bit[3]: enable auto-row
    // bit[2]: broadcast to SYNC FE
    // bit[1]: broadcast to LIN FE
    // bit[0]: broadcast to DIFF FE
    this->WriteChipReg(pRD53,"PIX_MODE",0x0);

    // @TMP@
    pRD53->resetMask();
    pRD53->enablePixel(50,148);
    // pRD53->enableAllPixels();

    std::vector<uint16_t> dataVec;
    uint16_t data;
    uint16_t row;
    uint16_t colPair;

    // @TMP@
    // for (unsigned int i = 0; i < NCOLS; i+=2)
    // for (unsigned int i = 128; i < 263; i+=2)
    for (unsigned int i = 144; i < 152; i+=2)
      {
	pRD53->ConvertRowCol2Cores (0,i,colPair,row);
	data = colPair;
	this->WriteChipReg(pRD53,"REGION_COL",data);
	this->WriteChipReg(pRD53,"REGION_ROW",0x0);

	for (unsigned int j = 0; j < NROWS; j++)
	  {
	    // @TMP@
	    // LOG (INFO) << BLUE << "Configuring row #" << j << RESET;
	    pRD53->ConvertRowCol2Cores (j,i,colPair,row);
	    data = row;
	    this->WriteChipReg(pRD53,"REGION_ROW",data);

	    data =
	      HIGHGAIN                                                                                       |
	      static_cast<uint16_t>((*pRD53->getPixelsConfig()) [i].Enable[j])                               |
	      (static_cast<uint16_t>((*pRD53->getPixelsConfig())[i].InjEn [j]) << NBIT_PIXEN)                |
	      (static_cast<uint16_t>((*pRD53->getPixelsConfig())[i].HitBus[j]) << (NBIT_PIXEN + NBIT_INJEN)) |
	      (static_cast<uint16_t>((*pRD53->getPixelsConfig())[i].TDAC  [j]) << (NBIT_PIXEN + NBIT_INJEN + NBIT_HITBUS));
	    data = data                                                                                          |
	      ((HIGHGAIN                                                                                         |
		static_cast<uint16_t>((*pRD53->getPixelsConfig()) [i+1].Enable[j])                               |
		(static_cast<uint16_t>((*pRD53->getPixelsConfig())[i+1].InjEn [j]) << NBIT_PIXEN)                |
		(static_cast<uint16_t>((*pRD53->getPixelsConfig())[i+1].HitBus[j]) << (NBIT_PIXEN + NBIT_INJEN)) |
		(static_cast<uint16_t>((*pRD53->getPixelsConfig())[i+1].TDAC  [j]) << (NBIT_PIXEN + NBIT_INJEN + NBIT_HITBUS))) << (NBIT_CMD/2));

	    // @TMP@
	    this->WriteChipReg(pRD53,"PIX_PORTAL",data);

	    dataVec.push_back(data);
	    if ((j % NDATAMAX_PERPIXEL) == (NDATAMAX_PERPIXEL-1))
	      {
		// this->WriteRD53Reg(pRD53,"PIX_PORTAL",&dataVec);
		dataVec.clear();
	      }
	  }
      }

    return true;
  }

  void RD53Interface::InitRD53Aurora (RD53* pRD53)
  {
    // ##############################
    // # 1 Autora acive lane        #
    // # OUTPUT_CONFIG = 0b00000100 #
    // # CML_CONFIG    = 0b00000001 #

    // # 2 Autora acive lanes       #
    // # OUTPUT_CONFIG = 0b00001100 #
    // # CML_CONFIG    = 0b00000011 #

    // # 4 Autora acive lanes       #
    // # OUTPUT_CONFIG = 0b00111100 #
    // # CML_CONFIG    = 0b00001111 #
    // ##############################

    this->WriteChipReg(pRD53,"OUTPUT_CONFIG",0x4);
    // bits [8:7]: number of 40 MHz clocks +2 for data transfer out of pixel matrix
    // Default 0 means 2 clocks, may need higher value in case of large propagation
    // delays, for example at low VDDD voltage after irradiation
    // bits [5:2]: Aurora lanes. Default 0001 means single lane mode on lane 0
    this->WriteChipReg(pRD53,"CML_CONFIG",0x1); // Default: 00_11_1111
    this->WriteChipReg(pRD53,"AURORA_CB_CONFIG0",0xF1);
    this->WriteChipReg(pRD53,"AURORA_CB_CONFIG1",0xF);
    this->WriteChipReg(pRD53,"GLOBAL_PULSE_ROUTE",0x30); // 0x30 = reset Aurora AND reset serializer
    this->WriteChipReg(pRD53,"GLOBAL_PULSE",0x1);

    usleep(DEEPSLEEP);
  }

  void RD53Interface::SyncRD53 (RD53* pRD53, unsigned int nSyncWords)
  {
    this->WriteChipReg(pRD53,"SYNC",0x0);
  }

  bool RD53Interface::WriteChipReg (Chip* cRD53, const std::string& pRegNode, const uint16_t data, bool pVerifLoop)
  {
    setBoard (cRD53->getBeBoardId());

    RD53* pRD53 = dynamic_cast<RD53*>(cRD53);

    // std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    RD53RegItem cRegItem(0,0,0);
    cRegItem.fValue = data;

    if (strcmp(pRegNode.c_str(),"GLOBAL_PULSE") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::GlobalPulse(), false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"SYNC") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::Sync(),        false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"RESET_BCRCTR") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ResetBcrCtr(), false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"RESET_EVTCTR") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ResetEvtCtr(), false, serialSymbols);
    else
      {
	cRegItem.fAddress = pRD53->getRegItem (pRegNode).fAddress;
	pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::WriteCmd(), false, serialSymbols);
	// pRD53->EncodeCMD (cRegItem, pRD53->getChipId(), RD53::WriteCmd(), symbols);

	std::pair< std::vector<uint16_t>,std::vector<uint16_t> > outputDecoded;
	unsigned int it      = 0;
	unsigned int row     = 0;
	unsigned int pixMode = 0;
	do
	  {
	    it++;
	    if (it > NWRITE_ATTEMPTS) break;

	    if (((strcmp(pRegNode.c_str(),"PIX_PORTAL") == 0) && (it == 1)) || (strcmp(pRegNode.c_str(),"PIX_PORTAL") != 0))
	      fBoardFW->WriteChipCommand (serialSymbols);

	    if ((strcmp(pRegNode.c_str(),"PIX_PORTAL") == 0) & (it == 1))
	      {
		outputDecoded = this->ReadRD53Reg (pRD53, "PIX_MODE");
    		pixMode = outputDecoded.second[0];

		outputDecoded = this->ReadRD53Reg (pRD53, "REGION_ROW");
    		row = outputDecoded.second[0];
	      }

	    if (pixMode == 0)
	      outputDecoded = this->ReadRD53Reg (pRD53, pRegNode);

	  }
	while ((pixMode == 0) &&
	       (((strcmp(pRegNode.c_str(),"PIX_PORTAL") != 0) && (outputDecoded.first[0] != cRegItem.fAddress)) ||
		((strcmp(pRegNode.c_str(),"PIX_PORTAL") == 0) && (outputDecoded.first[0] != row))               ||
	        (outputDecoded.second[0] != cRegItem.fValue)));

	if (it > NWRITE_ATTEMPTS)
	  {
	    LOG (INFO) << BOLDRED << "Error while writing into RD53: reached the maximum number of attempts (" << NWRITE_ATTEMPTS << ")" << RESET;
	    return false;
	  }
	else
	  {
	    pRD53->setReg (pRegNode, cRegItem.fValue);
	    return true;
	  }
      }

    // fBoardFW->SerializeSymbols (symbols,serialSymbols);
    fBoardFW->WriteChipCommand (serialSymbols);
    return true;
  }

  bool RD53Interface::WriteChipMultReg (Chip* cRD53, const std::vector< std::pair<std::string, uint16_t> >& pVecReg, bool pVerifLoop)
  {
    setBoard (cRD53->getBeBoardId());

    RD53* pRD53 = dynamic_cast<RD53*>(cRD53);

    std::vector<uint32_t> serialSymbols;
    ChipRegItem cRegItem;

    for (const auto& cReg : pVecReg)
      {
	cRegItem = pRD53->getRegItem (cReg.first);
	cRegItem.fValue = cReg.second;

	if (strcmp(cReg.first.c_str(),"GLOBAL_PULSE") == 0)
	  pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::GlobalPulse(), false, serialSymbols);
	else if (strcmp(cReg.first.c_str(),"SYNC") == 0)
	  pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::Sync(),        false, serialSymbols);
	else if (strcmp(cReg.first.c_str(),"RESET_BCRCTR") == 0)
	  pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ResetBcrCtr(), false, serialSymbols);
	else if (strcmp(cReg.first.c_str(),"RESET_EVTCTR") == 0)
	  pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ResetEvtCtr(), false, serialSymbols);
	else
	  {
	    pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::WriteCmd(), false, serialSymbols);
	    pRD53->setReg (cReg.first, cReg.second);
	  }
      }

    fBoardFW->WriteChipCommand (serialSymbols);
    return true;
  }

  bool RD53Interface::WriteRD53Reg (RD53* pRD53, const std::string& pRegNode, const std::vector<uint16_t>* dataVec)
  {
    setBoard (pRD53->getBeBoardId());
    
    std::vector<uint32_t> serialSymbols;
    pRD53->EncodeCMD (pRD53->getRegItem (pRegNode).fAddress, pRD53->getRegItem (pRegNode).fValue, pRD53->getChipId(), RD53::WriteCmd(), false, serialSymbols, dataVec);

    fBoardFW->WriteChipCommand (serialSymbols);
    return true;
  }

  std::pair< std::vector<uint16_t>,std::vector<uint16_t> > RD53Interface::ReadRD53Reg (RD53* pRD53, const std::string& pRegNode)
  {
    setBoard (pRD53->getBeBoardId());

    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > outputDecoded;
    std::vector<uint32_t> serialSymbols;
    ChipRegItem cRegItem = pRD53->getRegItem (pRegNode);

    pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ReadCmd(), false, serialSymbols);
    outputDecoded = fBoardFW->ReadChipRegisters (serialSymbols);

    for (unsigned int i = 0; i < outputDecoded.first.size(); i++)
      {
	// Removing bit for pixel portal reading
	outputDecoded.first[i] = outputDecoded.first[i] & static_cast<uint16_t>(pow(2,NBIT_ADDR)-1);
	// @TMP@
	// LOG (INFO) << BLUE << "\t--> Address: " << BOLDYELLOW << "0x" << std::hex << unsigned(outputDecoded.first[i])
	// 	   << BLUE << "\tValue: " << BOLDYELLOW << "0x" << unsigned(outputDecoded.second[i]) << std::dec << RESET;
      }

    return outputDecoded;
  }

  void RD53Interface::ResetRD53 (RD53* pRD53)
  {
    this->WriteChipReg(pRD53,"RESET_EVTCTR",0x0);
    this->WriteChipReg(pRD53,"RESET_BCRCTR",0x0);
  }

  void RD53Interface::SetResetCoreCol (RD53* pRD53, bool setT_resetF)
  {
    this->WriteChipReg(pRD53,"EN_CORE_COL_SYNC",(setT_resetF == true   ? 0xFFFF : 0x0));

    this->WriteChipReg(pRD53,"EN_CORE_COL_LIN_1",(setT_resetF == true  ? 0xFFFF : 0x0));
    this->WriteChipReg(pRD53,"EN_CORE_COL_LIN_2",(setT_resetF == true  ? 0x1    : 0x0));

    this->WriteChipReg(pRD53,"EN_CORE_COL_DIFF_1",(setT_resetF == true ? 0xFFFF : 0x0));
    this->WriteChipReg(pRD53,"EN_CORE_COL_DIFF_2",(setT_resetF == true ? 0x1    : 0x0));
  }

  void RD53Interface::ResetHitOrCnt (RD53* pRD53)
  {
    this->WriteChipReg(pRD53,"HITOR_0_CNT",0x0);
    this->WriteChipReg(pRD53,"HITOR_1_CNT",0x0);
    this->WriteChipReg(pRD53,"HITOR_2_CNT",0x0);
    this->WriteChipReg(pRD53,"HITOR_3_CNT",0x0);
  }
}
