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

    double Cluster::getBaricentre()
    {
        return fFirstStrip + double (fClusterWidth) / 2. - 0.5;
    }

    Event::Event ()
    {
    }

    Event::Event ( const Event& pEvent ) :
        fBunch ( pEvent.fBunch ),
        fOrbit ( pEvent.fOrbit ),
        fLumi ( pEvent.fLumi ),
        fEventCount ( pEvent.fEventCount ),
        fEventCountCBC ( pEvent.fEventCountCBC ),
        fTDC ( pEvent.fTDC ),
        fBeId (pEvent.fBeId),
        fBeFWType (pEvent.fBeFWType),
        fCBCDataType (pEvent.fCBCDataType),
        fNCbc (pEvent.fNCbc),
        fEventDataSize (pEvent.fEventDataSize),
        fBeStatus (pEvent.fBeStatus),
        fEventSize (pEvent.fEventSize),
        fEventDataMap ( pEvent.fEventDataMap )
    {

    }
    // Event implementation
    bool Event::operator== (const Event& pEvent) const
    {
        return fEventDataMap == pEvent.fEventDataMap;
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


        //now decode FEEvents
        //uint8_t cBeId = pBoard->getBeId();
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
                uint16_t cKey = encodeId (cFeId, cCbcId);

                uint32_t begin = EVENT_HEADER_SIZE_32 + cFeId * CBC_EVENT_SIZE_32 * cNCbc + cCbcId * CBC_EVENT_SIZE_32;
                uint32_t end = begin + CBC_EVENT_SIZE_32;

                std::vector<uint32_t> cCbcData (std::next (std::begin (list), begin), std::next (std::begin (list), end) );

                fEventDataMap[cKey] = cCbcData;
            }
        }
        //return 1;
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
}
