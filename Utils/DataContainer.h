/*

        \file                          Container.h
        \brief                         containers for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          06/06/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __DATA_CONTAINER_H__
#define __DATA_CONTAINER_H__

#include <iostream>
#include <vector>
#include <map>
#include "../Utils/ChannelGroupHandler.h"
#include "../Utils/Container.h"
#include "../Utils/EmptyContainer.h"

class ChannelDataContainerBase;
template <typename T>
class ChannelContainer;
class ChipContainer;
class SummaryContainerBase;

class SummaryBase
{
public:
	SummaryBase() {;}
	virtual ~SummaryBase() {;}
	virtual void makeSummary(const ChipContainer* theChannelList, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) = 0;
	virtual void makeSummary(const SummaryContainerBase* theSummaryList, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents) = 0;

};

class SummaryContainerBase 
{
public:
	SummaryContainerBase() {;}
	~SummaryContainerBase() {;}
	virtual void emplace_back(SummaryBase* theSummary) = 0;
};

template <typename T>
class SummaryContainer : public std::vector<T*>, public SummaryContainerBase
{
public:
	SummaryContainer() {;}
	~SummaryContainer() {;}
	void emplace_back(SummaryBase* theSummary) override
	{
		std::vector<T*>::emplace_back(theSummary);
	}
};


template <class S, class C>
class Summary : public SummaryBase
{
public:
	Summary() {;}
	Summary(const S& theSummary) {
		theSummary_ = theSummary;
	}
	Summary(S&& theSummary) {
		theSummary_ = std::move(theSummary);
	}
	Summary& operator= (S&& theSummary)
	{
		theSummary_ = std::move(theSummary);
	}
	Summary(const Summary<S,C>& summary) {
		theSummary_ = summary.theSummary_;
	}

	Summary& operator= (const Summary& summary)
    {
		theSummary_ = summary.theSummary_;
	}
	Summary& operator= (const Summary&& summary)
    {
		theSummary_ = std::move(summary.theSummary_);
	}		

	~Summary() {;}

	void makeSummary(const ChipContainer* theChipContainer, const ChannelGroupBase *chipOriginalMask, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) override
	{
		theSummary_.template makeAverage<C>(theChipContainer, chipOriginalMask, cTestChannelGroup, numberOfEvents);
	}
	void makeSummary(const SummaryContainerBase* theSummaryList, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint16_t numberOfEvents) override
	{
		const SummaryContainer<SummaryBase>* tmpSummaryContainer = static_cast<const SummaryContainer<SummaryBase>*>(theSummaryList);
		std::vector<S> tmpSummaryVector;
		for(auto summary : *tmpSummaryContainer) 
		{
			tmpSummaryVector.emplace_back(std::move(static_cast<Summary<S,C>*>(summary)->theSummary_));
		}
		theSummary_.makeAverage(&tmpSummaryVector,theNumberOfEnabledChannelsList, numberOfEvents);
		delete theSummaryList;
	}

	S theSummary_;
};

class BaseDataContainer
{
public:
	BaseDataContainer() 
	: summary_{nullptr}
	{;}

	virtual ~BaseDataContainer()
	{
		if(summary_ != nullptr)
		{
			delete summary_;
			summary_ = nullptr;
		}
	}

	virtual void initialize(void) {;}
	virtual uint32_t normalizeAndAverageContainers(const BaseContainer* theContainer, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) = 0;
	
	template<typename T>
	bool isSummaryContainerType()
	  {
		T* tmpSummaryContainer = dynamic_cast<T*>(summary_);
		if (tmpSummaryContainer == nullptr)
		{
			return false;
		}
		else return true;

		/* const std::type_info& containerTypeId = typeid(summary_); */
		/* const std::type_info& templateTypeId = typeid(T*); */

		/* return (containerTypeId.hash_code() == templateTypeId.hash_code()); */
	}

	template<typename S, typename T = EmptyContainer>
	S& getSummary()
	{
		return static_cast<Summary<S,T>*>(summary_)->theSummary_;
	}	

	SummaryBase *summary_;
};

template <class T>
class DataContainer : public Container<T> , public BaseDataContainer
{
public:
	DataContainer(int id) : Container<T>(id)
	{;}
	DataContainer(unsigned int size) : Container<T>(size) {}
	virtual ~DataContainer() {;}

	template <typename S, typename V>
	void initialize()
	{	
		if(!std::is_same<S, EmptyContainer>::value) summary_ = new Summary<S,V>();
	}
	template <typename S, typename V>
	void initialize(S& theSummary)
	{
		if(!std::is_same<S, EmptyContainer>::value) summary_ = new Summary<S,V>(theSummary);
	}
	SummaryContainerBase* getAllObjectSummaryContainers() const
	{
		SummaryContainerBase *SummaryContainerList = new SummaryContainer<SummaryBase>;
		for(auto container : *this) SummaryContainerList->emplace_back(container->summary_);
		return SummaryContainerList;
	}

	uint32_t normalizeAndAverageContainers(const BaseContainer* theContainer, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents)
	{

		int index = 0;
		uint32_t numberOfEnabledChannels_ = 0;
		std::vector<uint32_t> theNumberOfEnabledChannelsList;
		for(auto container : *this)
		{
			uint32_t numberOfContainerEnabledChannels = 0;
			if(container != nullptr) numberOfContainerEnabledChannels = container->normalizeAndAverageContainers(static_cast<const Container<T>*>(theContainer)->at(index++), cTestChannelGroup, numberOfEvents);
			theNumberOfEnabledChannelsList.emplace_back(numberOfContainerEnabledChannels);
			numberOfEnabledChannels_+=numberOfContainerEnabledChannels;

		}
		if(summary_ != nullptr) summary_->makeSummary(getAllObjectSummaryContainers(),theNumberOfEnabledChannelsList,numberOfEvents);//sum of chip container needed!!!
		return numberOfEnabledChannels_;
	}

	void cleanDataStored() override
	{
		delete summary_;
		summary_ = nullptr;
		for(auto container : *this)
		{
			container->cleanDataStored();
		}
	}

};

// class ChannelDataContainerBase
// {
// public:
// 	ChannelDataContainerBase() {;}
// 	virtual ~ChannelDataContainerBase(){;}
// 	virtual void normalize(uint16_t numberOfEvents) = 0;
// };

template <typename T>
class ChannelDataContainer: public ChannelContainer<T> //, public ChannelContainerBase
{
public:
	ChannelDataContainer(int size) : ChannelContainer<T>(size) {}
	ChannelDataContainer(int size, T initialValue) : ChannelContainer<T>(size, initialValue) {}
	ChannelDataContainer() : ChannelContainer<T>() {}
	
	void normalize(uint16_t numberOfEvents) override
	{
		for(auto& channel : *this) channel.normalize(numberOfEvents);
	}
};


class ChipDataContainer :  public ChipContainer , public BaseDataContainer
{
public:
	ChipDataContainer(int id)
	: ChipContainer(id)
	{}

	ChipDataContainer(int id, unsigned int numberOfRows, unsigned int numberOfCols=1)
	: ChipContainer(id, numberOfRows, numberOfCols)
	{}

	virtual ~ChipDataContainer() {;}

	template <typename S, typename V>
	void initialize()
	{	
		if(!std::is_same<S, EmptyContainer>::value) summary_ = new Summary<S,V>();
		if(!std::is_same<V, EmptyContainer>::value) container_ = new ChannelDataContainer<V>(nOfRows_*nOfCols_);
	}
	template <typename S, typename V>
	void initialize(S& theSummary, V& initialValue)
	{
		if(!std::is_same<S, EmptyContainer>::value) summary_ = new Summary<S,V>(theSummary);
		if(!std::is_same<V, EmptyContainer>::value) container_ = new ChannelDataContainer<V>(nOfRows_*nOfCols_, initialValue);
	}
	
	uint32_t normalizeAndAverageContainers(const BaseContainer* theContainer, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents)
	{
		if(container_ != nullptr) container_->normalize(numberOfEvents);
		if(summary_ != nullptr)   summary_->makeSummary(this,static_cast<const ChipContainer*>(theContainer)->getChipOriginalMask(), cTestChannelGroup, numberOfEvents);
		return cTestChannelGroup->getNumberOfEnabledChannels(static_cast<const ChipContainer*>(theContainer)->getChipOriginalMask());
	}

};

class ModuleDataContainer : public DataContainer<ChipDataContainer>
{
public:
	ModuleDataContainer(int id) : DataContainer<ChipDataContainer>(id){}
	template <typename T>
	T*             addChipDataContainer(int id, T* chip)     {return static_cast<T*>(DataContainer<ChipDataContainer>::addObject(id, chip));}
	ChipDataContainer* addChipDataContainer(int id, int row, int col=1){return DataContainer<ChipDataContainer>::addObject(id, new ChipDataContainer(id, row, col));}
private:
};

class BoardDataContainer : public DataContainer<ModuleDataContainer>
{
public:
	BoardDataContainer(int id) : DataContainer<ModuleDataContainer>(id){}
	template <class T>
	T*               addModuleDataContainer(int id, T* module){return static_cast<T*>(DataContainer<ModuleDataContainer>::addObject(id, module));}
	ModuleDataContainer* addModuleDataContainer(int id)                 {return DataContainer<ModuleDataContainer>::addObject(id, new ModuleDataContainer(id));}
private:
};

class DetectorDataContainer : public DataContainer<BoardDataContainer>
{
public:
	DetectorDataContainer(int id=0) : DataContainer<BoardDataContainer>(id){}
	~DetectorDataContainer() {}
	template <class T>
	T*              addBoardDataContainer(int id, T* board){return static_cast<T*>(DataContainer<BoardDataContainer>::addObject(id, board));}
	BoardDataContainer* addBoardDataContainer(int id)                {return DataContainer<BoardDataContainer>::addObject(id, new BoardDataContainer(id));}
private:
};


#endif
