/*!

  \file                   FileHeader.h
  \brief                  class for binary Raw data file header with infor
  \author                 Georg AUZINGER
  \version                1.0
  \date                   09/06/16
  Support :               mail to : georg.auzinger@cern.ch

*/

#ifndef FILEHEADER_H
#define FILEHEADER_H

#include "../HWDescription/Definition.h"
#include "../Utils/ConsoleColor.h"
#include "easylogging++.h"

#include <iostream>
#include <vector>


class FileHeader
{
 public:
  std::string fType;
  uint32_t fVersionMajor;
  uint32_t fVersionMinor;
  uint32_t fBeId;
  uint32_t fNCbc;
  uint32_t fEventSize32;
  static const uint32_t fHeaderSize32 = 12;
  EventType fEventType;
  bool fValid;

 public:
 FileHeader()
   : fType         ( "" )
   , fVersionMajor (0)
   , fVersionMinor (0)
   , fBeId         (0)
   , fNCbc         (0)
   , fEventSize32  (0)
   , fEventType    (EventType::VR)
   , fValid        (false)
   {}

  FileHeader (const std::string pType, const uint32_t& pFWMajor, const uint32_t& pFWMinor, const uint32_t& pBeId, const uint32_t& pNCbc, const uint32_t& pEventSize32, EventType pEventType = EventType::VR)
    : fType         (pType)
    , fVersionMajor (pFWMajor)
    , fVersionMinor (pFWMinor)
    , fBeId         (pBeId)
    , fNCbc         (pNCbc)
    , fEventSize32  (pEventSize32)
    , fEventType    (pEventType)
    , fValid        (true)
    {}

  BoardType getBoardType()
  {
    if      (fType == "D19C") return BoardType::D19C;
    else if (fType == "FC7")  return BoardType::FC7;
    else return BoardType::D19C;
  }

  std::vector<uint32_t> encodeHeader()
    {
      std::vector<uint32_t> cVec;

      // Surround every block with 10101...
      cVec.push_back (0xAAAAAAAA);
      char cType[8] = {0};

      if (fType.size() < 9) strcpy (cType, fType.c_str());
      else LOG (INFO) << "FileHeader: Error, type string can only be up to 8 characters long!";

      cVec.push_back(cType[0] << 24 | cType[1] << 16 | cType[2] << 8 | cType[3]);
      cVec.push_back(cType[4] << 24 | cType[5] << 16 | cType[6] << 8 | cType[7]);

      cVec.push_back(0xAAAAAAAA);
      cVec.push_back(fVersionMajor);
      cVec.push_back(fVersionMinor);

      cVec.push_back(0xAAAAAAAA);
      // 1 word w. BeBoardInfo: 2MSBs: EventType (VR or ZS), 10 LSBs: fBeId, ... to be filled as needed
      cVec.push_back((uint32_t (fEventType) & 0x3) << 30 | (fBeId & 0x000003FF));
      cVec.push_back(fNCbc);

      cVec.push_back(0xAAAAAAAA);
      cVec.push_back(fEventSize32);

      cVec.push_back (0xAAAAAAAA);

      std::string cEventTypeString;

      if (fEventType == EventType::VR) cEventTypeString = "EventType::VR" ;
      else                             cEventTypeString = "EventType::ZS";

      return cVec;
    }

  void decodeHeader (const std::vector<uint32_t>& pVec)
  {
    uint32_t cMask = 0xAAAAAAAA;

    if (pVec.at(0) == cMask && pVec.at(3) == cMask && pVec.at(6) == cMask && pVec.at(9) == cMask && pVec.at(11) == cMask)
      {
        char cType[8] = {0};
        cType[0] = (pVec.at (1) & 0xFF000000) >> 24;
        cType[1] = (pVec.at (1) & 0x00FF0000) >> 16;
        cType[2] = (pVec.at (1) & 0x0000FF00) >> 8;
        cType[3] = (pVec.at (1) & 0x000000FF);

        cType[4] = (pVec.at (2) & 0xFF000000) >> 24;
        cType[5] = (pVec.at (2) & 0x00FF0000) >> 16;
        cType[6] = (pVec.at (2) & 0x0000FF00) >> 8;
        cType[7] = (pVec.at (2) & 0x000000FF);

        std::string cTypeString (cType);
        fType = cTypeString;

        fVersionMajor = pVec.at (4);
        fVersionMinor = pVec.at (5);

        uint32_t cEventTypeId = (pVec.at (7) & 0xC0000000) >> 30;
        fBeId = pVec.at (7) & 0x000003FF;
        fNCbc = pVec.at (8);

        fEventSize32 = pVec.at (10);
        fValid = true;

        if      (cEventTypeId == 0) fEventType = EventType::VR;
        else if (cEventTypeId == 1) fEventType = EventType::ZS;
        else                        fEventType = EventType::VR;

        std::string cEventTypeString;

        if (fEventType == EventType::VR) cEventTypeString = "EventType::VR" ;
        else                             cEventTypeString = "EventType::ZS";

        LOG (INFO) << BOLDGREEN <<"Sucess, this is a valid header!" << RESET;
        LOG (INFO) << "Board Type: " << fType << " FWMajor " << fVersionMajor << " FWMinor " << fVersionMinor << " Event Type: " << cEventTypeString << " BeId " << fBeId << " fNCbc " << fNCbc << " EventSize32  " << fEventSize32 << " valid: " << fValid;
      }
    else
      {
        LOG (INFO) << BOLDRED << "Error, this is not a valid header!" << RESET;
        fValid = false;
      }
  }
};

#endif
