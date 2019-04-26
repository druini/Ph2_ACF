/*

        \file                          ContainerFactory.h
        \brief                         Container factory for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __CONTAINERFACTORY_H__
#define __CONTAINERFACTORY_H__

#include "../Utils/Container.h"
#include <iostream>
#include <vector>
#include <map>

class ChannelGroupBase;

class ContainerFactory
{
public:
	ContainerFactory(){;}
	~ContainerFactory(){;}

	void copyStructure(DetectorContainer& original, DetectorContainer& copy);


	template <typename T>
	void print(DetectorContainer& detector)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		for(std::vector<BoardContainer*>::iterator board = detector.begin(); board != detector.end(); board++)
		{
			std::cout << "Board" << std::endl;
			for(ModuleContainer* module : **board)
			{
				std::cout << "Module" << std::endl;
				for(ChipContainer* chip : *module)
				{
					std::cout << "Chip" << std::endl;
					for(typename ChannelContainer<T>::iterator channel=chip->begin<T>(); channel!=chip->end<T>(); channel++)
					//for(ChannelBase& channel : chip)
					{
						//T& c = static_cast<T&>(*channel);
						channel->print();
						std::cout << *channel << std::endl;
						//std::cout << "channel: " << *channel << std::endl;
					}
				}
			}
		}
	}

	// template<typename T>
	// void copyAndInitStructure(DetectorContainer& original, DetectorContainer& copy)
	// {
	// 	copy.initialize<T>();
	// 	for(std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
	// 	{
	// 		BoardContainer* copyBoard = copy.addBoardContainer((*board)->getId());
	// 		copy.back()->initialize<T>();
	// 		for(ModuleContainer* module : **board)
	// 		{
	// 			ModuleContainer* copyModule = copyBoard->addModuleContainer(module->getId());
	// 			copyBoard->back()->initialize<T>();
	// 			for(ChipContainer* chip : *module)
	// 			{
	// 				copyModule->addChipContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
	// 				copyModule->back()->initialize<T>();
	// 			}
	// 		}
	// 	}
	// }


	template<typename T, typename SC, typename SM, typename SB, typename SD>
	void copyAndInitStructure(DetectorContainer& original, DetectorContainer& copy)
	{
		copy.initialize<SD,SB>();
		for(std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
		{
			BoardContainer* copyBoard = copy.addBoardContainer((*board)->getId());
			copy.back()->initialize<SB,SM>();
			for(ModuleContainer* module : **board)
			{
				ModuleContainer* copyModule = copyBoard->addModuleContainer(module->getId());
				copyBoard->back()->initialize<SM,SC>();
				for(ChipContainer* chip : *module)
				{
					copyModule->addChipContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
					copyModule->back()->initialize<SC,T>();
				}
			}
		}
	}

	template<typename T>
	void copyAndInitStructure(DetectorContainer& original, DetectorContainer& copy)
	{
		copyAndInitStructure<T,T,T,T,T>(original, copy);
	}


	template<typename T, typename SC>
	void copyAndInitStructure(DetectorContainer& original, DetectorContainer& copy)
	{
		copyAndInitStructure<T,SC,SC,SC,SC>(original, copy);
	}

	//EXAMPLES
	//	void buildDetectorUsingFile(DetectorContainer& detector)
	//	{
	//		std::cout << __PRETTY_FUNCTION__ << std::endl;
	//		detector.addBoardContainer(1);
	//		//for(auto& board : theStripDetector)
	//		for(std::vector<BoardContainer*>::iterator board = detector.begin(); board != detector.end(); board++)
	//		{
	//			(*board)->addModuleContainer(12);
	//			std::cout << "Board" << std::endl;
	//			for(ModuleContainer* module : **board)
	//			{
	//				std::cout << "Module" << std::endl;
	//				module->addChipContainer(7, 10);
	//				for(ChipContainer* chip : *module)
	//				{
	//					chip->initialize<TrimAndMask>();
	//					std::cout << "Chip" << std::endl;
	//					int i = 0;
	//					for(ChannelContainer<TrimAndMask>::iterator channel =  chip->begin<TrimAndMask>(); channel != chip->end<TrimAndMask>(); channel++, i++)
	//					//TrimAndMask& trim = *(chip.begin<TrimAndMask>());
	//					//for(auto& channel : chip)
	//					{
	//						channel->trim  = i*0.1;
	//						channel->mask  = i;
	//						channel->trim2 = i*0.2;
	//						channel->print();
	//					}
	//				}
	//			}
	//		}
	//	}

	//	void buildDetectorUsingFile(Detector& detector)
	//	{
	//		std::cout << __PRETTY_FUNCTION__ << std::endl;
	//		detector.addBoard(1);
	//		//for(auto& board : theStripDetector)
	//		for(std::vector<Board>::iterator board = detector.begin(); board != detector.end(); board++)
	//		{
	//			(*board).addModule(12);
	//			std::cout << "Board" << std::endl;
	//			for(Module& module : (*board))
	//			{
	//				module.addChip<Chip>(7, 6);
	//				std::cout << "Module" << std::endl;
	//				for(Chip* chip : module)
	//				{
	//					std::cout << "Chip" << std::endl;
	//				}
	//			}
	//		}
	//	}

};

#endif
