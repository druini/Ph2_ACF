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
#include "../Utils/Exception.h"
#include "../Utils/ChannelGroupHandler.h"

class ChannelContainerBase;
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
	Summary(const Summary<S,C>& summary) {
		theSummary_ = summary.theSummary_;
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
		for(auto summary : *tmpSummaryContainer) tmpSummaryVector.emplace_back(static_cast<Summary<S,C>*>(summary)->theSummary_);
		theSummary_.makeAverage(&tmpSummaryVector,theNumberOfEnabledChannelsList, numberOfEvents);
		delete theSummaryList;
	}

	S theSummary_;
};


class BaseContainer
{
public:
	BaseContainer(int id=-1) 
: summary_(nullptr)
, theNumberOfEnabledChannels_(-1)
, id_(id){;}
	virtual ~BaseContainer()
	{
		if(summary_ != nullptr)
		{
			delete summary_;
			summary_ = nullptr;
		}
	}
	int getId(void) {return id_;}
	virtual void initialize() {;}
	virtual uint32_t normalizeAndAverageContainers(const BaseContainer* theContainer, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents) = 0;
	virtual void cleanDataStored() = 0;

	SummaryBase *summary_;
	uint32_t theNumberOfEnabledChannels_;

private:
	int id_;
};

template <class T>
class Container : public std::vector<T*> , public BaseContainer
{
public:
	Container(int id) : BaseContainer(id)
{
}
	Container(unsigned int size) : std::vector<T*>(size) {}
	virtual ~Container()
	{
		reset();
	}
	void reset()
	{
		for(auto object : *this)
		{
			delete object;
		}
		this->clear();
		idObjectMap_.clear();
	}

	T* getObject(int id)
	{
		if(idObjectMap_.find(id) == idObjectMap_.end()) throw Ph2_HwDescription::Exception("T* getObject(int id) : Object Id not found");
		return idObjectMap_[id];
	}
	template <typename S, typename V>
	void initialize() override
	{	
		summary_ = new Summary<S,V>();
	}
	template <typename S, typename V>
	void initialize(S& theSummary)
	{
		summary_ = new Summary<S,V>(theSummary);
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
			uint32_t numberOfContainerEnabledChannels = container->normalizeAndAverageContainers(static_cast<const Container<T>*>(theContainer)->at(index++), cTestChannelGroup, numberOfEvents);
			theNumberOfEnabledChannelsList.emplace_back(numberOfContainerEnabledChannels);
			numberOfEnabledChannels_+=numberOfContainerEnabledChannels;

		}
		summary_->makeSummary(getAllObjectSummaryContainers(),theNumberOfEnabledChannelsList,numberOfEvents);//sum of chip container needed!!!
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


protected:
	virtual T* addObject(int objectId, T* object)
	{
		std::vector<T*>::push_back(object);
		Container::idObjectMap_[objectId] = this->back();
		return this->back();
	}
	std::map<int, T*> idObjectMap_;
};



class ChannelContainerBase
{
public:
	ChannelContainerBase(){;}
	virtual ~ChannelContainerBase(){;}
	virtual void normalize(uint16_t numberOfEvents) = 0;

	virtual void print(void) = 0;
};

template <typename T>
class ChannelContainer: public std::vector<T>, public ChannelContainerBase
{
public:
	ChannelContainer(int size) : std::vector<T>(size) {}
	ChannelContainer(int size, T initialValue) : std::vector<T>(size, initialValue) {}
	ChannelContainer(){}
	void print(void)
	{
		for(auto& channel: *this)
			channel.print();
	}
	void normalize(uint16_t numberOfEvents) override
	{
		for(auto& channel : *this) channel.normalize(numberOfEvents);
	}

	T& getChannel(unsigned int channel) {return this->at(channel);}

	friend std::ostream& operator<<(std::ostream& os, const ChannelContainer& channelContainer)
	{
		for(auto& channel: channelContainer)
			os << channel;
		return os;
	}
};


class ChipContainer : public BaseContainer
{
public:
	ChipContainer(int id)
: BaseContainer(id)
, nOfRows_  (0)
, nOfCols_  (1)
,container_ (nullptr)
{}
	ChipContainer(int id, unsigned int numberOfRows, unsigned int numberOfCols=1)
	: BaseContainer(id)
	, nOfRows_  (numberOfRows)
	, nOfCols_  (numberOfCols)
	, container_(nullptr)
	{
	}
	virtual ~ChipContainer()
	{
		reset();
	}

	template <typename T>
	typename ChannelContainer<T>::iterator begin(){return static_cast<ChannelContainer<T>*>(container_)->begin();}
	template <typename T>
	typename ChannelContainer<T>::iterator end  (){return static_cast<ChannelContainer<T>*>(container_)->end();}

	template <typename T>
	typename ChannelContainer<T>::const_iterator begin() const {return static_cast<ChannelContainer<T>*>(container_)->begin();}
	template <typename T>
	typename ChannelContainer<T>::const_iterator end  () const {return static_cast<ChannelContainer<T>*>(container_)->end();}

	template <typename S, typename V>
	void initialize()
	{	
		summary_ = new Summary<S,V>();
		container_ = static_cast<ChannelContainerBase*>(new ChannelContainer<V>(nOfRows_*nOfCols_));
	}
	template <typename S, typename V>
	void initialize(S& theSummary, V& initialValue)
	{
		summary_ = new Summary<S,V>(theSummary);
		container_ = static_cast<ChannelContainerBase*>(new ChannelContainer<V>(nOfRows_*nOfCols_, initialValue));
	}
	void setNumberOfChannels(unsigned int numberOfRows, unsigned int numberOfCols=1){nOfRows_ = numberOfRows; nOfCols_ = numberOfCols;}
	virtual const ChannelGroupBase* getChipOriginalMask() const {return nullptr;};

	unsigned int size(void) const {return nOfRows_*nOfCols_;}
	unsigned int getNumberOfRows() const {return nOfRows_;}
	unsigned int getNumberOfCols() const {return nOfCols_;}

	template <class T>
	T& getChannel(unsigned int row, unsigned int col=0)
	{
		return static_cast<ChannelContainer<T>*>(container_)->getChannel(row+col*nOfRows_);
	}

	template <class T>
	const T& getChannel(unsigned int row, unsigned int col=0) const
	{
		return static_cast<ChannelContainer<T>*>(container_)->getChannel(row+col*nOfRows_);
	}

	template <typename T>
	T* getChannelContainer() {return static_cast<T*>(container_);}

	template <typename T>
	const T* getChannelContainer() const {return static_cast<T*>(container_);}

	template <typename T>
	void setChannelContainer(T* container) {container_ = container;}
	template<typename T>
	bool isChannelContainerType()
	{
		ChannelContainer<T>* tmpChannelContainer = dynamic_cast<ChannelContainer<T>*>(container_);
		if (tmpChannelContainer == nullptr)
		{
			return false;
		}
		else return true;
	}

	uint32_t normalizeAndAverageContainers(const BaseContainer* theContainer, const ChannelGroupBase *cTestChannelGroup, const uint16_t numberOfEvents)
	{
		container_->normalize(numberOfEvents);
		summary_->makeSummary(this,static_cast<const ChipContainer*>(theContainer)->getChipOriginalMask(), cTestChannelGroup, numberOfEvents);
		return cTestChannelGroup->getNumberOfEnabledChannels(static_cast<const ChipContainer*>(theContainer)->getChipOriginalMask());
	}

	void cleanDataStored() override
	{
		delete container_;
		container_ = nullptr;
	}


	void reset()
	{
		if(container_ != nullptr)
		{
			delete container_;
			container_ = nullptr;
		}
	}

	ChannelContainerBase* container_;
private:
	unsigned int nOfRows_;
	unsigned int nOfCols_;
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

class BoardContainer : public Container<ModuleContainer>
{
public:
	BoardContainer(int id) : Container<ModuleContainer>(id){}
	template <class T>
	T*               addModuleContainer(int id, T* module){return static_cast<T*>(Container<ModuleContainer>::addObject(id, module));}
	ModuleContainer* addModuleContainer(int id)                 {return Container<ModuleContainer>::addObject(id, new ModuleContainer(id));}
private:
};

class DetectorContainer : public Container<BoardContainer>
{
public:
	DetectorContainer(int id=-1) : Container<BoardContainer>(id){}
	template <class T>
	T*              addBoardContainer(int id, T* board){return static_cast<T*>(Container<BoardContainer>::addObject(id, board));}
	BoardContainer* addBoardContainer(int id)                {return Container<BoardContainer>::addObject(id, new BoardContainer(id));}
private:
};


#endif
