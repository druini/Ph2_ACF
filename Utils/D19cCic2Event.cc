/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/D19cCic2Event.h"
#include "../Utils/DataContainer.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ChannelGroupHandler.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/BeBoard.h"
#include "../HWDescription/OuterTrackerModule.h"

using namespace Ph2_HwDescription;

const unsigned N2SMODULES=12; 

namespace Ph2_HwInterface {

    // Event implementation
    D19cCic2Event::D19cCic2Event ( const BeBoard* pBoard,  uint32_t pNbCic, uint32_t pNFe, const std::vector<uint32_t>& list )
    {
        //LOG (DEBUG) << BOLDBLUE << "Event constructor ... event data list has  " << +fEventDataList.size() << " items." << RESET;
        SetEvent ( pBoard, pNbCic, list );
    }

    void D19cCic2Event::fillDataContainer(BoardDataContainer* boardContainer, const ChannelGroupBase *cTestChannelGroup)
    {
        for(auto opticalGroup : *boardContainer)
    	{
            for(auto hybrid : *opticalGroup)
            {
                LOG (DEBUG) << BOLDBLUE << "Filling data container for hybrid " << hybrid->getId() << RESET;
                for(auto chip : *hybrid)
                {
                    std::vector<uint32_t> cHits = this->GetHits ( hybrid->getId(), chip->getId() );
                    //LOG (DEBUG) << "\t.... " << +cHits.size() << " hits in chip." << RESET;
                    for( auto cHit : cHits ) 
                    {
                        if( cTestChannelGroup->isChannelEnabled(cHit)) 
                        {
                            chip->getChannelContainer<Occupancy>()->at(cHit).fOccupancy += 1.;
                        }
                    }
                }
            }
    	}
    }
    void D19cCic2Event::SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        // get the first CIC 
        auto theFirstCIC = static_cast<OuterTrackerModule*>(pBoard->at(0)->at(0))->fCic;
        bool cWithCIC2 = (theFirstCIC->getFrontEndType() == FrontEndType::CIC2);
           
        fIsSparsified=pBoard->getSparsification();
        
        fEventHitList.clear();
        fEventStubList.clear();
        for( size_t cFeIndex=0; cFeIndex < N2SMODULES*2 ; cFeIndex++)
        {
            FeData cFeData;
            fEventStubList.push_back( cFeData );
            if( fIsSparsified)
            {
                fEventHitList.push_back( cFeData );
            }
            else
            {
                RawFeData cRawFeData;
                fEventRawList.push_back( cRawFeData );
            }
        }
        fBeId = pBoard->getBeId();
        fBeFWType = 0;
        fCBCDataType = 0;
        fBeStatus = 0;
        fNCbc = pNbCbc;

        uint32_t cEventHeader = list.at(0);
        // block size
        fEventSize = 0x0000FFFF & cEventHeader ;
        fEventSize *= 4; // block size is in 128 bit words
        fEventDataSize = fEventSize;
        //LOG (DEBUG) << BOLDGREEN << std::bitset<32>(cEventHeader) << " : " << +fEventSize <<  " 32 bit words." << RESET;     
        //LOG (DEBUG) << BOLDBLUE << "Event header is " << std::bitset<32>(cEventHeader) << RESET;
        //LOG (DEBUG) << BOLDBLUE << "Block size is " << +fEventSize << " 32 bit words." << RESET;
        // check header
        if (((list.at(0) >> 16) & 0xFFFF) != 0xFFFF) 
        {
            LOG (ERROR) << "Event header does not contain 0xFFFF start sequence - please, check the firmware";
        }

        if (fEventSize != list.size() )
            LOG (ERROR) << "Vector size doesnt match the BLOCK_SIZE in Header1";

        // dummy size
        uint8_t fDummySize = (0xFF & list.at (1) ) >> 0;
        fDummySize *= 4;

        // counters
        fExternalTriggerID = (list.at(1) >> 16) & 0x7FFF;
        fTDC = (list.at(2) >> 24) & 0xFF;
        fEventCount = 0x00FFFFFF &  list.at (2);
        fBunch = 0xFFFFFFFF & list.at (3);
        LOG (DEBUG) << BOLDBLUE << "Event" << +fEventCount << " -- BxId " << +fBunch << RESET;

        
        auto cIterator = list.begin() + EVENT_HEADER_SIZE;
        LOG (INFO) << BOLDBLUE << "Event" << +fEventCount << " has " << +list.size() << " 32 bit words [ of which " << +fDummySize << " words are dummy]" << RESET;
        do
        {
            // L1 
            size_t cOffset= std::distance( list.begin() , cIterator );
            uint32_t cL1Header = *cIterator; 
            uint8_t  cHeader = (cL1Header & 0xF0000000) >> 28 ; 
            if( cHeader != 0xa ) 
            {
                LOG (ERROR) << BOLDRED << "Invalid header found in L1 packet." << RESET;
                exit(1);
            }
            uint8_t cErrorCode = (cL1Header & 0xF000000) >> 24;
            if( cErrorCode !=0 ) 
            {
                LOG (ERROR) << BOLDRED << "Error Code " << +cErrorCode << RESET;
                exit(1);
            }

            uint8_t cFeId = ( cL1Header & 0xFF0000) >> 16;
            LOG (DEBUG) << BOLDBLUE << "\t.. FE Id from firmware " << +cFeId << " .. putting data in event list .. offset " << +cOffset << RESET;
            std::pair<uint16_t,uint16_t> cL1Information; 
            uint32_t cL1DataSize = (cL1Header & 0xFFF)*4;
            cL1Information.first = ( *(cIterator + 2)  & 0x7FC000 ) >> 14;
            cL1Information.second = ( *(cIterator + 2)  & 0xFF800000 ) >> 23;
            LOG (DEBUG) << BOLDBLUE << "L1 counter for this event : " << +cL1Information.first << " . L1 data size is " << +(cL1DataSize) << " status " << std::bitset<9>(cL1Information.second) << RESET;

            if( fIsSparsified )
            {
                uint8_t cNClusters =  ( *(cIterator + 2)  & 0x7F);
                // clusters/hit data first 
                std::vector<std::bitset<CLUSTER_WORD_SIZE>> cL1Words(cNClusters, 0);
                this->splitStream(list , cL1Words , cOffset+3 , cNClusters ); // split 32 bit words in std::vector of CLUSTER_WORD_SIZE bits
                fEventHitList[cFeId].first = cL1Information;
                fEventHitList[cFeId].second.clear();
                for(auto cL1Word : cL1Words)
                {   
                    fEventHitList[cFeId].second.push_back( cL1Word.to_ulong() );
                }
            }
            else
            {
                //for( uint32_t cIndex=EVENT_HEADER_SIZE+3; cIndex < cL1DataSize; cIndex++)
                //  LOG (INFO) << BOLDBLUE << std::bitset<32>(*(cIterator+cIndex)) << RESET;

                size_t cHybridIndex=0;
                size_t cModuleIndex=0; 
                for( auto cModule : *pBoard ) 
                {
                    for (auto cFe : *cModule )
                    {
                        if( cFe->getId()== cFeId )
                        {
                            cModuleIndex = cModule->getIndex();
                            cHybridIndex = cFe->getIndex();
                        }
                    }
                }

                fEventRawList[cFeId].first = cL1Information;
                fEventRawList[cFeId].second.clear();
                
                auto cReadoutChips = pBoard->at(cModuleIndex)->at(cHybridIndex); 
                if( cWithCIC2 )
                {
                    const size_t cNblocks = RAW_L1_CBC*cReadoutChips->size()/L1_BLOCK_SIZE; // 275 bits per chip ... 8chips... blocks of 11 bits 
                    std::vector<std::bitset<L1_BLOCK_SIZE>> cL1Words(cNblocks , 0);
                    this->splitStream(list , cL1Words , cOffset+3 , cNblocks ); // split 32 bit words in  blocks of 11 bits 
                    
                    // now try and arrange them by CBC again ... 
                    for(size_t cChipIndex=0; cChipIndex < cReadoutChips->size() ; cChipIndex++)
                    {
                        std::bitset<RAW_L1_CBC> cBitset(0);
                        size_t cPosition=0;
                        for( size_t cBlockIndex =0; cBlockIndex < RAW_L1_CBC/L1_BLOCK_SIZE ; cBlockIndex ++) // RAW_L1_CBC/L1_BLOCK_SIZE blocks per chip
                        {
                            auto cIndex = cChipIndex + cReadoutChips->size()*cBlockIndex; 
                            auto& cL1block = cL1Words[cIndex];
                            LOG (DEBUG) << BOLDBLUE << "\t\t... L1 block " << +cIndex << " -- " << std::bitset<L1_BLOCK_SIZE>(cL1block) << RESET;
                            for(size_t cNbit=0; cNbit < cL1block.size() ; cNbit++ )
                            {
                                cBitset[cBitset.size()-1-cPosition] = cL1block[cL1block.size()-1-cNbit];
                                cPosition++;
                            }
                        }
                        LOG (DEBUG) << BOLDBLUE << "\t...  chip " << +cChipIndex << "\t -- " << std::bitset<RAW_L1_CBC>(cBitset) << RESET;
                        fEventRawList[cFeId].second.push_back( cBitset );
                    }
                }
                else
                {
                    const size_t cNblocks = cReadoutChips->size(); // 275 bits per chip ... 8chips... blocks of 11 bits 
                    std::vector<std::bitset<RAW_L1_CBC>> cL1Words(cNblocks , 0);
                    this->splitStream(list , cL1Words , cOffset+3 , cNblocks ); // split 32 bit words in  blocks of 11 bits 
                    
                    for(size_t cChipIndex=0; cChipIndex < cReadoutChips->size() ; cChipIndex++)
                    {
                        LOG (DEBUG) << BOLDBLUE << "\t...  chip " << +cChipIndex << "\t -- " << std::bitset<RAW_L1_CBC>(cL1Words[cChipIndex]) << RESET;
                        fEventRawList[cFeId].second.push_back( cL1Words[cChipIndex] );
                    }

                }
            }
            // then stubs 
            // using StubData = std::pair< std::pair<uint16_t,uint16_t>, std::vector<uint16_t>>;
            // using EventStubList = std::vector<StubData> ;
            std::pair<uint16_t,uint16_t> cStubInformation; 
            uint32_t cStubHeader = *(cIterator + cL1DataSize );
            uint32_t cStubDataSize = (cStubHeader & 0xFFF)*4;
            uint8_t cNStubs =  ( *(cIterator + cL1DataSize + 1)  & (0x3F << 16)) >> 16 ;
            cStubInformation.first = ( *(cIterator + cL1DataSize + 1)  & 0xFFF);
            cStubInformation.second = ( *(cIterator + cL1DataSize + 1)  & (0x1FF << 22)) >> 22 ;
            //LOG (DEBUG) << BOLDBLUE << "BxId for this event : " << +cStubInformation.first << " . Stub data size is " << +cStubDataSize << " status " << std::bitset<9>(cStubInformation.second) << " -- number of stubs in packet : " << +cNStubs << RESET;
            std::vector<std::bitset<STUB_WORD_SIZE>> cStubWords(cNStubs, 0);
            this->splitStream(list , cStubWords , cOffset + cL1DataSize + 2 , cNStubs ); // split 32 bit words in std::vector of STUB_WORD_SIZE bits
            fEventStubList[cFeId].first = cStubInformation;
            fEventStubList[cFeId].second.clear();
            for(auto cStubWord : cStubWords)
            {   
                fEventStubList[cFeId].second.push_back( cStubWord.to_ulong() );
            }
            cIterator += cL1DataSize + cStubDataSize; 
        }while( cIterator < list.end() - fDummySize );
       
    }

    std::bitset<RAW_L1_CBC> D19cCic2Event::getRawL1Word( uint8_t pFeId , uint8_t pReadoutChipId) const 
    {
        auto cChipIdMapped = 7 - std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , pReadoutChipId ) ) ; 
        auto& cDataBitset = fEventRawList[pFeId].second[ cChipIdMapped ];
        return cDataBitset;    
    }


    std::string D19cCic2Event::HexString() const
    {
        //std::stringbuf tmp;
        //std::ostream os ( &tmp );

        //os << std::hex;

        //for ( uint32_t i = 0; i < fEventSize; i++ )
        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( fEventData.at (i) & 0xFF000000 ) << " " << ( fEventData.at (i) & 0x00FF0000 ) << " " << ( fEventData.at (i) & 0x0000FF00 ) << " " << ( fEventData.at (i) & 0x000000FF );

        ////os << std::endl;

        //return tmp.str();
        return "";
    }

    std::string D19cCic2Event::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::stringbuf tmp;
        std::ostream os ( &tmp );
        std::ios oldState (nullptr);
        oldState.copyfmt (os);
        os << std::hex << std::setfill ('0');

        //get the CBC event for pFeId and pCbcId into vector<32bit> cbcData
        std::vector< uint32_t > cbcData;
        GetCbcEvent (pFeId, pCbcId, cbcData);

        // l1cnt
        os << std::setw (3) << ( (cbcData.at (2) & 0x01FF0000) >> 16) << std::endl;
        // pipeaddr
        os << std::setw (3) << ( (cbcData.at (2) & 0x000001FF) >> 0) << std::endl;
        // trigdata
        os << std::endl;
        os << std::setw (8) << cbcData.at (3) << std::endl;
        os << std::setw (8) << cbcData.at (4) << std::endl;
        os << std::setw (8) << cbcData.at (5) << std::endl;
        os << std::setw (8) << cbcData.at (6) << std::endl;
        os << std::setw (8) << cbcData.at (7) << std::endl;
        os << std::setw (8) << cbcData.at (8) << std::endl;
        os << std::setw (8) << cbcData.at (9) << std::endl;
        os << std::setw (8) << ((cbcData.at (10) & 0xFFFFFFFC) >> 2) << std::endl;
        // stubdata
        os << std::setw (8) << cbcData.at (13) << std::endl;
        os << std::setw (8) << cbcData.at (14) << std::endl;

        os.copyfmt (oldState);

        return tmp.str();
    }

    // NOT READY (what is i??????????)
    bool D19cCic2Event::Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        return Bit ( pFeId, pCbcId, D19C_OFFSET_ERROR_CBC3 );
    }

    uint32_t D19cCic2Event::Error ( uint8_t pFeId, uint8_t pReadoutChipId ) const
    {
        // now only 1 bit per chip - OR of a few error flags 
        if( fIsSparsified )
        {
            auto& cHitInformation = fEventHitList[pFeId].first;
            auto cChipIdMapped = std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , pReadoutChipId ) ) ; 
            uint32_t cError = (cHitInformation.second & (0x1 << (1+cChipIdMapped) )) >> (1+cChipIdMapped);
            return cError;
        }
        else
        {
            auto cDataBitset = getRawL1Word( pFeId, pReadoutChipId);
            std::bitset<2> cErrorBits(0);
            size_t cOffset=0;
            for(size_t cIndex=0;cIndex<cErrorBits.size(); cIndex++)
                cErrorBits[cErrorBits.size()-1-cIndex] = cDataBitset[cDataBitset.size()-cOffset-1-cIndex];
            return (uint32_t)(cErrorBits.to_ulong());
        }
    }
    uint32_t D19cCic2Event::BxId ( uint8_t pFeId ) const
    {
        auto& cStubInformation = fEventStubList[pFeId].first;
        return cStubInformation.first;
    }
    uint16_t D19cCic2Event::Status ( uint8_t pFeId ) const
    {
        auto& cStubInformation = fEventStubList[pFeId].first;
        return cStubInformation.second;
    }
    uint32_t D19cCic2Event::L1Id ( uint8_t pFeId, uint8_t pReadoutChipId ) const
    {
        if(fIsSparsified)
        {
            auto& cHitInformation = fEventHitList[pFeId].first;
            return cHitInformation.first;
        }
        else
        {
            auto cDataBitset = getRawL1Word( pFeId, pReadoutChipId);
            std::bitset<9> cL1Id(0);
            size_t cOffset = 9+2; 
            for(size_t cIndex=0;cIndex<cL1Id.size(); cIndex++)
                cL1Id[cL1Id.size()-1-cIndex] = cDataBitset[cDataBitset.size()-cOffset-1-cIndex];
            return cL1Id.to_ulong();
        }
    }
    // does not apply for sparsified event 
    uint32_t D19cCic2Event::PipelineAddress ( uint8_t pFeId, uint8_t pReadoutChipId ) const
    {
        if(fIsSparsified)
        {
            return 666;
        }
        else
        {
            auto cDataBitset = getRawL1Word( pFeId, pReadoutChipId);
            std::bitset<9> cPipeline(0);
            size_t cOffset = 2; 
            for(size_t cIndex=0;cIndex<cPipeline.size(); cIndex++)
                cPipeline[cPipeline.size()-1-cIndex] = cDataBitset[cDataBitset.size()-cOffset-1-cIndex];
            return cPipeline.to_ulong();
        }
    }
    std::bitset<NCHANNELS> D19cCic2Event::decodeClusters(uint8_t pFeId , uint8_t pReadoutChipId) const 
    {
        auto& cClusterWords = fEventHitList[pFeId].second;
        std::bitset<NCHANNELS> cBitSet(0);
        size_t cClusterId=0;
        //LOG (DEBUG) << BOLDBLUE << "Decoding clusters for FE" << +pFeId << " readout chip " << +pReadoutChipId << RESET;
        for(auto cClusterWord : cClusterWords )
        {
            uint8_t cChipId = (( cClusterWord & (0x7 << 11)) >> 11);
            auto cChipIdMapped = std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , cChipId ) ) ; 
            if( cChipIdMapped != pReadoutChipId )
                continue;
            
            uint8_t cCbcChannelAddress = (( cClusterWord & (0xFF << 3)) >> 3) & 0x7F ; // I think the MSB is the layer ...
            uint8_t cWidth = 1+(cClusterWord & 0x3); 
            LOG (DEBUG) << BOLDBLUE << "\t\t\t... Cluster " << +cClusterId << " : " << std::bitset<CLUSTER_WORD_SIZE>(cClusterWord) << "... " << +cWidth << " strip cluster in strip " << +cCbcChannelAddress << " of chip " << +cChipId << " [ real hybrid  " << +cChipIdMapped  << " ]" << RESET;
            
            for(size_t cOffset = 0 ; cOffset < cWidth ; cOffset++)
            {
                cBitSet[cCbcChannelAddress+cOffset] = 1;
            }
            cClusterId++;
        }
        //LOG (DEBUG) << BOLDBLUE << "Decoded clusters for FE" << +pFeId << " readout chip " << +pReadoutChipId << " : " << std::bitset<NCHANNELS>(cBitSet) << RESET;
        return cBitSet;
    }
    bool D19cCic2Event::DataBit ( uint8_t pFeId, uint8_t pReadoutChipId, uint32_t i ) const
    {
        if(fIsSparsified)
        {
            return ( decodeClusters(pFeId , pReadoutChipId  )[i] > 0 );
        }
        else
        {
            size_t cOffset=2+9+9; 
            return ( getRawL1Word( pFeId, pReadoutChipId)[cOffset+i] > 0 );
        }
    }

    std::string D19cCic2Event::DataBitString ( uint8_t pFeId, uint8_t pReadoutChipId ) const
    {
        if(fIsSparsified)
        {
            std::string cBitStream = std::bitset<NCHANNELS>( this->decodeClusters(pFeId, pReadoutChipId) ).to_string();
            //LOG (DEBUG) << BOLDBLUE << "Original bit stream was : " << cBitStream << RESET;
            return cBitStream;
        }
        else
        {
            size_t cOffset=2+9+9; 
            std::string cBitStream = std::bitset<RAW_L1_CBC>( getRawL1Word( pFeId, pReadoutChipId) ).to_string();
            return cBitStream.substr(cOffset, RAW_L1_CBC); 
        }
    }

    std::vector<bool> D19cCic2Event::DataBitVector ( uint8_t pFeId, uint8_t pReadoutChipId ) const
    {
        std::vector<bool> blist;
        if(fIsSparsified)
        {
            auto cDataBitset = this->decodeClusters(pFeId, pReadoutChipId);
            for( uint8_t cPos=0; cPos < NCHANNELS ; cPos++) 
            {
                blist.push_back( cDataBitset[cPos] == 1 );
            }
        }
        else
        {
            size_t cOffset=2+9+9; 
            auto cDataBitset = getRawL1Word( pFeId, pReadoutChipId);
            for( uint8_t cPos=0; cPos < NCHANNELS ; cPos++) 
            {
                blist.push_back( cDataBitset[cDataBitset.size()-cOffset-1-cPos] == 1 );
            }
        }
        return blist;
    }

    std::vector<bool> D19cCic2Event::DataBitVector ( uint8_t pFeId, uint8_t pReadoutChipId, const std::vector<uint8_t>& channelList ) const
    {
        std::vector<bool> blist;
        if(fIsSparsified)
        {
            auto cDataBitset = this->decodeClusters(pFeId, pReadoutChipId);
            for ( auto cChannel :  channelList )
            {
                blist.push_back( cDataBitset[cChannel] == 1 );
            }
        }
        else
        {
            size_t cOffset=2+9+9; 
            auto cDataBitset = getRawL1Word( pFeId, pReadoutChipId);
            for ( auto cChannel :  channelList )
            {
                blist.push_back( cDataBitset[cDataBitset.size()-cOffset-1-cChannel] == 1 );
            }
        }
        return blist;
    }

    std::string D19cCic2Event::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return "";
    }


    std::vector<Stub> D19cCic2Event::StubVector (uint8_t pFeId, uint8_t pReadoutChipId) const
    {
        auto& cStubWords = fEventStubList[pFeId].second;
        auto cChipIdMapped = std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , pReadoutChipId ) ) ; 
        std::vector<Stub> cStubVec;
        for(auto cStubWord : cStubWords )
        {
            uint8_t cChipId = static_cast<uint8_t>( (cStubWord  & (0x7 << 12) ) >> 12);
            uint8_t cStubAddress  = static_cast<uint8_t>( (cStubWord  & (0xFF << 4) ) >> 4);
            uint8_t cStubBend  = static_cast<uint8_t>( (cStubWord  & (0xF << 0) ) >> 0);
            if( cChipId == cChipIdMapped ) 
            {
                //LOG (DEBUG) << BOLDBLUE << "Stub package ..... " << std::bitset<15>(cStubWord) << " --  chip id from package " << +cChipId << " [ chip id on hybrid " << +pReadoutChipId << "]" << RESET; 
                cStubVec.emplace_back (cStubAddress, cStubBend) ;
            }
        }
        return cStubVec;
    }
    
    std::string D19cCic2Event::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::ostringstream os;

        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);

        for (auto cStub : cStubVector)
            os << std::bitset<8> (cStub.getPosition() ) << " " << std::bitset<4> (cStub.getBend() ) << " ";

        return os.str();
        //return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }
    
    bool D19cCic2Event::StubBit ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        //here just OR the stub positions
        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);
        return (cStubVector.size() > 0 );
    }

    uint32_t D19cCic2Event::GetNHits (uint8_t pFeId, uint8_t pReadoutChipId) const
    {
        uint32_t cNHits=0;
        if(fIsSparsified)
        {
            auto cDataBitset = this->decodeClusters(pFeId, pReadoutChipId);
            cNHits = cDataBitset.count();
        }
        else
        {
            size_t cOffset=2+9+9; 
            auto cDataBitset = this->getRawL1Word( pFeId, pReadoutChipId);
            for( uint8_t cPos=0; cPos < NCHANNELS ; cPos++) 
            {
                cNHits += ( cDataBitset[cDataBitset.size()-cOffset-1-cPos] == 1 );
            }
        }
        return cNHits;
        
    }

    std::vector<uint32_t> D19cCic2Event::GetHits (uint8_t pFeId, uint8_t pReadoutChipId) const
    {
        std::vector<uint32_t> cHits(0);
        if(fIsSparsified)
        {
            auto cDataBitset = this->decodeClusters(pFeId, pReadoutChipId);
            for ( uint32_t i = 0; i < NCHANNELS; ++i )
            {
                if( cDataBitset[i] > 0 ) 
                    cHits.push_back (i);
            }
        } 
        else
        {
            size_t cOffset=2+9+9; 
            auto cDataBitset = this->getRawL1Word( pFeId, pReadoutChipId);
            for( uint8_t cPos=0; cPos < NCHANNELS ; cPos++) 
            {
                if ( cDataBitset[cDataBitset.size()-cOffset-1-cPos] == 1 )
                    cHits.push_back(cPos);
            }
        }
        return cHits;
    }

    void D19cCic2Event::printL1Header (std::ostream& os, uint8_t pFeId, uint8_t pReadoutChipId) const
    {
        os << GREEN << "FEId = " << +pFeId << " CBCId = " << +pReadoutChipId << RESET << std::endl;
        os << YELLOW << "L1 Id: " << this->L1Id (pFeId, pReadoutChipId) << RESET << std::endl;
        os << YELLOW << "Bx Id: " << this->BxId (pFeId) << RESET << std::endl;
        os << RED << "Error: " << static_cast<std::bitset<1>> ( this->Error ( pFeId, pReadoutChipId ) ) << RESET << std::endl;
        os << CYAN << "Total number of hits: " << this->GetNHits ( pFeId, pReadoutChipId ) << RESET << std::endl;
    }

    void D19cCic2Event::print ( std::ostream& os) const
    {
        os << BOLDGREEN << "EventType: d19c CIC2" << RESET << std::endl;
        os << BOLDBLUE <<  "L1A Counter: " << this->GetEventCount() << RESET << std::endl;
        os << "          Be Id: " << +this->GetBeId() << std::endl;
        os << "Event Data size: " << +this->GetEventDataSize() << std::endl;
        os << "Bunch Counter: " << this->GetBunch() << std::endl;
        os << BOLDRED << "    TDC Counter: " << +this->GetTDC() << RESET << std::endl;
        os << BOLDRED << "    TLU Trigger ID: " << +this->GetExternalTriggerId() << RESET << std::endl;


        const int FIRST_LINE_WIDTH = 22;
        const int LINE_WIDTH = 32;
        const int LAST_LINE_WIDTH = 8; 
        //still need to work this out for sparsified data
        if(!fIsSparsified)
        {   
            for(uint8_t cFeId=0; cFeId < fEventRawList.size(); cFeId++ ) 
            {
                for( size_t cReadoutChipId = 0 ; cReadoutChipId < fEventRawList[cFeId].second.size() ; cReadoutChipId++)
                {
                    // print out information
                    printL1Header (os, cFeId, cReadoutChipId);
                    
                    std::vector<uint32_t> cHits = this->GetHits (cFeId, cReadoutChipId);
                    if (cHits.size() == NCHANNELS)
                        os << BOLDRED << "All channels firing!" << RESET << std::endl;
                    else
                    {
                        int cCounter = 0;
                        for (auto& cHit : cHits )
                        {
                            os << std::setw (3) << cHit << " ";
                            cCounter++;
                            if (cCounter == 10)
                            {
                                os << std::endl;
                                cCounter = 0;
                            }
                        }
                        os << RESET << std::endl;
                    }
                    // channel data 
                    std::string data ( this->DataBitString ( cFeId, cReadoutChipId ) ); 
                    os << "Ch. Data:      ";
                    for (int i = 0; i < FIRST_LINE_WIDTH; i += 2)
                        os << data.substr ( i, 2 ) << " ";

                    os << std::endl;

                    for ( int i = 0; i < 7; ++i )
                    {
                        for (int j = 0; j < LINE_WIDTH; j += 2)
                            os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i + j, 2 ) << " ";

                        os << std::endl;
                    }
                    for (int i = 0; i < LAST_LINE_WIDTH; i += 2)
                        os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * 7 + i, 2 ) << " ";
                    os << std::endl;
                    // stubs
                    uint8_t cCounter=0;
                    os << BOLDCYAN << "List of Stubs: " << RESET << std::endl;
                    for (auto& cStub : this->StubVector (cFeId, cReadoutChipId ) )
                    {
                        os << CYAN << "Stub: " << +cCounter << " Position: " << +cStub.getPosition() << " Bend: " << +cStub.getBend() << " Strip: " << cStub.getCenter() << RESET << std::endl;
                        cCounter++;
                    }
                }
            }
        }
        os << std::endl;
    }
    std::vector<Cluster> D19cCic2Event::clusterize( uint8_t pFeId ) const 
    {
        // even hits ---> bottom sensor : cSensorId ==0 [bottom], cSensorId == 1 [top]
        std::vector<Cluster> cClusters(0);
        auto& cClusterWords = fEventHitList[pFeId].second;
        std::bitset<NCHANNELS> cBitSet(0);
        for(auto cClusterWord : cClusterWords )
        {
            uint8_t cChipId = ( cClusterWord & (0x3 << 11)) >> 11;
            auto cChipIdMapped = std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , cChipId ) ) ; 
            
            Cluster cCluster;
            uint8_t cFirst = (( cClusterWord & (0xFF << 3)) >> 3) & 0x7F ;
            uint8_t cSensorId = ((( cClusterWord & (0xFF << 3)) >> 3) & (0x1 << 8) ) >> 8 ;
            cCluster.fFirstStrip = cChipIdMapped*127 + std::floor(cFirst/2.); // I think the MSB is the layer ...
            cCluster.fClusterWidth = 1+(cClusterWord & 0x3); 
            cCluster.fSensor = cSensorId;
            cClusters.push_back(cCluster);
        }
        return cClusters;
    }
    // TO-DO : replace all get clusters with clusterize 
    std::vector<Cluster> D19cCic2Event::getClusters ( uint8_t pFeId, uint8_t pReadoutChipId) const
    {   
        std::vector<Cluster> cClusters(0);
        auto& cClusterWords = fEventHitList[pFeId].second;
        std::bitset<NCHANNELS> cBitSet(0);
        for(auto cClusterWord : cClusterWords )
        {
            uint8_t cChipId = ( cClusterWord & (0x3 << 11)) >> 11;
            auto cChipIdMapped = std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , cChipId ) ) ; 
            if( cChipIdMapped != pReadoutChipId )
                continue;

            Cluster cCluster;
            uint8_t cFirst = (( cClusterWord & (0xFF << 3)) >> 3) & 0x7F ;
            uint8_t cSensorId = ((( cClusterWord & (0xFF << 3)) >> 3) & (0x1 << 8) ) >> 8 ;
            cCluster.fFirstStrip = pReadoutChipId*127 + std::floor(cFirst/2.); // I think the MSB is the layer ...
            cCluster.fClusterWidth = 1+(cClusterWord & 0x3); 
            cCluster.fSensor = cSensorId;
            cClusters.push_back(cCluster);
        }
        return cClusters;
    }
    
    // TO BE MODIFED FOR THIS 
    SLinkEvent D19cCic2Event::GetSLinkEvent (  BeBoard* pBoard ) const
    {
        uint32_t cEvtCount = this->GetEventCount();
        uint16_t cBunch = static_cast<uint16_t> (this->GetBunch() );
        SLinkEvent cEvent (EventType::VR, pBoard->getConditionDataSet()->getDebugMode(), FrontEndType::CBC3, cEvtCount, cBunch, SOURCE_ID );
        
        // // get link Ids 
        // std::vector<uint8_t> cLinkIds(0);
        // std::set<uint8_t> cEnabledFe;
        // for (auto& cFe : pBoard->fModuleVector)
        // {
        //     if ( std::find(cLinkIds.begin(), cLinkIds.end(), cFe->getLinkId() ) == cLinkIds.end() )
        //     {
        //         cEnabledFe.insert (cFe->getLinkId());
        //         cLinkIds.push_back(cFe->getLinkId() );
        //     }
        // }

        // //payload for the status bits
        // GenericPayload cStatusPayload;
        // LOG (DEBUG) << BOLDBLUE << "Generating S-link event " << RESET;
        // //for the hit payload 
        // GenericPayload cPayload;
        // uint16_t cCbcCounter = 0;
        // // now stub payload 
        // std::string cStubString = "";
        // std::string cHitString = "";
        // // 
        // GenericPayload cStubPayload;
        // for(auto cLinkId : cLinkIds )
        // {
        //     //int cFeWord=0;
        //     //int cFirstBitFePayload = cPayload.get_current_write_position();
        //     uint16_t cCbcPresenceWord = 0;
        //     uint8_t cFeStubCounter = 0;
        //     auto cPositionStubs = cStubString.length(); 
        //     auto cPositionHits = cHitString.length();
        //     for (auto cFe : pBoard->fModuleVector)
        //     {
        //         if( cFe->getLinkId() != cLinkId )
        //             continue;
                
        //         uint8_t cFeId = cFe->getFeId();
        //         for (auto cChip : cFe->fReadoutChipVector)
        //         {
        //             uint8_t cChipId = cChip->getChipId() ;
        //             auto cIndex = 7 - std::distance( fFeMapping.begin() , std::find( fFeMapping.begin(), fFeMapping.end() , cChipId ) ) ; 
        //         	if( cIndex >= (int)fEventDataList[cFeId].second.size() ) 
        //         	    continue;
        //             auto& cDataBitset = fEventDataList[cFeId].second[ cIndex ];
        //             uint32_t cError = this->Error ( cFeId , cChipId);
        //             uint32_t cPipeAddress = this->PipelineAddress( cFeId , cChipId); 
        //             uint32_t cL1ACounter = this->L1Id( cFeId , cChipId);
        //             uint32_t cStatusWord = cError << 18 | cPipeAddress << 9 | cL1ACounter;
                    
        //             auto cNHits = this->GetNHits(cFe->getFeId(), cChip->getChipId() );
        //             //now get the CBC status summary
        //             if (pBoard->getConditionDataSet()->getDebugMode() == SLinkDebugMode::ERROR)
        //                cStatusPayload.append ( (cError != 0) ? 1 : 0);
        //             else if (pBoard->getConditionDataSet()->getDebugMode() == SLinkDebugMode::FULL)
        //                 //assemble the error bits (63, 62, pipeline address and L1A counter) into a status word
        //                 cStatusPayload.append (cStatusWord, 20);
                
        //             //generate the payload
        //             //the first line sets the cbc presence bits
        //             cCbcPresenceWord |= 1 << (cChipId + 8*(cFe->getFeId()%2));

        //             //254 channels + 2 padding bits at the end 
        //             std::bitset<256> cBitsetHitData; 
        //             for( uint8_t cPos=0; cPos < NCHANNELS ; cPos++) 
        //             {
        //                 cBitsetHitData[NCHANNELS-1-cPos] = cDataBitset[NCHANNELS-1-cPos];
        //             }
        //             // convert bitset to string.. this is useful in case I need to reverse this later 
        //             cHitString += cBitsetHitData.to_string();
        //             if( cNHits > 0 )
        //             {
        //                 LOG (DEBUG) << BOLDBLUE << "Readout chip " << +cChip->getChipId() << " on link " << +cFe->getLinkId() << RESET;
        //                 //" : " << cBitsetHitData.to_string() << RESET;
        //                 auto cHits = this->GetHits(cFe->getFeId(), cChip->getChipId() );
        //                 for( auto cHit : this->GetHits(cFe->getFeId(), cChip->getChipId() ) )
        //                 {
        //                     LOG (DEBUG) << BOLDBLUE << "\t... Hit in channel " << +cHit << RESET;
        //                 }
        //             }
        //             // now stubs
        //             for( auto cStub : this->StubVector (cFe->getFeId() , cChip->getChipId()) )
        //             {
        //                 std::bitset<16> cBitsetStubs( ( (cChip->getChipId() + 8*(cFe->getFeId()%2)) << 12) | (cStub.getPosition() << 4) | cStub.getBend() );
        //                 cStubString += cBitsetStubs.to_string();
        //                 LOG (DEBUG) << BOLDBLUE << "\t.. stub in seed " << +cStub.getPosition() << " and bench code " << std::bitset<4>(cStub.getBend()) << RESET;
        //                 cFeStubCounter+=1;
        //             }
        //             cCbcCounter++;
        //         } // end of CBC loop
        //     }// end of Fe loop
        //     //for the hit payload, I need to insert the word with the number of CBCs at the index I remembered before 
        //     //cPayload.insert (cCbcPresenceWord, cFirstBitFePayload );
        //     std::bitset<16> cFeHeader(cCbcPresenceWord);
        //     cHitString.insert( cPositionHits,  cFeHeader.to_string() );     
        //     // for the stub payload .. do the same 
        //     std::bitset<6> cStubHeader( (( cFeStubCounter << 1 ) | 0x00 ) & 0x3F); // 0 for 2S , 1 for PS 
        //     LOG (DEBUG) << BOLDBLUE << "FE " << +cLinkId << " hit header " << cFeHeader << " - stub header  " << std::bitset<6>(cStubHeader) << " : " << +cFeStubCounter << " stub(s) found on this link." << RESET;
        //     cStubString.insert( cPositionStubs,  cStubHeader.to_string() ); 
        // }
        // for( size_t cIndex=0; cIndex < 1 + cHitString.length()/64 ; cIndex++ )
        // {
        //     auto cTmp = cHitString.substr(cIndex*64, 64 ); 
        //     std::bitset<64> cBitset( cHitString.substr(cIndex*64, 64)) ;
        //     uint64_t cWord = cBitset.to_ulong() << (64 - cTmp.length());
        //     LOG (DEBUG) << BOLDBLUE << "Hit word " << cTmp.c_str() <<  " -- word " << std::bitset<64>(cWord) << RESET;
        //     cPayload.append( cWord );
        // }
        // for( size_t cIndex=0; cIndex < 1 + cStubString.length()/64 ; cIndex++ )
        // {
        //     auto cTmp = cStubString.substr(cIndex*64, 64 ); 
        //     std::bitset<64> cBitset( cStubString.substr(cIndex*64, 64)) ;
        //     uint64_t cWord = cBitset.to_ulong() << (64 - cTmp.length());
        //     LOG (DEBUG) << BOLDBLUE << "Stub word " << cTmp.c_str() <<  " -- word " << std::bitset<64>(cWord) << RESET;
        //     cStubPayload.append( cWord );
        // }
        // std::vector<uint64_t> cStubData = cStubPayload.Data<uint64_t>();
        // for( auto cStub : cStubData )
        // {
        //    LOG (DEBUG) << BOLDBLUE << std::bitset<64>(cStub) << RESET;
        // }
        //LOG (DEBUG) << BOLDBLUE << +cCbcCounter << " CBCs present in this event... " << +cEnabledFe.size() << " FEs enabled." << RESET;
        
        //cEvent.generateTkHeader (cBeStatus, cCbcCounter, cEnabledFe, pBoard->getConditionDataSet()->getCondDataEnabled(), false);  // Be Status, total number CBC, condition data?, fake data?
        //generate a vector of uint64_t with the chip status
        // if (pBoard->getConditionDataSet()->getDebugMode() != SLinkDebugMode::SUMMARY) // do nothing
        //     cEvent.generateStatus (cStatusPayload.Data<uint64_t>() );

        // //PAYLOAD
        // cEvent.generatePayload (cPayload.Data<uint64_t>() );

        // //STUBS
        // cEvent.generateStubs (cStubPayload.Data<uint64_t>() );

        // // condition data, first update the values in the vector for I2C values
        // uint32_t cTDC = this->GetTDC();
        // pBoard->updateCondData (cTDC);
        // cEvent.generateConditionData (pBoard->getConditionDataSet() );
        // cEvent.generateDAQTrailer();
        return cEvent;
    }
}
