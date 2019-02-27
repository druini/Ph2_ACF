/*!
  \file                  RD53RegItem.h
  \brief                 RD53RegItem struct identified by Address, DefaultValue, Value
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53RegItem_h_
#define _RD53RegItem_h_


namespace Ph2_HwDescription
{
  struct RD53RegItem
  {
    RD53RegItem() {};
    RD53RegItem (uint16_t pAddress, uint16_t pDefValue, uint16_t pValue, bool pPrmptCfg = false) :
    fAddress (pAddress), fDefValue (pDefValue), fValue (pValue), fPrmptCfg(pPrmptCfg) {}
    
    uint16_t fAddress;
    uint16_t fDefValue;
    uint16_t fValue;
    bool     fPrmptCfg;
  };
}

#endif
