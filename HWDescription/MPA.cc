/*!

        Filename :                      MPA.cc
        Content :                       MPA Description class, config of the MPAs
        Programmer :                    Lorenzo BIDEGAIN
        Version :                       1.0
        Date of Creation :              25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#include "MPA.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string.h>
#include <iomanip>
#include "Definition.h"


namespace Ph2_HwDescription {
    // C'tors with object FE Description

    MPA::MPA ( const FrontEndDescription& pFeDesc, uint8_t pMPAId ) : FrontEndDescription ( pFeDesc ),
        fMPAId ( pMPAId )

    {}

    // C'tors which take BeId, FMCId, FeID, MPAId

    MPA::MPA (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pMPAId, const std::string &filename) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fMPAId ( pMPAId )
    {
        loadfRegMap ( filename );
        this->setFrontEndType (FrontEndType::MPA);
    }

    // Copy C'tor

    MPA::MPA ( const MPA& MPAobj ) : FrontEndDescription ( MPAobj ),
        fMPAId ( MPAobj.fMPAId )
    {
    }


    // D'Tor

    MPA::~MPA()
    {

    }



    //load fRegMap from file

    void MPA::loadfRegMap ( const std::string& filename )
    {
        std::ifstream file ( filename.c_str(), std::ios::in );

        if ( file )
        {
            std::string line, fName, fPage_str, fAddress_str, fDefValue_str, fValue_str;
            int cLineCounter = 0;
            ChipRegItem fRegItem;

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

                    fRegMap[fName] = fRegItem;
                    cLineCounter++;
                }
            }

            file.close();
        }
        else
        {
            LOG (ERROR) << "The MPA Settings File " << filename << " does not exist!" ;
            exit (1);
        }

        //for (auto cItem : fRegMap)
        //LOG (DEBUG) << cItem.first;
    }



    uint8_t MPA::getReg ( const std::string& pReg ) const
    {
        MPARegMap::const_iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
        {
            LOG (INFO) << "The MPA object: " << +fMPAId << " doesn't have " << pReg ;
            return 0;
        }
        else
            return i->second.fValue;
    }


    void MPA::setReg ( const std::string& pReg, uint8_t psetValue )
    {
        MPARegMap::iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
            LOG (INFO) << "The MPA object: " << +fMPAId << " doesn't have " << pReg ;
        else
            i->second.fValue = psetValue;
    }

    ChipRegItem MPA::getRegItem ( const std::string& pReg )
    {
        ChipRegItem cItem;
        MPARegMap::iterator i = fRegMap.find ( pReg );

        if ( i != std::end ( fRegMap ) ) return ( i->second );
        else
        {
            LOG (ERROR) << "Error, no Register " << pReg << " found in the RegisterMap of MPA " << +fMPAId << "!" ;
            throw Exception ( "MPA: no matching register found" );
            return cItem;
        }
    }


    //Write RegValues in a file

    void MPA::saveRegMap ( const std::string& filename )
    {

        std::ofstream file ( filename.c_str(), std::ios::out | std::ios::trunc );

        if ( file )
        {
            std::set<MPARegPair, MPARegItemComparer> fSetRegItem;

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


    bool MPARegItemComparer::operator() ( const MPARegPair& pRegItem1, const MPARegPair& pRegItem2 ) const
    {
        if ( pRegItem1.second.fPage != pRegItem2.second.fPage )
            return pRegItem1.second.fPage < pRegItem2.second.fPage;
        else return pRegItem1.second.fAddress < pRegItem2.second.fAddress;
    }



}
