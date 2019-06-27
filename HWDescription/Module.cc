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

}