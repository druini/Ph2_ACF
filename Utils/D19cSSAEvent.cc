#include "../Utils/D19cSSAEvent.h"
#include "../Utils/DataContainer.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../HWDescription/Definition.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface {
	D19cSSAEvent::D19cSSAEvent ( const BeBoard* pBoard,  uint32_t pNSSA, uint32_t pNFe, const std::vector<uint32_t>& list ) : fEventDataVector(pNSSA*pNFe)
    	{
    		LOG (INFO) << BOLDBLUE << "I'M AN EVENT!!" << RESET;
        	SetEvent ( pBoard, pNSSA, list );
    	}
    	void D19cSSAEvent::fillDataContainer(BoardDataContainer* boardContainer, const ChannelGroupBase *cTestChannelGroup)
    	{
    		LOG (INFO) << BOLDBLUE << "I'M AN FDC EVENT!!" << RESET;
        	for(auto module: *boardContainer)
    		{
    			for(auto chip: *module)
    			{
                		unsigned int i = 0;
    				for(ChannelDataContainer<Occupancy>::iterator channel =  chip->begin<Occupancy>(); channel != chip->end<Occupancy>(); channel++, i++)
				{
                    			if(cTestChannelGroup->isChannelEnabled(i))
                    			{
    						channel->fOccupancy  += (float)privateDataBit ( module->getId(), chip->getId(), i);
                    			}
				}
    			}
    		}
    	}
	void D19cSSAEvent::SetEvent ( const BeBoard* pBoard, uint32_t pNSSA, const std::vector<uint32_t>& list )
    	{
    	LOG (INFO) << BOLDBLUE << "I'M A SET EVENT!!" << RESET;
		uint32_t cSSAId = list[4] & 0xF000; // Chip ID
		uint32_t cFeId = list[4] & 0xFF0000; // Module ID
		fEventDataVector[encodeVectorIndex(cFeId, cSSAId, pNSSA)] = list;
	}
	std::string D19cSSAEvent::HexString() const {return "";}
	std::string D19cSSAEvent::DataHexString ( uint8_t pFeId, uint8_t pSSAId ) const {return "";}
	bool D19cSSAEvent::Error ( uint8_t pFeId, uint8_t pSSAId, uint32_t i ) const // FIXME NOT WORKING?!
    	{
        	return Bit ( pFeId, pSSAId, D19C_OFFSET_ERROR_CBC3 );
    	}
    	uint32_t D19cSSAEvent::Error ( uint8_t pFeId, uint8_t pSSAId ) const
    	{
        	try 
        	{
		    const std::vector<uint32_t> &hitVector = fEventDataVector.at(encodeVectorIndex(pFeId, pSSAId,fNCbc));
		    uint32_t cError = ( (hitVector.at (4) & 0xF000000) >> 24 );;
		    return cError;
		}
		catch (const std::out_of_range& outOfRange) {
		    LOG (ERROR) << "Word 4 for FE " << +pFeId << " SSA " << +pSSAId << " is not found:" ;
		    LOG (ERROR) << "Out of Range error: " << outOfRange.what() ;
		    return 0;
		}
    	}
	uint32_t D19cSSAEvent::PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const
	{
		LOG (ERROR) << "This is not a CBC, this is an SSA!" << RESET;
		assert(false);
	}
	std::string D19cSSAEvent::DataBitString ( uint8_t pFeId, uint8_t pSSAId ) const
	{
	        std::ostringstream os;
		for ( uint32_t i = 0; i < NSSACHANNELS; ++i )
		{
			os << privateDataBit(pFeId,pSSAId,i);
		}
		return os.str();
	}
	std::vector<bool> D19cSSAEvent::DataBitVector ( uint8_t pFeId, uint8_t pSSAId ) const
	{
		std::vector<bool> blist;
		for ( uint32_t i = 0; i < NCHANNELS; ++i )
		{
		    blist.push_back ( privateDataBit(pFeId,pSSAId,i) );
		}

		return blist;
	}
	std::vector<bool> D19cSSAEvent::DataBitVector ( uint8_t pFeId, uint8_t pSSAId, const std::vector<uint8_t>& channelList ) const
	{
		std::vector<bool> blist;

		for ( auto i :  channelList )
		{
		    blist.push_back ( privateDataBit(pFeId,pSSAId,i) );
		}
		return blist;
	}
	std::string D19cSSAEvent::GlibFlagString ( uint8_t pFeId, uint8_t pSSAId ) const
	{
		return "";
	}
	std::string D19cSSAEvent::StubBitString ( uint8_t pFeId, uint8_t pSSAId ) const // FIXME
	{
		LOG (ERROR) << "This is not a CBC, this is an SSA!" << RESET;
		assert(false);
	}
    	bool D19cSSAEvent::StubBit ( uint8_t pFeId, uint8_t pSSAId ) const
    	{
		LOG (ERROR) << "This is not a CBC, this is an SSA!" << RESET;
		assert(false);	
	}
	std::vector<Stub> D19cSSAEvent::StubVector (uint8_t pFeId, uint8_t pSSAId) const
    	{
		LOG (ERROR) << "This is not a CBC, this is an SSA!" << RESET;
		assert(false);	
	}
	 uint32_t D19cSSAEvent::GetNHits (uint8_t pFeId, uint8_t pSSAId) const
	{
		try 
       		{
			uint32_t cHits = 0; 
			const std::vector<uint32_t> &hitVector = fEventDataVector.at(encodeVectorIndex(pFeId, pSSAId,fNCbc));
            		cHits += __builtin_popcount ( hitVector.at (8) & 0xFFFFFFFF);
            		cHits += __builtin_popcount ( hitVector.at (9) & 0xFFFFFFFF);
            		cHits += __builtin_popcount ( hitVector.at (10) & 0xFFFFFFFF);
		        cHits += __builtin_popcount ( hitVector.at (11) & 0xFFFFFF00);			
			return cHits;
		}
        	catch (const std::out_of_range& outOfRange)
		{
            		LOG (ERROR) << "Hits for FE " << +pFeId << " SSA " << +pSSAId << " is not found:" ;
            		LOG (ERROR) << "Out of Range error: " << outOfRange.what() ;
            		return 0;
        	}
	}
    	std::vector<uint32_t> D19cSSAEvent::GetHits (uint8_t pFeId, uint8_t pSSAId) const
    	{
        	std::vector<uint32_t> cHits;
                for ( uint32_t i = 0; i < NSSACHANNELS; ++i )
        	{
        		cHits.push_back(privateDataBit(pFeId, pSSAId, i));
        	}
		return cHits;
	}
	void D19cSSAEvent::print ( std::ostream& os) const //TODO add info here as needed
    	{
		os << BOLDGREEN << "EventType: d19c SSA" << RESET << std::endl;
		size_t vectorIndex = 0;
		for (__attribute__((unused)) auto const& hitVector : fEventDataVector)
        	{
			uint8_t cFeId = getFeIdFromVectorIndex(vectorIndex,fNCbc);
            		uint8_t cSSAId = getSSAIdFromVectorIndex(vectorIndex++,fNCbc);
        		os << GREEN << "SSA Header:" << std::endl;
            		os << " FeId: " << +cFeId << " SSAId: " << +cSSAId << RESET << std::endl;
		}
		os << std::endl;
	}
	std::vector<Cluster> D19cSSAEvent::getClusters ( uint8_t pFeId, uint8_t pSSAId) const
    	{
		LOG (ERROR) << "This is not a CBC, this is an SSA!" << RESET;
		assert(false);	
	}
	SLinkEvent D19cSSAEvent::GetSLinkEvent (  BeBoard* pBoard) const
    	{
		LOG (ERROR) << "This is not a CBC, this is an SSA!" << RESET;
		assert(false);	
	}
}
