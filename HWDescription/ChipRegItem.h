/*!

        \file            ChipRegItem.h
        \brief                   ChipRegItem description, contents of the structure ChipRegItem with is the value of the ChipRegMap
        \author                  Lorenzo BIDEGAIN
        \version                 1.0
        \date                    25/06/14
        Support :                mail to : lorenzo.bidegain@cern.ch

 */

#ifndef _ChipRegItem_h__
#define _ChipRegItem_h__

#include <stdint.h>

namespace Ph2_HwDescription {

    /*!
     * \struct ChipRegItem
     * \brief Struct for CbcRegisterItem that is identified by Page, Address, DefaultValue, Value
     */
    struct ChipRegItem
    {
        ChipRegItem() {};
        ChipRegItem (uint8_t pPage, uint16_t pAddress, uint16_t pDefValue, uint16_t pValue) : fPage (pPage), fAddress (pAddress), fDefValue (pDefValue), fValue (pValue) {}

        uint8_t fPage;
        uint16_t fAddress;
        uint16_t fDefValue;
        uint16_t fValue;
        bool fPrmptCfg = false;
        uint8_t bit_size = 0;

    };
}

#endif
