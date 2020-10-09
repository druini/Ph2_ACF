/*!

        Filename :                              Hybrid.cc
        Content :                               Hybrid Description class
        Programmer :                    Lorenzo BIDEGAIN
        Version :               1.0
        Date of Creation :              25/06/14
        Support :                               mail to : lorenzo.bidegain@gmail.com

 */

#include "OuterTrackerHybrid.h"

namespace Ph2_HwDescription
{
// Default C'tor
OuterTrackerHybrid::OuterTrackerHybrid() : Hybrid(), fCic(nullptr) {}

OuterTrackerHybrid::OuterTrackerHybrid(const FrontEndDescription& pFeDesc, uint8_t pHybridId) : Hybrid(pFeDesc, pHybridId), fCic(nullptr) {}

OuterTrackerHybrid::OuterTrackerHybrid(uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pHybridId) : Hybrid(pBeId, pFMCId, pFeId, pHybridId), fCic(nullptr) {}

} // namespace Ph2_HwDescription