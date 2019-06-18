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
    size_t chipIndx;
    bool   totRequired = boardContainer->at(0)->at(0)->isChannelContainerType<OccupancyAndPh>();

    for (const auto& module : *boardContainer)
      for (const auto& chip : *module)
	{
	  if (this->isHittedChip(module->getId(), chip->getId(), chipIndx) == true)
	    {
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
				  chip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fOccupancy++;
				  chip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fPh      += float(hit.tots[i]);
				  chip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fPhError += float(hit.tots[i]*hit.tots[i]);

				  if (cTestChannelGroup->isChannelEnabled(hit.row,hit.col+i) == false)
				    chip->getChannel<OccupancyAndPh>(hit.row,hit.col+i).fErrors++;
			      	}
			      else chip->getChannel<Occupancy>(hit.row,hit.col+i).fOccupancy++;
			    }
			}
		    }
		}
	    }
	}
  }
}
