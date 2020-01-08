/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/D19cMPAEvent.h"
#include "../HWDescription/OuterTrackerModule.h"

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
    // Event implementation
    D19cMPAEvent::D19cMPAEvent ( const BeBoard* pBoard,  uint32_t pNbMPA, const std::vector<uint32_t>& list )
    {
        SetEvent ( pBoard, pNbMPA, list );
    }

    void D19cMPAEvent::SetEvent ( const BeBoard* pBoard, uint32_t pNbMPA, const std::vector<uint32_t>& list )
    {
        //std::cout << std::endl;
        //for(auto word : list ) std::cout << std::hex << word << std::dec << std::endl;

        // these two values come from width of the hybrid/MPA enabled mask
        uint8_t fMaxHybrids = 8;
        uint8_t fMaxMPAs = 8;

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
        fBunch = 0xFFFFFFFF & list.at (3);

        fTDC = 0x000000FF & list.at (4);
        // correct the tdc value
        if (fTDC >= 5) fTDC-=5;
        else fTDC+=3;

        fBeId = pBoard->getBeId();
        fBeFWType = 0;
        fCBCDataType = (0x0000FF00 & list.at(1)) >> 8;
        fBeStatus = 0;
        fNCbc = pNbMPA;
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

                if (fe_data_size != D19C_EVENT_SIZE_32_MPA * chips_with_data_nbr + D19C_EVENT_HEADER2_SIZE_32)
                    LOG (ERROR) << "Event size doesnt correspond to the one sent from firmware";

                uint32_t data_offset = address_offset + D19C_EVENT_HEADER2_SIZE_32;

                // iterating through the first hybrid chips
                for (uint8_t pMPAId = 0; pMPAId < fMaxMPAs; pMPAId++ )
                {
                    // check if we have data from this chip
                    if ( (chip_data_mask >> pMPAId) & 1)
                    {

                        //check the sync bits
                        uint8_t cSyncBit1 = (list.at(data_offset+31) & 0x02000000) >> 25;
                        uint8_t cSyncBit2 = (list.at(data_offset+31) & 0x01000000) >> 24;

                        if (cSyncBit1!=1) LOG (INFO) << BOLDRED << "Warning, sync bit 1 not 1, data frame probably misaligned!" << RESET;
                        if (cSyncBit2!=0) LOG (INFO) << BOLDRED << "Warning, sync bit 2 not 0, data frame probably misaligned!" << RESET;

                        uint16_t cKey = encodeId (pFeId, pMPAId);

                        uint32_t begin = data_offset;
                        uint32_t end = begin + D19C_EVENT_SIZE_32_MPA;

                        std::vector<uint32_t> cMPAData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );

                        fEventDataMap[cKey] = cMPAData;

                        data_offset += D19C_EVENT_SIZE_32_MPA;
                    }
                }

                address_offset = address_offset + D19C_EVENT_SIZE_32_MPA * (chips_with_data_nbr) + D19C_EVENT_HEADER2_SIZE_32;
            }
        }

    }

    bool D19cMPAEvent::Error(uint8_t pFeId, uint8_t pMPAId, uint32_t i) const
    {
        uint32_t error = Error(pFeId, pMPAId);
        if (i == 0) return ((error & 0x1) >> 0);
        else if (i == 1) return ((error & 0x2) >> 1);
        else {
            LOG(ERROR) << "bit id must be less or equals 1";
            return true;
        }
    }

    uint32_t D19cMPAEvent::Error ( uint8_t pFeId, uint8_t pMPAId ) const
    {
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
            uint32_t cError = ( (cData->second.at(0) & 0x00000003) >> 0 );
            return cError;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;
            return 0;
        }
    }

    uint32_t D19cMPAEvent::GetMPAL1Counter( uint8_t pFeId, uint8_t pMPAId ) const
     {
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
           uint32_t L1cnt = ( (cData->second.at(0) & 0x00001FF0) >> 4);
           return L1cnt;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;
            return 0;
        }
    }





    uint32_t D19cMPAEvent::GetNStripClusters( uint8_t pFeId, uint8_t pMPAId ) const
     {
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
           uint32_t Nstrip = ( (cData->second.at(0) & 0x001F0000) >> 16);
           return Nstrip;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;
            return 0;
        }
    }




    std::vector<SCluster> D19cMPAEvent::GetStripClusters ( uint8_t pFeId, uint8_t pMPAId) const
    {
        std::vector<SCluster> result;
        if (GetNStripClusters(pFeId, pMPAId) == 0) return result;
	
	SCluster aSCluster;

        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        uint32_t word_id = 0;
        while(2*word_id < GetNStripClusters(pFeId, pMPAId))
	{
		uint32_t word = cData->second.at(1 + word_id);

		aSCluster.fAddress = ((word & 0x0000007f) >> 0) - 1;
		aSCluster.fMip = (word & 0x00000080) >> 7;
		aSCluster.fWidth = (word & 0x00000700) >> 8;
	    	result.push_back(aSCluster);

	    	if((GetNStripClusters(pFeId, pMPAId)-2*word_id) > 1)
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

    uint32_t D19cMPAEvent::GetNPixelClusters( uint8_t pFeId, uint8_t pMPAId ) const
    {
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
           uint32_t Npix = ( (cData->second.at(0) & 0x1f000000) >> 24);
           return Npix;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;
            return 0;
        }
    }

    uint32_t D19cMPAEvent::DivideBy2RoundUp(uint32_t value) const
    {
	return (value + value%2)/2;
    }



    std::vector<PCluster> D19cMPAEvent::GetPixelClusters ( uint8_t pFeId, uint8_t pMPAId) const
    {
        std::vector<PCluster> result;
        if (GetNPixelClusters(pFeId, pMPAId) == 0) return result;

        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        uint32_t word_id = 0;
        PCluster aPCluster;
        while(2*word_id < GetNPixelClusters(pFeId, pMPAId))
        {
            uint32_t word = cData->second.at(13 + word_id);
            // -1 to make column 0-119
            aPCluster.fAddress = ((word & 0x0000007f) >> 0) - 1;
            aPCluster.fWidth = (word & 0x00000380) >> 7;
            aPCluster.fZpos = (word & 0x00003C00) >> 10;
            result.push_back(aPCluster);

            if((GetNPixelClusters(pFeId, pMPAId)-2*word_id) > 1)
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


    uint32_t D19cMPAEvent::GetSync1( uint8_t pFeId, uint8_t pMPAId) const
    {
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        return (cData->second.at(31) & 0x02000000) >> 25;
    }


    uint32_t D19cMPAEvent::GetSync2( uint8_t pFeId, uint8_t pMPAId) const
	{
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        return (cData->second.at(31) & 0x01000000) >> 24;
	}


    uint32_t D19cMPAEvent::GetBX1_NStubs( uint8_t pFeId, uint8_t pMPAId) const
	{
        uint16_t cKey = encodeId (pFeId, pMPAId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        return (cData->second.at(31) & 0x00070000) >> 16;
	}

    std::vector<Stub> D19cMPAEvent::StubVector (uint8_t pFeId, uint8_t pMPAId) const
    {
        std::vector<Stub> cStubVec;
        //here create stubs and return the vector
        uint16_t cKey = encodeId (pFeId, pMPAId);
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
            LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;

        return cStubVec;
    }



//These are unimplemented
    std::vector<Cluster> D19cMPAEvent::getClusters ( uint8_t pFeId, uint8_t pMPAId) const
    {
            std::vector<Cluster> result;
            return result;
    }

    uint32_t D19cMPAEvent::PipelineAddress( uint8_t pFeId, uint8_t pMPAId ) const
    {
            return 0;
    }




    void D19cMPAEvent::print ( std::ostream& os) const
    {
            os << "MPA Event #" << std::endl;
            for (auto const& cKey : this->fEventDataMap)
            {
                uint8_t cFeId;
                uint8_t cMpaId;
                this->decodeId (cKey.first, cFeId, cMpaId);
                os << "Hybrid " << +cFeId << ", Chip " << +cMpaId << std::endl;
                os << "\t L1 Counter: " << GetMPAL1Counter(cFeId, cMpaId) << std::endl;
                os << "\t Error: " << Error(cFeId, cMpaId) << std::endl;
                os << "\t N Pixel Clusters: " << GetNPixelClusters(cFeId, cMpaId) << std::endl;
                for(auto pcluster: GetPixelClusters(cFeId, cMpaId)) os << "\t\t Cluster Address: " << +pcluster.fAddress << ", Width: " << +pcluster.fWidth << ", ZPos: " << +pcluster.fZpos << std::endl;
                os << "\t N Strip Clusters: " << GetNStripClusters(cFeId, cMpaId) << std::endl;
                for(auto scluster: GetStripClusters(cFeId, cMpaId)) os << "\t\t Cluster Address: " << +scluster.fAddress << ", Width: " << +scluster.fWidth << ", MIP: " << +scluster.fMip << std::endl;
                os << std::endl;
            }
    }

    std::string D19cMPAEvent::HexString() const
    {
        return "";
    }





//BELOW: Not sure what most of these do -- probably dont work
    bool D19cMPAEvent::DataBit ( uint8_t pFeId, uint8_t pMPAId, uint32_t i ) const
        {
            if ( i >= NCHANNELS )
                return 0;

            uint32_t cWordP = 0;
            uint32_t cBitP = 0;
            calculate_address (cWordP, cBitP, i);

            uint16_t cKey = encodeId (pFeId, pMPAId);
            EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

            if (cData != std::end (fEventDataMap) )
            {
                if (cWordP >= cData->second.size() ) return false;

                return ( (cData->second.at (cWordP) >> (cBitP) ) & 0x1);
            }
            else
            {
                LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;
                return false;
            }

            //return Bit ( pFeId, pMPAId, i + OFFSET_MPADATA );
        }
    std::vector<uint32_t> D19cMPAEvent::GetHits (uint8_t pFeId, uint8_t pMPAId) const
        {
            std::vector<uint32_t> cHits;
            uint16_t cKey = encodeId (pFeId, pMPAId);
            EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

            if (cData != std::end (fEventDataMap) )
            {
                for ( uint32_t i = 0; i < NCHANNELS; ++i )
                {
                    uint32_t cWordP = 0;
                    uint32_t cBitP = 0;
                    calculate_address (cWordP, cBitP, i);

                    if ( cWordP >= cData->second.size() ) break;

                    if ( ( cData->second.at (cWordP) >> ( cBitP ) ) & 0x1) cHits.push_back (i);
                }
            }
            else
                LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;

            return cHits;
        }



    std::string D19cMPAEvent::DataHexString ( uint8_t pFeId, uint8_t pMPAId ) const
    {
                std::stringbuf tmp;
                std::ostream os ( &tmp );
                std::ios oldState (nullptr);
                oldState.copyfmt (os);
                os << std::hex << std::setfill ('0');

                //get the MPA event for pFeId and pMPAId into vector<32bit> MPAData
                std::vector< uint32_t > MPAData;
                GetCbcEvent (pFeId, pMPAId, MPAData);

                // trigdata
                os << std::endl;
                os << std::setw (8) << MPAData.at (0) << std::endl;
                os << std::setw (8) << MPAData.at (1) << std::endl;
                os << std::setw (8) << MPAData.at (2) << std::endl;
                os << std::setw (8) << (MPAData.at (3) & 0x7FFFFFFF) << std::endl;
                os << std::setw (8) << MPAData.at (4) << std::endl;
                os << std::setw (8) << MPAData.at (5) << std::endl;
                os << std::setw (8) << MPAData.at (6) << std::endl;
                os << std::setw (8) << (MPAData.at (7) & 0x7FFFFFFF) << std::endl;
                // l1cnt
                os << std::setw (3) << ( (MPAData.at (8) & 0x01FF0000) >> 16) << std::endl;
                // pipeaddr
                os << std::setw (3) << ( (MPAData.at (8) & 0x00001FF0) >> 4) << std::endl;
                // stubdata
                os << std::setw (8) << MPAData.at (9) << std::endl;
                os << std::setw (8) << MPAData.at (10) << std::endl;

                os.copyfmt (oldState);

                return tmp.str();
    }



    uint32_t D19cMPAEvent::GetNHits (uint8_t pFeId, uint8_t pMPAId) const
    {
        return GetNPixelClusters(pFeId, pMPAId) + GetNStripClusters(pFeId, pMPAId);
    }




    bool D19cMPAEvent::StubBit ( uint8_t pFeId, uint8_t pMPAId ) const
    {
        //here just OR the stub positions
        uint16_t cKey = encodeId (pFeId, pMPAId);
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
            LOG (INFO) << "Event: FE " << +pFeId << " MPA " << +pMPAId << " is not found." ;
            return false;
        }
    }


    SLinkEvent D19cMPAEvent::GetSLinkEvent (  BeBoard* pBoard) const
    {
        uint16_t cMPACounter = 0;
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
            uint16_t cMPAPresenceWord = 0;
            int cFirstBitFePayload = cPayload.get_current_write_position();
            int cFirstBitFeStub = cStubPayload.get_current_write_position();
            //stub counter per FE
            uint8_t cFeStubCounter = 0;

            for (auto cMPA : static_cast<OuterTrackerModule*>(cFe)->fMPAVector)
            {
                uint8_t cMPAId = cMPA->getMPAId();
                uint16_t cKey = encodeId (cFeId, cMPAId);
                EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

                if (cData != std::end (fEventDataMap) )
                {
                    uint16_t cError = ( cData->second.at (8) & 0x00000003 );

                    //now get the MPA status summary
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
                    //the first line sets the MPA presence bits
                    cMPAPresenceWord |= 1 << cMPAId;

                    //first MPA channel data word
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
                        cStubPayload.append ( uint16_t ( (cMPAId & 0x0F) << 12 | pos1 << 4 | (bend1 & 0xF)) );
                        cFeStubCounter++;
                    }

                    if (pos2 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cMPAId & 0x0F) << 12 | pos2 << 4 | (bend2 & 0xF)) );
                        cFeStubCounter++;
                    }

                    if (pos3 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cMPAId & 0x0F) << 12 | pos3 << 4 | (bend3 & 0xF)) );
                        cFeStubCounter++;
                    }
                }

                cMPACounter++;
            } // end of MPA loop

            //for the payload, I need to insert the status word at the index I remembered before
            cPayload.insert (cMPAPresenceWord, cFirstBitFePayload );

            //for the stubs for this FE, I need to prepend a 5 bit counter shifted by 1 to the right (to account for the 0 bit)
            cStubPayload.insert ( (cFeStubCounter & 0x1F) << 1, cFirstBitFeStub, 6);

        } // end of Fe loop

        uint32_t cEvtCount = this->GetEventCount();
        uint16_t cBunch = static_cast<uint16_t> (this->GetBunch() );
        uint32_t cBeStatus = this->fBeStatus;
        SLinkEvent cEvent (EventType::VR, pBoard->getConditionDataSet()->getDebugMode(), FrontEndType::MPA, cEvtCount, cBunch, SOURCE_ID );
        cEvent.generateTkHeader (cBeStatus, cMPACounter, cEnabledFe, pBoard->getConditionDataSet()->getCondDataEnabled(), false);  // Be Status, total number MPA, condition data?, fake data?

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




    std::string D19cMPAEvent::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return "";
    }


    std::string D19cMPAEvent::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::ostringstream os;

        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);

        for (auto cStub : cStubVector)
            os << std::bitset<8> (cStub.getPosition() ) << " " << std::bitset<4> (cStub.getBend() ) << " ";

        return os.str();


        //return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }


    std::string D19cMPAEvent::DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const
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
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }

        //return BitString ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
    }



    std::vector<bool> D19cMPAEvent::DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const
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
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }




    std::vector<bool> D19cMPAEvent::DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const
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
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }




}
