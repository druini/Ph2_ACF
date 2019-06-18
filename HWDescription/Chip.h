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
#include "../Utils/Container.h"

class ChannelGroupBase;
// Chip2 Chip HW Description Class


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    using ChipRegMap  = std::map < std::string, ChipRegItem >;
    using ChipRegPair = std::pair <std::string, ChipRegItem>;
    using CommentMap  = std::map <int, std::string>;

    /*!
     * \class Chip
     * \brief Read/Write Chip's registers on a file, contains a register map
     */
    class Chip : public FrontEndDescription, public ChipContainer
    {

      public:

        // C'tors which take BeId, FMCId, FeID, ChipId
        Chip ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId, uint16_t pMaxRegValue=256);

        // C'tors with object FE Description
        Chip ( const FrontEndDescription& pFeDesc, uint8_t pChipId, uint16_t pMaxRegValue=256);

        // Default C'tor
        Chip();

        // Copy C'tor
        Chip ( const Chip& cbcobj );

        // D'Tor
        virtual ~Chip();

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
        virtual void loadfRegMap ( const std::string& filename ) = 0;

        /*!
        * \brief Get any register from the Map
        * \param pReg
        * \return The value of the register
        */
        uint16_t getReg ( const std::string& pReg ) const;
        /*!
        * \brief Set any register of the Map
        * \param pReg
        * \param psetValue
        */
        void setReg ( const std::string& pReg, uint16_t psetValue, bool pPrmptCfg = false) ;
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
        virtual void saveRegMap ( const std::string& filename ) = 0;

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

        //virtual uint32_t getNumberOfChannels() const  = 0;
        //const ChannelGroupBase* getChipOriginalMask() const override {return fChipOriginalMask;}
        //virtual bool isDACLocal(const std::string &dacName)  = 0;

        virtual uint8_t getNumberOfBits(const std::string &dacName) = 0;

      protected:

        // uint16_t fNumberOfChannels;
        //Chip Description
        uint8_t fChipId;
        uint16_t fMaxRegValue;
        // bool fhasMaskedChannels;

        // Map of Register Name vs. RegisterItem that contains: Page, Address, Default Value, Value
        ChipRegMap fRegMap;
        CommentMap fCommentMap;
        //std::vector<uint8_t> fChipMask;
        //ChannelGroupBase*     fChipOriginalMask;
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
