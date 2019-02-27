/*!

        \file                   Cbc.h
        \brief                  Cbc Description class, config of the Cbcs
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef Cbc_h__
#define Cbc_h__

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
#include "../Utils/easylogging++.h"
#include "ChipRegItem.h"

// Cbc2 Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    using ChipRegMap = std::map < std::string, ChipRegItem >;
    using CbcRegPair = std::pair <std::string, ChipRegItem>;
    using CommentMap = std::map <int, std::string>;

    /*!
     * \class Cbc
     * \brief Read/Write Cbc's registers on a file, contains a register map
     */
    class Cbc : public Chip
    {

      public:

        // C'tors which take BeId, FMCId, FeID, CbcId
        Cbc ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pCbcId, const std::string& filename );

        // C'tors with object FE Description
        Cbc ( const FrontEndDescription& pFeDesc, uint8_t pCbcId, const std::string& filename );


        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        virtual void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );
        }
        // void accept( HwDescriptionVisitor& pVisitor ) const {
        //  pVisitor.visit( *this );
        // }
        /*!
        * \brief Load RegMap from a file
        * \param filename
        */
        void loadfRegMap ( const std::string& filename ) override;


        /*!
        * \brief Write the registers of the Map in a file
        * \param filename
        */
        void saveRegMap ( const std::string& filename );

        const uint16_t getNumberOfChannels() const override { return NCHANNELS; }

        bool isDACLocal(const std::string &dacName) override {
            if(dacName.find("MaskChannel-",0,12)!=std::string::npos || dacName.find("Channel",0,7)!=std::string::npos ) return true;
            else return false;
        }

        uint8_t getNumberOfBits(const std::string &dacName) override {
            if(dacName.find("MaskChannel-",0,12)!=std::string::npos) return 1;
            else if(dacName == "VCth") return 10;
            else if(dacName == "VCth2") return 2;
            else if(dacName == "TriggerLatency" ) return 9;
            else return 8;
        }


      protected:
        
    };


}

#endif
