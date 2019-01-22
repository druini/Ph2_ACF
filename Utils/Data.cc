/*

    FileName :                     Data.cc
    Content :                      Data handling from DAQ
    Programmer :                   Nicolas PIERRE
    Version :                      1.0
    Date of creation :             10/07/14
    Support :                      mail to : nicolas.pierre@icloud.com

 */

#include "../Utils/Data.h"
#include <iostream>

namespace Ph2_HwInterface {
    //Data Class

    // copy constructor
    Data::Data ( const Data& pD ) :

        // Initialise( pD.fNevents );
        fNevents ( pD.fNevents ),
        fCurrentEvent ( pD.fCurrentEvent ),
        fNCbc ( pD.fNCbc ),
        fEventSize ( pD.fEventSize )
    {
    }


    void Data::Set (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType)
    {
        if (pData.size() != 0)
            fFuture = std::async (&Data::privateSet, this, pBoard, pData, pNevents, pType);

        //fFuture.share();
    }


    void Data::privateSet (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType)
    {
        Reset();

        fNevents = static_cast<uint32_t> ( pNevents );
        // be aware that eventsize is not constant for the zs event, so we are not using it
        fEventSize = static_cast<uint32_t> ( (pData.size() ) / fNevents );

        EventType fEventType = pBoard->getEventType();

        if (pType == BoardType::D19C)
        {
            uint32_t fNFe = pBoard->getNFe();

            if (fEventType == EventType::ZS) fNCbc = 0;
            else fNCbc = (fEventSize - D19C_EVENT_HEADER1_SIZE_32_CBC3) / D19C_EVENT_SIZE_32_CBC3 / fNFe;
        }
        else if (pType == BoardType::CBC3FC7) fNCbc = (fEventSize - (EVENT_HEADER_SIZE_32_CBC3) ) / (CBC_EVENT_SIZE_32_CBC3);
        else fNCbc = ( fEventSize - ( EVENT_HEADER_TDC_SIZE_32 ) ) / ( CBC_EVENT_SIZE_32 );

        // to fill fEventList
        std::vector<uint32_t> lvec;

        //use a SwapIndex to decide wether to swap a word or not
        //use a WordIndex to pick events apart
        uint32_t cWordIndex = 0;
        uint32_t cSwapIndex = 0;
        // index of the word inside the event (ZS)
        uint32_t fZSEventSize = 0;
        uint32_t cZSWordIndex = 0;

        for ( auto word : pData )
        {
            //if the SwapIndex is greater than 0 and a multiple of the event size in 32 bit words, reset SwapIndex to 0
            if (cSwapIndex > 0 && cSwapIndex % fEventSize == 0) cSwapIndex = 0;

            if (pType == BoardType::ICGLIB || pType == BoardType::ICFC7)
                this->setIC (word, cWordIndex, cSwapIndex);
            else if (pType == BoardType::SUPERVISOR)
                this->setStrasbourgSupervisor (word);

            //else if (pType == BoardType::CBC3FC7)
            //this->setCbc3Fc7 (word);

#ifdef __CBCDAQ_DEV__
            //TODO
            LOG (DEBUG) << std::setw (3) << "Original " << cWordIndex << " ### " << std::bitset<32> (pData.at (cWordIndex) );
            //LOG (DEBUG) << std::setw (3) << "Treated  " << cWordIndex << " ### " << std::bitset<32> (word);

            if ( (cWordIndex + 1) % fEventSize == 0 && cWordIndex > 0 ) LOG (DEBUG) << std::endl << std::endl;

#endif

            lvec.push_back ( word );

            if (fEventType == EventType::ZS)
            {
                if ( cZSWordIndex == fZSEventSize - 1 )
                {
                    //LOG(INFO) << "Packing event # " << fEventList.size() << ", Event size is " << fZSEventSize << " words";
                    if (pType == BoardType::D19C)
                        fEventList.push_back ( new D19cCbc3EventZS ( pBoard, fZSEventSize, lvec ) );
                    else
                        fEventList.push_back ( new D19cCbc3EventZS ( pBoard, fZSEventSize, lvec ) );

                    lvec.clear();

                    if (fEventList.size() >= fNevents) break;
                }
                else if ( cZSWordIndex == fZSEventSize )
                {
                    // get next event size
                    cZSWordIndex = 0;

                    if (pType == BoardType::D19C) fZSEventSize = (0x0000FFFF & word);
                    else fZSEventSize = fEventSize;

                    if (fZSEventSize > pData.size() )
                    {
                        LOG (ERROR) << "Missaligned data, not accepted";
                        break;
                    }

                }

            }
            else
            {
                if ( cWordIndex > 0 &&  (cWordIndex + 1) % fEventSize == 0 )
                {
                    if (pType == BoardType::D19C)
                        fEventList.push_back ( new D19cCbc3Event ( pBoard, fNCbc, lvec ) );
                    else if (pType == BoardType::CBC3FC7)
                        fEventList.push_back ( new Cbc3Event ( pBoard, fNCbc, lvec ) );
                    else
                        fEventList.push_back ( new Cbc2Event ( pBoard, fNCbc, lvec ) );

                    lvec.clear();

                    if (fEventList.size() >= fNevents) break;
                }
            }

            cWordIndex++;
            cSwapIndex++;
            cZSWordIndex++;

        }
    }

    void Data::Reset()
    {
        for ( auto& pevt : fEventList )
            if (pevt) delete pevt;

        fEventList.clear();
        fCurrentEvent = 0;
    }

    void Data::setIC (uint32_t& pWord, uint32_t pWordIndex, uint32_t pSwapIndex)
    {

        if (this->is_channel_first_row (pSwapIndex) )
        {
            // here I need to shift out the Error bits and PipelineAddress
            //uint8_t cErrors = word & 0x00000003;
            uint8_t cPipeAddress = (pWord & 0x000003FC) >> 2;
            //next I need to reverse the bit order and mask out the corresponding bits for errors & pipe address
            pWord = this->reverse_bits (pWord) & 0xC03FFFFF;;
            //now just need to shift the Errors & Pipe address back in
            pWord |=  cPipeAddress << 22;
        }

        if (this->is_channel_last_row (pSwapIndex) )
        {
            //OLD METHOD
            // here i need to shift out the GlibFlags which are supposed to be 0 and the Stub word
            uint16_t cStubWord = (pWord & 0xFFF00000) >> 20;
            //uint16_t cGlibFlag = (word & 0x000FFF00) >> 8;
            //reverse the bit order and mask stuff out
            //word = reverse_bits (word) & 0xFF000000;
            pWord = this->reverse_bits (pWord) & 0xFFFFF000;
            //now shift the GlibFlag and the StubWord back in
            //word |= ( ( (cGlibFlag & 0x0FFF ) << 12) | (cStubWord & 0x0FFF) );
            pWord |= (cStubWord & 0x0FFF);
        }
        //is_channel_data will also be true for first and last word but since it's an else if, it should be ok
        else if ( this->is_channel_data (pSwapIndex) ) pWord = this->reverse_bits (pWord);
    }

    void Data::setStrasbourgSupervisor (uint32_t& pWord)
    {
        pWord = this->swap_bytes (pWord);
    }


    void Data::setCbc3Fc7 (uint32_t& pWord)
    {

    }
}
