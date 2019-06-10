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
  bool RD53Event::isThereAnHit (uint8_t module_id, uint8_t chip_id, uint32_t row, uint32_t col, size_t& ToT) const
  {
    for (auto j = 0; j < module_id_vec.size(); j++)
      {
    	if (module_id == module_id_vec[j])
    	  {
	    for (auto i = 0; i < chip_events.size(); i++)
	      {
		if (chip_id == chip_id_vec[i])
		  {
		    for (const auto& hit : chip_events[i].data)
		      {
			if (row == hit.row &&
			    (col-hit.col) >=0 &&
			    (col-hit.col) < 4 &&
			    hit.tots[col-hit.col] != NOHIT_TOT)
			  {
			    ToT = hit.tots[col-hit.col];
			    return true;
			  }
		      }
		  }
	      }
      	  }
      }
    
    return false;
  }


  void RD53Event::fillDataContainer(BoardDataContainer* boardContainer, const ChannelGroupBase* cTestChannelGroup)
  {
    for (const auto& module : *boardContainer)
      for (const auto& chip : *module)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    {
	      size_t ToT;
	      
	      if (this->isThereAnHit(module->getId(),chip->getId(),row,col,ToT) == true)
		{
		  chip->getChannel<OccupancyAndToT>(row,col).fOccupancy++;
		  chip->getChannel<OccupancyAndToT>(row,col).fToT      += float(ToT);
		  chip->getChannel<OccupancyAndToT>(row,col).fToTError += float(ToT*ToT);

		  if (cTestChannelGroup->isChannelEnabled(row,col) == false)
		    chip->getChannel<OccupancyAndToT>(row,col).fErrors++;
		}
	    }
  }
}
