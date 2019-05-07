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
    Module::Module()
    : ModuleContainer    (0)
    , FrontEndDescription( )
    , fModuleId          (0)
    {
    }

    Module::Module (const FrontEndDescription& pFeDesc, uint8_t pModuleId)
    : ModuleContainer    (pModuleId)
    , FrontEndDescription(pFeDesc  )
    , fModuleId          (pModuleId)
    {
    }

    Module::Module (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pModuleId)
    : ModuleContainer    (pModuleId)
    , FrontEndDescription(pBeId, pFMCId, pFeId)
    , fModuleId          (pModuleId)
    {
    }


    bool Module::removeChip ( uint8_t pChipId )
    {
        std::vector < Chip* > :: iterator chipIt;
        bool found = false;

        for ( chipIt = fChipVector.begin(); chipIt != fChipVector.end(); chipIt++ )
        {
            if ( (*chipIt)->getChipId() == pChipId )
            {
                found = true;
                break;
            }
        }

        if ( found )
        {
            fChipVector.erase ( chipIt );
            return true;
        }
        else
        {
            LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the chip " << +pChipId ;
            return false;
        }
    }

    Chip* Module::getChip ( uint8_t pChipId ) const
    {

        for ( Chip* chip : fChipVector )
        {
            if ( chip->getChipId() == pChipId )
                return chip;
        }

        return nullptr;

    }


    MPA* Module::getMPA ( uint8_t pMPAId ) const
    {

        for ( MPA* m : fMPAVector )
        {
            if ( m->getMPAId() == pMPAId )
                return m;
        }

        return nullptr;

    }



    bool Module::removeMPA ( uint8_t pMPAId )
    {
        std::vector < MPA* > :: iterator i;
        bool found = false;

        for ( i = fMPAVector.begin(); i != fMPAVector.end(); ++i )
        {
            if ( (*i)->getMPAId() == pMPAId )
            {
                found = true;
                break;
            }
        }

        if ( found )
        {
            fMPAVector.erase ( i );
            return true;
        }
        else
        {
            LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the MPA " << +pMPAId ;
            return false;
        }
    }


//   // #################
//   // # RD53 specific #
//   // #################  
//   bool Module::removeRD53 (uint8_t pRD53Id)
//   {
//     std::vector < RD53* > :: iterator i;
//     bool found = false;
    
//     for (i = fRD53Vector.begin(); i != fRD53Vector.end(); ++i)
//       {
// 	if ((*i)->getRD53Id() == pRD53Id)
// 	  {
// 	    found = true;
// 	    break;
// 	  }
//       }
    
//     if (found)
//       {
// 	fRD53Vector.erase (i);
// 	return true;
//       }
//     else
//       {
// 	LOG (INFO) << "Error:The Module " << +fModuleId << " doesn't have the RD53 " << +pRD53Id;
// 	return false;
//       }
//   }
  
//   RD53* Module::getRD53 (uint8_t pRD53Id) const
//   {
//     for (RD53* c : fRD53Vector)
//       {
// 	if (c->getRD53Id() == pRD53Id)
// 	  return c;
//       }
    
//     return nullptr;
//   }
//   // #################  
}
