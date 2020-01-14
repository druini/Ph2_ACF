/*!

        \file                   SSA.h
        \brief                  SSA Description class, config of the SSAs
        \author                 Lorenzo BIDEGAIN
        \version                1.0
        \date                   25/06/14
        Support :               mail to : lorenzo.bidegain@gmail.com

 */


#ifndef SSA_h__
#define SSA_h__

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
    using SSARegMap  = std::map  <std::string, ChipRegItem>;
    using SSARegPair = std::pair <std::string, ChipRegItem>;
    using CommentMap = std::map  <int, std::string>;

    class SSA : public FrontEndDescription
    {
      public:

        // C'tors which take BeId, FMCId, FeID, SSAId
        SSA ( uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pSSAId, uint8_t pSSASide, const std::string& filename);

        // C'tors with object FE Description
        SSA ( const FrontEndDescription& pFeDesc, uint8_t pSSAId , uint8_t pSSASide);

        // Default C'tor
        SSA();

        // Copy C'tor
        SSA ( const SSA& SSAobj );

        void loadfRegMap ( const std::string& filename );

        // D'Tor
        ~SSA();

        uint8_t getSSAId() const
        {
            return fSSAId;
        }
        /*!
         * \brief Set the SSA Id
         * \param pSSAId
         */
        void setSSAId ( uint8_t pSSAId )
        {
            fSSAId = pSSAId;
        }

        SSARegMap& getRegMap()
        {
            return fRegMap;
        }
        const SSARegMap& getRegMap() const
        {
            return fRegMap;
        }
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
        void CheckRegVals();

      protected:

        SSARegMap fRegMap;
        uint8_t fSSAId;
        uint8_t fSSASide;
        CommentMap fCommentMap;
    };
}

#endif
