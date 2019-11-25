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

namespace RootContainerFactory
{

	namespace details
	{
		inline void createAndOpenRootFileFolder(TFile *theOutputFile, std::string &folderName)
		{
			if(theOutputFile->GetDirectory(folderName.data()) == nullptr) theOutputFile->mkdir(folderName.data());
			theOutputFile->cd(folderName.data());
		}

		template<typename T, typename std::enable_if<!std::is_base_of<PlotContainer,T >::value, int>::type = 0>
		std::string getPlotName(T *plot)
		{
			return "NULL";
		}

		template<typename T, typename std::enable_if<std::is_base_of<PlotContainer,T >::value, int>::type = 0>
		std::string getPlotName(T *plot)
		{
			return plot->getName();
		}

		template<typename T, typename std::enable_if<!std::is_base_of<PlotContainer,T >::value, int>::type = 0>
		std::string getPlotTitle(T *plot)
		{
			return "NULL";
		}

		template<typename T, typename std::enable_if<std::is_base_of<PlotContainer,T >::value, int>::type = 0>
		std::string getPlotTitle(T *plot)
		{
			return plot->getTitle();
		}

		template<typename T, typename std::enable_if<!std::is_base_of<PlotContainer,T >::value, int>::type = 0>
		void initializePlot(T *plot, std::string name, std::string title, const T *reference) {;}

		template<typename T, typename std::enable_if<std::is_base_of<PlotContainer,T >::value, int>::type = 0>
		void initializePlot(T *plot, std::string name, std::string title, const T *reference) {
			plot->initialize(name, title, reference);
		}
	}

	using namespace details;

	template<typename T, typename SC, typename SM, typename SB, typename SD>
	void bookHistogramsFromStructure(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& channel, const SC& chipSummary, const SM& moduleSummary, const SB& boardSummary, const SD& detectorSummary)
	{
		std::string detectorFolder = "Detector";
		createAndOpenRootFileFolder(theOutputFile,detectorFolder);
		std::string channelHistogramGenericName         = getPlotName(&channel);
		std::string chipSummaryHistogramGenericName     = getPlotName(&chipSummary);
		std::string moduleSummaryHistogramGenericName   = getPlotName(&moduleSummary);
		std::string boardSummaryHistogramGenericName    = getPlotName(&boardSummary);
		std::string detectorSummaryHistogramGenericName = getPlotName(&detectorSummary);
		
		std::string channelHistogramGenericTitle         = getPlotTitle(&channel);
		std::string chipSummaryHistogramGenericTitle     = getPlotTitle(&chipSummary);
		std::string moduleSummaryHistogramGenericTitle   = getPlotTitle(&moduleSummary);
		std::string boardSummaryHistogramGenericTitle    = getPlotTitle(&boardSummary);
		std::string detectorSummaryHistogramGenericTitle = getPlotTitle(&detectorSummary);
		
		copy.initialize<SD,SB>();

		SD theDetectorSummary;
		initializePlot(&theDetectorSummary,
                       Form("%s_Detector",
                            detectorSummaryHistogramGenericName.data()),
                       Form("%s Detector",
                            detectorSummaryHistogramGenericTitle.data()),
                       &detectorSummary);
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
			initializePlot(&theBoardSummary,
                           Form("D_%s_Board_(%d)",
                                boardSummaryHistogramGenericName.data(),
                                board->getId()),
                           Form("D_%s_Board(%d)",
                                boardSummaryHistogramGenericTitle.data(),
                                board->getId()),
                           &boardSummary);
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
				initializePlot(&theModuleSummary,
                               Form("D_B(%d)_%s_Module(%d)",
                                    board->getId(),
                                    moduleSummaryHistogramGenericName.data(),
                                    module->getId()),
                               Form("D_B(%d)_%s_Module(%d)",
                                    board->getId(),
                                    moduleSummaryHistogramGenericTitle.data(),
                                    module->getId()),
                               &moduleSummary);
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
					initializePlot(&theChipSummary,
                                   Form("D_B(%d)_M(%d)_%s_Chip(%d)",
                                        board->getId(),
                                        module->getId(),
                                        chipSummaryHistogramGenericName.c_str(),
                                        chip->getId()),
                                   Form("D_B(%d)_M(%d)_%s_Chip(%d)",
                                        board->getId(),
                                        module->getId(),
                                        chipSummaryHistogramGenericTitle.c_str(),
                                        chip->getId()),
                                   &chipSummary);
					copyChip->getSummary<SC,T>() = std::move(theChipSummary) ;
	
					//Channels
					std::string channelFolder = "/Channel";
					std::string fullChannelFolder = detectorFolder + boardFolder + moduleFolder + chipFolder + channelFolder;
					createAndOpenRootFileFolder(theOutputFile, fullChannelFolder);

					for(uint32_t row=0; row < chip->getNumberOfRows(); ++row)
					{
						for(uint32_t col=0; col < chip->getNumberOfCols(); ++col)
						{
							if(channelHistogramGenericName != "NULL")
							{
								T theChannel;
								std::string histogramName;
								std::string histogramTitle;
                                if(chip->getNumberOfCols() == 1)
                                  {
                                    histogramName  = Form("D_B(%d)_M(%d)_C(%d)_%s_Channel(%d)",
                                                          board->getId(),
                                                          module->getId(),
                                                          chip->getId(),
                                                          channelHistogramGenericName.data(),
                                                          row);
                                    histogramTitle = Form("D_B(%d)_M(%d)_C(%d)_%s_Channel(%d)",
                                                          board->getId(),
                                                          module->getId(),
                                                          chip->getId(),
                                                          channelHistogramGenericTitle.data(),
                                                          row);
                                  }
                                else
                                  {
                                    histogramName = Form("D_B(%d)_M(%d)_C(%d)_%s_Row(%d)_Col(%d)",
                                                         board->getId(),
                                                         module->getId(),
                                                         chip->getId(),
                                                         channelHistogramGenericName.data(),
                                                         row,
                                                         col);
                                    histogramTitle = Form("D_B(%d)_M(%d)_C(%d)_%s_Row(%d)_Col(%d)",
                                                          board->getId(),
                                                          module->getId(),
                                                          chip->getId(),
                                                          channelHistogramGenericTitle.data(),
                                                          row,
                                                          col);
                                  }
								initializePlot(&theChannel,histogramName, histogramTitle, &channel);
								copyChip->getChannel<T>(row,col) = std::move(theChannel);
							}
						}	
					}
				}
			}
		}

		theOutputFile->cd();
	}

	template<typename T>
	void bookHistogramsFromStructure(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& channel)
	{
		bookHistogramsFromStructure<T,T,T,T,T>(theOutputFile, original, copy, channel, channel, channel, channel, channel);
	}

	template<typename T, typename S>
	void bookHistogramsFromStructure(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& channel, const S& summay)
	{
		bookHistogramsFromStructure<T,S,S,S,S>(theOutputFile, original, copy, channel, summay, summay, summay, summay);
	}


	template<typename T>
	void bookChannelHistograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& channel)
	{
		EmptyContainer theEmpty;
		bookHistogramsFromStructure<T,EmptyContainer,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, original, copy, channel, theEmpty, theEmpty, theEmpty, theEmpty);
	}

	template<typename T>
	void bookChipHistograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& chipSummary)
	{
		EmptyContainer theEmpty;
		bookHistogramsFromStructure<EmptyContainer,T,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, original, copy, theEmpty, chipSummary, theEmpty, theEmpty, theEmpty);
	}

	template<typename T>
	void bookModuleHistograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& moduleSummary)
	{
		EmptyContainer theEmpty;
		bookHistogramsFromStructure<EmptyContainer,EmptyContainer,T,EmptyContainer,EmptyContainer>(theOutputFile, original, copy, theEmpty, theEmpty, moduleSummary, theEmpty, theEmpty);
	}

	template<typename T>
	void bookBoardHistograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& boardSummary)
	{
		EmptyContainer theEmpty;
		bookHistogramsFromStructure<EmptyContainer,EmptyContainer,EmptyContainer,T,EmptyContainer>(theOutputFile, original, copy, theEmpty, theEmpty, theEmpty, boardSummary, theEmpty);
	}

	template<typename T>
	void bookDetectorHistograms(TFile *theOutputFile, const DetectorContainer& original, DetectorDataContainer& copy, const T& detectorSummary)
	{
		EmptyContainer theEmpty;
		bookHistogramsFromStructure<EmptyContainer,EmptyContainer,EmptyContainer,EmptyContainer,T>(theOutputFile, original, copy, theEmpty, theEmpty, theEmpty, theEmpty, detectorSummary);
	}

}


#endif
