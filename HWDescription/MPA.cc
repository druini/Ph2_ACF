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

    MPA::MPA ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pMPAId) : FrontEndDescription ( pBeId, pFMCId, pFeId ), fMPAId ( pMPAId )

    {}

    // Copy C'tor

    MPA::MPA ( const MPA& MPAobj ) : FrontEndDescription ( MPAobj ),
        fMPAId ( MPAobj.fMPAId )
    {
    }


    // D'Tor

    MPA::~MPA()
    {

    }

}
