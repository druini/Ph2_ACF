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
  RD53Interface::~RD53Interface ()                                                          {}

  bool RD53Interface::ConfigureChip (const Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    ChipRegMap pRD53RegMap = pChip->getRegMap();

    RD53* pRD53 = static_cast<RD53*>(const_cast<Chip*>(pChip));

    for (const auto& cRegItem : pRD53RegMap)
      {
	// ###############################
	// # Programmig global registers #
	// ###############################
	if (cRegItem.second.fPrmptCfg == true) this->WriteChipReg(pRD53, cRegItem.first, cRegItem.second.fValue);
      }

    // ###################################
    // # Programmig pixel cell registers #
    // ###################################
    this->WriteRD53Mask (pRD53, false, true, true);

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

    this->WriteChipReg(pRD53, "OUTPUT_CONFIG",      0x4);
    // bits [8:7]: number of 40 MHz clocks +2 for data transfer out of pixel matrix
    // Default 0 means 2 clocks, may need higher value in case of large propagation
    // delays, for example at low VDDD voltage after irradiation
    // bits [5:2]: Aurora lanes. Default 0001 means single lane mode
    this->WriteChipReg(pRD53, "CML_CONFIG", 0x1); // Default: 00_11_1111
    this->WriteChipReg(pRD53, "AURORA_CB_CONFIG0",  0xF1);
    this->WriteChipReg(pRD53, "AURORA_CB_CONFIG1",  0xF);
    this->WriteChipReg(pRD53, "GLOBAL_PULSE_ROUTE", 0x30); // 0x30 = reset Aurora AND reset serializer
    this->WriteChipReg(pRD53, "GLOBAL_PULSE",       0x1);
 
    // ###############################################################
    // # Enable monitoring (needed for AutoRead register monitoring) #
    // ###############################################################
    this->WriteChipReg(pRD53, "GLOBAL_PULSE_ROUTE", 0x100); // 0x100 = start monitoring
    this->WriteChipReg(pRD53, "GLOBAL_PULSE",       0x4);

    usleep(DEEPSLEEP);
  }

  void RD53Interface::SyncRD53 (RD53* pRD53, unsigned int nSyncWords)
  {
    this->WriteChipReg(pRD53, "SYNC", 0x0);
  }

  bool RD53Interface::WriteChipReg (Chip* pChip, const std::string& pRegNode, const uint16_t data, bool pVerifyLoop)
  {
    setBoard (pChip->getBeBoardId());

    RD53* pRD53 = static_cast<RD53*>(pChip);

    // std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    ChipRegItem cRegItem(0,0,0,0);
    cRegItem.fValue = data;

    if (strcmp(pRegNode.c_str(),"GLOBAL_PULSE") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::GlobalPulse(), false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"SYNC") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::Sync(),        false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"RESET_BCRCTR") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ResetBcrCtr(), false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"RESET_EVTCTR") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::ResetEvtCtr(), false, serialSymbols);
    else if (strcmp(pRegNode.c_str(),"CAL") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::Calibration(), false, serialSymbols);
    else
      {
	cRegItem.fAddress = pRD53->getRegItem (pRegNode).fAddress;
	pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::WriteCmd(), false, serialSymbols);
	// pRD53->EncodeCMD (cRegItem, pRD53->getChipId(), RD53::WriteCmd(), symbols);

	if (pVerifyLoop == true)
	  {
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
		
		if ((strcmp(pRegNode.c_str(),"PIX_PORTAL") == 0) && (it == 1))
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
		LOG (INFO) << BOLDRED << "Error while writing into RD53 reg. " << pRegNode << ": reached the maximum number of attempts (" << NWRITE_ATTEMPTS << ")" << RESET;
		return false;
	      }
	    else
	      {
		pRD53->setReg (pRegNode, cRegItem.fValue);
		return true;
	      }
	  }
      }

    // fBoardFW->SerializeSymbols (symbols,serialSymbols);
    fBoardFW->WriteChipCommand (serialSymbols);
    if ((strcmp(pRegNode.c_str(),"VCAL_HIGH") == 0) || (strcmp(pRegNode.c_str(),"VCAL_MED") == 0)) usleep(SHALLOWSLEEP); // @TMP@
    return true;
  }

  bool RD53Interface::WriteChipMultReg (Chip* pChip, const std::vector< std::pair<std::string, uint16_t> >& pVecReg, bool pVerifLoop)
  {
    setBoard (pChip->getBeBoardId());

    RD53* pRD53 = static_cast<RD53*>(pChip);

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
	else if (strcmp(cReg.first.c_str(),"CAL") == 0)
	  pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::Calibration(), false, serialSymbols);
	else
	  {
	    pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getChipId(), RD53::WriteCmd(), false, serialSymbols);
	    pRD53->setReg (cReg.first, cReg.second);
	  }
      }

    fBoardFW->WriteChipCommand (serialSymbols);
    return true;
  }

  bool RD53Interface::WriteRD53Reg (RD53* pRD53, const std::string& pRegNode, const std::vector<uint16_t>* dataVec, size_t nCmd)
  {
    setBoard (pRD53->getBeBoardId());
    
    size_t size = dataVec->size()/nCmd;
    std::vector<uint32_t> serialSymbols;
    for (auto i = 0; i < nCmd; i++)
      {
	std::vector<uint16_t> subDataVec(dataVec->begin() + size*i, dataVec->begin() + size*(i+1));
	pRD53->EncodeCMD (pRD53->getRegItem (pRegNode).fAddress, pRD53->getRegItem (pRegNode).fValue, pRD53->getChipId(), RD53::WriteCmd(), true, serialSymbols, &subDataVec);
      }

    fBoardFW->WriteChipCommand (serialSymbols,nCmd);
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

    for (auto i = 0; i < outputDecoded.first.size(); i++)
      // Removing bit related to PIX_PORTAL register identification
      outputDecoded.first[i] = outputDecoded.first[i] & static_cast<uint16_t>(RD53::SetBits<NBIT_ADDR>(NBIT_ADDR).to_ulong());

    return outputDecoded;
  }
  
  bool RD53Interface::WriteRD53Mask (RD53* pRD53, bool doSparse, bool doDefault, bool pVerifLoop)
  {
    std::vector<uint16_t> dataVec;
    uint16_t data;
    uint16_t colPair;

    std::vector<perPixelData>* mask;
    if (doDefault == true) mask = pRD53->getPixelsMaskDefault();
    else                   mask = pRD53->getPixelsMask();

    // ##########################
    // # Disable default config #
    // ##########################
    this->WriteChipReg(pRD53, "PIX_DEFAULT_CONFIG", 0x0, pVerifLoop);


    // ############
    // # PIX_MODE #
    // ############
    // bit[5]: enable broadcast
    // bit[4]: enable auto-col
    // bit[3]: enable auto-row
    // bit[2]: broadcast to SYNC FE
    // bit[1]: broadcast to LIN FE
    // bit[0]: broadcast to DIFF FE

    
    if (doSparse == true)
      {
	this->WriteChipReg(pRD53, "PIX_MODE", 0x27, pVerifLoop);
	this->WriteChipReg(pRD53, "PIX_PORTAL", 0x0, pVerifLoop);
	this->WriteChipReg(pRD53, "PIX_MODE", 0x0, pVerifLoop);
      }
    else this->WriteChipReg(pRD53, "PIX_MODE", 0x8, pVerifLoop);
    

    // for (uint16_t col = 0; col < RD53::nCols-1; col+=2) // @TMP@
    for (auto col = 128; col < 263; col+=2)
      {
	uint16_t row_;
	pRD53->ConvertRowCol2Cores (0,col,row_,colPair);
	this->WriteChipReg(pRD53, "REGION_COL", colPair, pVerifLoop);
	this->WriteChipReg(pRD53, "REGION_ROW", 0x0, pVerifLoop);
	
	size_t itPixCmd = 0;
	for (auto row = 0; row < RD53::nRows; row++)
	  {
	    data =
	      HIGHGAIN                                                                       |
	      static_cast<uint16_t> ((*mask)[col].Enable[row])                               |
	      (static_cast<uint16_t>((*mask)[col].InjEn [row]) << NBIT_PIXEN)                |
	      (static_cast<uint16_t>((*mask)[col].HitBus[row]) << (NBIT_PIXEN + NBIT_INJEN)) |
	      (static_cast<uint16_t>((*mask)[col].TDAC  [row]) << (NBIT_PIXEN + NBIT_INJEN + NBIT_HITBUS));
	    
	    data = data                                                                          |
	      ((HIGHGAIN                                                                         |
		static_cast<uint16_t> ((*mask)[col+1].Enable[row])                               |
		(static_cast<uint16_t>((*mask)[col+1].InjEn [row]) << NBIT_PIXEN)                |
		(static_cast<uint16_t>((*mask)[col+1].HitBus[row]) << (NBIT_PIXEN + NBIT_INJEN)) |
		(static_cast<uint16_t>((*mask)[col+1].TDAC  [row]) << (NBIT_PIXEN + NBIT_INJEN + NBIT_HITBUS))) << (NBIT_CMD/2));
	    
	    if (doSparse == true)
	      {
		if (((*mask)[col].Enable[row] == 1) || ((*mask)[col+1].Enable[row] == 1))
		  {
		    pRD53->ConvertRowCol2Cores (row,col,row_,colPair);
		    this->WriteChipReg(pRD53, "REGION_ROW", row_, pVerifLoop);
		    this->WriteChipReg(pRD53,"PIX_PORTAL",data,pVerifLoop);
		  }
	      }
	    else
	      {
		dataVec.push_back(data);
		if ((row % NDATAMAX_PERPIXEL) == (NDATAMAX_PERPIXEL-1))
		  {
		    itPixCmd++;
		    
		    if (itPixCmd == NPIXCMD)
		      {
			this->WriteRD53Reg(pRD53,"PIX_PORTAL",&dataVec,itPixCmd);
			dataVec.clear();
			itPixCmd = 0;
		      }
		  }
	      }
	  }
      }
    
    return true;
  }

  void RD53Interface::ResetRD53 (RD53* pRD53)
  {
    this->WriteChipReg(pRD53, "RESET_EVTCTR", 0x0);
    this->WriteChipReg(pRD53, "RESET_BCRCTR", 0x0);
  }

  uint16_t RD53Interface::ReadChipReg (Chip* pChip, const std::string& pRegNode)
  {
    return this->ReadRD53Reg(static_cast<RD53*>(pChip), pRegNode).second[0];
  }
  
  bool RD53Interface::ConfigureChipOriginalMask (Chip* pChip, bool pVerifLoop, uint32_t pBlockSize)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    this->WriteRD53Mask(pRD53, false, true, false);

    return true;
  }
  
  bool RD53Interface::MaskAllChannels (Chip* pChip, bool mask, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    if (mask == true) pRD53->disableAllPixels();
    else              pRD53->enableAllPixels();

    this->WriteRD53Mask(pRD53, false, false, false);

    return true;
  }

  bool RD53Interface::setInjectionSchema (Chip* pChip, const ChannelGroupBase* group, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);

    for (auto row = 0; row < RD53::nRows; row++)
      for (auto col = 0; col < RD53::nCols; col++)
	pRD53->injectPixel(row,col,group->isChannelEnabled(row,col));
    
    this->WriteRD53Mask(pRD53, false, false, false);

    return true;
  }

  bool RD53Interface::maskChannelsGroup (Chip* pChip, const ChannelGroupBase* group, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);
    
    for (auto row = 0; row < RD53::nRows; row++)
      for (auto col = 0; col < RD53::nCols; col++)
	pRD53->enablePixel(row,col,group->isChannelEnabled(row,col));

    this->WriteRD53Mask(pRD53, false, false, false);

    return true;
  }

  bool RD53Interface::WriteChipAllLocalReg (Chip* pChip, const std::string& dacName, ChipContainer& pValue, bool pVerifLoop)
  {
    RD53* pRD53 = static_cast<RD53*>(pChip);
    
    for (auto row = 0; row < RD53::nRows; row++)
      for (auto col = 0; col < RD53::nCols; col++)
	pRD53->setTDAC(row,col,pValue.getChannel<RegisterValue>(row,col).fRegisterValue);
    
    this->WriteRD53Mask(pRD53, false, true, false);
    
    return true;
  }
}
