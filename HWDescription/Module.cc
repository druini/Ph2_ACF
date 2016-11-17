/*!

        Filename :                              Module.cc
        Content :                               Module Description class
        Programmer :                    Lorenzo BIDEGAIN
        Version :               1.0
        Date of Creation :              25/06/14
        Support :                               mail to : lorenzo.bidegain@gmail.com

 */

#include "Module.h"

namespace Ph2_HwDescription {

    // Default C'tor
    Module::Module() : FrontEndDescription(), fModuleId ( 0 )
    {
    }

    Module::Module ( const FrontEndDescription& pFeDesc, uint8_t pModuleId ) : FrontEndDescription ( pFeDesc ), fModuleId ( pModuleId )
    {
    }

    Module::Module ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pModuleId ) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fModuleId ( pModuleId )
    {
    }


    bool Module::removeCbc ( uint8_t pCbcId )
    {
        std::vector < Cbc* > :: iterator i;
        bool found = false;

        for ( i = fCbcVector.begin(); i != fCbcVector.end(); ++i )
        {
            if ( (*i)->getCbcId() == pCbcId )
            {
                found = true;
                break;
            }
        }

        if ( found )
        {
            fCbcVector.erase ( i );
            return true;
        }
        else
        {
            LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the cbc " << +pCbcId ;
            return false;
        }
    }

    Cbc* Module::getCbc ( uint8_t pCbcId ) const
    {

        for ( Cbc* c : fCbcVector )
        {
            if ( c->getCbcId() == pCbcId )
                return c;
        }

        return nullptr;

    }
}
