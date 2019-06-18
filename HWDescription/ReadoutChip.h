/*!

        \file                   Chip.h
        \brief                  Chip Description class, config of the Chips
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef ReadoutChip_h__
#define ReadoutChip_h__

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
#include "../Utils/Container.h"

/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription {

    using ChipRegMap  = std::map < std::string, ChipRegItem >;
    using ChipRegPair = std::pair <std::string, ChipRegItem>;
    using CommentMap  = std::map <int, std::string>;

    /*!
     * \class ReadoutChip
     * \brief Read/Write Chip's registers on a file, contains a register map
     */
    class ReadoutChip : public Chip
    {

      public:
        // C'tors which take BeId, FMCId, FeID, ChipId
        ReadoutChip ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pChipId, uint16_t pMaxRegValue=256);

        // C'tors with object FE Description
        ReadoutChip ( const FrontEndDescription& pFeDesc, uint8_t pChipId, uint16_t pMaxRegValue=256);
        
        /*!
         * \brief acceptor method for HwDescriptionVisitor
         * \param pVisitor
         */
        virtual void accept ( HwDescriptionVisitor& pVisitor )
        {
            pVisitor.visit ( *this );
        }

        /*!
        * \brief Read the registers of the Map in a file
        * \param filename
        */
        virtual void loadfRegMap ( const std::string& filename )  = 0 ;
        /*!
        * \brief Write the registers of the Map in a file
        * \param filename
        */
        virtual void saveRegMap ( const std::string& filename ) = 0;

        virtual uint32_t getNumberOfChannels() const  = 0;

        virtual bool isDACLocal(const std::string &dacName)  = 0;

        virtual uint8_t getNumberOfBits(const std::string &dacName) = 0;

        const ChannelGroupBase* getChipOriginalMask() const override {return fChipOriginalMask;}

      protected:
        std::vector<uint8_t> fChipMask;
        ChannelGroupBase*     fChipOriginalMask;
    };
}

#endif
