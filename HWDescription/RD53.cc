/*!
  \file                  RD53.cc
  \brief                 RD53 implementation class, config of the RD53
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "RD53.h"
#include "../Utils/ChannelGroupHandler.h"

#include <unordered_map>

namespace Ph2_HwDescription
{
  RD53::RD53 (const FrontEndDescription& pFeDesc, uint8_t pRD53Id, const std::string& filename) : Chip (pFeDesc, pRD53Id)
  {
    loadfRegMap     (filename);
    setFrontEndType (FrontEndType::RD53);
    fChipOriginalMask = new ChannelGroup<NCOLS, NROWS>;
  }

  RD53::RD53 (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, const std::string& filename) : Chip (pBeId, pFMCId, pFeId, pRD53Id)
  {
    loadfRegMap     (filename);
    setFrontEndType (FrontEndType::RD53);
    fChipOriginalMask = new ChannelGroup<NCOLS, NROWS>;
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

  void RD53::enablePixel (unsigned int row, unsigned int col)
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

  void RD53::injectPixel (unsigned int row, unsigned int col)
  {
    fPixelsConfig[col].InjEn[row] = 1;
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
  
  void RD53::ConvertCores2Col4Row (uint16_t coreCol, uint16_t coreRowAndRegion, uint8_t side,
				   unsigned int& row, unsigned int& col)
  {
    row = coreRowAndRegion;
    col = 4 * ((coreCol << NBIT_SIDE) | side);
  }
  
  uint32_t RD53::getNumberOfChannels () const
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
    static const std::unordered_map<std::string, uint8_t> reg_length_map = {
      // #################
      // # Pixel Section #
      // #################
      {"PIX_PORTAL", 16},
      {"REGION_COL", 8},
      {"REGION_ROW", 8},
      {"PIX_MODE", 6},
      {"PIX_DEFAULT_CONFIG", 16},

      // #########################
      // # Synchronous Front End #
      // #########################
      {"IBIASP1_SYNC", 9},
      {"IBIASP2_SYNC", 9},
      {"IBIAS_SF_SYNC", 9},
      {"IBIAS_KRUM_SYNC", 9},
      {"IBIAS_DISC_SYNC", 9},
      {"ICTRL_SYNCT_SYNC", 10},
      {"VBL_SYNC", 10},
      {"VTH_SYNC", 10},
      {"VREF_KRUM_SYNC", 10},

      // ####################
      // # Linear Front End #
      // ####################
      {"PA_IN_BIAS_LIN", 9},
      {"FC_BIAS_LIN", 8},
      {"KRUM_CURR_LIN", 9},
      {"LDAC_LIN", 10},
      {"COMP_LIN", 9},
      {"REF_KRUM_LIN", 10},
      {"Vthreshold_LIN", 10},

      // ##########################
      // # Differential Front End #
      // ##########################
      {"PRMP_DIFF", 10},
      {"FOL_DIFF", 10},
      {"PRECOMP_DIFF", 10},
      {"COMP_DIFF", 10},
      {"VFF_DIFF", 10},
      {"VTH1_DIFF", 10},
      {"VTH2_DIFF", 10},
      {"LCC_DIFF", 10},

      // #######################
      // # Auxiliary Registers #
      // #######################
      {"CONF_FE_SYNC", 5},
      {"CONF_FE_DIFF", 2},
      {"VOLTAGE_TRIM", 10},

      // ##################
      // # Digital Matrix #
      // ##################
      {"EN_CORE_COL_SYNC", 16},
      {"EN_CORE_COL_LIN_1", 16},
      {"EN_CORE_COL_LIN_2", 1},
      {"EN_CORE_COL_DIFF_1", 16},
      {"EN_CORE_COL_DIFF_2", 1},
      {"LATENCY_CONFIG", 9},
      {"WR_SYNC_DELAY_SYNC", 5},

      // #############
      // # Injection #
      // #############
      {"INJECTION_SELECT", 6},
      {"CLK_DATA_DELAY", 9},
      {"VCAL_HIGH", 12},
      {"VCAL_MED", 12},
      {"CH_SYNC_CONF", 12},
      {"GLOBAL_PULSE_ROUTE", 16},
      {"MONITOR_FRAME_SKIP", 8},
      {"EN_MACRO_COL_CAL_SYNC_1", 16},
      {"EN_MACRO_COL_CAL_SYNC_2", 16},
      {"EN_MACRO_COL_CAL_SYNC_3", 16},
      {"EN_MACRO_COL_CAL_SYNC_4", 16},
      {"EN_MACRO_COL_CAL_LIN_1", 16},
      {"EN_MACRO_COL_CAL_LIN_2", 16},
      {"EN_MACRO_COL_CAL_LIN_3", 16},
      {"EN_MACRO_COL_CAL_LIN_4", 16},
      {"EN_MACRO_COL_CAL_LIN_5", 4},
      {"EN_MACRO_COL_CAL_DIFF_1", 16},
      {"EN_MACRO_COL_CAL_DIFF_2", 16},
      {"EN_MACRO_COL_CAL_DIFF_3", 16},
      {"EN_MACRO_COL_CAL_DIFF_4", 16},
      {"EN_MACRO_COL_CAL_DIFF_5", 4},

      // #######
      // # I/O #
      // #######
      {"DEBUG_CONFIG", 2},
      {"OUTPUT_CONFIG", 9},
      {"OUT_PAD_CONFIG", 14},
      {"GP_LVDS_ROUTE", 16},
      {"CDR_CONFIG", 14},
      {"CDR_VCO_BUFF_BIAS", 10},
      {"CDR_CP_IBIAS", 10},
      {"CDR_VCO_IBIAS", 10},
      {"SER_SEL_OUT", 8},    
      {"CML_CONFIG", 8},
      {"CML_TAP0_BIAS", 10},
      {"CML_TAP1_BIAS", 10},
      {"CML_TAP2_BIAS", 10},
      {"AURORA_CC_CONFIG", 8},
      {"AURORA_CB_CONFIG0", 8},
      {"AURORA_CB_CONFIG1", 16},
      {"AURORA_INIT_WAIT", 11},

      // #################################
      // # Test and Monitoring Functions #
      // #################################
      {"MONITOR_SELECT", 14},
      {"HITOR_0_MASK_SYNC", 16},
      {"HITOR_1_MASK_SYNC", 16},
      {"HITOR_2_MASK_SYNC", 16},
      {"HITOR_3_MASK_SYNC", 16},
      {"HITOR_0_MASK_LIN_0", 16},
      {"HITOR_0_MASK_LIN_1", 1},
      {"HITOR_1_MASK_LIN_0", 16},
      {"HITOR_1_MASK_LIN_1", 1},
      {"HITOR_2_MASK_LIN_0", 16},
      {"HITOR_2_MASK_LIN_1", 1},
      {"HITOR_3_MASK_LIN_0", 16},
      {"HITOR_3_MASK_LIN_1", 1},
      {"HITOR_0_MASK_DIFF_0", 16},
      {"HITOR_0_MASK_DIFF_1", 1},
      {"HITOR_1_MASK_DIFF_0", 16},
      {"HITOR_1_MASK_DIFF_1", 1},
      {"HITOR_2_MASK_DIFF_0", 16},
      {"HITOR_2_MASK_DIFF_1", 1},
      {"HITOR_3_MASK_DIFF_0", 16},
      {"HITOR_3_MASK_DIFF_1", 1},
      {"MONITOR_CONFIG", 11},
      {"SENSOR_CONFIG_0", 12},
      {"SENSOR_CONFIG_1", 12},
      {"AUTO_READ_0", 9},
      {"AUTO_READ_1", 9},
      {"AUTO_READ_2", 9},
      {"AUTO_READ_3", 9},
      {"AUTO_READ_4", 9},
      {"AUTO_READ_5", 9},
      {"AUTO_READ_6", 9},
      {"AUTO_READ_7", 9},
      {"RING_OSC_ENABLE", 8},
      {"RING_OSC_0", 16},
      {"RING_OSC_1", 16},
      {"RING_OSC_2", 16},
      {"RING_OSC_3", 16},
      {"RING_OSC_4", 16},
      {"RING_OSC_5", 16},
      {"RING_OSC_6", 16},
      {"RING_OSC_7", 16},
      {"BCID_CNT", 16},
      {"TRIG_CNT", 16},
      {"LOCKLOSS_CNT", 16},
      {"BITFLIP_WNG_CNT", 16},
      {"BITFLIP_ERR_CNT", 16},
      {"CMDERR_CNT", 16},
      {"WNGFIFO_FULL_CNT_0", 16},
      {"WNGFIFO_FULL_CNT_1", 16},
      {"WNGFIFO_FULL_CNT_2", 16},
      {"WNGFIFO_FULL_CNT_3", 16},
      {"AI_REGION_COL", 8},
      {"AI_REGION_ROW", 9},
      {"HITOR_0_CNT", 16},
      {"HITOR_1_CNT", 16},
      {"HITOR_2_CNT", 16},
      {"HITOR_3_CNT", 16},
      {"SKIPPED_TRIGGER_CNT", 16},
      {"ERRWNG_MASK", 14},
      {"MONITORING_DATA_ADC", 12},
      {"SELF_TRIGGER_ENABLE", 4}
    };

    auto it = reg_length_map.find(dacName);
    if (it != reg_length_map.end()) return it->second;
    return 0;
  }

  bool RD53::IsChannelUnMasked (uint32_t cChan) const
  {
    unsigned int row, col;
    RD53::fromVec2Matrix(cChan,row,col);
    return fPixelsConfig[col].Enable[row];
  }

  RD53::Event::Event(const uint32_t* data, size_t n)
  {
    uint32_t header;
    std::tie(header, trigger_id, trigger_tag, bc_id) = unpack_bits<NBIT_HEADER, NBIT_TRIGID, NBIT_TRGTAG, NBIT_BCID>(*data);
    if (header != 1) LOG (ERROR) << "Invalid RD53 event header" << RESET;
    for (size_t i = 1; i < n; i++)
      if (data[i]) this->data.emplace_back(data[i]);
  }
  
  RD53::HitData::HitData (const uint32_t data)
  {
    uint32_t core_col, side, all_tots;
    std::tie(core_col, row, side, all_tots) = unpack_bits<NBIT_CCOL, NBIT_ROW, NBIT_SIDE, NBIT_TOT>(data);
    
    unpack_array<NBIT_TOT / NPIX_REGION>(tots, all_tots);
    
    col = 4 * pack_bits<NBIT_CCOL, NBIT_SIDE>(core_col, side);
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
    return  pack_bits<NBIT_CHIPID,NBIT_CAL_EDGE_MODE,NBIT_CAL_EDGE_DELAY,NBIT_CAL_EDGE_WIDTH,NBIT_CAL_AUX_MODE,NBIT_CAL_AUX_DELAY>(chipId,
																   cal_edge_mode,
																   cal_edge_delay,
																   cal_edge_width,
																   cal_aux_mode,
																   cal_aux_delay);
  }

  std::vector<uint8_t>& RD53::getChipMask()
  {
    fChipMask.clear();
    std::vector<uint8_t> vec(NCOLS*NROWS/8, 0);
    fChipMask = vec;
    uint32_t chn;

    for (unsigned int col = 0; col < fPixelsConfig.size(); col++)
      for (unsigned int row = 0; row < fPixelsConfig[col].Enable.size(); row++)
	{
	  chn = RD53::fromMatrix2Vec(row,col);
	  fChipMask[chn/8] = fChipMask[chn/8] | (fPixelsConfig[col].Enable[row] << (chn % 8));
	}
    return fChipMask;
  }

  template<int NBITS>
  std::bitset<NBITS> RD53::SetBits (unsigned int nBit2Set)
  {
    std::bitset<NBITS> output(0);
    for (unsigned int i = 0; i < nBit2Set; i++) output[i] = 1;
    return output;
  }
}
