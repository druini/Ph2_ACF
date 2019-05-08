/*!
  \file                  RD53Event.cc
  \brief                 RD53Event implementation class
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinard@cern.ch
*/

#include "../Utils/RD53Event.h"

namespace Ph2_HwInterface
{
  bool RD53Event::DataBit (uint8_t /*module_id*/, uint8_t chip_id, uint32_t channel_id) const
  {
    for (size_t i = 0; i < chip_events.size(); i++)
      {
	if (chip_id == chip_id_vec[i])
	  {
	    for (const auto& hit : chip_events[i].data)
	      {
		if ((hit.row * NROWS + hit.col) / 4 == channel_id / 4 && hit.tots[channel_id % 4])
		  {
		    return true;
		  } 
	      }
	  }
      }    
    return false;
  }
  
  void RD53Event::fillDataContainer(BoardContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup)
  {
    for (auto module : *boardContainer)
      for (auto chip : *module)
	for (auto row = 0; row < NROWS; row++)
	  for (auto col = 0; col < NCOLS; col++)
	    {
	      if (cTestChannelGroup->isChannelEnabled(row,col))
		chip->getChannel<Occupancy>(row,col).fOccupancy += (float)this->DataBit(module->getId(),chip->getId(),row);
	    }
  }
}
