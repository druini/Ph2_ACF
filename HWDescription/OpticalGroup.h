/*!

        \file                           OpticalGroup.h
        \brief                          OpticalGroup Description class
        \author                         Fabio Ravera
        \version                        1.0
        \date                           02/04/20
        Support :                       mail to : fabio.ravera@cern.ch

 */

#ifndef OpticalGroup_h__
#define OpticalGroup_h__

#include "FrontEndDescription.h"
// #include "RD53.h"
#include "Module.h"
#include "../Utils/Visitor.h"
#include <vector>
#include "../Utils/Container.h"

// FE Hybrid HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    /*!
     * \class OpticalGroup
     * \brief handles a vector of Chip which are connected to the OpticalGroup
     */
    class OpticalGroup : public FrontEndDescription, public OpticalGroupContainer
    {

      public:

        // C'tors take FrontEndDescription or hierachy of connection
        OpticalGroup (const FrontEndDescription& pFeDesc, uint8_t pOpticalGroupId );
        OpticalGroup (uint8_t pBeId, uint8_t pFMCId, uint8_t pOpticalGroupId );

        // Default C'tor
        OpticalGroup();

        // D'tor
        ~OpticalGroup()
        {
        };

        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visitOpticalGroup ( *this );

            for ( auto* cHybrid : *this )
                static_cast<Module*>(cHybrid)->accept ( pVisitor );
        }

        uint8_t getOpticalGroupId() const
        {
            return fOpticalGroupId;
        };

        void setOpticalGroupId ( uint8_t pOpticalGroupId )
        {
            fOpticalGroupId = pOpticalGroupId;
        };

      protected:
        uint8_t fOpticalGroupId;
    };
}

#endif
