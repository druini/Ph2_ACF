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
#include "Cbc.h"
#include "MPA.h"
#include "../Utils/Visitor.h"
#include "../Utils/easylogging++.h"
#include <vector>
#include <stdint.h>

// FE Hybrid HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    /*!
     * \class Module
     * \brief handles a vector of Cbc which are connected to the Module
     */
    class Module : public FrontEndDescription
    {

      public:

        // C'tors take FrontEndDescription or hierachy of connection
        Module ( const FrontEndDescription& pFeDesc, uint8_t pModuleId );
        Module ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pModuleId );

        // Default C'tor
        Module();

        // D'tor
        ~Module()
        {
            for ( auto& pCbc : fCbcVector )
                delete pCbc;

            fCbcVector.clear();

            for ( auto& pMPA : fMPAVector )
                delete pMPA;

            fMPAVector.clear();

        };

        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );

            for ( Cbc* cCbc : fCbcVector )
                cCbc->accept ( pVisitor );
        }
        // void accept( HwDescriptionVisitor& pVisitor ) const {
        //  pVisitor.visit( *this );
        //  for ( auto& cCbc : fCbcVector )
        //      cCbc.accept( pVisitor );
        // }
        /*!
        * \brief Get the number of Cbc connected to the Module
        * \return The size of the vector
        */
        uint8_t getNCbc() const
        {
            return fCbcVector.size();
        }


        uint8_t getNMPA() const
        {
            return fMPAVector.size();
        }


        /*!
         * \brief Adding a Cbc to the vector
         * \param pCbc
         */
        void addCbc ( Cbc& pCbc )
        {
            fCbcVector.push_back ( &pCbc );
        }
        void addCbc ( Cbc* pCbc )
        {
            fCbcVector.push_back ( pCbc );
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
         * \brief Remove a Cbc from the vector
         * \param pCbcId
         * \return a bool which indicate if the removing was successful
         */
        bool   removeCbc ( uint8_t pCbcId );
        /*!
         * \brief Get a Cbc from the vector
         * \param pCbcId
         * \return a pointer of Cbc, so we can manipulate directly the Cbc contained in the vector
         */
        Cbc* getCbc ( uint8_t pCbcId ) const;

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


        std::vector < Cbc* > fCbcVector;
        std::vector < MPA* > fMPAVector;


      protected:

        //moduleID
        uint8_t fModuleId;
    };
}


#endif
