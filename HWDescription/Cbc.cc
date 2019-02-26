/*!

        Filename :                      Cbc.cc
        Content :                       Cbc Description class, config of the Cbcs
        Programmer :                    Lorenzo BIDEGAIN
        Version :                       1.0
        Date of Creation :              25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#include "Cbc.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string.h>
#include <iomanip>
#include "Definition.h"


namespace Ph2_HwDescription {
    // C'tors with object FE Description

    Cbc::Cbc ( const FrontEndDescription& pFeDesc, uint8_t pCbcId, const std::string& filename ) : FrontEndDescription ( pFeDesc ),
        fCbcId ( pCbcId )

    {
        loadfRegMap ( filename );

        // determine the chip type by checking for existence of VCth register (CBC2 only, called VCth1 & VCth2 for CBC3)
        if (fRegMap.find ("VCth2") != std::end (fRegMap) ) this->setChipType ( ChipType::CBC3);
        else this->setChipType ( ChipType::CBC2);
    }

    // C'tors which take BeId, FMCId, FeID, CbcId

    Cbc::Cbc ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pCbcId, const std::string& filename ) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fCbcId ( pCbcId )

    {
        loadfRegMap ( filename );

        // determine the chip type by checking for existence of VCth register (CBC2 only, called VCth1 & VCth2 for CBC3)
        if (fRegMap.find ("VCth2") != std::end (fRegMap) ) this->setChipType ( ChipType::CBC3);
        else this->setChipType ( ChipType::CBC2);
    }

    Cbc::Cbc ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pCbcId, const std::string& filename, ChipType pType ) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fCbcId ( pCbcId )

    {
        loadfRegMap ( filename );
        // fNumberOfChannels = 254;


        this->setChipType (pType);
    }

    // Copy C'tor

    Cbc::Cbc ( const Cbc& cbcobj ) : FrontEndDescription ( cbcobj ),
        fCbcId ( cbcobj.fCbcId ),
        fRegMap ( cbcobj.fRegMap ),
        fCommentMap (cbcobj.fCommentMap)
    {
    }


    // D'Tor

    Cbc::~Cbc()
    {

    }

    //load fRegMap from file

    void Cbc::loadfRegMap ( const std::string& filename )
    {
        std::ifstream file ( filename.c_str(), std::ios::in );

        if ( file )
        {
            std::string line, fName, fPage_str, fAddress_str, fDefValue_str, fValue_str;
            int cLineCounter = 0;
            ChipRegItem fRegItem;

            fAsMaskedChannels = false;
            while ( getline ( file, line ) )
            {
                if ( line.find_first_not_of ( " \t" ) == std::string::npos )
                {
                    fCommentMap[cLineCounter] = line;
                    cLineCounter++;
                    //continue;
                }

                else if ( line.at ( 0 ) == '#' || line.at ( 0 ) == '*' || line.empty() )
                {
                    //if it is a comment, save the line mapped to the line number so I can later insert it in the same place
                    fCommentMap[cLineCounter] = line;
                    cLineCounter++;
                    //continue;
                }
                else
                {
                    std::istringstream input ( line );
                    input >> fName >> fPage_str >> fAddress_str >> fDefValue_str >> fValue_str;

                    fRegItem.fPage = strtoul ( fPage_str.c_str(), 0, 16 );
                    fRegItem.fAddress = strtoul ( fAddress_str.c_str(), 0, 16 );
                    fRegItem.fDefValue = strtoul ( fDefValue_str.c_str(), 0, 16 );
                    fRegItem.fValue = strtoul ( fValue_str.c_str(), 0, 16 );

                    if(fRegItem.fPage==0x00 && fRegItem.fAddress>=0x20 && fRegItem.fAddress<=0x3F){ //Register is a Mask
                        fCbcMask32[(fRegItem.fAddress - 0x20)>>2] += fRegItem.fValue << (((fRegItem.fAddress - 0x20)&0x3)<<3);
                        fCbcMask[fRegItem.fAddress - 0x20] = fRegItem.fValue;
                        if(!fAsMaskedChannels && fRegItem.fValue!=0xFF) fAsMaskedChannels=true;
                    }

                    fRegMap[fName] = fRegItem;
                    cLineCounter++;
                }
            }

            file.close();

        }
        else
        {
            LOG (ERROR) << "The CBC Settings File " << filename << " does not exist!" ;
            exit (1);
        }

        //for (auto cItem : fRegMap)
        //LOG (DEBUG) << cItem.first;
    }


    uint8_t Cbc::getReg ( const std::string& pReg ) const
    {
        ChipRegMap::const_iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
        {
            LOG (INFO) << "The Cbc object: " << +fCbcId << " doesn't have " << pReg ;
            return 0;
        }
        else
            return i->second.fValue;
    }


    void Cbc::setReg ( const std::string& pReg, uint8_t psetValue )
    {
        ChipRegMap::iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
            LOG (INFO) << "The Cbc object: " << +fCbcId << " doesn't have " << pReg ;
        else
            i->second.fValue = psetValue;
    }

    ChipRegItem Cbc::getRegItem ( const std::string& pReg )
    {
        ChipRegItem cItem;
        ChipRegMap::iterator i = fRegMap.find ( pReg );

        if ( i != std::end ( fRegMap ) ) return ( i->second );
        else
        {
            LOG (ERROR) << "Error, no Register " << pReg << " found in the RegisterMap of CBC " << +fCbcId << "!" ;
            throw Exception ( "Cbc: no matching register found" );
            return cItem;
        }
    }


    //Write RegValues in a file

    void Cbc::saveRegMap ( const std::string& filename )
    {

        std::ofstream file ( filename.c_str(), std::ios::out | std::ios::trunc );

        if ( file )
        {
            std::set<CbcRegPair, RegItemComparer> fSetRegItem;

            for ( auto& it : fRegMap )
                fSetRegItem.insert ( {it.first, it.second} );

            int cLineCounter = 0;

            for ( const auto& v : fSetRegItem )
            {
                while (fCommentMap.find (cLineCounter) != std::end (fCommentMap) )
                {
                    auto cComment = fCommentMap.find (cLineCounter);

                    file << cComment->second << std::endl;
                    cLineCounter++;
                }

                file << v.first;

                for ( int j = 0; j < 48; j++ )
                    file << " ";

                file.seekp ( -v.first.size(), std::ios_base::cur );


                file << "0x" << std::setfill ( '0' ) << std::setw ( 2 ) << std::hex << std::uppercase << int ( v.second.fPage ) << "\t0x" << std::setfill ( '0' ) << std::setw ( 2 ) << std::hex << std::uppercase << int ( v.second.fAddress ) << "\t0x" << std::setfill ( '0' ) << std::setw ( 2 ) << std::hex << std::uppercase << int ( v.second.fDefValue ) << "\t0x" << std::setfill ( '0' ) << std::setw ( 2 ) << std::hex << std::uppercase << int ( v.second.fValue ) << std::endl;

                cLineCounter++;
            }

            file.close();
        }
        else
            LOG (ERROR) << "Error opening file" ;
    }




    bool CbcComparer::operator() ( const Cbc& cbc1, const Cbc& cbc2 ) const
    {
        if ( cbc1.getBeId() != cbc2.getBeId() ) return cbc1.getBeId() < cbc2.getBeId();
        else if ( cbc1.getFMCId() != cbc2.getFMCId() ) return cbc1.getFMCId() < cbc2.getFMCId();
        else if ( cbc1.getFeId() != cbc2.getFeId() ) return cbc1.getFeId() < cbc2.getFeId();
        else return cbc1.getChipId() < cbc2.getChipId();
    }


    bool RegItemComparer::operator() ( const CbcRegPair& pRegItem1, const CbcRegPair& pRegItem2 ) const
    {
        if ( pRegItem1.second.fPage != pRegItem2.second.fPage )
            return pRegItem1.second.fPage < pRegItem2.second.fPage;
        else return pRegItem1.second.fAddress < pRegItem2.second.fAddress;
    }

}
