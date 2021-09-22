#ifndef RD53Base_H
#define RD53Base_H

#include "ReadoutChip.h"

namespace Ph2_HwDescription
{

class RD53Base : public ReadoutChip {
public:
    RD53Base(const RD53Base& chipObj) : ReadoutChip(chipObj) {}

    RD53Base(uint8_t pBeId, uint8_t pFMCId, uint8_t pHybridId, uint8_t pRD53Id, uint8_t pRD53Lane) 
      : ReadoutChip(pBeId, pFMCId, pHybridId, pRD53Id)
    {
        chipLane = pRD53Lane;
    }

    virtual void    copyMaskToDefault() {}

    virtual size_t  getNbMaskedPixels() { return 0; }
    
    uint8_t getChipLane() const { return chipLane; }

private:
    uint8_t chipLane;
};

}

#endif