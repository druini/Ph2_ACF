/*

        FileName :                     Event.cc
        Content :                      Event handling from DAQ
        Programmer :                   Nicolas PIERRE
        Version :                      1.0
        Date of creation :             10/07/14
        Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Event.h"

using namespace Ph2_HwDescription;


namespace Ph2_HwInterface {

    // Event implementation
    Event::Event ( const BeBoard* pBoard,  uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {




        SetEvent ( pBoard, pNbCbc, list );
    }


    Event::Event ( const Event& pEvent ) :
        fBunch ( pEvent.fBunch ),
        fOrbit ( pEvent.fOrbit ),
        fLumi ( pEvent.fLumi ),
        fEventCount ( pEvent.fEventCount ),
        fEventCountCBC ( pEvent.fEventCountCBC ),
        fTDC ( pEvent.fTDC ),
        fEventDataMap ( pEvent.fEventDataMap )

    {

    }

    void Event::SetEvent ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list )
    {
        fEventSize = pNbCbc *  CBC_EVENT_SIZE_32  + EVENT_HEADER_TDC_SIZE_32;


        
        //now decode the header info
        fBunch = 0x00FFFFFF & list.at (0);
        fOrbit = 0x00FFFFFF & list.at (1);
        fLumi = 0x00FFFFFF & list.at (2);
        fEventCount = 0x00FFFFFF &  list.at (3);
        fEventCountCBC = 0x00FFFFFF & list.at (4);
        fTDC = 0x000000FF & list.back();


        //MPA header for now 



        ftotal_trigs = 0x00FFFFFF & list.at (0);
        ftrigger_total_counter = 0x00FFFFFF & list.at (1);
        ftrigger_counter = 0x00FFFFFF & list.at (2);


	ftrigger_offset_BEAM.clear();

	ftrigger_offset_MPA.clear();


        ftrigger_offset_BEAM.insert( ftrigger_offset_BEAM.begin(), list.begin()+3, list.begin()+(3+2048) );
        ftrigger_offset_MPA.insert( ftrigger_offset_MPA.begin(), list.begin()+(3+2048), list.begin()+(3+2048*2) );



        //now decode FEEvents
        //uint8_t cBeId = pBoard->getBeId();
        uint32_t cNFe = static_cast<uint32_t> ( pBoard->getNFe() );

        for ( uint8_t cFeId = 0; cFeId < cNFe; cFeId++ )
        {
            uint32_t cNCbc;
            uint32_t cNMPA;
            // if the NCbcDataSize in the FW Version node of the BeBoard is set, the DataSize is assumed fixed:
            // if 1 module is defined, the number of CBCs is the datasize given in the xml
            // if more than one module is defined, the number of CBCs for each FE is the datasize divided by the nFe
            if ( pBoard->getNCbcDataSize() )
            {
                if ( cNFe == 1 ) cNCbc = static_cast<uint32_t> ( pBoard->getNCbcDataSize() );
                else cNCbc = static_cast<uint32_t> ( pBoard->getNCbcDataSize() / cNFe );
            }
            // if there is no FWVersion node in the xml, the CBCs will be counted for each module according to the xml file
			else
				{
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
				std::cout << __PRETTY_FUNCTION__ << "I had to comment out the following line because I couldn't compile. Hopefully you'll never see this message!!!!" << std::endl;
					//cNCbc = static_cast<uint32_t>( pBoard->getModule( i )->getNCbc() );
				}

            cNMPA = static_cast<uint32_t> ( pBoard->getModule ( cFeId )->getNMPA() );

            //now loop the CBCs and encode the IDs in key
            for ( uint8_t cCbcId = 0; cCbcId < cNCbc; cCbcId++ )
            {
                uint16_t cKey = encodeId (cFeId, cCbcId);

                uint32_t begin = EVENT_HEADER_SIZE_32 + cFeId * CBC_EVENT_SIZE_32 * cNCbc + cCbcId * CBC_EVENT_SIZE_32;
                uint32_t end = begin + CBC_EVENT_SIZE_32 - 1;

                std::vector<uint32_t> cCbcData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );



                fEventDataMap[cKey] = cCbcData;
            }


            for ( uint8_t cMPAId = 0; cMPAId < cNMPA; cMPAId++ )
            {
                uint16_t cKey = encodeId (cFeId, cMPAId);
		
                uint32_t begin = MPA_HEADER_SIZE_32 + cFeId * MPA_EVENT_SIZE_32 * cNMPA + cMPAId * MPA_EVENT_SIZE_32;
                uint32_t end = begin + MPA_EVENT_SIZE_32;

                std::vector<uint32_t> cMPAData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );
		


                fEventDataMap[cKey] = cMPAData;
            }


        }
    }

    void Event::GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint32_t >& cbcData )  const
    {
        cbcData.clear();

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            cbcData.reserve (cData->second.size() );
            cbcData.assign (cData->second.begin(), cData->second.end() );
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
    }

    void Event::GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint8_t >& cbcData )  const
    {
        cbcData.clear();

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for (const auto& cWord : cData->second)
            {
                cbcData.push_back ( (cWord >> 24) & 0xFF);
                cbcData.push_back ( (cWord >> 16) & 0xFF);
                cbcData.push_back ( (cWord >> 8) & 0xFF);
                cbcData.push_back ( (cWord ) & 0xFF);
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found.";
    }

//    std::string Event::HexString() const
//    {
        //std::stringbuf tmp;
        //std::ostream os ( &tmp );

        //os << std::hex;

        //for ( uint32_t i = 0; i < fEventSize; i++ )
        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( fEventData.at (i) & 0xFF000000 ) << " " << ( fEventData.at (i) & 0x00FF0000 ) << " " << ( fEventData.at (i) & 0x0000FF00 ) << " " << ( fEventData.at (i) & 0x000000FF );

        ////os << std::endl;

        //return tmp.str();
//    }


    bool Event::Bit ( uint8_t pFeId, uint8_t pCbcId, uint32_t pPosition ) const
    {
        uint32_t cWordP = pPosition / 32;
        uint32_t cBitP = pPosition % 32;

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            if (cWordP >= cData->second.size() ) return false;

            return ( (cData->second.at (cWordP) >> (31 - cBitP) ) & 0x1);
        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return false;
        }
    }


    bool Event::Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        return Bit ( pFeId, pCbcId, i + OFFSET_ERROR );
    }

    uint32_t Event::Error ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
            return ( (cData->second.at (0) >> 30) & 0x00000003);
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }


    uint32_t Event::PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
            return ( (cData->second.at (0) >> 22) & 0x000000FF);
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return 0;
        }
    }

    bool Event::DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        if ( i > NCHANNELS )
            return 0;

        return Bit ( pFeId, pCbcId, i + OFFSET_CBCDATA );
    }


    std::string Event::BitString ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
    {
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < pWidth; ++i )
            {
                uint32_t pos = i + pOffset;
                uint32_t cWordP = pos / 32;
                uint32_t cBitP = pos % 32;

                if ( cWordP >= cData->second.size() ) break;

                //os << ((cbcData[cByteP] & ( 1 << ( 7 - cBitP ) ))?"1":"0");
                os << ( ( cData->second[cWordP] >> ( 31 - cBitP ) ) & 0x1 );
            }

            return os.str();

        }
        else
        {
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
            return "";
        }
    }

    std::vector<bool> Event::BitVector ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
    {
        std::vector<bool> blist;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::ostringstream os;

            for ( uint32_t i = 0; i < pWidth; ++i )
            {
                uint32_t pos = i + pOffset;
                uint32_t cWordP = pos / 32;
                uint32_t cBitP = pos % 32;

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second[cWordP] >> ( 31 - cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }

    std::string Event::DataBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return BitString ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
    }


    std::vector<bool> Event::DataBitVector ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return BitVector ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );
    }

    std::vector<bool> Event::DataBitVector ( uint8_t pFeId, uint8_t pCbcId, const std::vector<uint8_t>& channelList ) const
    {
        std::vector<bool> blist;

        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for ( auto i :  channelList )
            {
                uint32_t pos = i + OFFSET_CBCDATA;
                uint32_t cWordP = pos / 32;
                uint32_t cBitP = pos % 32;

                if ( cWordP >= cData->second.size() ) break;

                blist.push_back ( ( cData->second[cWordP] >> ( 31 - cBitP ) ) & 0x1 );
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return blist;
    }


    //#if 0
    std::string Event::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        std::stringbuf tmp;
        std::ostream os ( &tmp );
        os << std::hex << std::setfill ('0');

        //get the CBC event for pFeId and pCbcId into vector<32bit> cbcData
        std::vector< uint32_t > cbcData;
        GetCbcEvent (pFeId, pCbcId, cbcData);
        //for the first 32-bit word, use only the 22 LSBs
        //this unfortunately means that i have two leading bits that are always 0
        os << std::setw (6) << (cbcData.at (0) & 0x003FFFFF);

        //this is the body of 7 words that are full of data
        for ( uint32_t i = 1; i < 8; i++ )
            os << std::setw (8) << cbcData.at (i);

        //the last word with only 8 bits
        os << std::setw (2) << (cbcData.at (8) & 0xFF000000);


        //uint32_t cFirstByteP = OFFSET_CBCDATA / 8;
        //uint32_t cFirstBitP = OFFSET_CBCDATA % 8;
        //uint32_t cLastByteP = ( cFirstByteP + WIDTH_CBCDATA - 1 ) / 8;
        //uint32_t cLastBitP = ( cFirstByteP + WIDTH_CBCDATA - 1 ) % 8;

        //uint32_t cMask ( 0 );
        //uint32_t cMaskLastBit ( 0 );
        //uint32_t cMaskWidth ( 0 );

        ////First byte
        //cMaskLastBit = cFirstByteP < cLastByteP ? 7 : cLastBitP;
        //cMaskWidth = cMaskLastBit - cFirstBitP + 1;
        //cMask = ( 1 << ( 7 - cMaskLastBit ) );

        //for ( uint32_t i = 0; i < cMaskWidth; i++ )
        //{
        //cMask <<= 1;
        //cMask |= 1;
        //}

        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( GetCbcEvent ( pFeId, pCbcId, cbcData ) [cFirstByteP]&cMask );

        //if ( cFirstByteP == cLastByteP )
        //return tmp.str();

        ////Second to the second last byte
        //if ( cFirstByteP != cLastByteP - 1 )
        //{
        //for ( uint32_t j = cFirstByteP + 1; j < cLastByteP; j++ )
        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( GetCbcEvent ( pFeId, pCbcId, cbcData ) [j] & 0xFF );
        //}

        ////Last byte
        //cMaskLastBit = cLastBitP;
        //cMaskWidth = cMaskLastBit + 1;
        //cMask = ( 1 << ( 7 - cMaskLastBit ) );

        //for ( uint32_t i = 0; i < cMaskWidth; i++ )
        //{
        //cMask <<= 1;
        //cMask |= 1;
        //}

        //os << std::uppercase << std::setw ( 2 ) << std::setfill ( '0' ) << ( GetCbcEvent ( pFeId, pCbcId, cbcData ) [cFirstByteP]&cMask );

        return tmp.str();
    }
    //#endif


    std::string Event::GlibFlagString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return BitString ( pFeId, pCbcId, OFFSET_GLIBFLAG, WIDTH_GLIBFLAG );
    }


    std::string Event::StubBitString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return BitString ( pFeId, pCbcId, OFFSET_CBCSTUBDATA, WIDTH_CBCSTUBDATA );
    }

    bool Event::StubBit ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        return Bit ( pFeId, pCbcId, OFFSET_CBCSTUBDATA );
    }


    uint32_t Event::GetNHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        uint32_t cNHits = 0;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);


        if (cData != std::end (fEventDataMap) )
        {
            cNHits += __builtin_popcount (cData->second.front() & 0x003FFFFF);

            for (auto cWord = cData->second.begin() + 1; cWord != cData->second.end() - 1;  cWord++)
                cNHits += __builtin_popcount (*cWord);

            cNHits += __builtin_popcount (cData->second.back() & 0xFF000000);
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;

        return cNHits;
    }


    std::vector<uint32_t> Event::GetHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<uint32_t> cHits;
        uint16_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            uint8_t cIndex = 0;

            for ( uint32_t i = 0; i < WIDTH_CBCDATA; ++i )
            {
                uint32_t pos = i + OFFSET_CBCDATA;
                uint32_t cWordP = pos / 32;
                uint32_t cBitP = pos % 32;

                if ( cWordP >= cData->second.size() ) break;

                if ( ( cData->second[cWordP] >> ( 31 - cBitP ) ) & 0x1) cHits.push_back (cIndex);

                cIndex++;
            }
        }
        else
            LOG (INFO) << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." ;
        return cHits;
     }

    std::ostream& operator<< ( std::ostream& os, const Event& ev )
    {
        os << BOLDBLUE <<  "  L1A Counter: " << ev.GetEventCount() << std::endl;
        os << "  CBC Counter: " << ev.GetEventCountCBC() << RESET << std::endl;
        os << "Bunch Counter: " << ev.GetBunch() << std::endl;
        os << "Orbit Counter: " << ev.GetOrbit() << std::endl;
        os << " Lumi Section: " << ev.GetLumi() << std::endl;
        os << BOLDRED << "  TDC Counter: " << ev.GetTDC() << RESET << std::endl;

        os << "CBC Data:" << std::endl;
        //const EventMap& evmap = ev.GetEventMap();
        const int FIRST_LINE_WIDTH = 22;
        const int LINE_WIDTH = 32;
        const int LAST_LINE_WIDTH = 8;

        //for ( auto const& it : evmap )
        //{
        //uint32_t cFeId = it.first;

        //for ( auto const& jt : it.second )
        //{
        //uint32_t cCbcId = jt.first;
        for (auto const& cKey : ev.fEventDataMap)
        {
            uint8_t cFeId  = 0;
            uint8_t cCbcId = 0;
            ev.decodeId (cKey.first, cFeId, cCbcId);
            std::string data ( ev.DataBitString ( cFeId, cCbcId ) );
            os << GREEN << "FEId = " << cFeId << " CBCId = " << cCbcId << RESET << " len(data) = " << data.size() << std::endl;
            os << YELLOW << "PipelineAddress: " << ev.PipelineAddress (cFeId, cCbcId) << RESET << std::endl;
            os << RED << "Error: " << static_cast<std::bitset<2>> ( ev.Error ( cFeId, cCbcId ) ) << RESET << std::endl;
//            os << "Ch. Data:      ";
//
//            for (int i = 0; i < FIRST_LINE_WIDTH; i += 2)
//                os << data.substr ( i, 2 ) << " ";
//
//            os << std::endl;
//
//            for ( int i = 0; i < 7; ++i )
//            {
//                for (int j = 0; j < LINE_WIDTH; j += 2)
//                    //os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i, LINE_WIDTH ) << std::endl;
//                    os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * i + j, 2 ) << " ";
//
//                os << std::endl;
//            }

//            for (int i = 0; i < LAST_LINE_WIDTH; i += 2)
//                os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * 7 + i , 2 ) << " ";
//
//            os << std::endl;

            os << BLUE << "Stubs: " << ev.StubBitString ( cFeId, cCbcId ).c_str() << RESET << std::endl;
        }

        os << std::endl;
        //}

        return os;
    }

    std::vector<Cluster> Event::getClusters ( uint8_t pFeId, uint8_t pCbcId)
    {
        std::vector<Cluster> result;

        // Use the bool vector method (SLOW!) TODO: improve this
        std::vector<bool> stripBits = BitVector ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );

        // Cluster finding
        Cluster aCluster;

        for (unsigned int iSensor = 0; iSensor < 2; ++iSensor)
        {
            aCluster.fSensor = iSensor;
            bool inCluster = false;

            for (unsigned int iStrip = iSensor; iStrip < stripBits.size(); iStrip += 2)
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


    double Cluster::getBaricentre()
    {
        return fFirstStrip + double (fClusterWidth) / 2. - 0.5;
    }

}
