/*
        FileName :                     MPAInterface.cc
        Content :                      User Interface to the MPAs
        Programmer :                   K. nash, M. Haranko, D. Ceresa
        Version :                      1.0
        Date of creation :             5/01/18
 */

#include "MPAInterface.h"
#include "../Utils/ConsoleColor.h"
#include <typeinfo>

#define DEV_FLAG 0

using namespace Ph2_HwDescription;

namespace Ph2_HwInterface
{
MPAInterface::MPAInterface( const BeBoardFWMap& pBoardMap ) :
    fBoardMap( pBoardMap ),
    fBoardFW( nullptr ),
    prevBoardIdentifier( 65535 ),
    fRegisterCount( 0 ),
    fTransactionCount( 0 )
{
#ifdef COUNT_FLAG
    std::cout << "Counting number of Transactions!" << std::endl;
#endif
}

MPAInterface::~MPAInterface()
{
}

void MPAInterface::setBoard( uint16_t pBoardIdentifier )
{
    if ( prevBoardIdentifier != pBoardIdentifier )
    {
        BeBoardFWMap::iterator i = fBoardMap.find( pBoardIdentifier );

        if ( i == fBoardMap.end() )
            std::cout << "The Board: " << +( pBoardIdentifier >> 8 ) << "  doesn't exist" << std::endl;
        else
        {
            fBoardFW = i->second;
            fMPAFW = dynamic_cast<D19cFWInterface*>(fBoardFW);
            prevBoardIdentifier = pBoardIdentifier;
        }
    }
}




void MPAInterface::setFileHandler (FileHandler* pHandler)
{
    setBoard(0);
    fMPAFW->setFileHandler ( pHandler);
}


//Straight python port
void MPAInterface::PowerOn(float VDDPST , float DVDD , float AVDD , float VBG , uint8_t mpaid  , uint8_t ssaid  )
{
    setBoard(0);
    fMPAFW->PSInterfaceBoard_PowerOn_MPA( );

}


void MPAInterface::PowerOff(uint8_t mpaid , uint8_t ssaid )
{
    setBoard(0);
    fMPAFW->PSInterfaceBoard_PowerOff_MPA( );
}

void MPAInterface::MainPowerOn(uint8_t mpaid , uint8_t ssaid )
{
    setBoard(0);
    fMPAFW->PSInterfaceBoard_PowerOn( );
}



void MPAInterface::MainPowerOff()
{
    setBoard(0);
    fMPAFW->PSInterfaceBoard_PowerOff( );
}

bool MPAInterface::ConfigureMPA (const MPA* pMPA, bool pVerifLoop)
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pMPA->getBeBoardId() );

    //vector to encode all the registers into
    std::vector<uint32_t> cVec;

    //Deal with the ChipRegItems and encode them

    MPARegMap cMPARegMap = pMPA->getRegMap();

    for ( auto& cRegItem : cMPARegMap )
    {
        fBoardFW->EncodeReg (cRegItem.second, pMPA->getFeId(), pMPA->getMPAId(), cVec, pVerifLoop, true);
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registers, the answer will be in the same cVec
    // the number of times the write operation has been attempted is given by cWriteAttempts
    uint8_t cWriteAttempts = 0 ;
    bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, cWriteAttempts, pVerifLoop);

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    return cSuccess;
}



bool MPAInterface::WriteMPAReg ( MPA* pMPA, const std::string& pRegNode, uint8_t pValue, bool pVerifLoop )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pMPA->getBeBoardId() );

    //next, get the reg item
    ChipRegItem cRegItem = pMPA->getRegItem ( pRegNode );
    cRegItem.fValue = pValue;

    //vector for transaction
    std::vector<uint32_t> cVec;

    // encode the reg specific to the FW, pVerifLoop decides if it should be read back, true means to write it
    fBoardFW->EncodeReg ( cRegItem, pMPA->getFeId(), pMPA->getMPAId(), cVec, pVerifLoop, true );
    // write the registers, the answer will be in the same cVec
    // the number of times the write operation has been attempted is given by cWriteAttempts
    uint8_t cWriteAttempts = 0 ;
    bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );

    //update the HWDescription object
    if (cSuccess)
        pMPA->setReg ( pRegNode, pValue );

#ifdef COUNT_FLAG
    fRegisterCount++;
    fTransactionCount++;
#endif

    return cSuccess;
}




bool MPAInterface::WriteMPAMultReg ( MPA* pMPA, const std::vector< std::pair<std::string, uint8_t> >& pVecReq, bool pVerifLoop )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pMPA->getBeBoardId() );

    std::vector<uint32_t> cVec;

    //Deal with the ChipRegItems and encode them
    ChipRegItem cRegItem;

    for ( const auto& cReg : pVecReq )
    {
        cRegItem = pMPA->getRegItem ( cReg.first );
        cRegItem.fValue = cReg.second;

        fBoardFW->EncodeReg ( cRegItem, pMPA->getFeId(), pMPA->getMPAId(), cVec, pVerifLoop, true );
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registers, the answer will be in the same cVec
    // the number of times the write operation has been attempted is given by cWriteAttempts
    uint8_t cWriteAttempts = 0 ;
    bool cSuccess = fBoardFW->WriteChipBlockReg (  cVec, cWriteAttempts, pVerifLoop );

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    // if the transaction is successfull, update the HWDescription object
    if (cSuccess)
    {
        for ( const auto& cReg : pVecReq )
        {
            cRegItem = pMPA->getRegItem ( cReg.first );
            pMPA->setReg ( cReg.first, cReg.second );
        }
    }

    return cSuccess;
}

uint8_t MPAInterface::ReadMPAReg ( MPA* pMPA, const std::string& pRegNode )
{
    setBoard ( pMPA->getBeBoardId() );

    ChipRegItem cRegItem = pMPA->getRegItem ( pRegNode );

    std::vector<uint32_t> cVecReq;

    fBoardFW->EncodeReg ( cRegItem, pMPA->getFeId(), pMPA->getMPAId(), cVecReq, true, false );
    fBoardFW->ReadChipBlockReg (  cVecReq );

    //bools to find the values of failed and read
    bool cFailed = false;
    bool cRead;
    uint8_t cMPAId;
    fBoardFW->DecodeReg ( cRegItem, cMPAId, cVecReq[0], cRead, cFailed );

    if (!cFailed) pMPA->setReg ( pRegNode, cRegItem.fValue );

    return cRegItem.fValue;
}

void MPAInterface::ReadMPAMultReg ( MPA* pMPA, const std::vector<std::string>& pVecReg )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pMPA->getBeBoardId() );

    std::vector<uint32_t> cVec;

    //Deal with the ChipRegItems and encode them
    ChipRegItem cRegItem;

    for ( const auto& cReg : pVecReg )
    {
        cRegItem = pMPA->getRegItem ( cReg );

        fBoardFW->EncodeReg ( cRegItem, pMPA->getFeId(), pMPA->getMPAId(), cVec, true, false );
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registers, the answer will be in the same cVec
    fBoardFW->ReadChipBlockReg ( cVec);

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    bool cFailed = false;
    bool cRead;
    uint8_t cMPAId;
    //update the HWDescription object with the value I just read
    uint32_t idxReadWord = 0;

    for ( const auto& cReg : pVecReg )
        //for ( const auto& cReadWord : cVec )
    {
        uint32_t cReadWord = cVec[idxReadWord++];
        fBoardFW->DecodeReg ( cRegItem, cMPAId, cReadWord, cRead, cFailed );

        // here I need to find the string matching to the reg item!
        if (!cFailed)
            pMPA->setReg ( cReg, cRegItem.fValue );
    }
}

void MPAInterface::ReadMPA ( MPA* pMPA )
{
    //first, identify the correct BeBoardFWInterface
    setBoard ( pMPA->getBeBoardId() );

    //vector to encode all the registers into
    std::vector<uint32_t> cVec;
    //helper vector to store the register names in the same order as the ChipRegItems
    std::vector<std::string> cNameVec;

    //Deal with the ChipRegItems and encode them

    MPARegMap cMPARegMap = pMPA->getRegMap();

    for ( auto& cRegItem : cMPARegMap )
    {
        cRegItem.second.fValue = 0x00;
        fBoardFW->EncodeReg (cRegItem.second, pMPA->getFeId(), pMPA->getMPAId(), cVec, true, false);
        //push back the names in cNameVec for latercReg
        cNameVec.push_back (cRegItem.first);
#ifdef COUNT_FLAG
        fRegisterCount++;
#endif
    }

    // write the registers, the answer will be in the same cVec
    //bool cSuccess = fBoardFW->WriteChipBlockReg ( cVec, pVerifLoop);

    // write the registers, the answer will be in the same cVec
    fBoardFW->ReadChipBlockReg ( cVec);

#ifdef COUNT_FLAG
    fTransactionCount++;
#endif

    bool cFailed = false;
    bool cRead;
    uint8_t cMPAId;
    //update the HWDescription object with the value I just read
    uint32_t idxReadWord = 0;

    //for ( const auto& cReg : cVec )
    for ( const auto& cReadWord : cVec )
    {
        ChipRegItem cRegItem;
        std::string cName = cNameVec[idxReadWord++];
        fBoardFW->DecodeReg ( cRegItem, cMPAId, cReadWord, cRead, cFailed );

        // here I need to find the string matching to the reg item!
        if (!cFailed)
            pMPA->setReg ( cName, cRegItem.fValue );

    }

}

void MPAInterface::Pix_write(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel,uint32_t data)
{
    setBoard(0);
    return fMPAFW->Pix_write_MPA(cMPA,cRegItem,row, pixel, data);
}


uint32_t MPAInterface::Pix_read(MPA* cMPA,ChipRegItem cRegItem,uint32_t row,uint32_t pixel)
{
    setBoard(0);
    return fMPAFW->Pix_read_MPA(cMPA, cRegItem, row, pixel);
}


void MPAInterface::PS_Start_counters_read(uint32_t duration )
{
    setBoard(0);
    fMPAFW->PS_Start_counters_read(duration);
}


void MPAInterface::PS_Clear_counters(uint32_t duration)
{
    setBoard(0);
    fMPAFW->PS_Clear_counters(duration);
}

std::vector<uint16_t> MPAInterface::ReadoutCounters_MPA(uint32_t raw_mode_en)
{
    setBoard(0);
    return fMPAFW->ReadoutCounters_MPA(raw_mode_en);
}

Stubs MPAInterface::Format_stubs(std::vector<std::vector<uint8_t>> rawstubs)
{
    int j = 0;
    int cycle = 0;
    Stubs formstubs;
    for(int i=0; i<39; i++)
    {

        if ((rawstubs[0][i] & 0x80) == 128)
        {
            j = i+1;
            formstubs.pos.push_back(std::vector<uint8_t>(5,0));
            formstubs.row.push_back(std::vector<uint8_t>(5,0));
            formstubs.cur.push_back(std::vector<uint8_t>(5,0));


            formstubs.nst.push_back(((rawstubs[1][i] & 0x80) >> 5) | ((rawstubs[2][i] & 0x80) >> 6) | ((rawstubs[3][i] & 0x80) >> 7));
            formstubs.pos[cycle][0] = ((rawstubs[4][i] & 0x80) << 0) | ((rawstubs[0][i] & 0x40) << 0) | ((rawstubs[1][i] & 0x40) >> 1) | ((rawstubs[2][i] & 0x40) >> 2) | ((rawstubs[3][i] & 0x40) >> 3) | ((rawstubs[4][i] & 0x40) >> 4) | ((rawstubs[0][i] & 0x20) >> 4) | ((rawstubs[1][i] & 0x20) >> 5);
            formstubs.pos[cycle][1] = ((rawstubs[4][i] & 0x10) << 3) | ((rawstubs[0][i] &  0x8) << 3) | ((rawstubs[1][i] &  0x8) << 2) | ((rawstubs[2][i] &  0x8) << 1) | ((rawstubs[3][i] &  0x8) << 0) | ((rawstubs[4][i] &  0x8) >> 1) | ((rawstubs[0][i] &  0x4) >> 1) | ((rawstubs[1][i] &  0x4) >> 2);
            formstubs.pos[cycle][2] = ((rawstubs[4][i] &  0x2) << 6) | ((rawstubs[0][i] &  0x1) << 6) | ((rawstubs[1][i] &  0x1) << 5) | ((rawstubs[2][i] &  0x1) << 4) | ((rawstubs[3][i] &  0x1) << 3) | ((rawstubs[4][i] &  0x1) << 3) | ((rawstubs[1][j] & 0x80) >> 6) | ((rawstubs[2][j] & 0x80) >> 7);
            formstubs.pos[cycle][3] = ((rawstubs[0][j] & 0x20) << 2) | ((rawstubs[1][j] & 0x20) << 1) | ((rawstubs[2][j] & 0x20) << 0) | ((rawstubs[3][j] & 0x20) >> 1) | ((rawstubs[4][j] & 0x20) >> 2) | ((rawstubs[0][j] & 0x10) >> 2) | ((rawstubs[1][j] & 0x10) >> 3) | ((rawstubs[2][j] & 0x10) >> 4);
            formstubs.pos[cycle][4] = ((rawstubs[0][j] &  0x4) << 5) | ((rawstubs[1][j] &  0x4) << 4) | ((rawstubs[2][j] &  0x4) << 3) | ((rawstubs[3][j] &  0x4) << 2) | ((rawstubs[4][j] &  0x4) << 1) | ((rawstubs[0][j] &  0x2) << 1) | ((rawstubs[1][j] &  0x2) << 0) | ((rawstubs[2][j] &  0x2) >> 1);
            formstubs.row[cycle][0] = ((rawstubs[0][i] & 0x10) >> 1) | ((rawstubs[1][i] & 0x10) >> 2) | ((rawstubs[2][i] & 0x10) >> 3) | ((rawstubs[3][i] & 0x10) >> 4);
            formstubs.row[cycle][1] = ((rawstubs[0][i] &  0x2) << 2) | ((rawstubs[1][i] &  0x2) << 1) | ((rawstubs[2][i] &  0x2) << 0) | ((rawstubs[3][i] &  0x2) >> 1);
            formstubs.row[cycle][2] = ((rawstubs[1][j] & 0x40) >> 3) | ((rawstubs[2][j] & 0x40) >> 4) | ((rawstubs[3][j] & 0x40) >> 5) | ((rawstubs[4][j] & 0x40) >> 6);
            formstubs.row[cycle][3] = ((rawstubs[1][j] &  0x8) >> 0) | ((rawstubs[2][j] &  0x8) >> 1) | ((rawstubs[3][j] &  0x8) >> 2) | ((rawstubs[4][j] &  0x8) >> 3);
            formstubs.row[cycle][4] = ((rawstubs[1][j] &  0x1) << 3) | ((rawstubs[2][j] &  0x1) << 2) | ((rawstubs[3][j] &  0x1) << 1) | ((rawstubs[4][j] &  0x1) << 0);
            formstubs.cur[cycle][0] = ((rawstubs[2][i] & 0x20) >> 3) | ((rawstubs[3][i] & 0x20) >> 4) | ((rawstubs[4][i] & 0x20) >> 5);
            formstubs.cur[cycle][1] = ((rawstubs[2][i] &  0x4) >> 0) | ((rawstubs[3][i] &  0x4) >> 1) | ((rawstubs[4][i] &  0x4) >> 2);
            formstubs.cur[cycle][2] = ((rawstubs[3][j] & 0x80) >> 5) | ((rawstubs[4][j] & 0x80) >> 6) | ((rawstubs[0][j] & 0x40) >> 6);
            formstubs.cur[cycle][3] = ((rawstubs[3][j] & 0x10) >> 2) | ((rawstubs[4][j] & 0x10) >> 3) | ((rawstubs[0][j] &  0x8) >> 3);
            formstubs.cur[cycle][4] = ((rawstubs[3][j] &  0x2) << 1) | ((rawstubs[4][j] &  0x2) >> 0) | ((rawstubs[0][j] &  0x1) >> 0);
            //std::cout<<"RS1 "<<+formstubs.pos[cycle][0]<<std::endl;
            //std::cout<<"RS2 "<<+formstubs.pos[cycle][1]<<std::endl;
            //std::cout<<"RS3 "<<+formstubs.pos[cycle][2]<<std::endl;				std::cout<<"RS01"<<+rawstubs[1][i]<<std::endl;
            //std::cout<<"RS4 "<<+formstubs.pos[cycle][3]<<std::endl;
            cycle += 1;
        }
    }
    return formstubs;
}

L1data MPAInterface::Format_l1(std::vector<uint8_t> rawl1,bool verbose)
{
    bool found = false;
    uint8_t header,error(0),L1_ID,strip_counter,pixel_counter;
    L1data formL1data;

    std::vector<uint16_t> strip_data, pixel_data;
    uint16_t curdata;


    for (int i=1; i<200 ;i++)
    {
        if ((rawl1[i] == 255)&(rawl1[i-1] == 255)&(!found))
        {
            header = rawl1[i-1] << 11 | rawl1[i-1] << 3 | ((rawl1[i+1] & 0xE0) >> 5);
            error = ((rawl1[i+1] & 0x18) >> 3);
            L1_ID = ((rawl1[i+1] &  0x7) << 6) | ((rawl1[i+2] & 0xFC) >> 2);
            strip_counter = ((rawl1[i+2] &  0x1) << 4) | ((rawl1[i+3] & 0xF0) >> 4);
            pixel_counter = ((rawl1[i+3] &  0xF) << 1) | ((rawl1[i+4] & 0x80) >> 7);

            uint8_t wordl=11,counter=0;
            bool curbit;
            uint8_t bitmask = 0x80;
            for (int j=4; j<50 ;j++)
            {
                for(int k=0; k<8 ;k++)
                {
                    curbit = (rawl1[i+j]&(bitmask>>k));
                    counter += 1;
                    curdata += (curbit<<(wordl-counter));
                    if(counter==wordl)
                    {
                        if (wordl==11) strip_data.push_back(curdata);
                        else pixel_data.push_back(curdata);
                        if(strip_counter==strip_data.size()) wordl=14;
                        curdata = 0;
                        counter = 0;
                    }
                }
            }
            found = true;
        }
    }
    if (found)
    {
        formL1data.strip_counter = strip_counter;
        formL1data.pixel_counter = pixel_counter;
        if(verbose)
        {
            std::cout<<"Header: "<<std::bitset<8>(header)<<std::endl;
            std::cout<<"Error: "<<std::bitset<8>(error)<<std::endl;
            std::cout<<"L1 ID: "<<L1_ID<<std::endl;
            std::cout<<"Strip counter: "<<strip_counter<<std::endl;
            std::cout<<"Pixel counter: "<<pixel_counter<<std::endl;
            std::cout<<"Strip data:"<<std::endl;
        }

        for (auto& sdata : strip_data)
        {
            formL1data.pos_strip.push_back((sdata & 0x7F0) >> 4);
            formL1data.width_strip.push_back((sdata & 0xE) >> 1);
            formL1data.MIP.push_back((sdata & 0x1));

            if (verbose)std::cout<< "\tPosition: "<<formL1data.pos_strip.back()<<"\n\tWidth: "<<formL1data.width_strip.back()<<"\n\tMIP: "<<formL1data.MIP.back()<<std::endl;
        }
        if(verbose) std::cout<<"Pixel data:"<<std::endl;

        for (auto& pdata : pixel_data)
        {
            formL1data.pos_pixel.push_back((pdata & 0x3F80) >> 7);
            formL1data.width_pixel.push_back((pdata & 0x70) >> 4);
            formL1data.Z.push_back((pdata & 0xF) + 1);

            if(verbose) std::cout<< "\tPosition: " << formL1data.pos_pixel.back()<<"\n\tWidth: "<<formL1data.width_pixel.back()<<"\n\tRow Number: "<<formL1data.Z.back()<<std::endl;
        }

        return formL1data;
    }
    else std::cout<<"Header not found!"<<std::endl;
    
    return formL1data;
}

void MPAInterface::Activate_async(MPA* pMPA)
{
    WriteMPAReg( pMPA,"ReadoutMode",0x1);
}

void MPAInterface::Activate_sync(MPA* pMPA)
{
    WriteMPAReg(pMPA,"ReadoutMode",0x0);
}

void MPAInterface::Activate_pp(MPA* pMPA)
{
    WriteMPAReg(pMPA,"ECM",0x81);
}

void MPAInterface::Activate_ss(MPA* pMPA)
{
    WriteMPAReg(pMPA,"ECM",0x41);
}

void MPAInterface::Activate_ps(MPA* pMPA)
{
    WriteMPAReg(pMPA,"ECM", 0x8);
}




void MPAInterface::Pix_Set_enable(MPA* pMPA,uint32_t r,uint32_t p,uint32_t PixelMask=1,uint32_t Polarity=1,uint32_t EnEdgeBR=1,uint32_t EnLevelBR=0,uint32_t Encount=0,uint32_t DigCal=0,uint32_t AnCal=0,uint32_t BRclk=0)
{
    uint32_t comboword = (PixelMask) + (Polarity<<1) + (EnEdgeBR<<2) + (EnLevelBR<<3) + (Encount<<4) + (DigCal<<5) + (AnCal<<6)  + (BRclk<<7);
    Pix_write(pMPA,pMPA->getRegItem("ENFLAGS"), r, p, comboword );
}


void MPAInterface::Pix_Smode(MPA* pMPA,uint32_t r,uint32_t p, std::string smode = "edge")
{
    uint32_t smodewrite = 0x0;
    if (smode == "edge")
        smodewrite = 0x0;
    if (smode == "level")
        smodewrite = 0x1;
    if (smode == "or")
        smodewrite = 0x2;
    if (smode == "xor")
        smodewrite = 0x3;
    Pix_write(pMPA,pMPA->getRegItem("ModeSel"), r, p, smodewrite ) ;
}

void MPAInterface::Enable_pix_BRcal(MPA* pMPA,uint32_t r,uint32_t p,std::string polarity,std::string smode)
{
    uint32_t PixelMask=1,Polarity=1,EnEdgeBR=1,EnLevelBR=0,Encount=0,DigCal=0,AnCal=0,BRclk=0;

    if (polarity == "rise") Polarity = 1;
    else if (polarity == "fall") Polarity = 0;
    else
    {
        std::cout<<"bad pol option"<<std::endl;
        return;
    }
    if (smode == "level")
    {
        Pix_Smode(pMPA,r,p, "level");
        EnEdgeBR=0,EnLevelBR=1,Encount=1,AnCal=1;
    }
    else if (smode == "edge")
    {
        Pix_Smode(pMPA,r,p, "edge");
        EnEdgeBR=1,EnLevelBR=0,Encount=1,AnCal=1;
    }
    else
    {
        std::cout<<"bad edge option"<<std::endl;
        return;
    }
    Pix_Set_enable(pMPA,r,p,PixelMask,Polarity,EnEdgeBR,EnLevelBR,Encount,DigCal,AnCal,BRclk);
}

void MPAInterface::Enable_pix_counter(MPA* pMPA,uint32_t r,uint32_t p)
{
    uint32_t PixelMask=1,Polarity=1,EnEdgeBR=0,EnLevelBR=0,Encount=1,DigCal=0,AnCal=1,BRclk=0;
    Pix_Set_enable(pMPA,r,p,PixelMask,Polarity,EnEdgeBR,EnLevelBR,Encount,DigCal,AnCal,BRclk);
}

void MPAInterface::Enable_pix_sync(MPA* pMPA,uint32_t r,uint32_t p)
{
    uint32_t PixelMask=1,Polarity=1,EnEdgeBR=0,EnLevelBR=0,Encount=1,DigCal=0,AnCal=1,BRclk=0;
    Pix_Set_enable(pMPA,r,p,PixelMask,Polarity,EnEdgeBR,EnLevelBR,Encount,DigCal,AnCal,BRclk);
}

void MPAInterface::Disable_pixel(MPA* pMPA,uint32_t r,uint32_t p)
{
    uint32_t PixelMask=0,Polarity=0,EnEdgeBR=0,EnLevelBR=0,Encount=0,DigCal=0,AnCal=0,BRclk=0;
    Pix_Set_enable(pMPA,r,p,PixelMask,Polarity,EnEdgeBR,EnLevelBR,Encount,DigCal,AnCal,BRclk);
}

void MPAInterface::Enable_pix_digi(MPA* pMPA,uint32_t r,uint32_t p)
{
    uint32_t PixelMask=0,Polarity=0,EnEdgeBR=0,EnLevelBR=0,Encount=0,DigCal=1,AnCal=0,BRclk=0;
    Pix_Set_enable(pMPA,r,p,PixelMask,Polarity,EnEdgeBR,EnLevelBR,Encount,DigCal,AnCal,BRclk);
}

void MPAInterface::Set_calibration(MPA* pMPA,uint32_t cal)
{
    WriteMPAReg( pMPA,"CalDAC0",cal);
    WriteMPAReg( pMPA,"CalDAC1",cal);
    WriteMPAReg( pMPA,"CalDAC2",cal);
    WriteMPAReg( pMPA,"CalDAC3",cal);
    WriteMPAReg( pMPA,"CalDAC4",cal);
    WriteMPAReg( pMPA,"CalDAC5",cal);
    WriteMPAReg( pMPA,"CalDAC6",cal);
}

void MPAInterface::Set_threshold(MPA* pMPA,uint32_t th)
{
    setBoard(0);
    WriteMPAReg( pMPA,"ThDAC0",th);
    WriteMPAReg( pMPA,"ThDAC1",th);
    WriteMPAReg( pMPA,"ThDAC2",th);
    WriteMPAReg( pMPA,"ThDAC3",th);
    WriteMPAReg( pMPA,"ThDAC4",th);
    WriteMPAReg( pMPA,"ThDAC5",th);
    WriteMPAReg( pMPA,"ThDAC6",th);
}


void MPAInterface::Send_pulses(uint32_t n_pulse, uint32_t duration)
{

    fMPAFW->PS_Open_shutter();
    std::this_thread::sleep_for (std::chrono::milliseconds (10) );
    //notworking
    //for(int i=0; i<n_pulse; i++) fMPAFW->Send_test(duration);
    std::this_thread::sleep_for (std::chrono::milliseconds (1) );
    fMPAFW->PS_Close_shutter();
}


uint32_t MPAInterface::Read_pixel_counter(MPA* pMPA,uint32_t row, uint32_t pixel)
{
    setBoard(0);
    uint32_t data1 = Pix_read(pMPA,pMPA->getRegItem("ReadCounter_LSB"),row,pixel);
    uint32_t data2 = Pix_read(pMPA,pMPA->getRegItem("ReadCounter_MSB"),row,pixel);

    uint32_t data = ((data2 & 0x0ffffff) << 8) | (data1 & 0x0fffffff);
    return data;
}



uint32_t MPAInterface::ReadData( BeBoard* pBoard, bool pBreakTrigger, std::vector<uint32_t>& pData, bool pWait )
{
    setBoard(0);
    return fMPAFW->ReadData( pBoard, pBreakTrigger, pData, pWait );
}


void MPAInterface::Cleardata()
{
    setBoard(0);
    //fMPAFW->Cleardata( );
}


std::vector< uint32_t > MPAInterface::ReadConfig(const std::string& pFilename, int nmpa, int conf)
{

	   	        pugi::xml_document doc;
			std::string fullname = "settings/MPAFiles/Conf_"+pFilename+"_MPA"+std::to_string(nmpa)+"_config"+std::to_string(conf)+".xml";
	    		pugi::xml_parse_result result = doc.load_file( fullname.c_str() );
	    		if ( !result )
	    		{
				std::cout << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
				std::cout << "Error description : " << result.description() << std::endl;
	    		}

			std::vector< uint32_t > conf_upload(25);
			int perif = -1;
	    		for ( pugi::xml_node cBeBoardNode = doc.child( "CONF" ).child( "periphery" ).first_child(); cBeBoardNode; cBeBoardNode = cBeBoardNode.next_sibling() )
				{
				if (static_cast<std::string>(cBeBoardNode.name())=="OM") perif = convertAnyInt(cBeBoardNode.child_value());
				if (static_cast<std::string>(cBeBoardNode.name())=="RT") perif |= ((convertAnyInt(cBeBoardNode.child_value())& 3)   << 2 );
				if (static_cast<std::string>(cBeBoardNode.name())=="SCW") perif |= ((convertAnyInt(cBeBoardNode.child_value())& 15)   << 4 );
				if (static_cast<std::string>(cBeBoardNode.name())=="SH2") perif |= ((convertAnyInt(cBeBoardNode.child_value())& 15)  << 8 );
				if (static_cast<std::string>(cBeBoardNode.name())=="SH1") perif |= ((convertAnyInt(cBeBoardNode.child_value())& 15)  << 12);
				if (static_cast<std::string>(cBeBoardNode.name())=="CALDAC") perif |= ((convertAnyInt(cBeBoardNode.child_value())& 255) << 16);
				if (static_cast<std::string>(cBeBoardNode.name())=="THDAC") perif |= ((convertAnyInt(cBeBoardNode.child_value())& 255) << 24);
				}
			conf_upload[0] = perif;
	    		for ( pugi::xml_node cBeBoardNode = doc.child( "CONF" ).first_child(); cBeBoardNode; cBeBoardNode = cBeBoardNode.next_sibling() )
				{
				int pix = 0;
				if (static_cast<std::string>(cBeBoardNode.name())=="pixel")
					{
					int pixnum = convertAnyInt(cBeBoardNode.attribute("n").value());

					if (pixnum<17 and pixnum>8)
						{
							for ( pugi::xml_node cBeBoardNode1 = cBeBoardNode.first_child(); cBeBoardNode1; cBeBoardNode1 = cBeBoardNode1.next_sibling() )
							{
								if (static_cast<std::string>(cBeBoardNode1.name())=="PMR") pix |= convertAnyInt(cBeBoardNode1.child_value());		
								if (static_cast<std::string>(cBeBoardNode1.name())=="ARR") pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 1 );
								if (static_cast<std::string>(cBeBoardNode1.name())=="TRIMDACL")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 31)	<< 2 );
								if (static_cast<std::string>(cBeBoardNode1.name())=="CER")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 1)	<< 7 );
								if (static_cast<std::string>(cBeBoardNode1.name())=="SP")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 8 );
								if (static_cast<std::string>(cBeBoardNode1.name())=="SR")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 1)	<< 9 ) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="PML")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 1)	<< 10);
								if (static_cast<std::string>(cBeBoardNode1.name())=="ARL")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 11) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="TRIMDACR")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 31)	<< 12) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="CEL")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 17);
								if (static_cast<std::string>(cBeBoardNode1.name())=="CW")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 2)	<< 18);
			

							}
						}
					else if (pixnum<25 and pixnum>0)
						{
							for ( pugi::xml_node cBeBoardNode1 = cBeBoardNode.first_child(); cBeBoardNode1; cBeBoardNode1 = cBeBoardNode1.next_sibling() )
							{
								if (static_cast<std::string>(cBeBoardNode1.name())=="PML") pix |= convertAnyInt(cBeBoardNode1.child_value());
								if (static_cast<std::string>(cBeBoardNode1.name())=="ARL") pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 1 );
								if (static_cast<std::string>(cBeBoardNode1.name())=="TRIMDACL")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 31)	<< 2 );
								if (static_cast<std::string>(cBeBoardNode1.name())=="CEL")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 1)	<< 7 ) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="CW")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 3)	<< 8  );
								if (static_cast<std::string>(cBeBoardNode1.name())=="PMR")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 1)	<< 10) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="ARR")  pix |= ((convertAnyInt(cBeBoardNode1.child_value())& 1)	<< 11);
								if (static_cast<std::string>(cBeBoardNode1.name())=="TRIMDACR")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 31)	<< 12) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="CER")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 17) ;
								if (static_cast<std::string>(cBeBoardNode1.name())=="SP")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 18);
								if (static_cast<std::string>(cBeBoardNode1.name())=="SR")  pix |= ((convertAnyInt(cBeBoardNode1.child_value()) & 1)	<< 19);

							}
						}
					conf_upload[pixnum] = pix;
					}

				}
	  return conf_upload;
}


void MPAInterface::ModifyPerif(std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod , std::vector< uint32_t >* conf_upload)
{
	  std::vector<std::string> vars = mod.first;
	  std::vector< uint32_t > vals = mod.second;
	  uint32_t perif = conf_upload->at(0);

	  for (uint32_t iperif=0;iperif<vars.size(); iperif++)
	  {
		if (vars[iperif]=="OM") 
			{
			perif = (perif&~3);
			perif |= (vals[iperif]);
			}
		if (vars[iperif]=="RT") 
			{
			perif = (perif&~(3<<2));
			perif |= ((vals[iperif]& 3)   << 2 );
			}
		if (vars[iperif]=="SCW") 
			{
			perif = (perif&~(15<<4));
			perif |= ((vals[iperif]& 15)   << 4 );
			}
		if (vars[iperif]=="SH2") 
			{
			perif = (perif&~(15<<8));
			perif |= ((vals[iperif]& 15)  << 8 );
			}
		if (vars[iperif]=="SH1") 
			{
			perif = (perif&~(15<<12));
			perif |= ((vals[iperif]& 15)  << 12);
			}
		if (vars[iperif]=="CALDAC") 
			{
			perif = (perif&~(255<<16));
			perif |= ((vals[iperif]& 255) << 16);
			}
		if (vars[iperif]=="THDAC") 
			{
			perif = (perif&~(255<<24));
			perif |= ((vals[iperif]& 255) << 24);
			}

	  }
	  conf_upload->at(0) = perif;

}


}
