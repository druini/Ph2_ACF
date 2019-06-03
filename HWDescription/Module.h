/*!

        \file                           Module.h
        \brief                          Module Description class
        \author                         Lorenzo BIDEGAIN
        \version                        1.0
        \date                           25/06/14
        Support :                       mail to : lorenzo.bidegain@gmail.com

 */

#ifndef Module_h__
#define Module_h__

#include "FrontEndDescription.h"
// #include "RD53.h"
#include "ReadoutChip.h"
#include "Chip.h"
#include "MPA.h"
#include "SSA.h"
#include "../Utils/Visitor.h"
#include "../Utils/easylogging++.h"
#include <vector>
#include <stdint.h>
#include "../Utils/Container.h"

// FE Hybrid HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    /*!
     * \class Module
     * \brief handles a vector of Chip which are connected to the Module
     */
    class Module : public FrontEndDescription, public ModuleContainer
    {

      public:

        // C'tors take FrontEndDescription or hierachy of connection
        Module (const FrontEndDescription& pFeDesc, uint8_t pModuleId );
        Module (uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pModuleId );

        // Default C'tor
        Module();

        // D'tor
        ~Module()
        {
        };

        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );

            for ( Chip* cChip : fChipVector )
                cChip->accept ( pVisitor );
        }
        /*!
        * \brief Get the number of Chip connected to the Module
        * \return The size of the vector
        */
        uint8_t getNChip() const
        {
            return fChipVector.size();
        }


        uint8_t getNMPA() const
        {
            return fMPAVector.size();
        }

        uint8_t getNSSA() const
        {
            return fSSAVector.size();
        }

        /*!
         * \brief Adding a Chip to the vector
         * \param pChip
         */
        void addChip ( Chip& pChip )
        {
            //get the FrontEndType of the Chip and set the module one accordingly
            //this is the case when no chip type has been set so get the one from the Chip
            if (fType == FrontEndType::UNDEFINED)
                fType = pChip.getFrontEndType();
            //else, the chip type has already been set - if it is different from another Chip, rais a warning
            //no different chips should be on a module
            else if (fType != pChip.getFrontEndType() )
            {
                LOG (ERROR) << "Error, Chips of a module should not be of different type! - aborting";
                exit (1);
            }

            fChipVector.push_back ( &pChip );
        }
        void addChip ( Chip* pChip )
        {
            //get the FrontEndType of the Chip and set the module one accordingly
            //this is the case when no chip type has been set so get the one from the Chip
            if (fType == FrontEndType::UNDEFINED)
                fType = pChip->getFrontEndType();
            //else, the chip type has already been set - if it is different from another Chip, rais a warning
            //no different chips should be on a module
            else if (fType != pChip->getFrontEndType() )
            {
                LOG (ERROR) << "Error, Chips of a module should not be of different type! - aborting";
                exit (1);
            }

            fChipVector.push_back ( pChip );
        }
        
        void addReadoutChip ( ReadoutChip& pChip )
        {
            //get the FrontEndType of the Chip and set the module one accordingly
            //this is the case when no chip type has been set so get the one from the Chip
            if (fType == FrontEndType::UNDEFINED)
                fType = pChip.getFrontEndType();
            //else, the chip type has already been set - if it is different from another Chip, rais a warning
            //no different chips should be on a module
            else if (fType != pChip.getFrontEndType() )
            {
                LOG (ERROR) << "Error, Chips of a module should not be of different type! - aborting";
                exit (1);
            }

            fReadoutChipVector.push_back ( &pChip );
        }
        void addReadoutChip ( ReadoutChip* pChip )
        {
            //get the FrontEndType of the Chip and set the module one accordingly
            //this is the case when no chip type has been set so get the one from the Chip
            if (fType == FrontEndType::UNDEFINED)
                fType = pChip->getFrontEndType();
            //else, the chip type has already been set - if it is different from another Chip, rais a warning
            //no different chips should be on a module
            else if (fType != pChip->getFrontEndType() )
            {
                LOG (ERROR) << "Error, Chips of a module should not be of different type! - aborting";
                exit (1);
            }

            fReadoutChipVector.push_back ( pChip );
        }


        void addMPA ( MPA& pMPA )
        {
            fMPAVector.push_back ( &pMPA );
        }
        void addMPA ( MPA* pMPA )
        {
            fMPAVector.push_back ( pMPA );
        }


        /*!
         * \brief Remove a Chip from the vector
         * \param pChipId
         * \return a bool which indicate if the removing was successful
         */
        bool   removeChip ( uint8_t pChipId );
        /*!
         * \brief Get a Chip from the vector
         * \param pChipId
         * \return a pointer of Chip, so we can manipulate directly the Chip contained in the vector
         */
        Chip* getChip ( uint8_t pChipId ) const;


        /*!
         * \brief Remove a MPA from the vector
         * \param pMPAId
         * \return a bool which indicate if the removing was successful
         */
        bool   removeMPA ( uint8_t pMPAId );
        /*!
         * \brief Get a MPA from the vector
         * \param pMPAId
         * \return a pointer of MPA, so we can manipulate directly the MPA contained in the vector
         */
        MPA* getMPA ( uint8_t pMPAId ) const;

       /*!
         * \brief Remove a SSA from the vector
         * \param pSSAId
         * \return a bool which indicate if the removing was successful
         */
        bool   removeSSA ( uint8_t pSSAId );
        /*!
         * \brief Get a SSA from the vector
         * \param pSSAId
         * \return a pointer of SSA, so we can manipulate directly the SSA contained in the vector
         */
        SSA* getSSA ( uint8_t pSSAId ) const;


	// // #################
	// // # RD53 specific #
	// // #################
	// uint8_t getNRD53() const
 //        {
	//   return fRD53Vector.size();
 //        }
	// /*!
	//  * \brief Adding a RD53 to the vector
 //         * \param pRD53
 //         */
 //        void addRD53 (RD53& pRD53)
 //        {
	//   // Get the FrontEndType of the RD53 and set the module one accordingly
	//   // This is the case when no chip type has been set so get the one from the RD53
	//   if (fType == FrontEndType::UNDEFINED)
	//     fType = pRD53.getFrontEndType();
	//   // Else, the chip type has already been set - if it is different from another RD53, rais a warning
	//   // no different chips should be on a module
	//   else if (fType != pRD53.getFrontEndType())
	//     {
	//       LOG (ERROR) << "Error, Chips of a module should not be of different type! - aborting";
	//       exit (1);
	//     }
	  
	//   fRD53Vector.push_back (&pRD53);
 //        }
	
 //        void addRD53 (RD53* pRD53)
 //        {
	//   // Get the FrontEndType of the RD53 and set the module one accordingly
	//   // This is the case when no chip type has been set so get the one from the RD53
	//   if (fType == FrontEndType::UNDEFINED)
	//     fType = pRD53->getFrontEndType();
	//   // Else, the chip type has already been set - if it is different from another RD53, rais a warning
	//   // No different chips should be on a module
	//   else if (fType != pRD53->getFrontEndType())
 //            {
	//       LOG (ERROR) << "Error, Chips of a module should not be of different type! - aborting";
	//       exit (1);
 //            }
	  
	//   fRD53Vector.push_back (pRD53);
 //        }

	// /*!
 //         * \brief Remove a RD53 from the vector
 //         * \param pRD53Id
 //         * \return a bool which indicate if the removing was successful
 //         */
 //        bool removeRD53 (uint8_t pRD53Id);

 //        /*!
 //         * \brief Get a RD53 from the vector
 //         * \param pRD53Id
 //         * \return a pointer of RD53, so we can manipulate directly the RD53 contained in the vector
 //         */
 //        RD53* getRD53 (uint8_t pRD53Id) const;
	// // #################


        /*!
        * \brief Get the Module Id
        * \return The Module ID
        */



        uint8_t getModuleId() const
        {
            return fModuleId;
        };
        /*!
         * \brief Set the Module Id
         * \param pModuleId
         */
        void setModuleId ( uint8_t pModuleId )
        {
            fModuleId = pModuleId;
        };


        // std::vector < RD53* > fRD53Vector;
        std::vector < ReadoutChip* > fReadoutChipVector;
        std::vector < Chip* > fChipVector;
        std::vector < MPA* > fMPAVector;
        std::vector < SSA* > fSSAVector;

      protected:

        //moduleID
        uint8_t fModuleId;
    };
}


#endif
