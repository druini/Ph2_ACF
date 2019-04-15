/*

        \file                          Container.h
        \brief                         containers for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          08/04/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <iostream>
#include <vector>
#include <map>

class IdContainer
{
public:
	IdContainer(int id=-1) : id_(id){;}
	int getId(void) {return id_;}
private:
	int id_;

};

template <class T>
class Container : public std::vector<T*> , public IdContainer
{
public:
	Container(int id) : IdContainer(id) {}
	Container(unsigned int size) : std::vector<T*>(size) {}
	virtual ~Container()
	{
		//FIX REMEBER TO DESTROY!!!!!!!!!!!!!!!!!!!!!!
		//for(auto object : *this)
		//	delete object;
		//this->clear();
		//idObjectMap_.clear();
	}

protected:
	virtual T* addObject(int objectId, T* object)
	{
		//std::cout << __PRETTY_FUNCTION__ << "P: " << object << std::endl;
		std::vector<T*>::push_back(object);
		Container::idObjectMap_[objectId] = this->back();
		//std::cout << __PRETTY_FUNCTION__ << "P: " << Container::idObjectMap_[objectId] << std::endl;
		return this->back();
	}
	std::map<int, T*> idObjectMap_;
};


template <class T, class V>
class SummaryContainer : public Container<T>
{
public:
	SummaryContainer(V* object)
	: object_(object)
	{}
protected:
	V* object_;
};



//class Event
//{
//
//};

class ChannelBase
{
public:
	ChannelBase(){;}
	virtual ~ChannelBase(){;}
	virtual void print(void) = 0;
};

class Occupancy// : public ChannelBase
{
public:
	Occupancy()
	: fOccupancy(0)
	{;}
	~Occupancy(){;}
	void print(void){;}//{std::cout << __PRETTY_FUNCTION__ << std::endl;}// = 0;
	float fOccupancy;
};

//class RD53 : public ChannelBase
//{
//public:
//	RD53(){;}
//	void print(void){;}//{std::cout << __PRETTY_FUNCTION__ << std::endl;}// = 0;
//	float occupancy;
//	float pulseHeight;
//	float pulseHeightVariation;
//};

//class Plot : public ChannelBase
//{
//public:
//	void print(void){;}//{std::cout << __PRETTY_FUNCTION__ << std::endl;}// = 0;
//
//private:
//};

class TrimAndMask// : public ChannelBase
{
public:
  TrimAndMask(){;}
  ~TrimAndMask(){}
	void print(void){std::cout << __PRETTY_FUNCTION__ << "Mask: " << mask << " Trim: " << trim << " trim2: " << trim2 << std::endl;}// = 0;

	float trim = 1;
	int   mask = 0;
	float trim2 = 1;
	float trim3 = 1;
	float trim4 = 1;
	float trim5 = 1;
	float trim6 = 1;
	float trim7 = 1;
	float trim8 = 1;
	friend std::ostream& operator<<(std::ostream& os, const TrimAndMask& channel)
	{
   		 os << channel.trim << channel.mask << '\n';
    	return os;
	}


};

class ChannelContainerBase
{
public:
	ChannelContainerBase(){;}
	virtual ~ChannelContainerBase(){;}
	//typedef std::vector<ChannelBase>::iterator iterator;
	//typedef ChannelContainerBase::const_iterator const_iterator;

	virtual void print(void) = 0;
	virtual unsigned int size(void) = 0;
	//virtual ChannelBase& getChannel(unsigned int channel) = 0;
	//virtual iterator begin() = 0;
	//virtual iterator end  () = 0;
private:
    //virtual void dummy(){};
};

template <typename T>
class ChannelContainer: public std::vector<T>, public ChannelContainerBase
{
public:
	ChannelContainer(int size) : std::vector<T>(size) {}
	ChannelContainer(){}
	void print(void)
	{
		for(auto& channel: *this)
			channel.print();
	}
	unsigned int size(void){return this->size();}
	T& getChannel(unsigned int channel) {return this->at(channel);}
	//std::vector<ChannelBase>::iterator begin() override {return this->begin();}
	//std::vector<ChannelBase>::iterator end  () override {return this->end();}
	//typename std::vector<T>::iterator begin() {return this->begin();}
	//typename std::vector<T>::iterator end  () {return this->end();}
	friend std::ostream& operator<<(std::ostream& os, const ChannelContainer& channelContainer)
	{
		for(auto& channel: channelContainer)
			os << channel;
    	return os;
	}
};


class ChipContainer : public IdContainer//: public Container<void*>//, public ChipContainerBase
{
public:
	ChipContainer(int id)
	: IdContainer(id)
	, nOfRows_  (0)
	, nOfCols_  (1)
	,container_ (nullptr)
	{}
	ChipContainer(int id, unsigned int numberOfRows, unsigned int numberOfCols=1)
	: IdContainer(id)
	, nOfRows_  (numberOfRows)
	, nOfCols_  (numberOfCols)
	, container_(nullptr)
	{
	}
	template <typename T>
	typename ChannelContainer<T>::iterator begin(){return static_cast<ChannelContainer<T>*>(container_)->begin();}
	template <typename T>
	typename ChannelContainer<T>::iterator end  (){return static_cast<ChannelContainer<T>*>(container_)->end();}
	//ChannelContainerBase::iterator begin(){return container_->begin();}
	//ChannelContainerBase::iterator end  (){return container_->end();}
	template <typename T>
	void initialize()
	{
		container_ = static_cast<ChannelContainerBase*>(new ChannelContainer<T>(nOfRows_*nOfCols_));
	}
	virtual ~ChipContainer(){if(container_ != nullptr) delete container_;}
	void setNumberOfChannels(unsigned int numberOfRows, unsigned int numberOfCols=1){nOfRows_ = numberOfRows; nOfCols_ = numberOfCols;}

	unsigned int size(void){return nOfRows_*nOfCols_;}
	unsigned int getNumberOfRows(){return nOfRows_;}
	unsigned int getNumberOfCols(){return nOfCols_;}
    //int& operator[] (int x) {
    //    return a[x];
    //}
	template <class T>
	T& getChannel(unsigned int channel)
	{
			return static_cast<ChannelContainer<T>*>(container_)->getChannel(channel);
	}

private:
	unsigned int nOfRows_;
	unsigned int nOfCols_;
	ChannelContainerBase* container_;
};

class ModuleContainer : public Container<ChipContainer>
{
public:
	ModuleContainer(int id) : Container<ChipContainer>(id){}
	template <typename T>
	T*             addChipContainer(int id, T* chip)     {return static_cast<T*>(Container<ChipContainer>::addObject(id, chip));}
	ChipContainer* addChipContainer(int id, int row, int col=1){return Container<ChipContainer>::addObject(id, new ChipContainer(id, row, col));}
private:
};

//template<class T>
class BoardContainer : public Container<ModuleContainer>
{
public:
	BoardContainer(int id) : Container<ModuleContainer>(id){}
	//void addObject(int id){addModule(id);}
	//void fillFast(const Ph2_HwInterface::Event* event);
	template <class T>
	T*               addModuleContainer(int id, T* module){return static_cast<T*>(Container<ModuleContainer>::addObject(id, module));}
	ModuleContainer* addModuleContainer(int id)                 {return Container<ModuleContainer>::addObject(id, new ModuleContainer(id));}
private:
};

//class DetectorBase
//{
//
//};

//template<class T>
class DetectorContainer : public Container<BoardContainer> //, public DetectorBase
{
public:
	DetectorContainer(int id=-1) : Container<BoardContainer>(id){}
	template <class T>
	T*              addBoardContainer(int id, T* board){return static_cast<T*>(Container<BoardContainer>::addObject(id, board));}
	BoardContainer* addBoardContainer(int id)                {return Container<BoardContainer>::addObject(id, new BoardContainer(id));}
private:
};


/*
class Tool
{
public:
	Tool(){;}
	void inherit(Tool& tool)
	{

	}
	void measureOccupancy();
protected:

};


class PedeNoise : public Tool
{
	PedeNoise(){;}
};
*/

class DetectorFactory
{
public:
	DetectorFactory(){;}
	~DetectorFactory(){;}

	//template <class T>
	void buildDetectorUsingFile(DetectorContainer& detector)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		detector.addBoardContainer(1);
		//for(auto& board : theStripDetector)
		for(std::vector<BoardContainer*>::iterator board = detector.begin(); board != detector.end(); board++)
		{
			(*board)->addModuleContainer(12);
			std::cout << "Board" << std::endl;
			for(ModuleContainer* module : **board)
			{
				std::cout << "Module" << std::endl;
				module->addChipContainer(7, 10);
				for(ChipContainer* chip : *module)
				{
					chip->initialize<TrimAndMask>();
					std::cout << "Chip" << std::endl;
					int i = 0;
					for(ChannelContainer<TrimAndMask>::iterator channel =  chip->begin<TrimAndMask>(); channel != chip->end<TrimAndMask>(); channel++, i++)
					//TrimAndMask& trim = *(chip.begin<TrimAndMask>());
					//for(auto& channel : chip)
					{
						channel->trim  = i*0.1;
						channel->mask  = i;
						channel->trim2 = i*0.2;
						channel->print();
					}
				}
			}
		}
	}
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

	void copyStructure(DetectorContainer& original, DetectorContainer& copy)
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
					//copyModule.back().initialize<T>();
				}
			}
		}
	}
	template<typename T>
	void copyAndInitStructure(DetectorContainer& original, DetectorContainer& copy)
	{
		std::cout << __PRETTY_FUNCTION__  << &original  <<  " " << &copy << std::endl;
		for(std::vector<BoardContainer*>::iterator board = original.begin(); board != original.end(); board++)
		{
			BoardContainer* copyBoard = copy.addBoardContainer((*board)->getId());
			std::cout << __PRETTY_FUNCTION__  << copyBoard << std::endl;
			for(ModuleContainer* module : **board)
			{
				std::cout << "Module" << std::endl;
				ModuleContainer* copyModule = copyBoard->addModuleContainer(module->getId());
				for(ChipContainer* chip : *module)
				{
					std::cout << "Chip" << std::endl;
					copyModule->addChipContainer(chip->getId(), chip->getNumberOfRows(), chip->getNumberOfCols());
					copyModule->back()->initialize<T>();
				}
			}
		}
	}

};

/*
class Chip: public ChipContainer
{


};
*/
#endif

/*
int main()
{
	DetectorFactory   theDetectorFactory;
	DetectorContainer theDetectorStructure;

	theDetectorFactory.buildDetectorUsingFile(theDetectorStructure);
	std::cout << "PRINT" << std::endl;
	theDetectorFactory.print<TrimAndMask>(theDetectorStructure);
	std::cout << "Done!" << std::endl;
	std::cout<<"Press Enter...\n";
	std::cin.get();
return 1;

	//Copy doesn't copy!!!!!!!
	DetectorContainer theCopy;
	theDetectorFactory.copyStructure(theDetectorStructure, theCopy);

	std::cout<<"Press Enter...\n";
	std::cin.get();
//	Detector theStripDetector;
//	theDetectorFactory.buildDetectorUsingFile<int>(theStripDetector);
//	theDetectorFactory.print<int>(theStripDetector);
//	Detector theBoolDetector;
//	theDetectorFactory.buildDetectorUsingFile<EmptyValue>(theBoolDetector);
	//theDetectorFactory.print<int>(theStripDetector);

	return 1;
}
*/
