/*!
  \file                  RD53Event.cc
  \brief                 RD53Event implementation class
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "RD53Event.h"

namespace Ph2_HwInterface
{
  bool RD53Event::isHittedChip (uint8_t module_id, uint8_t chip_id, size_t& chipIndx) const
  {
    for (auto i = 0u; i < moduleAndChipIDs.size(); i++)
      if ((module_id == moduleAndChipIDs[i].first) && (chip_id == moduleAndChipIDs[i].second) && (chip_events[i].hit_data.size() != 0))
        {
          chipIndx = i;
          return true;
        }

    return false;
  }

  void RD53Event::fillDataContainer (BoardDataContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup)
  {
    bool   vectorRequired = boardContainer->at(0)->at(0)->isSummaryContainerType<Summary<GenericDataVector,OccupancyAndPh>>();
    size_t chipIndx;

    for (const auto& cModule : *boardContainer)
      for (const auto& cChip : *cModule)
        if (RD53Event::isHittedChip(cModule->getId(), cChip->getId(), chipIndx) == true)
          {
            if (vectorRequired == true)
              {
                cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1.push_back(chip_events[chipIndx].bc_id);
                cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2.push_back(chip_events[chipIndx].trigger_id);
              }

            for (const auto& hit : chip_events[chipIndx].hit_data)
              {
                cChip->getChannel<OccupancyAndPh>(hit.row+Ph2_HwDescription::RD53::nRows*(hit.col)).fOccupancy++;
                cChip->getChannel<OccupancyAndPh>(hit.row,hit.col).fPh      += float(hit.tot);
                cChip->getChannel<OccupancyAndPh>(hit.row,hit.col).fPhError += float(hit.tot*hit.tot);
                if (cTestChannelGroup->isChannelEnabled(hit.row,hit.col) == false)
                  cChip->getChannel<OccupancyAndPh>(hit.row,hit.col).readoutError = true;
              }
          }
  }
}
