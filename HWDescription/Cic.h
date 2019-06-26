/*!

        \file                   Cic.h
        \brief                  Cic Description class, config of the Cics
 */


#ifndef Cic_h__
#define Cic_h__

#include "FrontEndDescription.h"
#include "Chip.h"
#include "../Utils/Visitor.h"
#include "../Utils/Exception.h"
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <utility>
#include <set>
#include "../Utils/ConsoleColor.h"
#include "../Utils/easylogging++.h"
// Cic Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription 
{
    using CicRegPair = std::pair <std::string, ChipRegItem>;
    /*using ChipRegMap = std::map < std::string, ChipRegItem >;
    using CommentMap = std::map <int, std::string>;*/
    /*!
     * \class Cic
     * \brief Read/Write Cic's registers on a file, contains a register map
     */
    class Cic : public Chip
    {

      public:

        // C'tors which take BeId, FMCId, FeID, CicId
        Cic ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pCicId, const std::string& filename );

        // C'tors with object FE Description
        Cic ( const FrontEndDescription& pFeDesc, uint8_t pCicId, const std::string& filename );

        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        virtual void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );
        }
        /*!
        * \brief Load RegMap from a file
        * \param filename
        */
        void loadfRegMap ( const std::string& filename ) override;

        /*!
        * \brief Write the registers of the Map in a file
        * \param filename
        */
        void saveRegMap ( const std::string& filename ) override;

        virtual uint8_t getNumberOfBits(const std::string &dacName) {return 8;};

      protected:
 
    };


}

#endif
