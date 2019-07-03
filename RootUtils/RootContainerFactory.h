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
#include "../RootUtils/TH1FContainer.h"
#include "../RootUtils/TH2FContainer.h"

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
	void bookHistrogramsFromStructure(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& channel, SC& chipSummary, SM& moduleSummary, SB& boardSummary, SD& detectorSummary)
	{
		std::string detectorFolder = "Detector";
		createAndOpenRootFileFolder(theOutputFile,detectorFolder);
		std::string channelHistogramGenericName         = getPlotName<std::is_base_of<PlotContainer,T >::value>(&channel);
		std::string chipSummaryHistogramGenericName     = getPlotName<std::is_base_of<PlotContainer,SC>::value>(&chipSummary);
		std::string moduleSummaryHistogramGenericName   = getPlotName<std::is_base_of<PlotContainer,SM>::value>(&moduleSummary);
		std::string boardSummaryHistogramGenericName    = getPlotName<std::is_base_of<PlotContainer,SB>::value>(&boardSummary);
		std::string detectorSummaryHistogramGenericName = getPlotName<std::is_base_of<PlotContainer,SD>::value>(&detectorSummary);
		
		std::string channelHistogramGenericTitle         = getPlotTitle<std::is_base_of<PlotContainer,T >::value>(&channel);
		std::string chipSummaryHistogramGenericTitle     = getPlotTitle<std::is_base_of<PlotContainer,SC>::value>(&chipSummary);
		std::string moduleSummaryHistogramGenericTitle   = getPlotTitle<std::is_base_of<PlotContainer,SM>::value>(&moduleSummary);
		std::string boardSummaryHistogramGenericTitle    = getPlotTitle<std::is_base_of<PlotContainer,SB>::value>(&boardSummary);
		std::string detectorSummaryHistogramGenericTitle = getPlotTitle<std::is_base_of<PlotContainer,SD>::value>(&detectorSummary);
		
		copy.initialize<SD,SB>();

		SD theDetectorSummary;
		initializePlot<std::is_base_of<PlotContainer,SD>::value, SD>(&theDetectorSummary,Form("%s_Detector",detectorSummaryHistogramGenericName.data()), Form("%s Detector",detectorSummaryHistogramGenericTitle.data()), &detectorSummary);
		copy.getSummary<SD,SB>() = std::move(theDetectorSummary);

		//Boards
		// for(const std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
		for(const BoardContainer *board : original)
		{
			std::string boardFolder = "/Board_" + std::to_string(board->getId());
			std::string fullBoardFolder = detectorFolder + boardFolder;
			createAndOpenRootFileFolder(theOutputFile, fullBoardFolder);
		
			BoardDataContainer* copyBoard = copy.addBoardDataContainer(board->getId());
			copyBoard->initialize<SB,SM>();

			SB theBoardSummary;
			initializePlot<std::is_base_of<PlotContainer,SB>::value, SB>(&theBoardSummary,Form("%s_Board_%d",boardSummaryHistogramGenericName.data(),board->getId()), Form("%s Board_%d",boardSummaryHistogramGenericTitle.data(),board->getId()), &boardSummary);
			copyBoard->getSummary<SB,SM>() = std::move(theBoardSummary);

			//Modules
			for(const ModuleContainer* module : *board)
			{
				std::string moduleFolder = "/Module_" + std::to_string(module->getId());
				std::string fullModuleFolder = detectorFolder + boardFolder + moduleFolder;
				createAndOpenRootFileFolder(theOutputFile, fullModuleFolder);

				ModuleDataContainer* copyModule = copyBoard->addModuleDataContainer(module->getId());
				copyModule->initialize<SM,SC>();

				SM theModuleSummary;
				initializePlot<std::is_base_of<PlotContainer,SM>::value, SM>(&theModuleSummary,Form("%s_module_%d",moduleSummaryHistogramGenericName.data(),module->getId()), Form("%s module_%d",moduleSummaryHistogramGenericTitle.data(),module->getId()), &moduleSummary);
				copyModule->getSummary<SM,SC>() = std::move(theModuleSummary);

				//Chips
				for(const ChipContainer* chip : *module)
				{
					std::string chipFolder = "/Chip_" + std::to_string(chip->getId());
					std::string fullChipFolder = detectorFolder + boardFolder + moduleFolder + chipFolder;
					createAndOpenRootFileFolder(theOutputFile, fullChipFolder);

					ChipDataContainer* copyChip = copyModule->addChipDataContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
					copyChip->initialize<SC,T>();
					
					SC theChipSummary;
					initializePlot<std::is_base_of<PlotContainer,SC>::value,SC>(&theChipSummary,Form("%s_Chip_%d",chipSummaryHistogramGenericName.data(),chip->getId()), Form("%s Chip_%d",chipSummaryHistogramGenericTitle.data(),chip->getId()), &chipSummary);
					copyChip->getSummary<SC,T>() = std::move(theChipSummary) ;
	
					//Channels
					std::string channelFolder = "/Channel";
					std::string fullChannelFolder = detectorFolder + boardFolder + moduleFolder + chipFolder + channelFolder;
					createAndOpenRootFileFolder(theOutputFile, fullChannelFolder);

					for(uint32_t row=0; row < chip->getNumberOfRows(); ++row)
					{
						for(uint32_t col=0; col < chip->getNumberOfCols(); ++col)
						{
							T theChannel;
							if(moduleSummaryHistogramGenericName != "NULL") 
							{
								std::string histogramName;
								std::string histogramTitle;
								if(chip->getNumberOfCols() == 1)
								{
									histogramName  = Form("%s_Channel_%d",chipSummaryHistogramGenericName.data(),col);
									histogramTitle = Form("%s Channel %d",chipSummaryHistogramGenericName.data(),col);
								}
								else
								{
									histogramName  = Form("%s_Row_%d_Col_%d",chipSummaryHistogramGenericName.data(),row,col);
									histogramTitle = Form("%s Row_%d Col_%d",chipSummaryHistogramGenericName.data(),row,col);
								}
								
								initializePlot<std::is_base_of<PlotContainer,T>::value, T>(&theChannel,histogramName, histogramTitle, &channel);
							}
							copyChip->getChannel<T>(row,col) = std::move(theChannel);
						}	
					}
				}
			}
		}

		theOutputFile->cd();
	}

	template<typename T>
	void bookHistrogramsFromStructure(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& channel)
	{
		bookHistrogramsFromStructure<T,T,T,T,T>(theOutputFile, original, copy, channel, channel, channel, channel, channel);
	}

	template<typename T, typename S>
	void bookHistrogramsFromStructure(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& channel, S& summay)
	{
		bookHistrogramsFromStructure<T,S,S,S,S>(theOutputFile, original, copy, channel, summay, summay, summay, summay);
	}


	template<typename T>
	void bookChannelHistrograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& channel)
	{
		EmptyContainer theEmpty;
		bookHistrogramsFromStructure<T,EmptyContainer,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, original, copy, channel, theEmpty, theEmpty, theEmpty, theEmpty);
	}

	template<typename T>
	void bookChipHistrograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& chipSummary)
	{
		EmptyContainer theEmpty;
		bookHistrogramsFromStructure<EmptyContainer,T,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, original, copy, theEmpty, chipSummary, theEmpty, theEmpty, theEmpty);
	}

	template<typename T>
	void bookModuleHistrograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& moduleSummary)
	{
		EmptyContainer theEmpty;
		bookHistrogramsFromStructure<EmptyContainer,EmptyContainer,T,EmptyContainer,EmptyContainer>(theOutputFile, original, copy, theEmpty, theEmpty, moduleSummary, theEmpty, theEmpty);
	}

	template<typename T>
	void bookBoardHistrograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& boardSummary)
	{
		EmptyContainer theEmpty;
		bookHistrogramsFromStructure<EmptyContainer,EmptyContainer,EmptyContainer,T,EmptyContainer>(theOutputFile, original, copy, theEmpty, theEmpty, theEmpty, boardSummary, theEmpty);
	}

	template<typename T>
	void bookDetectorHistrograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, T& detectorSummary)
	{
		EmptyContainer theEmpty;
		bookHistrogramsFromStructure<EmptyContainer,EmptyContainer,EmptyContainer,EmptyContainer,T>(theOutputFile, original, copy, theEmpty, theEmpty, theEmpty, theEmpty, detectorSummary);
	}



private:
	void createAndOpenRootFileFolder(TFile *theOutputFile, std::string &folderName)
	{
		if(theOutputFile->GetDirectory(folderName.data()) == nullptr) theOutputFile->mkdir(folderName.data());
		theOutputFile->cd(folderName.data());
	}

	template<bool isPlot = false, typename T>
	std::string getPlotName(T *plot)
	{
		return "NULL";
	}


	template<bool isPlot = false, typename T>
	std::string getPlotTitle(T *plot)
	{
		return "NULL";
	}

	template<bool isPlot = false, typename T>
	void initializePlot(T *plot, std::string name, std::string title, const T *reference) {;}
};

template<>
std::string RootContainerFactory::getPlotName<true,TH1FContainer>(TH1FContainer *plot)
{
	return plot->getName();
}

template<>
std::string RootContainerFactory::getPlotTitle<true,TH1FContainer>(TH1FContainer *plot)
{
	return plot->getTitle();
}

template<>
void RootContainerFactory::initializePlot<true,TH1FContainer>(TH1FContainer *plot, std::string name, std::string title, const TH1FContainer *reference)
{
	plot->initialize(name, title, reference);
}


template<>
std::string RootContainerFactory::getPlotName<true,TH2FContainer>(TH2FContainer *plot)
{
	return plot->getName();
}

template<>
std::string RootContainerFactory::getPlotTitle<true,TH2FContainer>(TH2FContainer *plot)
{
	return plot->getTitle();
}

template<>
void RootContainerFactory::initializePlot<true,TH2FContainer>(TH2FContainer *plot, std::string name, std::string title, const TH2FContainer *reference)
{
	plot->initialize(name, title, reference);
}

#endif
