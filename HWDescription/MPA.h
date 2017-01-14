/*!

        \file                   MPA.h
        \brief                  MPA Description class, config of the MPAs
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef MPA_h__
#define MPA_h__

#include "FrontEndDescription.h"
#include "CbcRegItem.h"
#include "../Utils/Visitor.h"
#include "../Utils/Exception.h"
#include "../Utils/easylogging++.h"
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <utility>
#include <set>

// MPA2 Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {


    class MPA : public FrontEndDescription
    {

      public:

        // C'tors which take BeId, FMCId, FeID, MPAId
        MPA ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pMPAId);

        // C'tors with object FE Description
        MPA ( const FrontEndDescription& pFeDesc, uint8_t pMPAId);

        // Default C'tor
        MPA();

        // Copy C'tor
        MPA ( const MPA& MPAobj );

        // D'Tor
        ~MPA();

        uint8_t getMPAId() const
        {
            return fMPAId;
        }
        /*!
         * \brief Set the MPA Id
         * \param pMPAId
         */
        void setMPAId ( uint8_t pMPAId )
        {
            fMPAId = pMPAId;
        }


      protected:

        uint8_t fMPAId;


    };


}

#endif
