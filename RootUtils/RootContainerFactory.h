/*

        \file                          RootContainerFactory.h
        \brief                         Container factory for DQM
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          14/06/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __ROOTCONTAINERFACTORY_H__
#define __ROOTCONTAINERFACTORY_H__

#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"
#include "../RootUtils/PlotContainer.h"

#include "TFile.h"
#include <iostream>
#include <vector>
#include <map>

class RootContainerFactory
{
public:
	RootContainerFactory(){;}
	~RootContainerFactory(){;}

	template<typename T, typename SC, typename SM, typename SB, typename SD>
	void bookHistrogramFromStructure(TFile &theOutputFile, DetectorContainer& original, DetectorDataContainer& copy, T& channel, SC& chipSummary, SM& moduleSummary, SB& boardSummary, SD& detectorSummary)
	{
		std::string detectorFolder = "/Detector";
		createAndOpenRootFileFolder(theOutputFile,detectorFolder);
		std::string channelHistogramGenericName         = dynamic_cast<PlotContainer*>(&channel)         != nullptr ? channel.getName()         : "NULL";
		std::string chipSummaryHistogramGenericName     = dynamic_cast<PlotContainer*>(&chipSummary)     != nullptr ? chipSummary.getName()     : "NULL";
		std::string moduleSummaryHistogramGenericName   = dynamic_cast<PlotContainer*>(&moduleSummary)   != nullptr ? moduleSummary.getName()   : "NULL";
		std::string boardSummaryHistogramGenericName    = dynamic_cast<PlotContainer*>(&boardSummary)    != nullptr ? boardSummary.getName()    : "NULL";
		std::string detectorSummaryHistogramGenericName = dynamic_cast<PlotContainer*>(&detectorSummary) != nullptr ? detectorSummary.getName() : "NULL";
		
		static_cast<DetectorDataContainer&>(copy).initialize<SD,SB>(detectorSummary);

		for(std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
		{
			std::string boardFolder = "/Board_" + std::to_string((*board)->getId());
			std::string fullBoardFolder = detectorFolder + boardFolder;
			createAndOpenRootFileFolder(theOutputFile, fullBoardFolder);
		
			BoardDataContainer* copyBoard = copy.addBoardDataContainer((*board)->getId());
			static_cast<BoardDataContainer*>(copy.back())->initialize<SB,SM>(boardSummary);

			for(ModuleContainer* module : *board)
			{
				std::string moduleFolder = "/Module_" + std::to_string(module->getId());
				std::string fullModuleFolder = detectorFolder + boardFolder + moduleFolder;
				createAndOpenRootFileFolder(theOutputFile, fullModuleFolder);

				ModuleDataContainer* copyModule = copyBoard->addModuleDataContainer(module->getId());
				static_cast<ModuleDataContainer*>(copyBoard->back())->initialize<SM,SC>(moduleSummary);
				for(ChipContainer* chip : *module)
				{
					std::string chipFolder = "/Chip_" + std::to_string(chip->getId());
					std::string fullChipFolder = detectorFolder + boardFolder + moduleFolder + chipFolder;
					createAndOpenRootFileFolder(theOutputFile, fullChipFolder);

					copyModule->addChipDataContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());

					std::string channelFolder = "/Channel";
					std::string fullChannelFolder = detectorFolder + boardFolder + moduleFolder + chipFolder + channelFolder;
					createAndOpenRootFileFolder(theOutputFile, fullChannelFolder);

					static_cast<ChipDataContainer*>(copyModule->back())->initialize<SC,T>(chipSummary,channel);
				}
			}
		}
	}

	template<typename T>
	void bookHistrogramFromStructure(TFile &theOutputFile, DetectorContainer& original, DetectorDataContainer& copy, T& channel)
	{
		bookHistrogramFromStructure<T,T,T,T,T>(theOutputFile, original, copy, channel, channel, channel, channel, channel);
	}

	template<typename T, typename S>
	void bookHistrogramFromStructure(TFile &theOutputFile, DetectorContainer& original, DetectorDataContainer& copy, T& channel, S& summay)
	{
		bookHistrogramFromStructure<T,S,S,S,S>(original, copy, channel, summay, summay, summay, summay);
	}

private:
	void createAndOpenRootFileFolder(TFile &theOutputFile, std::string &folderName)
	{
		if(theOutputFile.GetDirectory(folderName.data()) == nullptr) theOutputFile.mkdir(folderName.data());
		theOutputFile.cd(folderName.data());
	}

};

#endif
