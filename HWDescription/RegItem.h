/*!

        \file            RegItem.h
        \brief                   RegItem description, contents of the structure RegItem with is the value of the CbcRegMap
        \author                  Lorenzo BIDEGAIN
        \version                 1.0
        \date                    25/06/14
        Support :                mail to : lorenzo.bidegain@cern.ch

 */

#ifndef _RegItem_h__
#define _RegItem_h__

#include <stdint.h>

namespace Ph2_HwDescription {

    /*!
     * \struct RegItem
     * \brief Struct for CbcRegisterItem that is identified by Page, Address, DefaultValue, Value
     */
    struct RegItem
    {
        RegItem() {};
        RegItem (uint8_t pPage, uint16_t pAddress, uint8_t pDefValue, uint8_t pValue) : fPage (pPage), fAddress (pAddress), fDefValue (pDefValue), fValue (pValue) {}

        uint8_t fPage;
        uint16_t fAddress;
        uint8_t fDefValue;
        uint8_t fValue;

    };
}

#endif
