/*

        \file                          Container.cc
        \brief                         containers for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#include "../Utils/Container.h"
/*
#include "../Utils/Event.h"

void BoardContainer::fillFast(const Ph2_HwInterface::Event* event)
{
	for(auto& module: *this)
	{
		for(auto& chip: module)
		{
			for(unsigned int i=0; i<chip.size(); i++)
			{
				chip.getChannel(i).fill(event, module.getId(), chip.getId(), i);
			}
		}
	}
}

void Occupancy::fill(const Ph2_HwInterface::Event* event)
{
//    if ( event->DataBit ( cFe->getFeId(), cChip->getChipId(), cChan) )
//    {
//        ++stripOccupancy->at(cChan);
//    }
	;
}
void Occupancy::fill(const Ph2_HwInterface::Event* event, int moduleId, int chipId, int channel)
{
    if ( event->DataBit ( cFe->getFeId(), cChip->getChipId(), cChan) )
    {
        ++fOccupancy;
    }
}
*/
