/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/D19cCbc3Event.h"

using namespace Ph2_HwDescription;


namespace Ph2_HwInterface {

    // Event implementation
    D19cCbc3Event::D19cCbc3Event ( const BeBoard* pBoard,  uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        SetEvent ( pBoard, pNbCbc, list );
    }


    //D19cCbc3Event::D19cCbc3Event ( const Event& pEvent ) :
    //fBunch ( pEvent.fBunch ),
    //fOrbit ( pEvent.fOrbit ),
    //fLumi ( pEvent.fLumi ),
    //fEventCount ( pEvent.fEventCount ),
    //fEventCountCBC ( pEvent.fEventCountCBC ),
    //fTDC ( pEvent.fTDC ),
    //fEventSize (pEvent.fEventSize),
    //fEventDataMap ( pEvent.fEventDataMap )
    //{

    //}

    void D19cCbc3Event::SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        // these two values come from width of the hybrid/cbc enabled mask
        uint8_t fMaxHybrids = 8;
        uint8_t fMaxCBCs = 8;

        fEventSize = 0x0000FFFF & list.at (0);

        if (fEventSize != list.size() )
            LOG (ERROR) << "Vector size doesnt match the BLOCK_SIZE in Header1";

        uint8_t header1_size = (0xFF000000 & list.at (0) ) >> 24;

        if (header1_size != D19C_EVENT_HEADER1_SIZE_32_CBC3)
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

        fDummySize = 0x000000FF & list.at (1);
        fEventCount = 0x00FFFFFF &  list.at (2);
        fBunch = 0xFFFFFFFF & list.at (3);
        fTDC = 0x000000FF & list.at (4);
        fTLUTriggerID = (0x00FFFF00 & list.at (4) ) >> 8;

        fBeId = pBoard->getBeId();
        fBeFWType = 0;
        fCBCDataType = (0x0000FF00 & list.at(1)) >> 8;
        fBeStatus = 0;
        fNCbc = pNbCbc;
        fEventDataSize = fEventSize;


        // not iterate through modules
        uint32_t address_offset = D19C_EVENT_HEADER1_SIZE_32_CBC3;

        for (uint8_t cFeId = 0; cFeId < fMaxHybrids; cFeId++)
        {
            if ( (cFeMask >> cFeId) & 1)
            {

                uint8_t chip_data_mask = static_cast<uint8_t> ( ( (0xFF000000) & list.at (address_offset + 0) ) >> 24);
                uint8_t chips_with_data_nbr = 0;

                for (uint8_t bit = 0; bit < 8; bit++)
                {
                    if ( (chip_data_mask >> bit) & 1)
                        chips_with_data_nbr ++;
                }

                uint8_t header2_size = (0x00FF0000 & list.at (address_offset + 0) ) >> 16;

                if (header2_size != D19C_EVENT_HEADER2_SIZE_32_CBC3)
                    LOG (ERROR) << "Header2 size doesnt correspond to the one sent from firmware";

                uint16_t fe_data_size = (0x0000FFFF & list.at (address_offset + 0) );

                if (fe_data_size != CBC_EVENT_SIZE_32_CBC3 * chips_with_data_nbr + D19C_EVENT_HEADER2_SIZE_32_CBC3)
                    LOG (ERROR) << "Event size doesnt correspond to the one sent from firmware";

                uint32_t data_offset = address_offset + D19C_EVENT_HEADER2_SIZE_32_CBC3;

                // iterating through the first hybrid chips
                for (uint8_t cCbcId = 0; cCbcId < fMaxCBCs; cCbcId++ )
                {
                    // check if we have data from this chip
                    if ( (chip_data_mask >> cCbcId) & 1)
                    {

                        //check the sync bit
                        uint8_t cSyncBit = (0x00000008 & list.at(data_offset+10)) >> 3;

                        if (!cSyncBit) LOG (INFO) << BOLDRED << "Warning, sync bit not 1, data frame probably misaligned!" << RESET;

                        uint16_t cKey = encodeId (cFeId, cCbcId);

                        uint32_t begin = data_offset;
                        uint32_t end = begin + CBC_EVENT_SIZE_32_CBC3;

                        std::vector<uint32_t> cCbcData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );

                        fEventDataMap[cKey] = cCbcData;

                        data_offset += CBC_EVENT_SIZE_32_CBC3;
                    }
                }

                address_offset = address_offset + CBC_EVENT_SIZE_32_CBC3 * (chips_with_data_nbr) + D19C_EVENT_HEADER2_SIZE_32_CBC3;
            }
        }

    }


    std::string D19cCbc3Event::HexString() const
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

    std::string D19cCbc3Event::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
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
        os << std::setw (3) << ( (cbcData.at (0) & 0x01FF0000) >> 16) << std::endl;
        // pipeaddr
        os << std::setw (3) << ( (cbcData.at (0) & 0x00001FF0) >> 4) << std::endl;
        // trigdata
        os << std::endl;
        os << std::setw (8) << cbcData.at (1) << std::endl;
        os << std::setw (8) << cbcData.at (2) << std::endl;
        os << std::setw (8) << cbcData.at (3) << std::endl;
        os << std::setw (8) << (cbcData.at (4) & 0x7FFFFFFF) << std::endl;
        os << std::setw (8) << cbcData.at (5) << std::endl;
        os << std::setw (8) << cbcData.at (6) << std::endl;
        os << std::setw (8) << cbcData.at (7) << std::endl;
        os << std::setw (8) << (cbcData.at (8) & 0x7FFFFFFF) << std::endl;
        // stubdata
        os << std::setw (8) << cbcData.at (9) << std::endl;
        os << std::setw (8) << cbcData.at (10) << std::endl;

        os.copyfmt (oldState);

        return tmp.str();
    }

    // NOT READY (what is i??????????)
    bool D19cCbc3Event::Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        return Bit ( pFeId, pCbcId, D19C_OFFSET_ERROR_CBC3 );
    }

    uint32_t D19cCbc3Event::Error ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            // buf overflow and lat error
            uint32_t cError = ( (cData->second.at (0) & 0x00000003) >> 0 );;
            return cError;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    uint32_t D19cCbc3Event::PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint32_t cPipeAddress = ( (cData->second.at (0) & 0x00001FF0) >> 4 );
            return cPipeAddress;
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    bool D19cCbc3Event::DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        if ( i >= NCHANNELS )
            return 0;

        uint32_t cWordP = 0;
        uint32_t cBitP = 0;
        calculate_address (cWordP, cBitP, i);

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            if (cWordP >= cData->second.size() ) return false;

            return ( (cData->second.at (cWordP) >> (cBitP) ) & 0x1);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return false;
        }

        //return Bit ( pFeId, pCbcId, i + OFFSET_CBCDATA );
    }

    std::string D19cCbc3Event::DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const
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

    std::vector<bool> D19cCbc3Event::DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const
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

    std::vector<bool> D19cCbc3Event::DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const
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

    std::string D19cCbc3Event::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return "";
    }


    std::string D19cCbc3Event::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::ostringstream os;

        std::vector<Stub> cStubVector = this->StubVector (pFeId, pCbcId);

        for (auto cStub : cStubVector)
            os << std::bitset<8> (cStub.getPosition() ) << " " << std::bitset<4> (cStub.getBend() ) << " ";

        return os.str();


        //return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }

    bool D19cCbc3Event::StubBit ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        //here just OR the stub positions
        uint16_t cKey = encodeId (pFeId, pCbcId);
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
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return false;
        }
    }

    std::vector<Stub> D19cCbc3Event::StubVector (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<Stub> cStubVec;
        //here create stubs and return the vector
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t pos1 =  (cData->second.at (9) &  0x000000FF) ;
            uint8_t pos2 =   (cData->second.at (9) & 0x0000FF00) >> 8;
            uint8_t pos3 =   (cData->second.at (9) & 0x00FF0000) >> 16;
            //LOG (DEBUG) << std::bitset<8> (pos1);
            //LOG (DEBUG) << std::bitset<8> (pos2);
            //LOG (DEBUG) << std::bitset<8> (pos3);
            uint8_t bend1 = (cData->second.at (10) & 0x00000F00) >> 8;
            uint8_t bend2 = (cData->second.at (10) & 0x000F0000) >> 16;
            uint8_t bend3 = (cData->second.at (10) & 0x0F000000) >> 24;

            if (pos1 != 0 ) cStubVec.emplace_back (pos1, bend1) ;

            if (pos2 != 0 ) cStubVec.emplace_back (pos2, bend2) ;

            if (pos3 != 0 ) cStubVec.emplace_back (pos3, bend3) ;
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cStubVec;
    }

    uint32_t D19cCbc3Event::GetNHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        uint32_t cNHits = 0;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);


        if (cData != std::end (fEventDataMap) )
        {
            cNHits += __builtin_popcount ( cData->second.at (8) & 0x7FFFFFFF);
            cNHits += __builtin_popcount ( cData->second.at (7) & 0xFFFFFFFF);
            cNHits += __builtin_popcount ( cData->second.at (6) & 0xFFFFFFFF);
            cNHits += __builtin_popcount ( cData->second.at (5) & 0xFFFFFFFF);

            cNHits += __builtin_popcount ( cData->second.at (4) & 0x7FFFFFFF);
            cNHits += __builtin_popcount ( cData->second.at (3) & 0xFFFFFFFF);
            cNHits += __builtin_popcount ( cData->second.at (2) & 0xFFFFFFFF);
            cNHits += __builtin_popcount ( cData->second.at (1) & 0xFFFFFFFF);
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cNHits;
    }

    std::vector<uint32_t> D19cCbc3Event::GetHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<uint32_t> cHits;
        uint16_t cKey = encodeId (pFeId, pCbcId);
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
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cHits;
    }

    void D19cCbc3Event::printCbcHeader (std::ostream& os, uint8_t pFeId, uint8_t pCbcId) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t cBeId =  0;
            uint8_t cFeId =  pFeId;
            uint8_t cCbcId = pCbcId;
            uint16_t cCbcDataSize = CBC_EVENT_SIZE_32_CBC3;
            os << GREEN << "CBC Header:" << std::endl;
            os << "BeId: " << +cBeId << " FeId: " << +cFeId << " CbcId: " << +cCbcId << " DataSize: " << cCbcDataSize << RESET << std::endl;
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

    }

    void D19cCbc3Event::print ( std::ostream& os) const
    {
        os << BOLDGREEN << "EventType: d19c CBC3" << RESET << std::endl;
        os << BOLDBLUE <<  "L1A Counter: " << this->GetEventCount() << RESET << std::endl;
        os << "          Be Id: " << +this->GetBeId() << std::endl;
        //os << "          Be FW: " << +this->GetFWType() << std::endl;
        //os << "      Be Status: " << +this->GetBeStatus() << std::endl;
        //os << "  Cbc Data type: " << +this->GetCbcDataType() << std::endl;
        //os << "          N Cbc: " << +this->GetNCbc() << std::endl;
        os << "Event Data size: " << +this->GetEventDataSize() << std::endl;
        //os << "  CBC Counter: " << this->GetEventCountCBC() << RESET << std::endl;
        os << "Bunch Counter: " << this->GetBunch() << std::endl;
        //os << "Orbit Counter: " << this->GetOrbit() << std::endl;
        //os << " Lumi Section: " << this->GetLumi() << std::endl;
        os << BOLDRED << "    TDC Counter: " << +this->GetTDC() << RESET << std::endl;

        const int FIRST_LINE_WIDTH = 22;
        const int LINE_WIDTH = 32;
        const int LAST_LINE_WIDTH = 8;


        for (auto const& cKey : this->fEventDataMap)
        {
            uint8_t cFeId;
            uint8_t cCbcId;
            this->decodeId (cKey.first, cFeId, cCbcId);

            //here display the Cbc Header manually
            this->printCbcHeader (os, cFeId, cCbcId);

            // here print a list of stubs
            uint8_t cCounter = 1;

            if (this->StubBit (cFeId, cCbcId) )
            {
                os << BOLDCYAN << "List of Stubs: " << RESET << std::endl;

                for (auto& cStub : this->StubVector (cFeId, cCbcId) )
                {
                    os << CYAN << "Stub: " << +cCounter << " Position: " << +cStub.getPosition() << " Bend: " << +cStub.getBend() << " Strip: " << cStub.getCenter() << RESET << std::endl;
                    cCounter++;
                }
            }

            // here list other bits in the stub stream

            std::string data ( this->DataBitString ( cFeId, cCbcId ) );
            os << GREEN << "FEId = " << +cFeId << " CBCId = " << +cCbcId << RESET << " len(data) = " << data.size() << std::endl;
            os << YELLOW << "PipelineAddress: " << this->PipelineAddress (cFeId, cCbcId) << RESET << std::endl;
            os << RED << "Error: " << static_cast<std::bitset<2>> ( this->Error ( cFeId, cCbcId ) ) << RESET << std::endl;
            os << CYAN << "Total number of hits: " << this->GetNHits ( cFeId, cCbcId ) << RESET << std::endl;
            os << BLUE << "List of hits: " << RESET << std::endl;
            std::vector<uint32_t> cHits = this->GetHits (cFeId, cCbcId);

            if (cHits.size() == 254) os << "All channels firing!" << std::endl;
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

            os << "Ch. Data:      ";

            for (int i = 0; i < FIRST_LINE_WIDTH; i += 2)
                os << data.substr ( i, 2 ) << " ";

            os << std::endl;

            for ( int i = 0; i < 7; ++i )
            {
                for (int j = 0; j < LINE_WIDTH; j += 2)
                    //os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i, LINE_WIDTH ) << std::endl;
                    os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i + j, 2 ) << " ";

                os << std::endl;
            }

            for (int i = 0; i < LAST_LINE_WIDTH; i += 2)
                os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * 7 + i, 2 ) << " ";

            os << std::endl;

            os << BLUE << "Stubs: " << this->StubBitString ( cFeId, cCbcId ).c_str() << RESET << std::endl;
        }

        os << std::endl;
    }

    std::vector<Cluster> D19cCbc3Event::getClusters ( uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<Cluster> result;

        // Use the bool vector method (SLOW!) TODO: improve this
        std::vector<bool> stripBits = DataBitVector ( pFeId, pCbcId );

        // Cluster finding
        Cluster aCluster;

        for (int iSensor = 0; iSensor < 2; ++iSensor)
        {
            aCluster.fSensor = iSensor;
            bool inCluster = false;

            for (size_t iStrip = iSensor; iStrip < stripBits.size(); iStrip += 2)
            {
                if (stripBits.at (iStrip) )
                {
                    // The strip is on
                    if (!inCluster)
                    {
                        // New cluster
                        aCluster.fFirstStrip = iStrip / 2;
                        aCluster.fClusterWidth = 1;
                        inCluster = true;
                    }
                    else
                    {
                        // Increase cluster
                        aCluster.fClusterWidth++;
                    }
                }
                else
                {
                    // The strip is off
                    if (inCluster)
                    {
                        inCluster = false;
                        result.push_back (aCluster);
                    }
                }
            }

            // Fix clusters at the end of the sensor
            if (inCluster) result.push_back (aCluster);
        }

        return result;
    }
    SLinkEvent D19cCbc3Event::GetSLinkEvent (  BeBoard* pBoard) const
    {
        uint16_t cCbcCounter = 0;
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
            uint16_t cCbcPresenceWord = 0;
            int cFirstBitFePayload = cPayload.get_current_write_position();
            int cFirstBitFeStub = cStubPayload.get_current_write_position();
            //stub counter per FE
            uint8_t cFeStubCounter = 0;

            for (auto cCbc : cFe->fCbcVector)
            {
                uint8_t cCbcId = cCbc->getCbcId();
                uint16_t cKey = encodeId (cFeId, cCbcId);
                EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

                if (cData != std::end (fEventDataMap) )
                {
                    uint16_t cError = ( cData->second.at (0) & 0x00000003 );

                    //now get the CBC status summary
                    if (pBoard->getConditionDataSet()->getDebugMode() == SLinkDebugMode::ERROR)
                        cStatusPayload.append ( (cError != 0) ? 1 : 0);

                    else if (pBoard->getConditionDataSet()->getDebugMode() == SLinkDebugMode::FULL)
                    {
                        //assemble the error bits (63, 62, pipeline address and L1A counter) into a status word
                        uint16_t cPipeAddress = (cData->second.at (0) & 0x00001FF0) >> 4;
                        uint16_t cL1ACounter = (cData->second.at (0) &  0x01FF0000) >> 16;
                        uint32_t cStatusWord = cError << 18 | cPipeAddress << 9 | cL1ACounter;
                        cStatusPayload.append (cStatusWord, 20);
                    }

                    //generate the payload
                    //the first line sets the cbc presence bits
                    cCbcPresenceWord |= 1 << cCbcId;

                    //first CBC3 channel data word
                    //since the D19C FW splits in even and odd channels, I need to
                    //Morton-encode these bits into words of the double size
                    //but first I need to reverse the bit order
                    uint32_t cFirstChanWordEven = reverse_bits (cData->second.at (4) ) >> 1;
                    uint32_t cFirstChanWordOdd = reverse_bits (cData->second.at (8) ) >> 1;
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
                    uint8_t pos1 =  (cData->second.at (9) &  0x000000FF) ;
                    uint8_t pos2 =   (cData->second.at (9) & 0x0000FF00) >> 8;
                    uint8_t pos3 =   (cData->second.at (9) & 0x00FF0000) >> 16;
                    uint8_t bend1 = (cData->second.at (10) & 0x00000F00) >> 8;
                    uint8_t bend2 = (cData->second.at (10) & 0x000F0000) >> 16;
                    uint8_t bend3 = (cData->second.at (10) & 0x0F000000) >> 24;

                    if (pos1 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cCbcId & 0x0F) << 12 | pos1 << 4 | (bend1 & 0xF)) );
                        cFeStubCounter++;
                    }

                    if (pos2 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cCbcId & 0x0F) << 12 | pos2 << 4 | (bend2 & 0xF)) );
                        cFeStubCounter++;
                    }

                    if (pos3 != 0)
                    {
                        cStubPayload.append ( uint16_t ( (cCbcId & 0x0F) << 12 | pos3 << 4 | (bend3 & 0xF)) );
                        cFeStubCounter++;
                    }
                }

                cCbcCounter++;
            } // end of CBC loop

            //for the payload, I need to insert the status word at the index I remembered before
            cPayload.insert (cCbcPresenceWord, cFirstBitFePayload );

            //for the stubs for this FE, I need to prepend a 5 bit counter shifted by 1 to the right (to account for the 0 bit)
            cStubPayload.insert ( (cFeStubCounter & 0x1F) << 1, cFirstBitFeStub, 6);

        } // end of Fe loop

        uint32_t cEvtCount = this->GetEventCount();
        uint16_t cBunch = static_cast<uint16_t> (this->GetBunch() );
        uint32_t cBeStatus = this->fBeStatus;
        SLinkEvent cEvent (EventType::VR, pBoard->getConditionDataSet()->getDebugMode(), ChipType::CBC3, cEvtCount, cBunch, SOURCE_ID );
        cEvent.generateTkHeader (cBeStatus, cCbcCounter, cEnabledFe, pBoard->getConditionDataSet()->getCondDataEnabled(), false);  // Be Status, total number CBC, condition data?, fake data?

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
}
