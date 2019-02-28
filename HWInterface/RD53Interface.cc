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
  RD53Interface::RD53Interface (const BeBoardFWMap& pBoardMap) :
    fBoardMap           (pBoardMap),
    fBoardFW            (nullptr),
    prevBoardIdentifier (65535)
  {}
  
  RD53Interface::~RD53Interface () {}
    
  void RD53Interface::setBoard (uint16_t pBoardIdentifier)
  {
    if (prevBoardIdentifier != pBoardIdentifier)
      {
	BeBoardFWMap::iterator it = fBoardMap.find (pBoardIdentifier);

	if (it == fBoardMap.end())
	  LOG (INFO) << BOLDRED << "The Board: " << (pBoardIdentifier >> 8) << " doesn't exist" << RESET;
	else
	  {
	    fBoardFW = it->second;
	    prevBoardIdentifier = pBoardIdentifier;
	  }
      }
  }

  void RD53Interface::ConfigureRD53 (RD53* pRD53)
  {
    RD53RegMap cRD53RegMap = pRD53->getRegMap();
    for (const auto& cRegItem : cRD53RegMap)
      {
	// ###############################
	// # Programmig global registers #
	// ###############################
	if (cRegItem.second.fPrmptCfg == true) this->WriteRD53Reg(pRD53,cRegItem.first,cRegItem.second.fValue);
      }

    // ###############################################################
    // # Enable monitoring (needed for AutoRead register monitoring) #
    // ###############################################################
    // @TMP@
    this->WriteRD53Reg(pRD53,"GLOBAL_PULSE_ROUTE",0x100); // 0x100 = start monitoring
    this->WriteRD53Reg(pRD53,"GLOBAL_PULSE",0x4);

    // ###################################
    // # Programmig pixel cell registers #
    // ###################################

    // ##########################
    // # Disable default config #
    // ##########################
    this->WriteRD53Reg(pRD53,"PIX_DEFAULT_CONFIG",0x0);

    // PIX_MODE
    // bit[5]: broadcast
    // bit[4]: enable auto-col
    // bit[3]: enable auto-row
    // bit[2]: broadcast to SYNC FE
    // bit[1]: broadcast to LIN FE
    // bit[0]: broadcast to DIFF FE
    this->WriteRD53Reg(pRD53,"PIX_MODE",0x8);

    // @TMP@
    pRD53->resetMask();
    pRD53->enablePixel(50,148);
    // pRD53->enableAllPixels();

    uint16_t coreCol;
    uint16_t coreRow;
    uint16_t regionCoreCol;
    uint16_t pixelRegion;
    uint16_t regionCoreRow;

    std::vector<uint16_t> dataVec;
    uint16_t data;

    // @TMP@
    // for (unsigned int i = 0; i < NCOLS; i+=2)
    for (unsigned int i = 144; i < 152; i+=2)
      {
	pRD53->ConvertRowCol2Cores (0,i,coreCol,coreRow,regionCoreCol,pixelRegion,regionCoreRow);
    	data = pixelRegion | (regionCoreCol << NBIT_NPIX_REGION) | (coreCol << (NBIT_NPIX_REGION+NBIT_NREGION_CORECOL));
	this->WriteRD53Reg(pRD53,"REGION_COL",data);
	this->WriteRD53Reg(pRD53,"REGION_ROW",0x0);

    	for (unsigned int j = 0; j < NROWS; j++)
    	  {
	    // @TMP@
	    // LOG (INFO) << BLUE << "\nConfiguring row #" << j << RESET;
	    // pRD53->ConvertRowCol2Cores (j,i,coreCol,coreRow,regionCoreCol,pixelRegion,regionCoreRow);
	    // data = regionCoreRow | (coreRow << NBIT_NREGION_COREROW);
	    // this->WriteRD53Reg(pRD53,"REGION_ROW",data);

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
	    // this->WriteRD53Reg(pRD53,"PIX_PORTAL",data);

	    dataVec.push_back(data);
	    if ((j % NDATAMAX_PERPIXEL) == (NDATAMAX_PERPIXEL-1))
	      {
		this->WriteRD53Reg(pRD53,"PIX_PORTAL",data, &dataVec);
		dataVec.clear();
	      }
   	  }
      }
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

    this->WriteRD53Reg(pRD53,"OUTPUT_CONFIG",0x4);
    // bits [8:7]: number of 40 MHz clocks +2 for data transfer out of pixel matrix
    // Default 0 means 2 clocks, may need higher value in case of large propagation
    // delays, for example at low VDDD voltage after irradiation
    // bits [5:2]: Aurora lanes. Default 0001 means single lane mode on lane 0
    this->WriteRD53Reg(pRD53,"CML_CONFIG",0x1); // Default: 00_11_1111
    this->WriteRD53Reg(pRD53,"AURORA_CB_CONFIG0",0xF1);
    this->WriteRD53Reg(pRD53,"AURORA_CB_CONFIG1",0xF);
    this->WriteRD53Reg(pRD53,"GLOBAL_PULSE_ROUTE",0x30); // 0x30 = reset Aurora AND reset serializer
    this->WriteRD53Reg(pRD53,"GLOBAL_PULSE",0x1);

    usleep(DEEPSLEEP);
  }

  void RD53Interface::SyncRD53 (RD53* pRD53, unsigned int nSyncWords)
  {
    this->WriteRD53Reg(pRD53,"SYNC",0x0);
  }

  bool RD53Interface::WriteRD53Reg (RD53* pRD53, const std::string& pRegNode, const uint16_t data, const std::vector<uint16_t> * dataVec)
  {
    setBoard (pRD53->getBeBoardId());

    std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    RD53RegItem cRegItem(0,0,0);
    cRegItem.fValue = data;

    if (strcmp(pRegNode.c_str(),"GLOBAL_PULSE") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::GlobalPulse(), serialSymbols);
    else if (strcmp(pRegNode.c_str(),"SYNC") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::Sync(),        serialSymbols);
    else if (strcmp(pRegNode.c_str(),"RESET_BCRCTR") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::ResetBcrCtr(), serialSymbols);
    else if (strcmp(pRegNode.c_str(),"RESET_EVTCTR") == 0)
      pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::ResetEvtCtr(), serialSymbols);
    else
      {
	cRegItem.fAddress = pRD53->getRegItem (pRegNode).fAddress;
	pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::WriteCmd(), serialSymbols, dataVec);
	// pRD53->EncodeCMD (cRegItem, pRD53->getRD53Id(), RD53::WriteCmd(), symbols);

	std::pair< std::vector<uint16_t>,std::vector<uint16_t> > outputDecoded;
	unsigned int it      = 0;
	unsigned int pixMode = 0;
	do
	  {
	    it++;
	    if (it > NWRITE_ATTEMPTS) break;

	    if ((strcmp(pRegNode.c_str(),"PIX_PORTAL") == 0) && (it == 1) || (strcmp(pRegNode.c_str(),"PIX_PORTAL") != 0))
	      fBoardFW->WriteChipCommand (serialSymbols);

	    if ((strcmp(pRegNode.c_str(),"PIX_PORTAL") == 0) & (it == 1))
	      {
		outputDecoded = this->ReadRD53Reg (pRD53, "PIX_MODE");
		pixMode = outputDecoded.second[0];
	      }
	    
	    if (pixMode == 0)
	      outputDecoded = this->ReadRD53Reg (pRD53, pRegNode);
	  }
	while ((pixMode == 0) && (outputDecoded.first[0] != cRegItem.fAddress) && (outputDecoded.second[0] != cRegItem.fValue));
	  
	if (it > NWRITE_ATTEMPTS)
	  {
	    LOG (INFO) << BOLDRED << "Error while writing into RD53: reached the maximum number of attempts" << RESET;
	    return false;
	  }
	else
	  {
	    if (dataVec == NULL) pRD53->setReg (pRegNode, cRegItem.fValue);
	    return true;
	  }
      }

    // fBoardFW->SerializeSymbols (symbols,serialSymbols);
    fBoardFW->WriteChipCommand (serialSymbols);
    return true;
  }

  void RD53Interface::WriteRD53MultReg (RD53* pRD53, const std::vector< std::pair<std::string, uint16_t> >& pVecReg)
  {
    setBoard (pRD53->getBeBoardId());

    std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    RD53RegItem cRegItem;

    for (const auto& cReg : pVecReg)
      {
	cRegItem = pRD53->getRegItem (cReg.first);
	cRegItem.fValue = cReg.second;
	pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::WriteCmd(), serialSymbols);
	// pRD53->EncodeCMD (cRegItem, pRD53->getRD53Id(), RD53::WriteCmd(), symbols);
      }
    // fBoardFW->SerializeSymbols (symbols,serialSymbols);
    fBoardFW->WriteChipCommand (serialSymbols);

    for (const auto& cReg : pVecReg)
      pRD53->setReg (cReg.first, cReg.second);
  }

  void RD53Interface::WriteRD53Broadcast (const Module* pModule, const std::string& pRegNode, uint16_t pValue)
  {
    setBoard (pModule->getBeBoardId());

    std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    RD53RegItem cRegItem = pModule->fRD53Vector.at(0)->getRegItem (pRegNode);
    cRegItem.fValue = pValue;

    pModule->fRD53Vector.at(0)->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pModule->fRD53Vector.at(0)->getRD53Id(), RD53::WriteCmd(), serialSymbols);
    // pModule->fRD53Vector.at(0)->EncodeCMD (cRegItem, pModule->fRD53Vector.at(0)->getRD53Id(), RD53::WriteCmd(), symbols);
    // fBoardFW->SerializeSymbols (symbols,serialSymbols);
    fBoardFW->WriteChipCommand (serialSymbols);

    for (auto& cRD53 : pModule->fRD53Vector) cRD53->setReg (pRegNode, pValue);
  }
  
  void RD53Interface::WriteRD53BroadcastMultReg (const Module* pModule, const std::vector<std::pair<std::string, uint16_t>> pVecReg)
  {
    setBoard (pModule->getBeBoardId());

    std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    RD53RegItem cRegItem;

    for (const auto& cReg : pVecReg)
      {
	cRegItem =  pModule->fRD53Vector.at(0)->getRegItem (cReg.first);
	cRegItem.fValue = cReg.second;
	pModule->fRD53Vector.at(0)->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pModule->fRD53Vector.at(0)->getRD53Id(), RD53::WriteCmd(), serialSymbols);
	// pModule->fRD53Vector.at(0)->EncodeCMD (cRegItem, pModule->fRD53Vector.at(0)->getRD53Id(), RD53::WriteCmd(), symbols);
      }
    // fBoardFW->SerializeSymbols (symbols,serialSymbols);
    fBoardFW->WriteChipCommand (serialSymbols);

    for (const auto& cRD53 : pModule->fRD53Vector)
      for (const auto& cReg : pVecReg)
	{
	  cRegItem = cRD53->getRegItem (cReg.first);
	  cRD53->setReg (cReg.first, cReg.second);
	}
  }

  std::pair< std::vector<uint16_t>,std::vector<uint16_t> > RD53Interface::ReadRD53Reg (RD53* pRD53, const std::string& pRegNode)
  {
    setBoard (pRD53->getBeBoardId());

    std::pair< std::vector<uint16_t>,std::vector<uint16_t> > outputDecoded;

    std::vector<std::vector<uint16_t> > symbols; // Useful in case the encoding is done in the software
    std::vector<uint32_t> serialSymbols;
    RD53RegItem cRegItem = pRD53->getRegItem (pRegNode);

    pRD53->EncodeCMD (cRegItem.fAddress, cRegItem.fValue, pRD53->getRD53Id(), RD53::ReadCmd(), serialSymbols);
    // pRD53->EncodeCMD (cRegItem, pRD53->getRD53Id(), RD53::ReadCmd(), symbols);
    // fBoardFW->SerializeSymbols (symbols,serialSymbols);

    outputDecoded = fBoardFW->ReadChipRegisters (serialSymbols);

    for (unsigned int i = 0; i < outputDecoded.first.size(); i++)
      {
	// Removing bit for pixel portal reading
	outputDecoded.first[i] = outputDecoded.first[i] & static_cast<uint16_t>(pow(2,NBIT_ADDR)-1);
	LOG (INFO) << BLUE << "\t--> Address: " << BOLDYELLOW << "0x" << std::hex << unsigned(outputDecoded.first[i])
	 	   << BLUE << "\tValue: " << BOLDYELLOW << "0x" << unsigned(outputDecoded.second[i]) << std::dec << RESET;
      }

    return outputDecoded;
  }

  void RD53Interface::ResetRD53 (RD53* pRD53)
  {
    this->WriteRD53Reg(pRD53,"RESET_EVTCTR",0x0);
    this->WriteRD53Reg(pRD53,"RESET_BCRCTR",0x0);
  }

  void RD53Interface::SetResetCoreCol (RD53* pRD53, bool setT_resetF)
  {
    this->WriteRD53Reg(pRD53,"EN_CORE_COL_SYNC",(setT_resetF == true   ? 0xFFFF : 0x0));
    
    this->WriteRD53Reg(pRD53,"EN_CORE_COL_LIN_1",(setT_resetF == true  ? 0xFFFF : 0x0));
    this->WriteRD53Reg(pRD53,"EN_CORE_COL_LIN_2",(setT_resetF == true  ? 0x1    : 0x0));
    
    this->WriteRD53Reg(pRD53,"EN_CORE_COL_DIFF_1",(setT_resetF == true ? 0xFFFF : 0x0));
    this->WriteRD53Reg(pRD53,"EN_CORE_COL_DIFF_2",(setT_resetF == true ? 0x1    : 0x0));
  }
}
