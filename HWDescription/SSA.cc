/*!

        Filename :                      SSA.cc
        Content :                       SSA Description class, config of the SSAs
        Programmer :                    Lorenzo BIDEGAIN
        Version :                       1.0
        Date of Creation :              25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#include "SSA.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <string.h>
#include <iomanip>
#include "Definition.h"


namespace Ph2_HwDescription {
    // C'tors with object FE Description

    SSA::SSA ( const FrontEndDescription& pFeDesc, uint8_t pSSAId, uint8_t) : FrontEndDescription ( pFeDesc ),
        fSSAId ( pSSAId )

    {}

    // C'tors which take BeId, FMCId, FeID, SSAId

    SSA::SSA ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pSSAId, uint8_t pSSASide, const std::string &filename) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fSSAId ( pSSAId )

    {  loadfRegMap ( filename );}

    // Copy C'tor

    SSA::SSA ( const SSA& SSAobj ) : FrontEndDescription ( SSAobj ),
        fSSAId ( SSAobj.fSSAId )
    {
    }

    void SSA::loadfRegMap ( const std::string& filename )
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
            LOG (ERROR) << "The SSA Settings File " << filename << " does not exist!" ;
            exit (1);
        }

        //for (auto cItem : fRegMap)
        //LOG (DEBUG) << cItem.first;
    }

    uint8_t SSA::getReg ( const std::string& pReg ) const
    {
        SSARegMap::const_iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
        {
            LOG (INFO) << "The SSA object: " << +fSSAId << " doesn't have " << pReg ;
            return 0;
        }
        else
            return i->second.fValue;
    }
    void SSA::setReg ( const std::string& pReg, uint8_t psetValue )
    {
        SSARegMap::iterator i = fRegMap.find ( pReg );

        if ( i == fRegMap.end() )
            LOG (INFO) << "The SSA object: " << +fSSAId << " doesn't have " << pReg ;
        else
            i->second.fValue = psetValue;
    }

    ChipRegItem SSA::getRegItem ( const std::string& pReg )
    {
        ChipRegItem cItem;
        SSARegMap::iterator i = fRegMap.find ( pReg );

        if ( i != std::end ( fRegMap ) ) return ( i->second );
        else
        {
            LOG (ERROR) << "Error, no Register " << pReg << " found in the RegisterMap of SSA " << +fSSAId << "!" ;
            throw Exception ( "SSA: no matching register found" );
            return cItem;
        }
    }

    void SSA::CheckRegVals()
	{

		LOG (INFO) << "Checking values currently written to registers: ";
	
	}

    // D'Tor

    SSA::~SSA()
    {

    }

}
