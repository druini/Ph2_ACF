/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/D19cSSAEvent.h"
#include "../HWDescription/OuterTrackerModule.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
    // Event implementation
    D19cSSAEvent::D19cSSAEvent ( const BeBoard* pBoard,  uint32_t pNbSSA, const std::vector<uint32_t>& list )
    {
        SetEvent ( pBoard, pNbSSA, list );
    }

    void D19cSSAEvent::SetEvent ( const BeBoard* pBoard, uint32_t pNbSSA, const std::vector<uint32_t>& list )
    {
	uint32_t EN;
	uint32_t BX;
	LOG (DEBUG) << "EVENT DATA: ";
	uint32_t linecount = 0;
	uint32_t HIPFLAG1 = 0;
	uint32_t HIPFLAG2 = 0;
	std::vector<uint8_t> hits;
	for (auto& D : list) 
	{
		std::bitset<32> X(D);
		linecount++;
		if (linecount == 1)
			{	LOG (DEBUG) << BOLDBLUE << "NEW EVENT: " << RESET;
				uint32_t H1S = (D & 0xFF000000)>>24;
				uint32_t FDM = (D & 0xFF0000)>>16;
				uint32_t BS = (D & 0xFFFF);
				LOG (DEBUG) << BLUE << "HEADER1_SIZE: " << H1S << RESET;
				LOG (DEBUG) << BLUE << "FE_DATA_MASK: " << FDM << RESET;
				LOG (DEBUG) << BLUE << "BLOCK_SIZE: " << BS << RESET;
			}
		if (linecount == 2)
			{
				uint32_t CID = (D & 0xFF000000)>>24;
				uint32_t CHID = (D & 0xFF0000)>>16;
				uint32_t DFV = (D & 0xFF00)>>8;
				uint32_t DS = (D & 0xFF);
				LOG (DEBUG) << BLUE << "CIC_ID: " << CID << RESET;
				LOG (DEBUG) << BLUE << "CHIP_ID: " << CHID << RESET;
				LOG (DEBUG) << BLUE << "DATA_FORMAT_VER: " << DFV << RESET;
				LOG (DEBUG) << BLUE << "DUMMY_SIZE: " << DS << RESET;
			}
		if (linecount == 3)
			{
				uint32_t TDS = (D & 0xFF000000)>>24;
				EN = (D & 0xFFFFFF);
				LOG (DEBUG) << BLUE << "TRIGDATA_SIZE: " << TDS << RESET;
				LOG (DEBUG) << BLUE << "EVENT_NBR(LIA_CNT): " << EN << RESET;
			}
		if (linecount == 4)
			{
				BX = D;
				LOG (DEBUG) << BLUE << "BX_CNT: " << D << RESET;
			}
		if (linecount == 5)
			{
				uint32_t SDS = (D & 0xFF000000)>>24;
				uint32_t TTID = (D & 0xFFFF00)>>8;
				uint32_t TDC = (D & 0xFF);
				LOG (DEBUG) << BLUE << "STUBDATA_SIZE: " << SDS << RESET;
				LOG (DEBUG) << BLUE << "TLU_TRIGGER_ID: " << TTID << RESET;
				LOG (DEBUG) << BLUE << "TDC: " << TDC << RESET;
			}
		if (linecount == 6)
			{
				uint32_t CDM = (D & 0xFF000000)>>24;
				uint32_t H2S = (D & 0xFF0000)>>16;
				uint32_t ES = (D & 0xFFFF);
				LOG (DEBUG) << BLUE << "CHIP_DATA_MASK: " << CDM << RESET;
				LOG (DEBUG) << BLUE << "HEADER2_SIZE: " << H2S << RESET;
				LOG (DEBUG) << BLUE << "EVENT_SIZE: " << ES << RESET;
			}
		if (linecount == 7)
			{
				HIPFLAG1 = (D & 0xFFFF0000)>>16;
				uint32_t L1C = (D & 0xF000)>>12;
				uint32_t BXC = (D & 0x1FF);
				LOG (DEBUG) << BLUE << "L1 Counter: " << L1C << RESET;
				LOG (DEBUG) << BLUE << "BX Counter: " << BXC << RESET;
			}
		if (linecount == 8)
			{
				HIPFLAG2 = ((D & 0xFF)<<16)|HIPFLAG1;
				LOG (DEBUG) << BLUE << "HIP Flag: " << HIPFLAG2 << RESET;
				for (std::size_t i = 1; i < 25; ++i)
				{
					if (X.test(i + 7))
					{
						hits.push_back(i);
					}
				}
			}
		if (linecount == 9 or linecount == 10 or linecount == 11)
			{
				for (std::size_t i = 0; i < 32; ++i)
				{
					if (X.test(i))
					{
						hits.push_back(i+25 + (32*(linecount - 9)));
					}
				}
			}
		if (linecount == 12 or linecount == 13)
			{
				uint32_t C1 = (D & 0xFE)>>1;
				uint32_t f1 = (D & 0x1);
				uint32_t C2 = (D & 0xFE00)>>1;
				uint32_t f2 = (D & 0x100);
				uint32_t C3 = (D & 0xFE0000)>>1;
				uint32_t f3 = (D & 0x10000);
				uint32_t C4 = (D & 0xFE000000)>>1;
				uint32_t f4 = (D & 0x1000000);
				LOG (DEBUG) << BLUE << "Centroid " << 1 + (4*(linecount-12)) << " " << C1 << " + " << f1 + "/2" << RESET;
				LOG (DEBUG) << BLUE << "Centroid " << 2 + (4*(linecount-12)) << " " << C2 << " + " << f2 + "/2" << RESET;
				LOG (DEBUG) << BLUE << "Centroid " << 3 + (4*(linecount-12)) << " " << C3 << " + " << f3 + "/2" << RESET;
				LOG (DEBUG) << BLUE << "Centroid " << 4 + (4*(linecount-12)) << " " << C4 << " + " << f4 + "/2" << RESET;
			}
		if (linecount == 16)
			{

			LOG (INFO) << BLUE << "Stips hits (" << hits.size() << "): " << RESET;;
			for (auto& s : hits) std::cout<<"  "<<static_cast<int16_t>(s);
			std::cout<<std::endl;			

			linecount = 0;
			hits.clear();
			}
	}
        //std::cout << std::endl;
        //for(auto word : list ) std::cout << std::hex << word << std::dec << std::endl;

        // these two values come from width of the hybrid/SSA enabled mask
        uint8_t fMaxHybrids = 8;
        uint8_t fMaxSSAs = 8;

        fEventSize = 0x0000FFFF & list.at (0);

        if (fEventSize != list.size() )
            LOG (ERROR) << "Vector size doesnt match the BLOCK_SIZE in Header1";

        uint8_t header1_size = (0xFF000000 & list.at (0) ) >> 24;

        if (header1_size != D19C_EVENT_HEADER1_SIZE_32)
            LOG (ERROR) << "Header1 size doesnt correspond to the one sent from firmware";

        uint8_t cNFe_software = static_cast<uint8_t> (pBoard->getNFe() );
        uint8_t cFeMask = static_cast<uint8_t> ( (0x00FF0000 & list.at (0) ) >> 16);
        uint8_t cNFe_event = 0;

        for (uint8_t bit = 0; bit < fMaxHybrids; bit++)
        {
            if ( (cFeMask >> bit) & 1)
                cNFe_event ++;
        }

        if (cNFe_software != cNFe_event)
            LOG (ERROR) << "Number of Modules in event header (" << cNFe_event << ") doesnt match the amount of modules defined in firmware.";

        fEventCount = 0x00FFFFFF &  list.at (2);
        
        fTDC = 0x000000FF & list.at (4);
        // correct the tdc value
        if (fTDC >= 5) fTDC-=5;
        else fTDC+=3;

        fBeId = pBoard->getBeId();
        fBeFWType = 0;
        fCBCDataType = (0x0000FF00 & list.at(1)) >> 8;
        fBeStatus = 0;
        fNCbc = pNbSSA;
        fEventDataSize = fEventSize;


        // not iterate through modules
        uint32_t address_offset = D19C_EVENT_HEADER1_SIZE_32;

        for (uint8_t pFeId = 0; pFeId < fMaxHybrids; pFeId++)
        {
            if ( (cFeMask >> pFeId) & 1)
            {

                uint8_t chip_data_mask = static_cast<uint8_t> ( ( (0xFF000000) & list.at (address_offset + 0) ) >> 24);
                uint8_t chips_with_data_nbr = 0;

                for (uint8_t bit = 0; bit < 8; bit++)
                {
                    if ( (chip_data_mask >> bit) & 1)
                        chips_with_data_nbr ++;
                }

                uint8_t header2_size = (0x00FF0000 & list.at (address_offset + 0) ) >> 16;

                if (header2_size != D19C_EVENT_HEADER2_SIZE_32)
                    LOG (ERROR) << "Header2 size doesnt correspond to the one sent from firmware";

                uint16_t fe_data_size = (0x0000FFFF & list.at (address_offset + 0) );

                if (fe_data_size != D19C_EVENT_SIZE_32_SSA * chips_with_data_nbr + D19C_EVENT_HEADER2_SIZE_32)
                    LOG (ERROR) << "Event size doesnt correspond to the one sent from firmware";

                uint32_t data_offset = address_offset + D19C_EVENT_HEADER2_SIZE_32;
		LOG (DEBUG) << BLUE << data_offset << RESET;
                // iterating through the first hybrid chips
                for (uint8_t pSSAId = 0; pSSAId < fMaxSSAs; pSSAId++ )
                {
                    // check if we have data from this chip
                    if ( (chip_data_mask >> pSSAId) & 1)
                    {
                        uint16_t cKey = encodeId (pFeId, pSSAId);

                        uint32_t begin = data_offset;
                        uint32_t end = begin + D19C_EVENT_SIZE_32_SSA;
		  	LOG (DEBUG) << BOLDRED << begin << " " << end << RESET;
                        std::vector<uint32_t> cSSAData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );
			cSSAData.push_back(BX);
			cSSAData.push_back(EN);

                        fEventDataMap[cKey] = cSSAData;

                        data_offset += D19C_EVENT_SIZE_32_SSA;
                    }
                }

                address_offset = address_offset + D19C_EVENT_SIZE_32_SSA * (chips_with_data_nbr) + D19C_EVENT_HEADER2_SIZE_32;
            }
        }

    }
   
    bool D19cSSAEvent::Error(uint8_t pFeId, uint8_t pSSAId, uint32_t i) const
    {
        uint32_t error = Error(pFeId, pSSAId);
        if (i == 0) return ((error & 0x1) >> 0);
        else if (i == 1) return ((error & 0x2) >> 1);
        else {
            LOG(ERROR) << "bit id must be less or equals 1";
            return true;
        }
    }

    uint32_t D19cSSAEvent::Error ( uint8_t pFeId, uint8_t pSSAId ) const
    {
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
            uint32_t cError = ( (cData->second.at(0) & 0x00000003) >> 0 );
            return cError;
        }
        else
        {
            LOG (DEBUG) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;
            return 0;
        }
    }


    uint32_t D19cSSAEvent::GetNStripClusters( uint8_t pFeId, uint8_t pSSAId ) const
     {
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
           uint32_t Nstrip = ( (cData->second.at(0) & 0x001F0000) >> 16);
           return Nstrip;
        }
        else
        {
            LOG (DEBUG) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;
            return 0;
        }
    }




    std::vector<SCluster> D19cSSAEvent::GetStripClusters ( uint8_t pFeId, uint8_t pSSAId) const
    {
        std::vector<SCluster> result;
        if (GetNStripClusters(pFeId, pSSAId) == 0) return result;
	
	SCluster aSCluster;

        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        uint32_t word_id = 0;
        while(2*word_id < GetNStripClusters(pFeId, pSSAId))
	{
		uint32_t word = cData->second.at(1 + word_id);

		aSCluster.fAddress = ((word & 0x0000007f) >> 0) - 1;
		aSCluster.fMip = (word & 0x00000080) >> 7;
		aSCluster.fWidth = (word & 0x00000700) >> 8;
	    	result.push_back(aSCluster);

	    	if((GetNStripClusters(pFeId, pSSAId)-2*word_id) > 1)
                {
			aSCluster.fAddress = ((word & 0x007f0000) >> 16) - 1;
			aSCluster.fMip = (word & 0x00800000) >> 23;
			aSCluster.fWidth = (word & 0x07000000) >> 24;
			result.push_back(aSCluster);
                }
    		word_id += 1;
	}
        return result;
    }

    uint32_t D19cSSAEvent::GetNPixelClusters( uint8_t pFeId, uint8_t pSSAId ) const
    {
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
           uint32_t Npix = ( (cData->second.at(0) & 0x1f000000) >> 24);
           return Npix;
        }
        else
        {
            LOG (DEBUG) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;
            return 0;
        }
    }

    uint32_t D19cSSAEvent::DivideBy2RoundUp(uint32_t value) const
    {
	return (value + value%2)/2;
    }



    std::vector<PCluster> D19cSSAEvent::GetPixelClusters ( uint8_t pFeId, uint8_t pSSAId) const
    {
        std::vector<PCluster> result;
        if (GetNPixelClusters(pFeId, pSSAId) == 0) return result;

        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        uint32_t word_id = 0;
        PCluster aPCluster;
        while(2*word_id < GetNPixelClusters(pFeId, pSSAId))
        {
            uint32_t word = cData->second.at(13 + word_id);
            // -1 to make column 0-119
            aPCluster.fAddress = ((word & 0x0000007f) >> 0) - 1;
            aPCluster.fWidth = (word & 0x00000380) >> 7;
            aPCluster.fZpos = (word & 0x00003C00) >> 10;
            result.push_back(aPCluster);

            if((GetNPixelClusters(pFeId, pSSAId)-2*word_id) > 1)
            {
                // -1 to make column 0-119
                aPCluster.fAddress = ((word & 0x007f0000) >> 16) - 1;
                aPCluster.fWidth = (word & 0x03800000) >> 23;
                aPCluster.fZpos = (word & 0x3C000000) >> 26;
                result.push_back(aPCluster);
            }
            word_id += 1;
        }

        return result;
    }


    uint32_t D19cSSAEvent::GetSync1( uint8_t pFeId, uint8_t pSSAId) const
    {
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        return (cData->second.at(31) & 0x02000000) >> 25;
    }


    uint32_t D19cSSAEvent::GetSync2( uint8_t pFeId, uint8_t pSSAId) const
	{
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        return (cData->second.at(31) & 0x01000000) >> 24;
	}


    uint32_t D19cSSAEvent::GetBX1_NStubs( uint8_t pFeId, uint8_t pSSAId) const
	{
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        return (cData->second.at(31) & 0x00070000) >> 16;
	}

    std::vector<Stub> D19cSSAEvent::StubVector (uint8_t pFeId, uint8_t pSSAId) const
    {
        std::vector<Stub> cStubVec;
        //here create stubs and return the vector
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t pos1 =   (cData->second.at (29) & 0x000000FF) >> 0;
            uint8_t pos2 =   (cData->second.at (29) & 0x00FF0000) >> 16 ;
            uint8_t pos3 =   (cData->second.at (30) & 0x000000FF) >> 0;
            uint8_t pos4 =   (cData->second.at (30) & 0x00FF0000) >> 16 ;
            uint8_t pos5 =   (cData->second.at (31) & 0x000000FF) >> 0;

            uint8_t bend1 = (cData->second.at (29) & 0x0000F000) >> 12;
            uint8_t bend2 = (cData->second.at (29) & 0xF0000000) >> 28;
            uint8_t bend3 = (cData->second.at (30) & 0x0000F000) >> 12;
            uint8_t bend4 = (cData->second.at (30) & 0xF0000000) >> 28;
            uint8_t bend5 = (cData->second.at (31) & 0x0000F000) >> 12;


            uint8_t row1 = (cData->second.at (29) & 0x00000F00) >> 12;
            uint8_t row2 = (cData->second.at (29) & 0x0F000000) >> 28;
            uint8_t row3 = (cData->second.at (30) & 0x00000F00) >> 12;
            uint8_t row4 = (cData->second.at (30) & 0x0F000000) >> 28;
            uint8_t row5 = (cData->second.at (31) & 0x00000F00) >> 12;



            if (pos1 != 0 ) cStubVec.emplace_back (pos1, bend1, row1) ;
            if (pos2 != 0 ) cStubVec.emplace_back (pos2, bend2, row2) ;
            if (pos3 != 0 ) cStubVec.emplace_back (pos3, bend3, row3) ;
            if (pos4 != 0 ) cStubVec.emplace_back (pos4, bend4, row4) ;
            if (pos5 != 0 ) cStubVec.emplace_back (pos5, bend5, row5) ;

        }
        else
            LOG (DEBUG) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;

        return cStubVec;
    }



//These are unimplemented
    std::vector<Cluster> D19cSSAEvent::getClusters ( uint8_t pFeId, uint8_t pSSAId) const
    {
            std::vector<Cluster> result;
            return result;
    }

    uint32_t D19cSSAEvent::PipelineAddress( uint8_t pFeId, uint8_t pSSAId ) const
    {
            return 0;
    }




    void D19cSSAEvent::print ( std::ostream& os) const
    {
    	for (auto const& cKey : this->fEventDataMap)
        {
		os <<std::endl << "event - ";
                uint8_t cFeId;
                uint8_t cSSAId;
                this->decodeId (cKey.first, cFeId, cSSAId);
		os << std::to_string(cFeId) << ", ";
		os << std::to_string(cSSAId) << std::endl;
		uint16_t Key = encodeId (cFeId, cSSAId);
		EventDataMap::const_iterator cData = fEventDataMap.find (Key);

		uint32_t r1 = cData->second.at(0);
		uint32_t r2 = cData->second.at(1);
		uint32_t r3 = cData->second.at(2);
		uint32_t r4 = cData->second.at(3);
		uint32_t r5 = cData->second.at(4);
		uint32_t r6 = cData->second.at(5);
		uint32_t r7 = cData->second.at(6);

		std::vector<uint8_t> hits;
		std::bitset<32> b2(r2);
		std::bitset<32> b3(r3);
		std::bitset<32> b4(r4);
		std::bitset<32> b5(r5);
		for (std::size_t i = 1; i < 25; ++i)
				{
					if (b2.test(i + 7))
					{
						hits.push_back(i);
					}
				}
		for (std::size_t i = 0; i < 32; ++i)
				{
					if (b3.test(i))
					{
						hits.push_back(i+25);
					}
				}
		for (std::size_t i = 0; i < 32; ++i)
				{
					if (b4.test(i))
					{
						hits.push_back(i+25 + 32);
					}
				}
		for (std::size_t i = 0; i < 32; ++i)
				{
					if (b5.test(i))
					{
						hits.push_back(i+25 + 64);
					}
				}

		uint32_t L1C = (r1 & 0xF000)>>12;
		uint32_t EN = cData->second.at(8);
		uint32_t BX = cData->second.at(7);
		os << EN << std::endl;
		os << BX << std::endl;
		os << L1C << std::endl;


		uint32_t C1 = (r6 & 0xFE)>>1;
		uint32_t f1 = (r6 & 0x1);
		uint32_t C2 = (r6 & 0xFE00)>>1;
		uint32_t f2 = (r6 & 0x100);
		uint32_t C3 = (r6 & 0xFE0000)>>1;
		uint32_t f3 = (r6 & 0x10000);
		uint32_t C4 = (r6 & 0xFE000000)>>1;
		uint32_t f4 = (r6 & 0x1000000);
		uint32_t C5 = (r7 & 0xFE)>>1;
		uint32_t f5 = (r7 & 0x1);
		uint32_t C6 = (r7 & 0xFE00)>>1;
		uint32_t f6 = (r7 & 0x100);
		uint32_t C7 = (r7 & 0xFE0000)>>1;
		uint32_t f7 = (r7 & 0x10000);
		uint32_t C8 = (r7 & 0xFE000000)>>1;
		uint32_t f8 = (r7 & 0x1000000);
		
		
		os << std::to_string(C1)<<" "<< std::to_string(C2)<<" "<< std::to_string(C3)<<" "<< std::to_string(C4)<<" "<< std::to_string(C5)<<" "<< std::to_string(C6)<<" "<< std::to_string(C7)<<" "<< std::to_string(C8)<<" "<<std::endl;
		os << std::to_string(f1)<<" "<< std::to_string(f2)<<" "<< std::to_string(f3)<<" "<< std::to_string(f4)<<" "<< std::to_string(f5)<<" "<< std::to_string(f6)<<" "<< std::to_string(f7)<<" "<< std::to_string(f8)<<" "<<std::endl;

		uint32_t HIPFLAG1 = 0;
		uint32_t HIPFLAG2 = 0;

		HIPFLAG1 = (r1 & 0xFFFF0000)>>16;
		HIPFLAG2 = ((r2 & 0xFF)<<16)|HIPFLAG1;
		os << std::to_string(HIPFLAG2)<<std::endl;
		
		for (auto& s : hits) os <<"  "<<static_cast<int16_t>(s);


	}
    }

    std::string D19cSSAEvent::HexString() const
    {
        return "";
    }





//BELOW: Not sure what most of these do -- probably dont work
    bool D19cSSAEvent::DataBit ( uint8_t pFeId, uint8_t pSSAId, uint32_t i ) const
        {
            if ( i >= NCHANNELS )
                return 0;

            uint32_t cWordP = 0;
            uint32_t cBitP = 0;
            calculate_address (cWordP, cBitP, i);

            uint16_t cKey = encodeId (pFeId, pSSAId);
            EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

            if (cData != std::end (fEventDataMap) )
            {
                if (cWordP >= cData->second.size() ) return false;

                return ( (cData->second.at (cWordP) >> (cBitP) ) & 0x1);
            }
            else
            {
                LOG (DEBUG) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;
                return false;
            }

            //return Bit ( pFeId, pSSAId, i + OFFSET_SSADATA );
        }
    std::vector<uint32_t> D19cSSAEvent::GetHits (uint8_t pFeId, uint8_t pSSAId) const
        {
            std::vector<uint32_t> hits;
            uint16_t cKey = encodeId (pFeId, pSSAId);
            EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

            if (cData != std::end (fEventDataMap) )
            {
		uint32_t r2 = cData->second.at(1);
		uint32_t r3 = cData->second.at(2);
		uint32_t r4 = cData->second.at(3);
		uint32_t r5 = cData->second.at(4);

		std::bitset<32> b2(r2);
		std::bitset<32> b3(r3);
		std::bitset<32> b4(r4);
		std::bitset<32> b5(r5);
		for (std::size_t i = 1; i < 25; ++i)
				{
					if (b2.test(i + 7))
					{
						hits.push_back(i);
					}
				}
		for (std::size_t i = 0; i < 32; ++i)
				{
					if (b3.test(i))
					{
						hits.push_back(i+25);
					}
				}
		for (std::size_t i = 0; i < 32; ++i)
				{
					if (b4.test(i))
					{
						hits.push_back(i+25 + 32);
					}
				}
		for (std::size_t i = 0; i < 32; ++i)
				{
					if (b5.test(i))
					{
						hits.push_back(i+25 + 64);
					}
				}
            }
            else
                LOG (INFO) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;

            return hits;
        }



    std::string D19cSSAEvent::DataHexString ( uint8_t pFeId, uint8_t pSSAId ) const
    {
                std::stringbuf tmp;
                std::ostream os ( &tmp );
                std::ios oldState (nullptr);
                oldState.copyfmt (os);
                os << std::hex << std::setfill ('0');

                //get the SSA event for pFeId and pSSAId into vector<32bit> SSAData
                std::vector< uint32_t > SSAData;
                GetCbcEvent (pFeId, pSSAId, SSAData);

                // trigdata
                os << std::endl;
                os << std::setw (8) << SSAData.at (0) << std::endl;
                os << std::setw (8) << SSAData.at (1) << std::endl;
                os << std::setw (8) << SSAData.at (2) << std::endl;
                os << std::setw (8) << (SSAData.at (3) & 0x7FFFFFFF) << std::endl;
                os << std::setw (8) << SSAData.at (4) << std::endl;
                os << std::setw (8) << SSAData.at (5) << std::endl;
                os << std::setw (8) << SSAData.at (6) << std::endl;
                os << std::setw (8) << (SSAData.at (7) & 0x7FFFFFFF) << std::endl;
                // l1cnt
                os << std::setw (3) << ( (SSAData.at (8) & 0x01FF0000) >> 16) << std::endl;
                // pipeaddr
                os << std::setw (3) << ( (SSAData.at (8) & 0x00001FF0) >> 4) << std::endl;
                // stubdata
                os << std::setw (8) << SSAData.at (9) << std::endl;
                os << std::setw (8) << SSAData.at (10) << std::endl;

                os.copyfmt (oldState);

                return tmp.str();
    }



    uint32_t D19cSSAEvent::GetNHits (uint8_t pFeId, uint8_t pSSAId) const
    {
        return GetNPixelClusters(pFeId, pSSAId) + GetNStripClusters(pFeId, pSSAId);
    }




    bool D19cSSAEvent::StubBit ( uint8_t pFeId, uint8_t pSSAId ) const
    {
        //here just OR the stub positions
        uint16_t cKey = encodeId (pFeId, pSSAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t pos1 = (cData->second.at (9) & 0x000000FF);
            uint8_t pos2 = (cData->second.at (9) & 0x0000FF00) >> 8;
            uint8_t pos3 = (cData->second.at (9) & 0x00FF0000) >> 16;
            return (pos1 || pos2 || pos3);
        }
        else
        {
            LOG (DEBUG) << "Event: FE " << +pFeId << " SSA " << +pSSAId << " is not found." ;
            return false;
        }
    }


    SLinkEvent D19cSSAEvent::GetSLinkEvent (  BeBoard* pBoard) const
    {
        uint16_t cSSACounter = 0;
        std::set<uint8_t> cEnabledFe;

        //payload for the status bits
        GenericPayload cStatusPayload;
        //for the payload and the stubs
        GenericPayload cPayload;
        GenericPayload cStubPayload;

        for (auto cFe : pBoard->fModuleVector)
        {
            uint8_t cFeId = cFe->getFeId();

            // firt get the list of enabled front ends
            if (cEnabledFe.find (cFeId) == std::end (cEnabledFe) )
                cEnabledFe.insert (cFeId);

            //now on to the payload
            uint16_t cSSAPresenceWord = 0;
            int cFirstBitFePayload = cPayload.get_current_write_position();
            int cFirstBitFeStub = cStubPayload.get_current_write_position();
            //stub counter per FE
            uint8_t cFeStubCounter = 0;

            for (auto cSSA : static_cast<OuterTrackerModule*>(cFe)->fSSAVector)
            {
                uint8_t cSSAId = cSSA->getSSAId();
                uint16_t cKey = encodeId (cFeId, cSSAId);
                EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

                if (cData != std::end (fEventDataMap) )
                {
                    uint16_t cError = ( cData->second.at (8) & 0x00000003 );

                    //now get the SSA status summary
                    if (pBoard->getConditionDataSet()->getDebugMode() == SLinkDebugMode::ERROR)
                        cStatusPayload.append ( (cError != 0) ? 1 : 0);

                    else if (pBoard->getConditionDataSet()->getDebugMode() == SLinkDebugMode::FULL)
                    {
                        //assemble the error bits (63, 62, pipeline address and L1A counter) into a status word
                        uint16_t cPipeAddress = (cData->second.at (8) & 0x00001FF0) >> 4;
                        uint16_t cL1ACounter = (cData->second.at (8) &  0x01FF0000) >> 16;
                        uint32_t cStatusWord = cError << 18 | cPipeAddress << 9 | cL1ACounter;
                        cStatusPayload.append (cStatusWord, 20);
                    }

                    //generate the payload
                    //the first line sets the SSA presence bits
                    cSSAPresenceWord |= 1 << cSSAId;

                    //first SSA channel data word
                    //since the D19C FW splits in even and odd channels, I need to
                    //Morton-encode these bits into words of the double size
                    //but first I need to reverse the bit order
                    uint32_t cFirstChanWordEven = reverse_bits (cData->second.at (3) ) >> 1;
                    uint32_t cFirstChanWordOdd = reverse_bits (cData->second.at (7) ) >> 1;
                    //now both words are swapped to have channel 0/1 at bit 30 and channel 60/61 at bit 0
                    //I can now interleave/morton encode and append them but only the 62 LSBs
                    cPayload.appendD19CData (cFirstChanWordEven, cFirstChanWordOdd, 62);

                    for (size_t i = 3; i > 0; i--)
                    {
                        uint32_t cEvenWord = reverse_bits (cData->second.at (i - 1) );
                        uint32_t cOddWord = reverse_bits (cData->second.at (i + 3) );
                        cPayload.appendD19CData (cEvenWord, cOddWord);
                    }

                    //don't forget the two padding 0s
                    cPayload.padZero (2);

                    //stubs
                    uint8_t pos1 =  (cData->second.at (9) &  0x000000FF);
                    uint8_t pos2 =  (cData->second.at (9) & 0x0000FF00) >> 8;
                    uint8_t pos3 =  (cData->second.at (9) & 0x00FF0000) >> 16;
                    uint8_t bend1 = (cData->second.at (10) & 0x00000F00) >> 8;
                    uint8_t bend2 = (cData->second.at (10) & 0x000F0000) >> 16;
                    uint8_t bend3 = (cData->second.at (10) & 0x0F000000) >> 24;

                    if (pos1 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cSSAId & 0x0F) << 12 | pos1 << 4 | (bend1 & 0xF)) );
                        cFeStubCounter++;
                    }

                    if (pos2 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cSSAId & 0x0F) << 12 | pos2 << 4 | (bend2 & 0xF)) );
                        cFeStubCounter++;
                    }

                    if (pos3 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cSSAId & 0x0F) << 12 | pos3 << 4 | (bend3 & 0xF)) );
                        cFeStubCounter++;
                    }
                }

                cSSACounter++;
            } // end of SSA loop

            //for the payload, I need to insert the status word at the index I remembered before
            cPayload.insert (cSSAPresenceWord, cFirstBitFePayload );

            //for the stubs for this FE, I need to prepend a 5 bit counter shifted by 1 to the right (to account for the 0 bit)
            cStubPayload.insert ( (cFeStubCounter & 0x1F) << 1, cFirstBitFeStub, 6);

        } // end of Fe loop

        uint32_t cEvtCount = this->GetEventCount();
        uint16_t cBunch = static_cast<uint16_t> (this->GetBunch() );
        uint32_t cBeStatus = this->fBeStatus;
        SLinkEvent cEvent (EventType::VR, pBoard->getConditionDataSet()->getDebugMode(), FrontEndType::SSA, cEvtCount, cBunch, SOURCE_ID );
        cEvent.generateTkHeader (cBeStatus, cSSACounter, cEnabledFe, pBoard->getConditionDataSet()->getCondDataEnabled(), false);  // Be Status, total number SSA, condition data?, fake data?

        //generate a vector of uint64_t with the chip status
        if (pBoard->getConditionDataSet()->getDebugMode() != SLinkDebugMode::SUMMARY) // do nothing
            cEvent.generateStatus (cStatusPayload.Data<uint64_t>() );

        //PAYLOAD
        cEvent.generatePayload (cPayload.Data<uint64_t>() );

        //STUBS
        cEvent.generateStubs (cStubPayload.Data<uint64_t>() );

        // condition data, first update the values in the vector for I2C values
        uint32_t cTDC = this->GetTDC();
        pBoard->updateCondData (cTDC);
        cEvent.generateConditionData (pBoard->getConditionDataSet() );

        cEvent.generateDAQTrailer();

        return cEvent;
    }




    std::string D19cSSAEvent::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return "";
    }


    std::string D19cSSAEvent::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::ostringstream os;

        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);

        for (auto cStub : cStubVector)
            os << std::bitset<8> (cStub.getPosition() ) << " " << std::bitset<4> (cStub.getBend() ) << " ";

        return os.str();


        //return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }


    std::string D19cSSAEvent::DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < NCHANNELS; ++i )
            {

                uint32_t cWordP = 0;
                uint32_t cBitP = 0;
                calculate_address (cWordP, cBitP, i);

                if ( cWordP >= cData->second.size() ) break;

                os << ( ( cData->second.at (cWordP) >> (cBitP ) ) & 0x1 );
            }

            return os.str();

        }
        else
        {
            LOG (DEBUG) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }

        //return BitString ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
    }



    std::vector<bool> D19cSSAEvent::DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::vector<bool> blist;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < NCHANNELS; ++i )
            {

                uint32_t cWordP = 0;
                uint32_t cBitP = 0;
                calculate_address (cWordP, cBitP, i);

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second.at (cWordP) >> (cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (DEBUG) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }




    std::vector<bool> D19cSSAEvent::DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const
    {
        std::vector<bool> blist;

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for ( auto i :  channelList )
            {

                uint32_t cWordP = 0;
                uint32_t cBitP = 0;
                calculate_address (cWordP, cBitP, i);

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second.at (cWordP) >> (cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (DEBUG) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }




}
