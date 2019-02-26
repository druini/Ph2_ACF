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
        ChipRegItem (uint8_t pPage, uint8_t pAddress, uint8_t pDefValue, uint8_t pValue) : fPage (pPage), fAddress (pAddress), fDefValue (pDefValue), fValue (pValue) {}

        uint8_t fPage;
        uint8_t fAddress;
        uint8_t fDefValue;
        uint8_t fValue;

    };
}

#endif
