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
  RD53::RD53 ( const FrontEndDescription& pFeDesc, uint8_t pRD53Id, const std::string& filename ) : Chip (pFeDesc, pRD53Id)
  {
    loadfRegMap     (filename);
    setFrontEndType (FrontEndType::RD53);
  }

  RD53::RD53 (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, const std::string& filename ) : Chip (pBeId, pFMCId, pFeId, pRD53Id)
  {
    loadfRegMap     (filename);
    setFrontEndType (FrontEndType::RD53);
  }

  RD53::~RD53 () {}
  
  void RD53::loadfRegMap (const std::string& filename)
  {
    std::ifstream file (filename.c_str(), std::ios::in);
    std::stringstream myString;
    perPixelData pixData;

    if (file)
      {
	std::string line, fName, fAddress_str, fDefValue_str, fValue_str;
	bool foundPixelConfig = false;
	int cLineCounter = 0;
	ChipRegItem fRegItem;
	fhasMaskedChannels = false;

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
		    unsigned int it = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.Enable[it] = atoi(readWord.c_str());
			    it++;
			    if (pixData.Enable[it] == 0) fhasMaskedChannels = true;
			  }
		      }

		    if (it < NROWS)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << it << ") for column " << fPixelsConfig.size();
			throw Exception (myString.str().c_str());
		      }
		  }
		else if (line.find("HITBUS") != std::string::npos)
		  {
		    line.erase(line.find("HITBUS"),6);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int it = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.HitBus[it] = atoi(readWord.c_str());
			    it++;
			  }
		      }

		    if (it < NROWS)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << it << ") for column " << fPixelsConfig.size();
			throw Exception (myString.str().c_str());
		      }
		  }
		else if (line.find("INJEN") != std::string::npos)
		  {
		    line.erase(line.find("INJEN"),5);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int it = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.InjEn[it] = atoi(readWord.c_str());
			    it++;
			  }
		      }

		    if (it < NROWS)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << it << ") for column " << fPixelsConfig.size();
			throw Exception (myString.str().c_str());
		      }
		  }
		else if (line.find("TDAC") != std::string::npos)
		  {
		    line.erase(line.find("TDAC"),4);
		    myString.str(""); myString.clear();
		    myString << line;
		    unsigned int it = 0;
		    std::string readWord;

		    while (getline(myString,readWord,','))
		      {
			readWord.erase(std::remove_if(readWord.begin(), readWord.end(), isspace), readWord.end());
			if (std::all_of(readWord.begin(), readWord.end(), isdigit))
			  {
			    pixData.TDAC.push_back(atoi(readWord.c_str()));
			    it++;
			  }
		      }

		    if (it < NROWS)
		      {
			myString.str(""); myString.clear();
			myString << "[RD53::loadfRegMap]\tError, problem reading RD53 config file: too few rows (" << it << ") for column " << fPixelsConfig.size();
			throw Exception (myString.str().c_str());
		      }

		    fPixelsConfig.push_back(pixData);
		  }
	      }
	    else
	      {
		myString.str(""); myString.clear();
		myString << line;
		myString >> fName >> fAddress_str >> fDefValue_str >> fValue_str;

		fRegItem.fAddress = strtoul (fAddress_str.c_str(),  0, 16);

		int baseType;
		if      (fDefValue_str.compare(0,2,"0x") == 0) baseType = 16;
		else if (fDefValue_str.compare(0,2,"0d") == 0) baseType = 10;
		else if (fDefValue_str.compare(0,2,"0b") == 0) baseType = 2;
		else
		  {
		    LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << "\tError, unknown base " << fDefValue_str << RESET;
		    throw Exception ("[RD53::loadfRegMap]\tError, unknown base");
		  }
		fDefValue_str.erase(0,2);
		fRegItem.fDefValue = strtoul (fDefValue_str.c_str(), 0, baseType);

		if      (fValue_str.compare(0,2,"0x") == 0) baseType = 16;
		else if (fValue_str.compare(0,2,"0d") == 0) baseType = 10;
		else if (fValue_str.compare(0,2,"0b") == 0) baseType = 2;
		else
		  {
		    LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << "\tError, unknown base " << fValue_str << RESET;
		    throw Exception ("[RD53::loadfRegMap]\tError, unknown base");
		  }

		fValue_str.erase(0,2);
		fRegItem.fValue = strtoul (fValue_str.c_str(), 0, baseType);

		fDefValue_str.erase(0,2);
		fRegItem.fDefValue = strtoul (fDefValue_str.c_str(), 0, baseType);

		fRegItem.fPage = 0;

		fRegMap[fName] = fRegItem;
	      }

	    cLineCounter++;
	  }

	fPixelsConfigDefault = fPixelsConfig;
	file.close();
      }
    else
      {
	LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << "\tThe RD53 file settings " << filename << " does not exist" << RESET;
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
	for (auto& it : fRegMap)
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
	    for (int j = 0; j < Nspaces; j++)
	      file << " ";
	    file.seekp (-v.first.size(), std::ios_base::cur);
	    file << "0x"       << std::setfill ('0') << std::setw (2) << std::hex << std::uppercase << int (v.second.fAddress)
		 << "\t0x"     << std::setfill ('0') << std::setw (4) << std::hex << std::uppercase << int (v.second.fDefValue)
		 << "\t\t\t0x" << std::setfill ('0') << std::setw (4) << std::hex << std::uppercase << int (v.second.fValue) << std::endl;

	    cLineCounter++;
	  }

	file << std::dec << std::endl;
	file << "*------------------------------------------------------------------------------------------" << std::endl;
	file << "PIXELCONFIGURATION" << std::endl;
	file << "*------------------------------------------------------------------------------------------" << std::endl;
	for (unsigned int i = 0; i < fPixelsConfig.size(); i++)
	  {
	    file << "COL					" << std::setfill ('0') << std::setw (3) << i << std::endl;

	    file << "ENABLE " << fPixelsConfig[i].Enable[0];
	    for (unsigned int j = 1; j < fPixelsConfig[i].Enable.size(); j++)
	      file << "," << fPixelsConfig[i].Enable[j];
	    file << std::endl;

	    file << "HITBUS " << fPixelsConfig[i].HitBus[0];
	    for (unsigned int j = 1; j < fPixelsConfig[i].HitBus.size(); j++)
	      file << "," << fPixelsConfig[i].HitBus[j];
	    file << std::endl;

	    file << "INJEN  " << fPixelsConfig[i].InjEn[0];
	    for (unsigned int j = 1; j < fPixelsConfig[i].InjEn.size(); j++)
	      file << "," << fPixelsConfig[i].InjEn[j];
	    file << std::endl;

	    file << "TDAC   " << unsigned(fPixelsConfig[i].TDAC[0]);
	    for (unsigned int j = 1; j < fPixelsConfig[i].TDAC.size(); j++)
	      file << "," << unsigned(fPixelsConfig[i].TDAC[j]);
	    file << std::endl;

	    file << std::endl;
	  }

	file.close();
      }
    else
      LOG (ERROR) << BOLDRED << __PRETTY_FUNCTION__ << "\tError opening file " << filename << RESET;
  }
  
  void RD53::resetMask()
  {
    for (unsigned int i = 0; i < fPixelsConfig.size(); i++)
      {
	fPixelsConfig[i].Enable.reset();
	fPixelsConfig[i].HitBus.reset();
	fPixelsConfig[i].InjEn .reset();
	for (unsigned int j = 0; j < fPixelsConfig[i].TDAC.size(); j++) fPixelsConfig[i].TDAC[j] = 0;
      }
  }

  void RD53::enableAllPixels()
  {
    for (unsigned int i = 0; i < fPixelsConfig.size(); i++)
      {
	fPixelsConfig[i].Enable.set();
	fPixelsConfig[i].HitBus.set();
      }
  }

  void RD53::enablePixel(unsigned int row, unsigned int col)
  {
    fPixelsConfig[col].Enable[row] = 1;
    fPixelsConfig[col].HitBus[row] = 1;
  }

  void RD53::injectAllPixels()
  {
    for (unsigned int i = 0; i < fPixelsConfig.size(); i++)
      {
	fPixelsConfig[i].InjEn.set();
      }
  }

  void RD53::EncodeCMD (const RD53RegItem                   & pRegItem,
  			const uint8_t                         pRD53Id,
  			const uint16_t                        pRD53Cmd,
  			std::vector<std::vector<uint16_t> > & pVecReg)
  {
    const unsigned int nBits = NBIT_CHIPID + NBIT_ADDR + NBIT_DATA;
    
    std::bitset<nBits> idANDaddANDdata(pRD53Id           << (NBIT_ADDR + NBIT_DATA) |
  				       pRegItem.fAddress << NBIT_DATA               |
  				       pRegItem.fValue);

    std::bitset<nBits> mask = this->SetBits<nBits>(NBIT_CHIPID);
    std::vector<uint16_t> frame;

    frame.push_back(pRD53Cmd);
    frame.push_back(pRD53Cmd);
    
    std::bitset<nBits> tmp;
    for (int i = nBits/NBIT_DATA-1; i >= 0; i-=2)
      {
  	tmp = (idANDaddANDdata & (mask << NBIT_CHIPID*i)) >> NBIT_CHIPID*i;
  	unsigned long long data1 = tmp.to_ullong();
	
  	tmp = (idANDaddANDdata & (mask << NBIT_CHIPID*(i-1))) >> NBIT_CHIPID*(i-1);
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

    if ((pRD53Cmd == (RESET_ECR & 0x00FF)) ||
	(pRD53Cmd == (RESET_BCR & 0x00FF)) ||
	(pRD53Cmd == (NOOP      & 0x00FF)))
      {
	word = 0 | (pRD53Cmd << NBIT_5BITW);
      }
    else if (pRD53Cmd == (SYNC & 0x00FF))
      {
	word = 0 | (pRD53Cmd << NBIT_5BITW);	
      }
    else if (pRD53Cmd == (GLOB_PULSE & 0x00FF))
      {
	word  = 2 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1); // @TMP ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2));
	frame = 0 | ((data & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1);                        // @TMP@ D[3..0],0
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME));
      }
    else if (pRD53Cmd == (CAL & 0x00FF))
      {
	word  = 4 | (pRD53Cmd << NBIT_5BITW);
	frame = ((data & (this->SetBits<16>(NBIT_DATA).to_ulong() << NBIT_FRAME*3)) >> NBIT_FRAME*3) |
	  ((pRD53Id & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1);                                // @TMP@ ID[3..0],D[15]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (data & (this->SetBits<16>(NBIT_FRAME*3).to_ulong() << NBIT_FRAME*2)) >> NBIT_FRAME*2; // D[14..10]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));
	frame = (data & (this->SetBits<16>(NBIT_FRAME*2).to_ulong() << NBIT_FRAME*1)) >> NBIT_FRAME*1; // D[9..5]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));
	frame = (data & (this->SetBits<16>(NBIT_FRAME*2).to_ulong() << NBIT_FRAME*0)) >> NBIT_FRAME*0; // D[4..0]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
      }
    else if (pRD53Cmd == (READCMD & 0x00FF))
      {
	word  = 4 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1); // @TMP@ ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (address & (this->SetBits<16>(NBIT_ADDR).to_ulong() << NBIT_CHIPID)) >> NBIT_CHIPID;  // A[8..4]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));
	frame = (address & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1;                           // @TMP@ A[3..0]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));
	frame = 0;
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
      }
    else if ((pRD53Cmd == (WRITECMD & 0x00FF)) && (dataVec == NULL))
      {
	word  = 6 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1);  // @TMP@ ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (address & (this->SetBits<16>(NBIT_ADDR).to_ulong() << NBIT_CHIPID)) >> NBIT_CHIPID;   // A[8..4]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));

 	frame = ((data & (this->SetBits<16>(NBIT_DATA).to_ulong() << NBIT_FRAME*3)) >> NBIT_FRAME*3) |
	  ((address & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1);                                // @TMP@ A[3..0],D[15]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*2));       
	frame = (data & (this->SetBits<16>(NBIT_FRAME*3).to_ulong() << NBIT_FRAME*2)) >> NBIT_FRAME*2; // D[14..10]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*3));
	pVecReg.push_back(word);

	frame = (data & (this->SetBits<16>(NBIT_FRAME*2).to_ulong() << NBIT_FRAME*1)) >> NBIT_FRAME*1; // D[9..5]
	word  = frame.to_ulong() << NBIT_FRAME*0;
	frame = (data & (this->SetBits<16>(NBIT_FRAME*1).to_ulong() << NBIT_FRAME*0)) >> NBIT_FRAME*0; // D[4..0]
	word  = word | (frame.to_ulong() << NBIT_FRAME*1);
      }
    else if ((pRD53Cmd == (WRITECMD & 0x00FF)) && (dataVec != NULL) && (dataVec->size() == NDATAMAX_PERPIXEL))
      {
	std::bitset<NBIT_DATA*NDATAMAX_PERPIXEL> dataBitStream(0);
	std::bitset<NBIT_DATA*NDATAMAX_PERPIXEL> tmp(0);
	for (unsigned int i = 0; i < NDATAMAX_PERPIXEL; i++)
	  {
	    tmp = (*dataVec)[i];
	    dataBitStream |= (tmp << NBIT_DATA*i);
	  }

	word  = 7 | (pRD53Cmd << NBIT_5BITW);
	frame = (isBroadcast ? 1 : 0) | ((pRD53Id & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1);                                // @TMP@ ID[3..0],isBroadcast
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*0));
	frame = (address & (this->SetBits<16>(NBIT_ADDR).to_ulong() << NBIT_CHIPID)) >> NBIT_CHIPID;                                 // A[8..4]
	word  = word | (frame.to_ulong() << (NBIT_5BITW + NBIT_CMD/2 + NBIT_FRAME*1));

	tmp   = (dataBitStream & (this->SetBits<NBIT_DATA*NDATAMAX_PERPIXEL>(1) << NBIT_DATA*NDATAMAX_PERPIXEL-1)) >> NBIT_FRAME*19;
	frame = tmp.to_ulong() | ((address & this->SetBits<16>(NBIT_CHIPID).to_ulong()) << 1);                                       // @TMP@ A[3..0],D[95]
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

  void RD53::ConvertRowCol2Cores (unsigned int _row, unsigned int col, uint16_t& colPair, uint16_t& row)
  {
    colPair = col >> (NPIXCOL_PROG/2);
    row     = _row;
  }
  
  template<int NBITS>
  std::bitset<NBITS> RD53::SetBits(unsigned int nBit2Set)
  {
    std::bitset<NBITS> output(0);
    for (unsigned int i = 0; i < nBit2Set; i++) output[i] = 1;
    return output;
  }

  uint16_t RD53::getNumberOfChannels () const
  {
    return NCOLS * NROWS;
  }

  bool RD53::isDACLocal (const std::string& dacName)
  {
    if (dacName != "PIX_PORTAL") return false;
    return true;
  }

  uint8_t RD53::getNumberOfBits (const std::string& dacName)
  {
    // #################
    // # Pixel Section #
    // #################
    if (dacName == "PIX_PORTAL")
	  return 16;
    if (dacName == "REGION_COL")
	  return 8;
    if (dacName == "REGION_ROW")
      return 8;
    if (dacName == "PIX_MODE")
      return 6;
    if (dacName == "PIX_DEFAULT_CONFIG")
      return 16;

    // #########################
    // # Synchronous Front End #
    // #########################
    if (dacName == "IBIASP1_SYNC")
      return 9;
    if (dacName == "IBIASP2_SYNC")
      return 9;
    if (dacName == "IBIAS_SF_SYNC")
      return 9;
    if (dacName == "IBIAS_KRUM_SYNC")
      return 9;
    if (dacName == "IBIAS_DISC_SYNC")
      return 9;
    if (dacName == "ICTRL_SYNCT_SYNC")
      return 10;
    if (dacName == "VBL_SYNC")
      return 10;
    if (dacName == "VTH_SYNC")
      return 10;
    if (dacName == "VREF_KRUM_SYNC")
      return 10;

    // ####################
    // # Linear Front End #
    // ####################
    if (dacName == "PA_IN_BIAS_LIN")
      return 9;
    if (dacName == "FC_BIAS_LIN")
      return 8;
    if (dacName == "KRUM_CURR_LIN")
      return 9;
    if (dacName == "LDAC_LIN")
      return 10;
    if (dacName == "COMP_LIN")
      return 9;
    if (dacName == "REF_KRUM_LIN")
      return 10;
    if (dacName == "Vthreshold_LIN")
      return 10;

    // ##########################
    // # Differential Front End #
    // ##########################
    if (dacName == "PRMP_DIFF")
      return 10;
    if (dacName == "FOL_DIFF")
      return 10;
    if (dacName == "PRECOMP_DIFF")
      return 10;
    if (dacName == "COMP_DIFF")
      return 10;
    if (dacName == "VFF_DIFF")
      return 10;
    if (dacName == "VTH1_DIFF")
      return 10;
    if (dacName == "VTH2_DIFF")
      return 10;
    if (dacName == "LCC_DIFF")
      return 10;

    // #######################
    // # Auxiliary Registers #
    // #######################
    if (dacName == "CONF_FE_SYNC")
      return 5;
    if (dacName == "CONF_FE_DIFF")
      return 2;
    if (dacName == "VOLTAGE_TRIM")
      return 10;

    // ##################
    // # Digital Matrix #
    // ##################
    if (dacName == "EN_CORE_COL_SYNC")
      return 16;
    if (dacName == "EN_CORE_COL_LIN_1")
      return 16;
    if (dacName == "EN_CORE_COL_LIN_2")
      return 1;
    if (dacName == "EN_CORE_COL_DIFF_1")
      return 16;
    if (dacName == "EN_CORE_COL_DIFF_2")
      return 1;
    if (dacName == "LATENCY_CONFIG")
      return 9;
    if (dacName == "WR_SYNC_DELAY_SYNC")
      return 5;

    // #############
    // # Injection #
    // #############
    if (dacName == "INJECTION_SELECT")
      return 6;
    if (dacName == "CLK_DATA_DELAY")
      return 9;
    if (dacName == "VCAL_HIGH")
      return 12;
    if (dacName == "VCAL_MED")
      return 12;
    if (dacName == "CH_SYNC_CONF")
      return 12;
    if (dacName == "GLOBAL_PULSE_ROUTE")
      return 16;
    if (dacName == "MONITOR_FRAME_SKIP")
      return 8;
    if (dacName == "EN_MACRO_COL_CAL_SYNC_1")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_SYNC_2")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_SYNC_3")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_SYNC_4")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_LIN_1")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_LIN_2")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_LIN_3")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_LIN_4")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_LIN_5")
      return 4;
    if (dacName == "EN_MACRO_COL_CAL_DIFF_1")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_DIFF_2")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_DIFF_3")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_DIFF_4")
      return 16;
    if (dacName == "EN_MACRO_COL_CAL_DIFF_5")
      return 4;

    // #######
    // # I/O #
    // #######
    if (dacName == "DEBUG_CONFIG")
      return 2;
    if (dacName == "OUTPUT_CONFIG")
      return 9;
    if (dacName == "OUT_PAD_CONFIG")
      return 14;
    if (dacName == "GP_LVDS_ROUTE")
      return 16;
    if (dacName == "CDR_CONFIG")
      return 14;
    if (dacName == "CDR_VCO_BUFF_BIAS")
      return 10;
    if (dacName == "CDR_CP_IBIAS")
      return 10;
    if (dacName == "CDR_VCO_IBIAS")
      return 10;
    if (dacName == "SER_SEL_OUT")
      return 8;    
    if (dacName == "CML_CONFIG")
      return 8;
    if (dacName == "CML_TAP0_BIAS")
      return 10;
    if (dacName == "CML_TAP1_BIAS")
      return 10;
    if (dacName == "CML_TAP2_BIAS")
      return 10;
    if (dacName == "AURORA_CC_CONFIG")
      return 8;
    if (dacName == "AURORA_CB_CONFIG0")
      return 8;
    if (dacName == "AURORA_CB_CONFIG1")
      return 16;
    if (dacName == "AURORA_INIT_WAIT")
      return 11;

    // #################################
    // # Test and Monitoring Functions #
    // #################################
    if (dacName == "MONITOR_SELECT")
      return 14;
    if (dacName == "HITOR_0_MASK_SYNC")
      return 16;
    if (dacName == "HITOR_1_MASK_SYNC")
      return 16;
    if (dacName == "HITOR_2_MASK_SYNC")
      return 16;
    if (dacName == "HITOR_3_MASK_SYNC")
      return 16;
    if (dacName == "HITOR_0_MASK_LIN_0")
      return 16;
    if (dacName == "HITOR_0_MASK_LIN_1")
      return 1;
    if (dacName == "HITOR_1_MASK_LIN_0")
      return 16;
    if (dacName == "HITOR_1_MASK_LIN_1")
      return 1;
    if (dacName == "HITOR_2_MASK_LIN_0")
      return 16;
    if (dacName == "HITOR_2_MASK_LIN_1")
      return 1;
    if (dacName == "HITOR_3_MASK_LIN_0")
      return 16;
    if (dacName == "HITOR_3_MASK_LIN_1")
      return 1;
    if (dacName == "HITOR_0_MASK_DIFF_0")
      return 16;
    if (dacName == "HITOR_0_MASK_DIFF_1")
      return 1;
    if (dacName == "HITOR_1_MASK_DIFF_0")
      return 16;
    if (dacName == "HITOR_1_MASK_DIFF_1")
      return 1;
    if (dacName == "HITOR_2_MASK_DIFF_0")
      return 16;
    if (dacName == "HITOR_2_MASK_DIFF_1")
      return 1;
    if (dacName == "HITOR_3_MASK_DIFF_0")
      return 16;
    if (dacName == "HITOR_3_MASK_DIFF_1")
      return 1;
    if (dacName == "MONITOR_CONFIG")
      return 11;
    if (dacName == "SENSOR_CONFIG_0")
      return 12;
    if (dacName == "SENSOR_CONFIG_1")
      return 12;
    if (dacName == "AUTO_READ_0")
      return 9;
    if (dacName == "AUTO_READ_1")
      return 9;
    if (dacName == "AUTO_READ_2")
      return 9;
    if (dacName == "AUTO_READ_3")
      return 9;
    if (dacName == "AUTO_READ_4")
      return 9;
    if (dacName == "AUTO_READ_5")
      return 9;
    if (dacName == "AUTO_READ_6")
      return 9;
    if (dacName == "AUTO_READ_7")
      return 9;
    if (dacName == "RING_OSC_ENABLE")
      return 8;
    if (dacName == "RING_OSC_0")
      return 16;
    if (dacName == "RING_OSC_1")
      return 16;
    if (dacName == "RING_OSC_2")
      return 16;
    if (dacName == "RING_OSC_3")
      return 16;
    if (dacName == "RING_OSC_4")
      return 16;
    if (dacName == "RING_OSC_5")
      return 16;
    if (dacName == "RING_OSC_6")
      return 16;
    if (dacName == "RING_OSC_7")
      return 16;
    if (dacName == "BCID_CNT")
      return 16;
    if (dacName == "TRIG_CNT")
      return 16;
    if (dacName == "LOCKLOSS_CNT")
      return 16;
    if (dacName == "BITFLIP_WNG_CNT")
      return 16;
    if (dacName == "BITFLIP_ERR_CNT")
      return 16;
    if (dacName == "CMDERR_CNT")
      return 16;
    if (dacName == "WNGFIFO_FULL_CNT_0")
      return 16;
    if (dacName == "WNGFIFO_FULL_CNT_1")
      return 16;
    if (dacName == "WNGFIFO_FULL_CNT_2")
      return 16;
    if (dacName == "WNGFIFO_FULL_CNT_3")
      return 16;
    if (dacName == "AI_REGION_COL")
      return 8;
    if (dacName == "AI_REGION_ROW")
      return 9;
    if (dacName == "HITOR_0_CNT")
      return 16;
    if (dacName == "HITOR_1_CNT")
      return 16;
    if (dacName == "HITOR_2_CNT")
      return 16;
    if (dacName == "HITOR_3_CNT")
      return 16;
    if (dacName == "SKIPPED_TRIGGER_CNT")
      return 16;
    if (dacName == "ERRWNG_MASK")
      return 14;
    if (dacName == "MONITORING_DATA_ADC")
      return 12;
    if (dacName == "SELF_TRIGGER_ENABLE")
      return 4;

    return 0;
  }

  bool RD53::IsChannelUnMasked (uint32_t cChan) const
  {
    int row, col;
    RD53::fromVec2Matrix(cChan,row,col);
    return fPixelsConfig[col].Enable[row];
  }

  RD53::EventHeader::EventHeader(const uint32_t data)
  {
    // mypause();
    uint32_t header;
    std::tie(header, trigger_id, trigger_tag, bc_id) = unpack_bits<NBIT_HEADER, NBIT_TRIGID, NBIT_TRGTAG, NBIT_BCID>(data);
    if (header != 1) {
      LOG (ERROR) << "Invalid RD53 Event Header." << RESET;
    }
  }
    
  RD53::HitData::HitData(const uint32_t data)
  {
    uint32_t core_col, side, all_tots;
    std::tie(core_col, row, side, all_tots) = unpack_bits<NBIT_CCOL, NBIT_ROW, NBIT_SIDE, NBIT_TOT>(data);
    
    unpack_array<NBIT_TOT / N_REGION>(tots, all_tots);
    
    col = 4 * pack_bits<NBIT_CCOL, NBIT_SIDE>(core_col, side);
  }
}
