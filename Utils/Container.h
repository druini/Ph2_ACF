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

class ChannelContainerBase;
template <typename T>
class ChannelContainer;
class SummaryContainerBase;

class SummaryBase
{
public:
	SummaryBase() {;}
	virtual ~SummaryBase() {;}
	virtual void makeSummary(const ChannelContainerBase* theChannelList, const uint32_t numberOfEnabledChannels) = 0;
	virtual void makeSummary(const SummaryContainerBase* theSummaryList, const std::vector<uint32_t>& theNumberOfEnabledChannelsList) = 0;
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
	~Summary() {;}
	void makeSummary(const ChannelContainerBase* theChannelList, const uint32_t numberOfEnabledChannels) override
	{
		theSummary_.makeAverage(static_cast<const ChannelContainer<C>*>(theChannelList), numberOfEnabledChannels);
	}
	void makeSummary(const SummaryContainerBase* theSummaryList, const std::vector<uint32_t>& theNumberOfEnabledChannelsList) override
	{
		const SummaryContainer<SummaryBase>* tmpSummaryContainer = static_cast<const SummaryContainer<SummaryBase>*>(theSummaryList);
		std::vector<C> tmpSummaryVector;
		for(auto summary : *tmpSummaryContainer) tmpSummaryVector.emplace_back(static_cast<Summary<S,C>*>(summary)->theSummary_);
		theSummary_.makeAverage(&tmpSummaryVector,theNumberOfEnabledChannelsList);
		// theSummary_.makeAverage(static_cast<const std::vector<C*>*>(static_cast<const SummaryContainer<C>*>(theSummaryList)),theNumberOfEnabledChannelsList);
	}

	S theSummary_;
};


class IdContainer
{
public:
	IdContainer(int id=-1) : id_(id){;}
	int getId(void) {return id_;}
	virtual void initialize() {;}
	SummaryBase *summary_;
	
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
	SummaryContainerBase* getAllObjectSummaryContainers() const
	{
		SummaryContainerBase *SummaryContainerList = new SummaryContainer<SummaryBase>;
		for(auto container : *this) SummaryContainerList->emplace_back(container->summary_);
		return SummaryContainerList;
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



class ChannelContainerBase
{
public:
	ChannelContainerBase(){;}
	virtual ~ChannelContainerBase(){;}
	virtual void normalize(uint16_t numberOfEvents) = 0;
	//typedef std::vector<ChannelBase>::iterator iterator;
	//typedef ChannelContainerBase::const_iterator const_iterator;

	virtual void print(void) = 0;
	//virtual unsigned int size(void) = 0;
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
	void normalize(uint16_t numberOfEvents) override
	{
		for(auto& channel : *this) channel.normalize(numberOfEvents);
	}

	//unsigned int size(void){return std::vector<T>::size();}
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
	template <typename S, typename V>
	void initialize() override
	{	
		summary_ = new Summary<S,V>();
		container_ = static_cast<ChannelContainerBase*>(new ChannelContainer<V>(nOfRows_*nOfCols_));
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
	template <typename T>
	T* getChannelContainer() {return static_cast<T*>(container_);}
	template <typename T>
	void setChannelContainer(T* container) {container_ = container;}

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
	//void addObject(int id){addModule(id);}
	//void fillFast(const Ph2_HwInterface::Event* event);
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
