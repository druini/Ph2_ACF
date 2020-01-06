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
  RD53::RD53 (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pRD53Id, uint8_t pRD53Lane, const std::string& fileName)
    : ReadoutChip (pBeId, pFMCId, pFeId, pRD53Id)
  {
    fMaxRegValue      = RD53::setBits(RD53Constants::NBIT_MAXREG);
    fChipOriginalMask = new ChannelGroup<nRows, nCols>;
    configFileName    = fileName;
    loadfRegMap(configFileName);
    setFrontEndType(FrontEndType::RD53);
    myChipLane = pRD53Lane;
  }

  RD53::RD53 (const RD53& chipObj) : ReadoutChip (chipObj) {}

  void RD53::loadfRegMap (const std::string& fileName)
  {
    std::ifstream     file (fileName.c_str(), std::ios::in);
    std::stringstream myString;
    perPixelData      pixData;

    if (file.good() == true)
      {
        std::string line, fName, fAddress_str, fDefValue_str, fValue_str, fBitSize_str;
        bool foundPixelConfig = false;
        int cLineCounter      = 0;
        unsigned int col      = 0;
        ChipRegItem fRegItem;

        while (getline (file, line))
          {
            if (line.find_first_not_of (" \t") == std::string::npos || line.at (0) == '#' || line.at (0) == '*' || line.empty()) myCommentMap[cLineCounter] = line;
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
                    LOG (ERROR) << BOLDRED << "Unknown base " << BOLDYELLOW << fDefValue_str << RESET;
                    throw Exception ("[RD53::loadfRegMap]\tError, unknown base");
                  }
                fDefValue_str.erase(0,2);
                fRegItem.fDefValue = strtoul (fDefValue_str.c_str(), 0, baseType);

                if      (fValue_str.compare(0,2,"0x") == 0) baseType = 16;
                else if (fValue_str.compare(0,2,"0d") == 0) baseType = 10;
                else if (fValue_str.compare(0,2,"0b") == 0) baseType = 2;
                else
                  {
                    LOG (ERROR) << BOLDRED << "Unknown base " << BOLDYELLOW << fValue_str << RESET;
                    throw Exception ("[RD53::loadfRegMap]\tError, unknown base");
                  }

                fValue_str.erase(0,2);
                fRegItem.fValue = strtoul (fValue_str.c_str(), 0, baseType);

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
        LOG (ERROR) << BOLDRED << "The RD53 file settings " << BOLDYELLOW << fileName << BOLDRED << " does not exist" << RESET;
        exit (EXIT_FAILURE);
      }
  }

  void RD53::saveRegMap (const std::string& fName2Add)
  {
    const int Nspaces = 26;

    std::string output = RD53::composeFileName(configFileName,fName2Add);
    std::ofstream file (output.c_str(), std::ios::out | std::ios::trunc);

    if (file)
      {
        std::set<ChipRegPair, RegItemComparer> fSetRegItem;
        for (const auto& it : fRegMap)
          fSetRegItem.insert ({it.first, it.second});

        int cLineCounter = 0;
        for (const auto& v : fSetRegItem)
          {
            while (myCommentMap.find (cLineCounter) != std::end (myCommentMap))
              {
                auto cComment = myCommentMap.find (cLineCounter);

                file << cComment->second << std::endl;
                cLineCounter++;
              }

            file << v.first;
            for (auto j = 0; j < Nspaces; j++)
              file << " ";
            file.seekp (-v.first.size(), std::ios_base::cur);
            file << "0x" << std::setfill ('0') << std::setw (2) << std::hex << std::uppercase << int (v.second.fAddress)
                 << "          0x" << std::setfill ('0') << std::setw (4) << std::hex << std::uppercase << int (v.second.fDefValue)
                 << "                  0x" << std::setfill ('0') << std::setw (4) << std::hex << std::uppercase << int (v.second.fValue)
                 << "                             " << std::setfill ('0') << std::setw (2) << std::dec << std::uppercase << int (v.second.fBitSize) << std::endl;

            cLineCounter++;
          }

        file << std::dec << std::endl;
        file << "*-------------------------------------------------------------------------------------------------------" << std::endl;
        file << "PIXELCONFIGURATION" << std::endl;
        file << "*-------------------------------------------------------------------------------------------------------" << std::endl;
        for (auto i = 0u; i < fPixelsMask.size(); i++)
          {
            file << "COL                  " << std::setfill ('0') << std::setw (3) << i << std::endl;

            file << "ENABLE " << fPixelsMask[i].Enable[0];
            for (auto j = 1u; j < fPixelsMask[i].Enable.size(); j++)
              file << "," << fPixelsMask[i].Enable[j];
            file << std::endl;

            file << "HITBUS " << fPixelsMask[i].HitBus[0];
            for (auto j = 1u; j < fPixelsMask[i].HitBus.size(); j++)
              file << "," << fPixelsMask[i].HitBus[j];
            file << std::endl;

            file << "INJEN  " << fPixelsMask[i].InjEn[0];
            for (auto j = 1u; j < fPixelsMask[i].InjEn.size(); j++)
              file << "," << fPixelsMask[i].InjEn[j];
            file << std::endl;

            file << "TDAC   " << unsigned(fPixelsMask[i].TDAC[0]);
            for (auto j = 1u; j < fPixelsMask[i].TDAC.size(); j++)
              file << "," << unsigned(fPixelsMask[i].TDAC[j]);
            file << std::endl;

            file << std::endl;
          }

        file.close();
      }
    else
      LOG (ERROR) << BOLDRED << "Error opening file " << BOLDYELLOW << output << RESET;
  }

  void RD53::copyMaskFromDefault()
  {
    for (auto i = 0u; i < fPixelsMask.size(); i++)
      {
        fPixelsMask[i].Enable = fPixelsMaskDefault[i].Enable;
        fPixelsMask[i].HitBus = fPixelsMaskDefault[i].HitBus;
        fPixelsMask[i].InjEn = fPixelsMaskDefault[i].InjEn;
        for (auto j = 0u; j < fPixelsMask[i].TDAC.size(); j++) fPixelsMask[i].TDAC[j] = fPixelsMaskDefault[i].TDAC[j];
      }
  }

  void RD53::copyMaskToDefault()
  {
    for (auto i = 0u; i < fPixelsMaskDefault.size(); i++)
      {
        fPixelsMaskDefault[i].Enable = fPixelsMask[i].Enable;
        fPixelsMaskDefault[i].HitBus = fPixelsMask[i].HitBus;
        fPixelsMaskDefault[i].InjEn = fPixelsMask[i].InjEn;
        for (auto j = 0u; j < fPixelsMaskDefault[i].TDAC.size(); j++) fPixelsMaskDefault[i].TDAC[j] = fPixelsMask[i].TDAC[j];
      }
  }

  void RD53::resetMask()
  {
    for (auto i = 0u; i < fPixelsMask.size(); i++)
      {
        fPixelsMask[i].Enable.reset();
        fPixelsMask[i].HitBus.reset();
        fPixelsMask[i].InjEn.reset();
        for (auto j = 0u; j < fPixelsMask[i].TDAC.size(); j++) fPixelsMask[i].TDAC[j] = RD53::setBits(RD53EvtEncoder::NBIT_TOT / RD53Constants::NPIX_REGION) / 2;
      }
  }

  void RD53::enableAllPixels()
  {
    for (auto i = 0u; i < fPixelsMask.size(); i++)
      {
        fPixelsMask[i].Enable.set();
        fPixelsMask[i].HitBus.set();
      }
  }

  void RD53::disableAllPixels()
  {
    for (auto i = 0u; i < fPixelsMask.size(); i++)
      {
        fPixelsMask[i].Enable.reset();
        fPixelsMask[i].HitBus.reset();
      }
  }

  size_t RD53::getNbMaskedPixels()
  {
    size_t cnt = 0;

    for (auto i = 0u; i < fPixelsMask.size(); i++)
      for (auto j = 0u; j < fPixelsMask[i].Enable.size(); j++)
        if (fPixelsMask[i].Enable[j] == 0) cnt++;

    return cnt;
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

  uint8_t RD53::getTDAC (unsigned int row, unsigned int col)
  {
    return fPixelsMask[col].TDAC[row];
  }

  uint32_t RD53::getNumberOfChannels() const
  {
    return nRows * nCols;
  }

  bool RD53::isDACLocal (const std::string& regName)
  {
    if (regName != "PIX_PORTAL") return false;
    return true;
  }

  uint8_t RD53::getNumberOfBits (const std::string& regName)
  {
    auto it = fRegMap.find(regName);
    if (it == fRegMap.end()) return 0;
    return it->second.fBitSize;
  }

  void RD53::Event::DecodeQuad (uint32_t data)
  {
    uint32_t core_col, side, row, col, all_tots;

    std::tie(core_col, row, side, all_tots) = bits::unpack<RD53EvtEncoder::NBIT_CCOL, RD53EvtEncoder::NBIT_ROW, RD53EvtEncoder::NBIT_SIDE, RD53EvtEncoder::NBIT_TOT>(data);
    col                                     = RD53Constants::NPIX_REGION * bits::pack<RD53EvtEncoder::NBIT_CCOL, RD53EvtEncoder::NBIT_SIDE>(core_col, side);

    uint8_t tots[RD53Constants::NPIX_REGION];
    bits::RangePacker<RD53EvtEncoder::NBIT_TOT / RD53Constants::NPIX_REGION>::unpack_reverse(all_tots, tots);

    for (int i = 0; i < RD53Constants::NPIX_REGION; i++) if (tots[i] != RD53::setBits(RD53EvtEncoder::NBIT_TOT / RD53Constants::NPIX_REGION)) hit_data.emplace_back(row, col + i, tots[i]);
    if ((row >= RD53::nRows) || (col >= (RD53::nCols - (RD53Constants::NPIX_REGION-1)))) evtStatus |= RD53EvtEncoder::CHIPPIX;
  }

  RD53::Event::Event (const uint32_t* data, size_t n)
  {
    uint32_t header;

    evtStatus = RD53EvtEncoder::CHIPGOOD;

    std::tie(header, trigger_id, trigger_tag, bc_id) = bits::unpack<RD53EvtEncoder::NBIT_HEADER, RD53EvtEncoder::NBIT_TRIGID, RD53EvtEncoder::NBIT_TRGTAG, RD53EvtEncoder::NBIT_BCID>(*data);
    if (header != RD53EvtEncoder::HEADER) evtStatus |= RD53EvtEncoder::CHIPHEAD;

    const size_t noHitToT = RD53::setBits(RD53EvtEncoder::NBIT_TOT);
    for (auto i = 1u; i < n; i++) if (data[i] != noHitToT) DecodeQuad(data[i]);
    // #######################################################
    // # If the number of 32bit words do not make an integer #
    // # number of 128bit words, then 0x0000FFFF words are   #
    // # added to the event                                  #
    // #######################################################
    if (n == 1) evtStatus |= RD53EvtEncoder::CHIPNOHIT;
  }

  RD53::CalCmd::CalCmd (const uint8_t& cal_edge_mode,
                        const uint8_t& cal_edge_delay,
                        const uint8_t& cal_edge_width,
                        const uint8_t& cal_aux_mode,
                        const uint8_t& cal_aux_delay)
    : cal_edge_mode  (cal_edge_mode)
    , cal_edge_delay (cal_edge_delay)
    , cal_edge_width (cal_edge_width)
    , cal_aux_mode   (cal_aux_mode)
    , cal_aux_delay  (cal_aux_delay)
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
    return bits::pack<4, 1 , 3, 6, 1, 5>(chipId,
                                         cal_edge_mode,
                                         cal_edge_delay,
                                         cal_edge_width,
                                         cal_aux_mode,
                                         cal_aux_delay);
  }
}


// ###############################
// # RD53 command base functions #
// ###############################
namespace RD53Cmd
{
  GlobalPulse::GlobalPulse (uint8_t chip_id, uint8_t data)
  {
    fields[0] = packAndEncode<4, 1>(chip_id, 0);
    fields[1] = packAndEncode<4, 1>(data, 0);
  }

  Cal::Cal (uint8_t chip_id, bool cal_edge_mode, uint8_t cal_edge_delay, uint8_t cal_edge_width, bool cal_aux_mode, uint8_t cal_aux_delay)
  {
    fields[0] = packAndEncode<4, 1>(chip_id, cal_edge_mode);
    fields[1] = packAndEncode<3, 2>(cal_edge_delay, cal_edge_width >> 4);
    fields[2] = packAndEncode<4, 1>(cal_edge_width, cal_aux_mode);
    fields[3] = packAndEncode<5   >(cal_aux_delay);
  }

  WrReg::WrReg (uint8_t chip_id, uint16_t address, uint16_t value)
  {
    fields[0] = packAndEncode<4, 1>(chip_id, 0);
    fields[1] = packAndEncode<5   >(address >> 4);
    fields[2] = packAndEncode<4, 1>(address, value >> 15);
    fields[3] = packAndEncode<5   >(value >> 10);
    fields[4] = packAndEncode<5   >(value >> 5);
    fields[5] = packAndEncode<5   >(value);
  }

  WrRegLong::WrRegLong (uint8_t chip_id, uint16_t address, const std::vector<uint16_t>& values)
  {
    fields[0] = packAndEncode<4, 1>(chip_id, 1);
    fields[1] = packAndEncode<5   >(address >> 4);
    fields[2] = packAndEncode<4, 1>(address, values[0] >> 15);
    fields[3] = packAndEncode<5   >(values[0] >> 10);
    fields[4] = packAndEncode<5   >(values[0] >> 5);
    fields[5] = packAndEncode<5   >(values[0]);

    bits::unpack_range<5>(values.begin() + 1, values.end(), fields.begin() + 6);
    for (auto i = 6u; i < fields.size(); i++) fields[i] = map5to8bit[fields[i]];
  }

  RdReg::RdReg (uint8_t chip_id, uint16_t address)
  {
    fields[0] = packAndEncode<4, 1>(chip_id, 0);
    fields[1] = packAndEncode<5   >(address >> 4);
    fields[2] = packAndEncode<4, 1>(address, 0);
    fields[3] = packAndEncode<5   >(0);
  }
}
