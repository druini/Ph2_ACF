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
        Cbc ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pCbcId, const std::string& filename, ChipType pType );

        const uint16_t getNumberOfChannels() const override { return NCHANNELS; }

        const std::vector<uint8_t>& getChipMask() const
        {
            return fCbcMask;
        }
        const bool asMaskedChannels() const
        {
            return fAsMaskedChannels;
        }

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

        // uint16_t fNumberOfChannels;
        uint8_t fCbcId;
        bool fAsMaskedChannels;

        // Map of Register Name vs. RegisterItem that contains: Page, Address, Default Value, Value
        ChipRegMap fRegMap;
        CommentMap fCommentMap;
        std::vector<uint8_t> fCbcMask = std::vector<uint8_t>(32,0);
        
    };


    /*!
     * \struct CbcComparer
     * \brief Compare two Cbc by their ID
     */
    struct CbcComparer
    {

        bool operator() ( const Cbc& cbc1, const Cbc& cbc2 ) const;

    };

    /*!
     * \struct RegItemComparer
     * \brief Compare two pair of Register Name Versus ChipRegItem by the Page and Adress of the ChipRegItem
     */
    struct CbcRegItemComparer
    {

        bool operator() ( const CbcRegPair& pRegItem1, const CbcRegPair& pRegItem2 ) const;

    };

}

#endif
