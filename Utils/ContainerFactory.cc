/*

        \file                          ContainerFactory.cc
        \brief                         Container factory for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Container.h"
#include "../HWDescription/Chip.h"

void ContainerFactory::copyStructure(const DetectorContainer& original, DetectorDataContainer& copy)
{
	for(const BoardContainer *board : original)
	{
		BoardDataContainer* copyBoard = copy.addBoardDataContainer(board->getId());
		for(const OpticalGroupContainer *opticalGroup : *board)
		{
			OpticalGroupDataContainer* copyOpticalGroup = copyBoard->addOpticalGroupDataContainer(opticalGroup->getId());

			for(const ModuleContainer* hybrid : *opticalGroup)
			{
				ModuleDataContainer* copyModule = copyOpticalGroup->addModuleDataContainer(hybrid->getId());
				for(const ChipContainer* chip : *hybrid)
				{
					copyModule->addChipDataContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
				}
			}
		}
	}
}



