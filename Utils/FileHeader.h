/*!

        \file                   FileHeader.h
        \brief                  class for binary Raw data file header with infor
        \author                 Georg AUZINGER
        \version                1.0
        \date                   09/06/16
        Support :               mail to : georg.auzinger@cern.ch

 */
#ifndef __FILEHEADER_H__
#define __FILEHEADER_H__

#include <iostream>
#include <vector>

/*!
 * \class FileHandler
 * \brief Class to write Data objects in binary file using multithreading
*/

class FileHeader
{
  public:
    bool fValid;
    // FW type
    std::string fType;
    //char fType[8];
    uint32_t fVersionMajor;
    uint32_t fVersionMinor;
    // Info about HW
    uint32_t fBeId;
    uint32_t fNCbc;
    //EventSize
    uint32_t fEventSize32;
    //Header Size useful for encoding and decoding
    static const uint32_t fHeaderSize32 = 12;

  public:
    FileHeader() :
        fType ( "" ),
        fValid (false),
        fVersionMajor (0),
        fVersionMinor (0),
        fBeId (0),
        fNCbc (0),
        fEventSize32 (0)
    {
    }

    FileHeader (const std::string pType, const uint32_t& pFWMajor, const uint32_t& pFWMinor, const uint32_t& pBeId, const uint32_t& pNCbc, const uint32_t& pEventSize32) :
        fVersionMajor (pFWMajor),
        fVersionMinor (pFWMinor),
        fBeId (pBeId),
        fNCbc (pNCbc),
        fEventSize32 (pEventSize32),
        fValid (true),
        fType (pType)
    {
        //strcpy (fType, pType.c_str() );
    }


    std::vector<uint32_t> encodeHeader()
    {
        std::vector<uint32_t> cVec;

        //surround every block with 10101....
        cVec.push_back (0xAAAAAAAA);
        //insert the header
        //2 words
        char cType[8] = {0};

        if (fType.size() < 9)
            strcpy (cType, fType.c_str() );
        else
            std::cout << "FileHeader: Error, type string can only be up to 8 characters long!" << std::endl;

        cVec.push_back (cType[0] << 24 | cType[1] << 16 | cType[2] << 8 | cType[3]);
        cVec.push_back (cType[4] << 24 | cType[5] << 16 | cType[6] << 8 | cType[7]);

        cVec.push_back (0xAAAAAAAA);
        // 2 words FW version
        cVec.push_back (fVersionMajor);
        cVec.push_back (fVersionMinor);

        cVec.push_back (0xAAAAAAAA);
        // 1 word w. BeBoardInfo: 10 LSBs: fBeId, ... to be filled as needed
        cVec.push_back (fBeId & 0x000003FF);
        // the number of CBCs
        cVec.push_back (fNCbc);

        cVec.push_back (0xAAAAAAAA);
        //1 word event size
        cVec.push_back (fEventSize32);
        cVec.push_back (0xAAAAAAAA);

        std::cout << "Board Type: " << fType << " FWMajor " << fVersionMajor << " FWMinor " << fVersionMinor << " BeId " << fBeId << " fNCbc " << fNCbc << " EventSize32  " << fEventSize32 << " valid: " << fValid << std::endl;
        return cVec;
    }

    void decodeHeader (const std::vector<uint32_t>& pVec)
    {
        uint32_t cMask = 0xAAAAAAAA;

        if (pVec.at (0) == cMask && pVec.at (3) == cMask && pVec.at (6) == cMask && pVec.at (9) == cMask && pVec.at (11) == cMask )
        {
            char cType[8] = {0};
            cType[0] = (pVec.at (1) && 0xFF000000) >> 24;
            cType[1] = (pVec.at (1) && 0x00FF0000) >> 16;
            cType[2] = (pVec.at (1) && 0x0000FF00) >> 8;
            cType[3] = (pVec.at (1) && 0x000000FF);

            cType[4] = (pVec.at (2) && 0xFF000000) >> 24;
            cType[5] = (pVec.at (2) && 0x00FF0000) >> 16;
            cType[6] = (pVec.at (2) && 0x0000FF00) >> 8;
            cType[7] = (pVec.at (2) && 0x000000FF);

            std::string cTypeString (cType);
            fType = cTypeString;

            fVersionMajor = pVec.at (4);
            fVersionMinor = pVec.at (5);

            fBeId = pVec.at (7) & 0x000003FF;
            fNCbc = pVec.at (8);

            fEventSize32 = pVec.at (10);
            fValid = true;
            std::cout << "Sucess, this is a valid header!" << std::endl;
            std::cout << "Board Type: " << fType << " FWMajor " << fVersionMajor << " FWMinor " << fVersionMinor << " BeId " << fBeId << " fNCbc " << fNCbc << " EventSize32  " << fEventSize32 << " valid: " << fValid << std::endl;
        }
        else
        {
            std::cout << "Error, this is not a valid header!" << std::endl;
            fValid = false;
        }
    }
};

#endif
