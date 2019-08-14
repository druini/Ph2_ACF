/*

        \file                          ContainerStream.h
        \brief                         ContainerStream for DAQ
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          14/07/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __CONTAINERSTREAM_H__
#define __CONTAINERSTREAM_H__
// pointers to base class
#include <iostream>
#include <type_traits>
#include <sstream>
#include <vector>
#include <cxxabi.h>
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <cmath>
#include <type_traits>
#include "../Utils/ObjectStream.h"
#include "../Utils/DataContainer.h"
#include "../NetworkUtils/TCPPublishServer.h"
#include "../HWDescription/ReadoutChip.h"

// workaround missing "is_trivially_copyable" in g++ < 5.0
// #if __cplusplus < 201402
// namespace std
// {
// 	template<typename T>
// 	struct is_trivially_copyable
// 	{
// 		static const bool value = __has_trivial_copy(T);
// 	};
// }
// #endif

// Here I am using the The Curiously Recurring Template Pattern (CRTP)
// To avoid another inheritance I create a Header base class which takes as template the Child class
// (I need this to do the do sizeof(H), otheswise, since no virtual functions are present this will point to HeaderStreamContainerBase)
// In this base class I have all the datamember I need for a base container
template<typename H>
class HeaderStreamContainerBase : public DataStreamBase
{
public:
	HeaderStreamContainerBase()
	: fBoardId (0)
	{};
	~HeaderStreamContainerBase() {};

	uint32_t size(void) override
	{
		fDataSize = uint64_t(this) + sizeof(H) - uint64_t(&fDataSize);
		std::cout << __PRETTY_FUNCTION__ << " | " << fDataSize << std::endl;
		return fDataSize;
	}

public:
	uint16_t fBoardId;
	
}__attribute__((packed));

// Generic Header which allows to add other members to the header
// !!! IMPORTANT: the members that you add need to be continuos in memory or data want to shipped and you will get a crash !!!
// ContainerStream<Occupancy,int>         -->  OK
// ComtainerStream<Occupancy,char*>       --> ERROR
// ComtainerStream<Occupancy,vector<int>> --> ERROR

template<typename... I>
class HeaderStreamContainer : public HeaderStreamContainerBase<HeaderStreamContainer<I...>>
{

	template <std::size_t N>
    using TupleElementType = typename std::tuple_element<N, std::tuple<I...>>::type;

	template<typename T>
	static void constexpr check_if_retrivable() 
	{
		static_assert( !std::is_pointer<T>::value, "No pointers can be retreived from the stream" );
		static_assert( !std::is_reference<T>::value, "No references can be retreived from the stream" );
		static_assert( std::is_pod<T>::value, "No object not continously allocated in memory can be retreived" );
	}

public:
	HeaderStreamContainer()  {};
	~HeaderStreamContainer() {};

	template<std::size_t N = 0>
    void setHeaderInfo(TupleElementType<N> theInfo)
	{
		std::get<N>(fInfo) = theInfo ;
	}

	template<std::size_t N = 0>
	TupleElementType<N> getHeaderInfo() const
	{
		check_if_retrivable<TupleElementType<N>>();
		return std::get<N>(fInfo);
	}

	typename std::tuple<I...> fInfo;

};

// Specialized Header class when the parameter pack is empty
template<>
class HeaderStreamContainer<> : public HeaderStreamContainerBase<HeaderStreamContainer<>>
{}__attribute__((packed));


template <typename C>
class DataStreamChipContainer : public DataStreamBase
{
public:
	DataStreamChipContainer() : fChannelContainer(nullptr){}
	~DataStreamChipContainer() {if(fChannelContainer != nullptr) delete fChannelContainer; fChannelContainer = nullptr;}

	uint32_t size(void) override
	{
		fDataSize = sizeof(fDataSize)+fChannelContainer->size()*sizeof(C);
		return fDataSize;
	}

	void copyToStream(char* bufferBegin)
	{
		memcpy(bufferBegin, &fDataSize, sizeof(fDataSize));
		memcpy(&bufferBegin[sizeof(fDataSize)], &fChannelContainer->at(0), fChannelContainer->size()*sizeof(C));
		fChannelContainer = nullptr;
	}
	void copyFromStream(const char *bufferBegin)
	{
		memcpy(&fDataSize, bufferBegin, sizeof(fDataSize));
		fChannelContainer = new ChannelContainer<C>((fDataSize-sizeof(fDataSize))/sizeof(C));
		memcpy(&fChannelContainer->at(0), &bufferBegin[sizeof(fDataSize)], fDataSize-sizeof(fDataSize));
	}

public:
	ChannelContainer<C>* fChannelContainer;
}__attribute__((packed));

template <typename C, typename... I> 
class ChannelContainerStream : public ObjectStream<HeaderStreamContainer<uint16_t, uint16_t, I...>,DataStreamChipContainer<C> >
{
	enum HeaderId {ModuleId , ChipId};
	static constexpr size_t getEnumSize() {return ChipId+1;}

public:
	ChannelContainerStream(const std::string& creatorName) : ObjectStream<HeaderStreamContainer<uint16_t, uint16_t, I...>,DataStreamChipContainer<C> >(creatorName) {;}
	~ChannelContainerStream(){;}
	
	void streamAndSendBoard(BoardDataContainer* board, TCPPublishServer* networkStreamer)
	{
		for(auto module: *board)
		{
			for(auto chip: *module)
			{
				retrieveChipData(board->getIndex(), module->getIndex(), chip);
				const std::vector<char>& stream = this->encodeStream();
				this->incrementStreamPacketNumber();
				networkStreamer->broadcast(stream);
			}
		}
	}

	void decodeChipData(DetectorDataContainer& detectorContainer)
	{
		detectorContainer.getObject(this->fHeaderStream.fBoardId)
						->getObject(this->fHeaderStream.template getHeaderInfo<HeaderId::ModuleId>())
						->getObject(this->fHeaderStream.template getHeaderInfo<HeaderId::ChipId>())
						->template setChannelContainer<ChannelContainer<C>>(this->fDataStream.fChannelContainer);
		this->fDataStream.fChannelContainer = nullptr;
	}

	template <std::size_t N>
    using TupleElementType = typename std::tuple_element<N, std::tuple<I...>>::type;

	template<std::size_t N = 0>
	void setHeaderElement(TupleElementType<N> theInfo)
	{
		this->fHeaderStream.template setHeaderInfo<N + getEnumSize()>(theInfo);
	}

	template<std::size_t N = 0>
	TupleElementType<N> getHeaderElement() const
	{
		return this->fHeaderStream.template getHeaderInfo<N + getEnumSize()>();
	}

	typename std::tuple<I...> fInfo;


protected:

	void retrieveChipData(uint16_t boardId, uint16_t moduleId, ChipDataContainer* chip)
	{
		this->fHeaderStream.fBoardId         = boardId;
		this->fHeaderStream.template setHeaderInfo<HeaderId::ModuleId>(moduleId);
		this->fHeaderStream.template setHeaderInfo<HeaderId::ChipId>(chip->getIndex());
		this->fDataStream.fChannelContainer = chip->getChannelContainer<C>();
	}

};

#endif
