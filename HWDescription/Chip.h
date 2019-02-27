/*!

        \file                   Chip.h
        \brief                  Chip Description class, config of the Chips
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef Chip_h__
#define Chip_h__

#include "FrontEndDescription.h"
#include "ChipRegItem.h"
#include "../Utils/Visitor.h"
#include "../Utils/Exception.h"
#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <utility>
#include <set>
#include "../Utils/easylogging++.h"

// Chip2 Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    using ChipRegMap = std::map < std::string, ChipRegItem >;
    using ChipRegPair = std::pair <std::string, ChipRegItem>;
    using CommentMap = std::map <int, std::string>;

    /*!
     * \class Chip
     * \brief Read/Write Chip's registers on a file, contains a register map
     */
    class Chip : public FrontEndDescription
    {

      public:

        // C'tors which take BeId, FMCId, FeID, ChipId
        Chip ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId, const std::string& filename );

        // C'tors with object FE Description
        Chip ( const FrontEndDescription& pFeDesc, uint8_t pChipId, const std::string& filename );
        Chip ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId, const std::string& filename, ChipType pType );

        // Default C'tor
        Chip();

        // Copy C'tor
        Chip ( const Chip& cbcobj );

        // D'Tor
        ~Chip();

        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        void accept ( HwDescriptionVisitor& pVisitor )
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
        void loadfRegMap ( const std::string& filename );

        /*!
        * \brief Get any register from the Map
        * \param pReg
        * \return The value of the register
        */
        uint8_t getReg ( const std::string& pReg ) const;
        /*!
        * \brief Set any register of the Map
        * \param pReg
        * \param psetValue
        */
        void setReg ( const std::string& pReg, uint8_t psetValue );
        /*!
        * \brief Get any registeritem of the Map
        * \param pReg
        * \return  RegItem
        */
        ChipRegItem getRegItem ( const std::string& pReg );
        /*!
        * \brief Write the registers of the Map in a file
        * \param filename
        */
        void saveRegMap ( const std::string& filename );

        /*!
        * \brief Get the Map of the registers
        * \return The map of register
        */
        ChipRegMap& getRegMap()
        {
            return fRegMap;
        }
        const ChipRegMap& getRegMap() const
        {
            return fRegMap;
        }
        /*!
        * \brief Get the Chip Id
        * \return The Chip ID
        */
        uint8_t getChipId() const
        {
            return fChipId;
        }
        /*!
         * \brief Set the Chip Id
         * \param pChipId
         */
        void setChipId ( uint8_t pChipId )
        {
            fChipId = pChipId;
        }

        virtual const uint16_t getNumberOfChannels() const { return NCHANNELS; }

        const std::vector<uint8_t>& getChipMask() const
        {
            return fChipMask;
        }
        const bool asMaskedChannels() const
        {
            return fAsMaskedChannels;
        }

        virtual bool isDACLocal(const std::string &dacName) {
            if(dacName.find("MaskChannel-",0,12)!=std::string::npos || dacName.find("Channel",0,7)!=std::string::npos ) return true;
            else return false;
        }

        virtual uint8_t getNumberOfBits(const std::string &dacName) {
            if(dacName.find("MaskChannel-",0,12)!=std::string::npos) return 1;
            else if(dacName == "VCth") return 10;
            else if(dacName == "VCth2") return 2;
            else if(dacName == "TriggerLatency" ) return 9;
            else return 8;
        }

      protected:

        // uint16_t fNumberOfChannels;
        uint8_t fChipId;
        bool fAsMaskedChannels;

        // Map of Register Name vs. RegisterItem that contains: Page, Address, Default Value, Value
        ChipRegMap fRegMap;
        CommentMap fCommentMap;
        std::vector<uint8_t> fChipMask = std::vector<uint8_t>(32,0);
        
    };


    /*!
     * \struct ChipComparer
     * \brief Compare two Chip by their ID
     */
    struct ChipComparer
    {

        bool operator() ( const Chip& cbc1, const Chip& cbc2 ) const;

    };

    /*!
     * \struct RegItemComparer
     * \brief Compare two pair of Register Name Versus ChipRegItem by the Page and Adress of the ChipRegItem
     */
    struct RegItemComparer
    {

        bool operator() ( const ChipRegPair& pRegItem1, const ChipRegPair& pRegItem2 ) const;

    };

}

#endif
