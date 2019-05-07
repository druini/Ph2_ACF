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

void ContainerFactory::copyStructure(DetectorContainer& original, DetectorContainer& copy)
{
	std::cout << __PRETTY_FUNCTION__ << std::endl;
	for(std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
	{
		BoardContainer* copyBoard = copy.addBoardContainer((*board)->getId());
		for(ModuleContainer* module : **board)
		{
			std::cout << "Module" << std::endl;
			ModuleContainer* copyModule = copyBoard->addModuleContainer(module->getId());
			for(ChipContainer* chip : *module)
			{
				std::cout << "Chip" << std::endl;
				copyModule->addChipContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
			}
		}
	}
}


