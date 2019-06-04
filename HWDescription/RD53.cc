/*!
  \file                  RD53.cc
  \brief                 RD53 implementation class, config of the RD53
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "RD53.h"

namespace Ph2_HwDescription
{
  RD53::RD53 (const FrontEndDescription& pFeDesc, uint8_t pRD53Id, const std::string& filename) : Chip (pFeDesc, pRD53Id)
  {
    fChipOriginalMask = new ChannelGroup<nRows, nCols>;
    loadfRegMap (filename);
    setFrontEndType (FrontEndType::RD53);
    fRD53Id = pRD53Id;
  }

  RD53::RD53 (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, const std::string& filename) : Chip (pBeId, pFMCId, pFeId, pRD53Id)
  {
    fChipOriginalMask = new ChannelGroup<nRows, nCols>;
    loadfRegMap (filename);
    setFrontEndType (FrontEndType::RD53);
    fRD53Id = pRD53Id;
  }

  RD53::~RD53 () {}
  
  void RD53::loadfRegMap (const std::string& filename)
  {
    std::ifstream file (filename.c_str(), std::ios::in);
    std::stringstream myString;
    perPixelData pixData;

    if (file)
      {
	std::string line, fName, fAddress_str, fDefValue_str, fValue_str, fBitSize_str;
	bool foundPixelConfig = false;
	int cLineCounter      = 0;
	unsigned int col      = 0;
	ChipRegItem fRegItem;

	while (getline (file, line))
	  {
	    if (line.find_first_not_of (" \t") == std::string::npos)
	      {
		fCommentMap[cLineCounter] = line;
	      }
	    else if (line.at (0) == '#' || line.at (0) == '*' || line.empty())
	      {
		// If it is a comment, save the line mapped to the line number so I can later insert it in the same place
		fCommentMap[cLineCounter] = line;
	      }
	    else if ((line.find("PIXELCONFIGURATION") != std::string::npos) || (foundPixelConfig == true))
 	      {
		foundPixelConfig = true;
		
		if (line.find("COL") != std::string::npos)
		  {
		    pixData.Enable.reset();
		    pixData.HitBus.reset();
		    pixData.InjEn .reset();
		    pixData.TDAC  .clear();
		  }
		else if (line.find("ENABLE") != std::string::npos)
		  {
		    line.erase(line.find("ENABLE"),6);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int row = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.Enable[row] = atoi(readWord.c_str());
			    if (pixData.Enable[row] == 0) fChipOriginalMask->disableChannel(row,col);
			    row++;
			  }
		      }
		    
		    if (row < nRows)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << row << ") for column " << fPixelsMask.size();
			throw Exception (myString.str().c_str());
		      }
		    
		    col++;
		  }
		else if (line.find("HITBUS") != std::string::npos)
		  {
		    line.erase(line.find("HITBUS"),6);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int row = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.HitBus[row] = atoi(readWord.c_str());
			    row++;
			  }
		      }

		    if (row < nRows)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << row << ") for column " << fPixelsMask.size();
			throw Exception (myString.str().c_str());
		      }
		  }
		else if (line.find("INJEN") != std::string::npos)
		  {
		    line.erase(line.find("INJEN"),5);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int row = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.InjEn[row] = atoi(readWord.c_str());
			    row++;
			  }
		      }

		    if (row < nRows)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << row << ") for column " << fPixelsMask.size();
			throw Exception (myString.str().c_str());
		      }
		  }
		else if (line.find("TDAC") != std::string::npos)
		  {
		    line.erase(line.find("TDAC"),4);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int row = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.TDAC.push_back(atoi(readWord.c_str()));
			    row++;
			  }
		      }

		    if (row < nRows)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << row << ") for column " << fPixelsMask.size();
			throw Exception (myString.str().c_str());
		      }

		    fPixelsMask.push_back(pixData);
		  }
	      }
	    else
	      {
		myString.str(""); myString.clear();
		myString << line;
		myString >> fName >> fAddress_str >> fDefValue_str >> fValue_str >> fBitSize_str;

		fRegItem.fAddress = strtoul (fAddress_str.c_str(),  0, 16);

		int baseType;
		if      (fDefValue_str.compare(0,2,"0x") == 0) baseType = 16;
		else if (fDefValue_str.compare(0,2,"0d") == 0) baseType = 10;
		else if (fDefValue_str.compare(0,2,"0b") == 0) baseType = 2;
		else
		  {
		    LOG (ERROR) << BOLDRED << "Unknown base " << fDefValue_str << RESET;
		    throw Exception ("[RD53::loadfRegMap]\tError, unknown base");
		  }
		fDefValue_str.erase(0,2);
		fRegItem.fDefValue = strtoul (fDefValue_str.c_str(), 0, baseType);

		if      (fValue_str.compare(0,2,"0x") == 0) baseType = 16;
		else if (fValue_str.compare(0,2,"0d") == 0) baseType = 10;
		else if (fValue_str.compare(0,2,"0b") == 0) baseType = 2;
		else
		  {
		    LOG (ERROR) << BOLDRED << "Unknown base " << fValue_str << RESET;
		    throw Exception ("[RD53::loadfRegMap]\tError, unknown base");
		  }

		fValue_str.erase(0,2);
		fRegItem.fValue = strtoul (fValue_str.c_str(), 0, baseType);

		fDefValue_str.erase(0,2);
		fRegItem.fDefValue = strtoul (fDefValue_str.c_str(), 0, baseType);

		fRegItem.fPage    = 0;
		fRegItem.fBitSize = strtoul (fBitSize_str.c_str(), 0, 10);
		fRegMap[fName]    = fRegItem;
	      }

	    cLineCounter++;
	  }

	fPixelsMaskDefault = fPixelsMask;
	file.close();
      }
    else
      {
	LOG (ERROR) << BOLDRED << "The RD53 file settings " << filename << " does not exist" << RESET;
	exit (1);
      }
  }

  uint16_t RD53::getReg (const std::string& pReg) const
  {
    ChipRegMap::const_iterator i = fRegMap.find (pReg);

    if (i == fRegMap.end())
      {
	LOG (INFO) << "The RD53 object: " << fRD53Id << " doesn't have " << pReg;
	return 0;
      }
    else
      return i->second.fValue;
  }

  void RD53::setReg (const std::string& pReg, uint16_t psetValue, bool pPrmptCfg)
  {
    ChipRegMap::iterator i = fRegMap.find (pReg);

    if (i == fRegMap.end())
      LOG (INFO) << "The RD53 object: " << fRD53Id << " doesn't have " << pReg;
    else
      {
	i->second.fValue = psetValue;
	i->second.fPrmptCfg = pPrmptCfg;
      }
  }

  void RD53::saveRegMap (const std::string& filename)
  {
    const int Nspaces = 40;

    std::ofstream file (filename.c_str(), std::ios::out | std::ios::trunc);

    if (file)
      {
	std::set<ChipRegPair, RegItemComparer> fSetRegItem;
	for (const auto& it : fRegMap)
	  fSetRegItem.insert ({it.first, it.second});

	int cLineCounter = 0;	
	for (const auto& v : fSetRegItem)
	  {
	    while (fCommentMap.find (cLineCounter) != std::end (fCommentMap))
	      {
		auto cComment = fCommentMap.find (cLineCounter);

		file << cComment->second << std::endl;
		cLineCounter++;
	      }

	    file << v.first;
	    for (auto j = 0; j < Nspaces; j++)
	      file << " ";
	    file.seekp (-v.first.size(), std::ios_base::cur);
	    file << "0x"         << std::setfill ('0') << std::setw (2) << std::hex << std::uppercase << int (v.second.fAddress)
		 << "\t0x"       << std::setfill ('0') << std::setw (4) << std::hex << std::uppercase << int (v.second.fDefValue)
		 << "\t\t\t0x"   << std::setfill ('0') << std::setw (4) << std::hex << std::uppercase << int (v.second.fValue)
		 << "\t\t\t\t\t" << std::setfill ('0') << std::setw (2) << std::dec << std::uppercase << int (v.second.fBitSize) << std::endl;

	    cLineCounter++;
	  }

	file << std::dec << std::endl;
	file << "*-------------------------------------------------------------------------------------------------------" << std::endl;
	file << "PIXELCONFIGURATION" << std::endl;
	file << "*-------------------------------------------------------------------------------------------------------" << std::endl;
	for (auto i = 0; i < fPixelsMask.size(); i++)
	  {
	    file << "COL					" << std::setfill ('0') << std::setw (3) << i << std::endl;

	    file << "ENABLE " << fPixelsMask[i].Enable[0];
	    for (auto j = 1; j < fPixelsMask[i].Enable.size(); j++)
	      file << "," << fPixelsMask[i].Enable[j];
	    file << std::endl;

	    file << "HITBUS " << fPixelsMask[i].HitBus[0];
	    for (auto j = 1; j < fPixelsMask[i].HitBus.size(); j++)
	      file << "," << fPixelsMask[i].HitBus[j];
	    file << std::endl;

	    file << "INJEN  " << fPixelsMask[i].InjEn[0];
	    for (auto j = 1; j < fPixelsMask[i].InjEn.size(); j++)
	      file << "," << fPixelsMask[i].InjEn[j];
	    file << std::endl;

	    file << "TDAC   " << unsigned(fPixelsMask[i].TDAC[0]);
	    for (auto j = 1; j < fPixelsMask[i].TDAC.size(); j++)
	      file << "," << unsigned(fPixelsMask[i].TDAC[j]);
	    file << std::endl;

	    file << std::endl;
	  }

	file.close();
      }
    else
      LOG (ERROR) << BOLDRED << "Error opening file " << filename << RESET;
  }
  
  void RD53::resetMask ()
  {
    for (auto i = 0; i < fPixelsMask.size(); i++)
      {
	fPixelsMask[i].Enable.reset();
	fPixelsMask[i].HitBus.reset();
	fPixelsMask[i].InjEn .reset();
	for (auto j = 0; j < fPixelsMask[i].TDAC.size(); j++) fPixelsMask[i].TDAC[j] = 0;
      }
  }

  void RD53::enableAllPixels ()
  {
    for (auto i = 0; i < fPixelsMask.size(); i++)
      {
	fPixelsMask[i].Enable.set();
	fPixelsMask[i].HitBus.set();
      }
  }

  void RD53::disableAllPixels ()
  {
    for (auto i = 0; i < fPixelsMask.size(); i++)
      {
	fPixelsMask[i].Enable.reset();
	fPixelsMask[i].HitBus.reset();
      }
  }

  void RD53::enablePixel (unsigned int row, unsigned int col, bool enable)
  {
    fPixelsMask[col].Enable[row] = enable;
    fPixelsMask[col].HitBus[row] = enable;
  }

  void RD53::injectPixel (unsigned int row, unsigned int col, bool inject)
  {
    fPixelsMask[col].InjEn[row] = inject;
  }

  void RD53::setTDAC (unsigned int row, unsigned int col, uint8_t TDAC)
  {
    fPixelsMask[col].TDAC[row] = TDAC;
  }

  void RD53::EncodeCMD (const ChipRegItem                   & pRegItem,
  			const uint8_t                         pRD53Id,
  			const uint16_t                        pRD53Cmd,
  			std::vector<std::vector<uint16_t> > & pVecReg)
  {
    const unsigned int nBits = NBIT_ID + NBIT_ADDR + NBIT_DATA;
    
    std::bitset<nBits> idANDaddANDdata(pRD53Id           << (NBIT_ADDR + NBIT_DATA) |
  				       pRegItem.fAddress << NBIT_DATA               |
  				       pRegItem.fValue);

    std::bitset<nBits> mask = this->SetBits<nBits>(NBIT_ID);
    std::vector<uint16_t> frame;

    frame.push_back(pRD53Cmd);
    frame.push_back(pRD53Cmd);
    
    std::bitset<nBits> tmp;
    for (auto i = nBits/NBIT_DATA-1; i >= 0; i-=2)
      {
  	tmp = (idANDaddANDdata & (mask << NBIT_ID*i)) >> NBIT_ID*i;
  	unsigned long long data1 = tmp.to_ullong();
	
  	tmp = (idANDaddANDdata & (mask << NBIT_ID*(i-1))) >> NBIT_ID*(i-1);
  	unsigned long long data2 = tmp.to_ullong();
	
  	frame.push_back(cmd_data_map[data1] << NBIT_SYMBOL | cmd_data_map[data1]);
      }
    
    pVecReg.push_back(frame);
  }

  void RD53::EncodeCMD (const uint16_t                address,
			const uint16_t                data,
  			const uint8_t                 pRD53Id,
  			const uint8_t                 pRD53Cmd,
			const bool                    isBroadcast,
  			std::vector<uint32_t>       & pVecReg,
			const std::vector<uint16_t> * dataVec)
  {
    uint32_t word = 0;
    std::bitset<NBIT_FRAME> frame(0);

    if ((pRD53Cmd == (RD53CmdEncoder::RESET_ECR & 0x00FF)) ||
	(pRD53Cmd == (RD53CmdEncoder::RESET_BCR & 0x00FF)) ||
	(pRD53Cmd == (RD53CmdEncoder::NOOP      & 0x00FF)))
      {
	word = 0 | (pRD53Cmd << NBIT_5BITW);
      }
    else if (pRD53Cmd == (RD53CmdEncoder::SYNC & 0x00FF))
      {
	word = 0 | (pRD53Cmd << NBIT_5BITW);	
      }
    else if (pRD53Cmd == (RD53CmdEncoder::GLOB_PULSE & 0x00FF))
      {
	word  = 2 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);      // @TMP ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2));
	frame = 0 | ((data & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);                             // @TMP@ D[3..0],0
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME));
      }
    else if (pRD53Cmd == (RD53CmdEncoder::CAL & 0x00FF))
      {
	word  = 4 | (pRD53Cmd << NBIT_5BITW);
	frame = ((data & (this->SetBits<16>(NBIT_DATA).to_ulong() << NBIT_FRAME*3)) >> NBIT_FRAME*3) |
	  ((pRD53Id & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);                                    // @TMP@ ID[3..0],D[15]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (data & (this->SetBits<16>(NBIT_FRAME*3).to_ulong() << NBIT_FRAME*2)) >> NBIT_FRAME*2; // D[14..10]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));
	frame = (data & (this->SetBits<16>(NBIT_FRAME*2).to_ulong() << NBIT_FRAME*1)) >> NBIT_FRAME*1; // D[9..5]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));
	frame = (data & (this->SetBits<16>(NBIT_FRAME*2).to_ulong() << NBIT_FRAME*0)) >> NBIT_FRAME*0; // D[4..0]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
      }
    else if (pRD53Cmd == (RD53CmdEncoder::READ & 0x00FF))
      {
	word  = 4 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);      // @TMP@ ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (address & (this->SetBits<16>(NBIT_ADDR).to_ulong() << NBIT_ID)) >> NBIT_ID;           // A[8..4]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));
	frame = (address & this->SetBits<16>(NBIT_ID).to_ulong()) << 1;                                // @TMP@ A[3..0]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));
	frame = 0;
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
      }
    else if ((pRD53Cmd == (RD53CmdEncoder::WRITE & 0x00FF)) && (dataVec == NULL))
      {
	word  = 6 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);      // @TMP@ ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (address & (this->SetBits<16>(NBIT_ADDR).to_ulong() << NBIT_ID)) >> NBIT_ID;           // A[8..4]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));

 	frame = ((data & (this->SetBits<16>(NBIT_DATA).to_ulong() << NBIT_FRAME*3)) >> NBIT_FRAME*3) |
	  ((address & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);                                    // @TMP@ A[3..0],D[15]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));       
	frame = (data & (this->SetBits<16>(NBIT_FRAME*3).to_ulong() << NBIT_FRAME*2)) >> NBIT_FRAME*2; // D[14..10]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
	pVecReg.push_back(word);

	frame = (data & (this->SetBits<16>(NBIT_FRAME*2).to_ulong() << NBIT_FRAME*1)) >> NBIT_FRAME*1; // D[9..5]
	word  = frame.to_ulong() << NBIT_FRAME*0;
	frame = (data & (this->SetBits<16>(NBIT_FRAME*1).to_ulong() << NBIT_FRAME*0)) >> NBIT_FRAME*0; // D[4..0]
	word  = word | (frame.to_ulong() << NBIT_FRAME*1);
      }
    else if ((pRD53Cmd == (RD53CmdEncoder::WRITE & 0x00FF)) && (dataVec != NULL) && (dataVec->size() == NDATAMAX_PERPIXEL))
      {
	std::bitset<NBIT_DATA*NDATAMAX_PERPIXEL> dataBitStream(0);
	std::bitset<NBIT_DATA*NDATAMAX_PERPIXEL> tmp(0);
	for (auto i = 0; i < NDATAMAX_PERPIXEL; i++)
	  {
	    tmp = (*dataVec)[NDATAMAX_PERPIXEL - i - 1];
	    dataBitStream |= (tmp << NBIT_DATA*i);
	  }

	word  = 7 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);                                    // @TMP@ ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (address & (this->SetBits<16>(NBIT_ADDR).to_ulong() << NBIT_ID)) >> NBIT_ID;                                         // A[8..4]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));

	tmp   = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(1) << NBIT_DATA*NDATAMAX_PERPIXEL-1)) >> NBIT_FRAME*19;
	frame = tmp.to_ulong() | ((address & this->SetBits<16>(NBIT_ID).to_ulong()) << 1);                                           // @TMP@ A[3..0],D[95]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));
	tmp   = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*18)) >> NBIT_FRAME*18;        // D[94..90]
	word  = word | (tmp.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
	pVecReg.push_back(word);

	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*17)) >> NBIT_FRAME*17;         // D[89..85]
	word = tmp.to_ulong() << NBIT_FRAME*0;
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*16)) >> NBIT_FRAME*16;         // D[84..80]
	word = word | (tmp.to_ulong() << NBIT_FRAME*1);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*15)) >> NBIT_FRAME*15;         // D[79..75]
	word = word | (tmp.to_ulong() << NBIT_FRAME*2);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*14)) >> NBIT_FRAME*14;         // D[74..70]
	word = word | (tmp.to_ulong() << NBIT_FRAME*3);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*13)) >> NBIT_FRAME*13;         // D[69..65]
	word = word | (tmp.to_ulong() << NBIT_FRAME*4);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*12)) >> NBIT_FRAME*12;         // D[64..60]
	word = word | (tmp.to_ulong() << NBIT_FRAME*5);
	pVecReg.push_back(word);

	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*11)) >> NBIT_FRAME*11;         // D[59..55]
	word = tmp.to_ulong() << NBIT_FRAME*0;
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*10)) >> NBIT_FRAME*10;         // D[54..50]
	word = word | (tmp.to_ulong() << NBIT_FRAME*1);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*9)) >> NBIT_FRAME*9;           // D[49..45]
	word = word | (tmp.to_ulong() << NBIT_FRAME*2);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*8)) >> NBIT_FRAME*8;           // D[44..40]
	word = word | (tmp.to_ulong() << NBIT_FRAME*3);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*7)) >> NBIT_FRAME*7;           // D[39..35]
	word = word | (tmp.to_ulong() << NBIT_FRAME*4);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*6)) >> NBIT_FRAME*6;           // D[34..30]
	word = word | (tmp.to_ulong() << NBIT_FRAME*5);
	pVecReg.push_back(word);

	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*5)) >> NBIT_FRAME*5;           // D[29..25]
	word = tmp.to_ulong() << NBIT_FRAME*0;
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*4)) >> NBIT_FRAME*4;           // D[24..20]
	word = word | (tmp.to_ulong() << NBIT_FRAME*1);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*3)) >> NBIT_FRAME*3;           // D[19..15]
	word = word | (tmp.to_ulong() << NBIT_FRAME*2);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*2)) >> NBIT_FRAME*2;           // D[14..10]
	word = word | (tmp.to_ulong() << NBIT_FRAME*3);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*1)) >> NBIT_FRAME*1;           // D[9..5]
	word = word | (tmp.to_ulong() << NBIT_FRAME*4);
	tmp  = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(NBIT_FRAME) << NBIT_FRAME*0)) >> NBIT_FRAME*0;           // D[4..0]
	word = word | (tmp.to_ulong() << NBIT_FRAME*5);
      }

    pVecReg.push_back(word);
  }

  void RD53::ConvertRowCol2Cores (unsigned int _row, unsigned int col, uint16_t& row, uint16_t& colPair)
  {
    colPair = col >> (NPIXCOL_PROG/2);
    row     = _row;
  }
  
  void RD53::ConvertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side,
				   unsigned int& row, unsigned int& col)
  {
    row = coreRowAndRegion;
    col = NPIX_REGION * ((coreCol << RD53EvtEncoder::NBIT_SIDE) | side);
  }
  
  uint32_t RD53::getNumberOfChannels () const
  {
    return nRows * nCols;
  }

  bool RD53::isDACLocal (const std::string& dacName)
  {
    if (dacName != "PIX_PORTAL") return false;
    return true;
  }

  uint8_t RD53::getNumberOfBits (const std::string& dacName)
  {
    auto it = fRegMap.find(dacName);
    if (it == fRegMap.end()) return 0;
    return it->second.fBitSize;
  }

  RD53::Event::Event(const uint32_t* data, size_t n)
  {
    uint32_t header;
    std::tie(header, trigger_id, trigger_tag, bc_id) = unpack_bits<RD53EvtEncoder::NBIT_HEADER, RD53EvtEncoder::NBIT_TRIGID, RD53EvtEncoder::NBIT_TRGTAG, RD53EvtEncoder::NBIT_BCID>(*data);
    if (header != RD53EvtEncoder::HEADER)
      {
	LOG (ERROR) << BOLDRED << "Invalid RD53 event header" << RESET;
	return;
      }
    size_t noHitToT = RD53::SetBits<RD53EvtEncoder::NBIT_TOT>(RD53EvtEncoder::NBIT_TOT).to_ulong();
    for (auto i = 1; i < n; i++)
      if (data[i] != noHitToT) this->data.emplace_back(data[i]);
  }

  RD53::HitData::HitData (const uint32_t data)
  {
    uint32_t core_col, side, all_tots;
    std::tie(core_col, row, side, all_tots) = unpack_bits<RD53EvtEncoder::NBIT_CCOL, RD53EvtEncoder::NBIT_ROW, RD53EvtEncoder::NBIT_SIDE, RD53EvtEncoder::NBIT_TOT>(data);
    RangePacker<RD53EvtEncoder::NBIT_TOT / NPIX_REGION>::unpack_reverse(all_tots, tots);
    col = NPIX_REGION * pack_bits<RD53EvtEncoder::NBIT_CCOL, RD53EvtEncoder::NBIT_SIDE>(core_col, side);
  }

  RD53::CalCmd::CalCmd (const uint8_t& _cal_edge_mode,
			const uint8_t& _cal_edge_delay,
			const uint8_t& _cal_edge_width,
			const uint8_t& _cal_aux_mode,
			const uint8_t& _cal_aux_delay) :
    cal_edge_mode(_cal_edge_mode),
    cal_edge_delay(_cal_edge_delay),
    cal_edge_width(_cal_edge_width),
    cal_aux_mode(_cal_aux_mode),
    cal_aux_delay(_cal_aux_delay)
  {}
  
  void RD53::CalCmd::setCalCmd (const uint8_t& _cal_edge_mode,
				const uint8_t& _cal_edge_delay,
				const uint8_t& _cal_edge_width,
				const uint8_t& _cal_aux_mode,
				const uint8_t& _cal_aux_delay)
  {
    cal_edge_mode  = _cal_edge_mode;
    cal_edge_delay = _cal_edge_delay;
    cal_edge_width = _cal_edge_width;
    cal_aux_mode   = _cal_aux_mode;
    cal_aux_delay  = _cal_aux_delay;
  }

  uint32_t RD53::CalCmd::getCalCmd (const uint8_t& chipId)
  {
    return pack_bits<NBIT_ID,
		     RD53InjEncoder::NBIT_CAL_EDGE_MODE,
		     RD53InjEncoder::NBIT_CAL_EDGE_DELAY,
		     RD53InjEncoder::NBIT_CAL_EDGE_WIDTH,
		     RD53InjEncoder::NBIT_CAL_AUX_MODE,
		     RD53InjEncoder::NBIT_CAL_AUX_DELAY>(chipId,
							 cal_edge_mode,
							 cal_edge_delay,
							 cal_edge_width,
							 cal_aux_mode,
							 cal_aux_delay);
  }
}
