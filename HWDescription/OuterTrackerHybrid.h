/*!

    \file                           OuterTrackerHybrid.h
    \brief                          OuterTrackerHybrid Description class
    \author                         Sarah Seif El Nasr-Storey, Fabio Ravera
    \version                        1.0
    \date                           25/06/19
    Support :                       mail to : fabio.ravera@cern.ch

*/

#ifndef OuterTrackerHybrid_h__
#define OuterTrackerHybrid_h__

#include "../Utils/Container.h"
#include "../Utils/Visitor.h"
#include "../Utils/easylogging++.h"
#include "Cic.h"
#include "FrontEndDescription.h"
#include "MPA.h"
#include "Hybrid.h"
#include "SSA.h"
#include <stdint.h>
#include <vector>

// FE Hybrid HW Description Class

/*!
 * \namespace Ph2_HwDescription
 * \brief Namespace regrouping all the hardware description
 */
namespace Ph2_HwDescription
{
/*!
 * \class OuterTrackerHybrid
 * \brief handles a vector of Chip which are connected to the OuterTrackerHybrid
 */
class OuterTrackerHybrid : public Hybrid
{
  public:
    // C'tors take FrontEndDescription or hierachy of connection
    OuterTrackerHybrid(const FrontEndDescription& pFeDesc, uint8_t pHybridId);
    OuterTrackerHybrid(uint8_t pBeId, uint8_t pFMCId, uint8_t pFeId, uint8_t pHybridId);

    // Default C'tor
    OuterTrackerHybrid();

    // D'tor
    ~OuterTrackerHybrid()
    {
        delete fCic;
        fCic = nullptr;
    };

    void addCic(Cic* pCic)
    {
        if(fCic != nullptr)
        {
            LOG(ERROR) << "Error, Cic for this hybrid was already initialized - aborting";
            exit(1);
        }
        fCic = pCic;
        pCic = nullptr;
    }

    Cic* fCic;

  protected:
    uint8_t fHybridId;
};
} // namespace Ph2_HwDescription

#endif
