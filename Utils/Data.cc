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


    void Data::Set ( const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, bool swapBits, bool swapBytes )
    {
        Reset();

        fNevents = static_cast<uint32_t> ( pNevents );
        fEventSize = static_cast<uint32_t> ( (pData.size() ) / fNevents );
        fNCbc = ( fEventSize - ( EVENT_HEADER_TDC_SIZE_32 ) ) / ( CBC_EVENT_SIZE_32 );

        //use a SwapIndex to decide wether to swap a word or not
        //use a WordIndex to pick events apart
        uint32_t cWordIndex = 0;
        uint32_t cSwapIndex = 0;

        // to Fill fEventList
        std::vector<uint32_t> lvec;

        for ( auto word : pData )
        {
            //if the SwapIndex is greater than 0 and a multiple of the event size in 32 bit words, reset SwapIndex to 0
            if (cSwapIndex > 0 && cSwapIndex % fEventSize == 0) cSwapIndex = 0;

            // old version
            //if (swapBits && is_channel_data (cSwapIndex, fNCbc) ) word = reverse_bits (word);
            //new version
            if (swapBits)
            {
                if (is_channel_first_row (cSwapIndex) )
                {
                    // here I need to shift out the Error bits and PipelineAddress
                    //uint8_t cErrors = word & 0x00000003;
                    uint8_t cPipeAddress = (word & 0x000003FC) >> 2;
                    //next I need to reverse the bit order and mask out the corresponding bits for errors & pipe address
                    word = reverse_bits (word) & 0xC03FFFFF;;
                    //now just need to shift the Errors & Pipe address back in
                    word |=  cPipeAddress << 22;
                }

                if (is_channel_last_row (cSwapIndex) )
                {
                    //OLD METHOD
                    // here i need to shift out the GlibFlags which are supposed to be 0 and the Stub word
                    uint16_t cStubWord = (word & 0xFFF00000) >> 20;
                    //uint16_t cGlibFlag = (word & 0x000FFF00) >> 8;
                    //reverse the bit order and mask stuff out
                    //word = reverse_bits (word) & 0xFF000000;
                    word = reverse_bits (word) & 0xFFFFF000;
                    //now shift the GlibFlag and the StubWord back in
                    //word |= ( ( (cGlibFlag & 0x0FFF ) << 12) | (cStubWord & 0x0FFF) );
                    word |= (cStubWord & 0x0FFF);
                }
                //is_channel_data will also be true for first and last word but since it's an else if, it should be ok
                else if ( is_channel_data (cSwapIndex) ) word = reverse_bits (word);
            }
            else if (swapBytes)
                word = swap_bytes (word);

#ifdef __CBCDAQ_DEV__
            std::cout << std::setw (3) << "Original " << cWordIndex << " ### " << std::bitset<32> (pData.at (cWordIndex) ) << std::endl;
            std::cout << std::setw (3) << "Treated  " << cWordIndex << " ### " << std::bitset<32> (word) << std::endl;

            if ( (cWordIndex + 1) % fEventSize == 0 && cWordIndex > 0 ) std::cout << std::endl << std::endl;

#endif

            lvec.push_back ( word );

            if ( cWordIndex > 0 &&  (cWordIndex + 1) % fEventSize == 0 )
            {
                fEventList.push_back ( new Event ( pBoard, fNCbc, lvec ) );
                lvec.clear();
            }

            cWordIndex++;
            cSwapIndex++;
        }

        //#ifdef __CBCDAQ_DEV__
        //std::cout << "Initializing list with " << pData.size() << " 32 bit words
        //<< " containing data from "
        //<< fNevents << "  Events with an eventbuffer size of " << fEventSize << " and " << fNCbc
        //<< " CBCs each! " << EVENT_HEADER_TDC_SIZE_32 << " " << CBC_EVENT_SIZE_32 << std::endl;
        //#endif
    }

    void Data::Reset()
    {
        for ( auto& pevt : fEventList )
            delete pevt;

        fEventList.clear();
        fCurrentEvent = 0;
    }
}
