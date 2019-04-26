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


void ContainerFactory::normalizeAndAverageContainer(DetectorContainer& theDetector, DetectorContainer& theDetectorData, uint16_t numberOfEvents, const ChannelGroupBase *cTestChannelGroup)
{
    ChipContainer *theChipData;
    std::vector<uint32_t> numberOfBoardUnmaskedChannelsList;
    for(std::vector<BoardContainer*>::iterator board = theDetector.begin(); board != theDetector.end(); board++)
    {
        std::vector<uint32_t> numberOfModuleUnmaskedChannelsList;
        BoardContainer* theBoardData = theDetectorData.at((*board)->getId());
        for(ModuleContainer* module : **board)
        {
            std::vector<uint32_t> numberOfChipUnmaskedChannelsList;
            ModuleContainer* theModuleData = theBoardData->at(module->getId());
            for(ChipContainer* chip : *module)
            {
                theChipData = theModuleData->at(chip->getId());
                theChipData->container_->normalize(numberOfEvents);
                uint32_t numberOfChipUnmaskedChannels = cTestChannelGroup->getNumberOfEnabledChannels(static_cast<Ph2_HwDescription::Chip*>(chip)->getChipOriginalMask());
                theChipData->summary_->makeSummary(theChipData->container_,numberOfChipUnmaskedChannels);
                numberOfChipUnmaskedChannelsList.emplace_back(numberOfChipUnmaskedChannels);
            }
            theModuleData->summary_->makeSummary(theModuleData->getAllObjectSummaryContainers(),numberOfChipUnmaskedChannelsList);//sum of chip container needed!!!
            int32_t sumNumberOfChipUnmaskedChannels = 0;
            for(auto numberOfChannels : numberOfChipUnmaskedChannelsList) sumNumberOfChipUnmaskedChannels+=numberOfChannels;
            numberOfModuleUnmaskedChannelsList.emplace_back(sumNumberOfChipUnmaskedChannels);
        }
        theBoardData->summary_->makeSummary(theBoardData->getAllObjectSummaryContainers(),numberOfModuleUnmaskedChannelsList);//sum of chip container needed!!!
        int32_t sumNumberOfModuleUnmaskedChannels = 0;
        for(auto numberOfChannels : numberOfModuleUnmaskedChannelsList) sumNumberOfModuleUnmaskedChannels+=numberOfChannels;
        numberOfBoardUnmaskedChannelsList.emplace_back(sumNumberOfModuleUnmaskedChannels);
    }
    theDetectorData.summary_->makeSummary(theDetectorData.getAllObjectSummaryContainers(),numberOfBoardUnmaskedChannelsList);//sum of chip container needed!!!
}

