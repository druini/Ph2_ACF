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
#include "ChipRegItem.h"
#include "../Utils/Visitor.h"
#include "../Utils/Exception.h"
#include "../Utils/easylogging++.h"

#include <iostream>
#include <map>
#include <string>
#include <stdint.h>
#include <utility>
#include <set>


/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription
 {
    using MPARegMap  = std::map  <std::string, ChipRegItem>;
    using MPARegPair = std::pair <std::string, ChipRegItem>;
    using CommentMap = std::map  <int, std::string>;

    class MPA : public FrontEndDescription
    {
      public:

        // C'tors which take BeId, FMCId, FeID, MPAId
        MPA ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pMPAId, const std::string& filename);

        // C'tors with object FE Description
        MPA ( const FrontEndDescription& pFeDesc, uint8_t pMPAId );

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
        * \return ChipRegItem
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
        MPARegMap& getRegMap()
        {
            return fRegMap;
        }
        const MPARegMap& getRegMap() const
        {
            return fRegMap;
        }

      protected:
        MPARegMap  fRegMap;
        uint8_t    fMPAId;
        CommentMap fCommentMap;

    };

    struct MPARegItemComparer
    {
        bool operator() ( const MPARegPair& pRegItem1, const MPARegPair& pRegItem2 ) const;
    };
}

#endif
