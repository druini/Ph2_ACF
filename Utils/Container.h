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
#include <typeinfo>

class ChannelContainerBase;
template <typename T>
class ChannelContainer;
class ChipContainer;

class BaseContainer
{
public:
	BaseContainer(uint16_t id=-1) 
	: id_                        (id)
	, index_                     (0)
	{;}

	virtual ~BaseContainer() {;}
	uint16_t getId   (void) const {return id_;}
	uint16_t getIndex(void) const {return index_;}
	virtual void     cleanDataStored              (void) = 0;
	virtual BaseContainer* getElement(uint16_t index) const = 0;

	void setIndex(uint16_t index) {index_ = index;}

private:
	uint16_t id_;
	uint16_t index_;
};

template <class T>
class Container : public std::vector<T*> , public BaseContainer
{
public:
	Container(uint16_t id) : BaseContainer(id)
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
			object = nullptr;
		}
		this->clear();
		idObjectMap_.clear();
	}

	T* getObject(uint16_t id)
	{
		if(idObjectMap_.find(id) == idObjectMap_.end()) throw Ph2_HwDescription::Exception("T* getObject(uint16_t id) : Object Id not found");
		return idObjectMap_[id];
	}

	void cleanDataStored() override
	{
		for(auto container : *this)
		{
			container->cleanDataStored();
		}
	}

	BaseContainer* getElement(uint16_t index) const override
	{
		return this->at(index);
	}


protected:
	virtual T* addObject(uint16_t objectId, T* object)
	{
		object->setIndex(this->size());
		std::vector<T*>::push_back(object);
		Container::idObjectMap_[objectId] = this->back();
		return this->back();
	}
	std::map<uint16_t, T*> idObjectMap_;
};



class ChannelContainerBase
{
public:
	ChannelContainerBase(){;}
	virtual ~ChannelContainerBase(){;}
	virtual void normalize(uint32_t numberOfEvents) {;}
};

template <typename T>
class ChannelContainer: public std::vector<T>, public ChannelContainerBase
{
public:
	ChannelContainer(uint32_t size) : std::vector<T>(size) {}
	ChannelContainer(uint32_t size, T initialValue) : std::vector<T>(size, initialValue) {}
	ChannelContainer(){}

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
	ChipContainer(uint16_t id)
	: BaseContainer(id)
	, nOfRows_  (0)
	, nOfCols_  (1)
	,container_ (nullptr)
	{}
	ChipContainer(uint16_t id, unsigned int numberOfRows, unsigned int numberOfCols=1)
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
	ChannelContainer<T>* getChannelContainer() {return static_cast<ChannelContainer<T>*>(container_);}

	template <typename T>
	const ChannelContainer<T>* getChannelContainer() const {return static_cast<ChannelContainer<T>*>(container_);}

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

		/* const std::type_info& containerTypeId = typeid(container_); */
		/* const std::type_info& templateTypeId = typeid(T*); */

		/* return (containerTypeId.hash_code() == templateTypeId.hash_code()); */
	}

	void cleanDataStored() override
	{
		delete container_;
		container_ = nullptr;
	}


	BaseContainer* getElement(uint16_t index) const override
	{
		std::cout<<__PRETTY_FUNCTION__<<" This function should never be called!!! Aborting...";
		abort();
		return nullptr;
	}


	void reset()
	{
		if(container_ != nullptr)
		{
			delete container_;
			container_ = nullptr;
		}
	}

protected:
	unsigned int nOfRows_;
	unsigned int nOfCols_;
	ChannelContainerBase* container_;
};

class ModuleContainer : public Container<ChipContainer>
{
public:
	ModuleContainer(uint16_t id) : Container<ChipContainer>(id){}
	template <typename T>
	T*             addChipContainer(uint16_t id, T* chip)     {return static_cast<T*>(Container<ChipContainer>::addObject(id, chip));}
	ChipContainer* addChipContainer(uint16_t id, uint16_t row, uint16_t col=1){return Container<ChipContainer>::addObject(id, new ChipContainer(id, row, col));}
private:
};

class BoardContainer : public Container<ModuleContainer>
{
public:
	BoardContainer(uint16_t id) : Container<ModuleContainer>(id){}
	template <class T>
	T*               addModuleContainer(uint16_t id, T* module){return static_cast<T*>(Container<ModuleContainer>::addObject(id, module));}
	ModuleContainer* addModuleContainer(uint16_t id)                 {return Container<ModuleContainer>::addObject(id, new ModuleContainer(id));}
private:
};

class DetectorContainer : public Container<BoardContainer>
{
public:
	DetectorContainer(uint16_t id=0) : Container<BoardContainer>(id){}
	template <class T>
	T*              addBoardContainer(uint16_t id, T* board){return static_cast<T*>(Container<BoardContainer>::addObject(id, board));}
	BoardContainer* addBoardContainer(uint16_t id)                {return Container<BoardContainer>::addObject(id, new BoardContainer(id));}
private:
};


#endif
