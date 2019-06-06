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

void ContainerFactory::copyStructure(DetectorContainer& original, DetectorDataContainer& copy)
{
	for(std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
	{
		BoardDataContainer* copyBoard = copy.addBoardDataContainer((*board)->getId());
		for(ModuleContainer* module : **board)
		{
			ModuleDataContainer* copyModule = copyBoard->addModuleDataContainer(module->getId());
			for(ChipContainer* chip : *module)
			{
				copyModule->addChipDataContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
			}
		}
	}
}


