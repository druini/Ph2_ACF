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
    for (auto j = 0; j < module_id_vec.size(); j++)
      {
    	if (module_id == module_id_vec[j])
    	  {
	    for (auto i = 0; i < chip_events.size(); i++)
	      if (chip_id == chip_id_vec[i])
		{
		  chipIndx = i;
		  return true;
		}
      	  }
      }
    
    return false;
  }
  
  void RD53Event::fillDataContainer (BoardDataContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup)
  {
    bool   totRequired    = boardContainer->at(0)->at(0)->isChannelContainerType<OccupancyAndPh>();
    bool   vectorRequired = boardContainer->at(0)->at(0)->isSummaryContainerType<Summary<GenericDataVector,EmptyContainer>>();
    size_t chipIndx;

    for (const auto& cModule : *boardContainer)
      for (const auto& cChip : *cModule)
	{
	  if (this->isHittedChip(cModule->getId(), cChip->getId(), chipIndx) == true)
	    {
	      if (vectorRequired == true)
		{
		  std::cout << "AAA " << (chip_events[chipIndx].bc_id) << std::endl;
		  boardContainer->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,EmptyContainer>().theSummary_.data.push_back(chip_events[chipIndx].bc_id);
		}

	      for (const auto& hit : chip_events[chipIndx].data)
		{
		  if ((hit.row >= 0) && (hit.row < RD53::nRows) &&
		      (hit.col >= 0) && (hit.col < RD53::nCols))
		    {
		      for (auto i = 0; i < NPIX_REGION; i++)
			{
			  if (hit.tots[i] != NOHIT_TOT)
			    {
			      if (totRequired == true)
			      	{
				  cChip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fOccupancy++;
				  cChip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fPh      += float(hit.tots[i]);
				  cChip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fPhError += float(hit.tots[i]*hit.tots[i]);

				  if (cTestChannelGroup->isChannelEnabled(hit.row,hit.col+i) == false)
				    cChip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fErrors++;
			      	}
			      else cChip->getChannel<Occupancy>(hit.row,hit.col+i).fOccupancy++;
			    }
			}
		    }
		}
	    }
	}
  }
}
