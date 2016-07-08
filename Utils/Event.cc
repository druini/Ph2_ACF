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

    Event::Event ( const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& list ) :
        fChannelMask ("000000000000000000000000111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110000000000")
    {
        SetEvent (pBoard, pNbCbc, list);
    }

    Event::Event ( const Event& pEvent ) :
        fBunch ( pEvent.fBunch ),
        fOrbit ( pEvent.fOrbit ),
        fLumi ( pEvent.fLumi ),
        fEventCount ( pEvent.fEventCount ),
        fEventCountCBC ( pEvent.fEventCountCBC ),
        fTDC ( pEvent.fTDC ),
        fEventDataMap ( pEvent.fEventDataMap ),
        fChannelMask ("000000000000000000000000111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110000000000")
    {
    }

    void Event::SetEvent (const BeBoard* pBoard, uint32_t pNbCbc, const std::vector<uint32_t>& pList )
    {
        fEventSize = pNbCbc *  CBC_EVENT_SIZE_32  + EVENT_HEADER_TDC_SIZE_32;

        // now decode the header Info
        fBunch = 0x00FFFFFF & pList.at (0);
        fOrbit = 0x00FFFFFF & pList.at (1);
        fLumi = 0x00FFFFFF & pList.at (2);
        fEventCount = 0x00FFFFFF &  pList.at (3);
        fEventCountCBC = 0x00FFFFFF & pList.at (4);
        fTDC = 0x000000FF & pList.back();

        //now decode FEEvents
        uint8_t cBeId = pBoard->getBeId();
        uint32_t cNFe = static_cast<uint32_t> ( pBoard->getNFe() );

        for ( uint8_t cFeId = 0; cFeId < cNFe; cFeId++ )
        {
            uint32_t cNCbc;

            // if the NCbcDataSize in the FW Version node of the BeBoard is set, the DataSize is assumed fixed:
            // if 1 module is defined, the number of CBCs is the datasize given in the xml
            // if more than one module is defined, the number of CBCs for each FE is the datasize divided by the nFe
            if ( pBoard->getNCbcDataSize() )
            {
                if ( cNFe == 1 ) cNCbc = static_cast<uint32_t> ( pBoard->getNCbcDataSize() );
                else cNCbc = static_cast<uint32_t> ( pBoard->getNCbcDataSize() / cNFe );
            }
            // if there is no FWVersion node in the xml, the CBCs will be counted for each module according to the xml file
            else cNCbc = static_cast<uint32_t> ( pBoard->getModule ( cFeId )->getNCbc() );

            //now loop the CBCs and encode the IDs in key
            for ( uint8_t cCbcId = 0; cCbcId < cNCbc; cCbcId++ )
            {
                uint32_t cKey = encodeId (cFeId, cCbcId);
                std::bitset<CBC_EVENT_SIZE_32 * 32> cCbcData;

                uint32_t begin = EVENT_HEADER_SIZE_32 + cFeId * CBC_EVENT_SIZE_32 * cNCbc + cCbcId * CBC_EVENT_SIZE_32;
                uint32_t end = begin + CBC_EVENT_SIZE_32 - 1;
                //std::cout << "DEBUG Be: " << +cBeId << " FE: " << +cFeId << " Cbc: " << +cCbcId << " begins at " << begin << " ends at  " << end << std::endl;

                //ok, now I have the begin and end index for each CBC - just insert the corresponding words in a bitset
                //TODO: check if the indices are inclusive end or not
                uint32_t cIndex = 0;

                for (uint32_t cWordIndex = begin; cWordIndex <= end; cWordIndex++)
                {
                    //add words to bitset
                    this->setBitsetValue (cCbcData, pList.at (cWordIndex), cIndex * CBC_EVENT_SIZE_32);
                    cIndex++;
                }

                //now insert in map
                fEventDataMap[cKey] = cCbcData;
            }
        }
    }

    void Event::GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint32_t >& cbcData )  const
    {
        cbcData.clear();

        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            //use cData.second and convert it to std::vector<uint32_t>
            for (uint32_t cWord = 0; cWord < CBC_EVENT_SIZE_32; cWord++)
                cbcData.push_back ( this->subset (cData->second, cWord * 32, 32 )  ) ;
        }
        else
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." << std::endl;
    }

    void Event::GetCbcEvent ( const uint8_t& pFeId, const uint8_t& pCbcId, std::vector< uint8_t >& cbcData )  const
    {
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            //use cData.second and convert it to std::vector<uint32_t>
            //TODO: continue here
            for (size_t cWord = 0; cWord < CBC_EVENT_SIZE_32; cWord++)
            {
                uint32_t cWord32 = this->subset (cData->second, cWord * 32, 32);
                cbcData.push_back ( (cWord32 >> 24) & 0xFF);
                cbcData.push_back ( (cWord32 >> 16) & 0xFF);
                cbcData.push_back ( (cWord32 >> 8) & 0xFF);
                cbcData.push_back ( (cWord32 ) & 0xFF);
            }
        }
        else
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << "is not found." << std::endl;
    }


    bool Event::Bit ( uint8_t pFeId, uint8_t pCbcId, uint32_t pPosition ) const
    {
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
            return cData->second.test (pPosition);
        else
        {
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." << std::endl;
            return false;
        }
    }


    bool Event::Error ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        return Bit ( pFeId, pCbcId, i + OFFSET_ERROR );
    }

    uint32_t Event::Error ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
            return this->subset (cData->second, OFFSET_ERROR, WIDTH_ERROR, 0x00000003);

        else
        {
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." << std::endl;
            return 0;
        }
    }


    uint32_t Event::PipelineAddress ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
            return this->subset (cData->second, OFFSET_PIPELINE_ADDRESS, WIDTH_PIPELINE_ADDRESS, 0x000000FF);

        else
        {
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." << std::endl;
            return 0;
        }
    }

    bool Event::DataBit ( uint8_t pFeId, uint8_t pCbcId, uint32_t i ) const
    {
        if ( i > NCHANNELS )
            return false;
        else
            return this->Bit (pFeId, pCbcId, i + OFFSET_CBCDATA);
    }


    std::string Event::BitString ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
    {
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            std::string cStr = cData->second.to_string();
            cStr = std::string (cStr.rbegin(), cStr.rend() );
            return cStr.substr (pOffset, pWidth);
        }
        else
        {
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." << std::endl;
            return "";
        }
    }

    std::vector<bool> Event::BitVector ( uint8_t pFeId, uint8_t pCbcId, uint32_t pOffset, uint32_t pWidth ) const
    {
        std::vector<bool> blist;
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for (uint32_t cBit = pOffset; cBit <= pWidth; cBit++)
                blist.push_back (cData->second.test (cBit) );
        }

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
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
        {
            for (auto& cChan : channelList)
                blist.push_back (cData->second.test (OFFSET_CBCDATA + cChan) );
        }

        return blist;
    }

    //#if 0
    std::string Event::DataHexString ( uint8_t pFeId, uint8_t pCbcId ) const
    {
        //start from here
        //uin32_t cKey = encodeId (pFeId, pCbcId);
        //EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        //if (cData != std::end (fEventDataMap) )
        //return  this->subset (cData.second, pOffset, pOffset + pWidth).to_string() ;

        //else
        //{
        //std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId " is not found." << std::endl;
        //return "";
        //}
        return "";
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
        uint32_t cKey = encodeId (pFeId, pCbcId);
        EventDataMap::const_iterator cData = fEventDataMap.find (cKey);

        if (cData != std::end (fEventDataMap) )
            return static_cast<uint32_t> ( (cData->second & fChannelMask).count() );
        else
        {
            std::cout << "Event: FE " << +pFeId << " CBC " << +pCbcId << " is not found." << std::endl;
            return 0;
        }
    }

    std::vector<uint8_t> Event::GetHits (uint8_t pFeId, uint8_t pCbcId) const
    {
        std::vector<uint8_t> cHits;

        for (uint8_t cChan = 0; cChan < NCHANNELS; cChan++)
        {
            if (DataBit (pFeId, pCbcId, cChan) )
                cHits.push_back (cChan);
        }

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
        const int FIRST_LINE_WIDTH = 22;
        const int LINE_WIDTH = 32;
        const int LAST_LINE_WIDTH = 8;

        for ( auto const& cKey : ev.fEventDataMap )
        {
            uint8_t cFeId;
            uint8_t cCbcId;
            ev.decodeId (cKey.first, cFeId, cCbcId);
            std::string data ( ev.DataBitString ( cFeId, cCbcId ) );
            os << GREEN << "FEId = " << +cFeId << " CBCId = " << +cCbcId << RESET << " len(data) = " << data.size() << std::endl;
            os << YELLOW << "PipelineAddress: " << ev.PipelineAddress (cFeId, cCbcId) << RESET << std::endl;
            os << RED << "Error: " << static_cast<std::bitset<2>> ( ev.Error ( cFeId, cCbcId ) ) << RESET << std::endl;
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
                os << data.substr ( FIRST_LINE_WIDTH + LINE_WIDTH * 7 + i , 2 ) << " ";

            os << std::endl;

            os << BLUE << "Stubs: " << ev.StubBitString ( cFeId, cCbcId ).c_str() << RESET << std::endl;
        }

        os << std::endl;

        return os;
    }

    std::vector<Cluster> Event::getClusters ( uint8_t pFeId, uint8_t pCbcId)
    {
        std::vector<Cluster> result;

        // Use the bool vector method (SLOW!) TODO: improve this
        std::vector<bool> stripBits = BitVector ( pFeId, pCbcId, OFFSET_CBCDATA, WIDTH_CBCDATA );

        // Cluster finding
        Cluster aCluster;

        for (int iSensor = 0; iSensor < 2; ++iSensor)
        {
            aCluster.fSensor = iSensor;
            bool inCluster = false;

            for (int iStrip = iSensor; iStrip < stripBits.size(); iStrip += 2)
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
