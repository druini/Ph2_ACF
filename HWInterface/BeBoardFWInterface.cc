/*

        FileName :                    BeBoardFWInterface.cc
        Content :                     BeBoardFWInterface base class of all type of boards
        Programmer :                  Lorenzo BIDEGAIN, Nicolas PIERRE
        Version :                     1.0
        Date of creation :            31/07/14
        Support :                     mail to : lorenzo.bidegain@gmail.com, nico.pierre@icloud.com

 */



#include "BeBoardFWInterface.h"
#include "FpgaConfig.h"

namespace Ph2_HwInterface {

    //Constructor, makes the board map
    BeBoardFWInterface::BeBoardFWInterface ( const char* puHalConfigFileName, uint32_t pBoardId ) :
        RegManager ( puHalConfigFileName, pBoardId ),
        //runningAcquisition ( false ),
        numAcq ( 0 ),
        fSaveToFile ( false ),
        fFileHandler ( nullptr )
    {
    }

    //Constructor, makes the board map
    BeBoardFWInterface::BeBoardFWInterface ( const char* pId, const char* pUri, const char* pAddressTable ) :
        RegManager ( pId, pUri, pAddressTable ),
        //runningAcquisition ( false ),
        numAcq ( 0 ),
        fSaveToFile ( false ),
        fFileHandler ( nullptr )
    {
    }

    std::string BeBoardFWInterface::readBoardType()
    {
        std::string cBoardTypeString;

        uhal::ValWord<uint32_t> cBoardType = ReadReg ( "board_id" );

        char cChar = ( ( cBoardType & cMask4 ) >> 24 );
        cBoardTypeString.push_back ( cChar );

        cChar = ( ( cBoardType & cMask3 ) >> 16 );
        cBoardTypeString.push_back ( cChar );

        cChar = ( ( cBoardType & cMask2 ) >> 8 );
        cBoardTypeString.push_back ( cChar );

        cChar = ( cBoardType & cMask1 );
        cBoardTypeString.push_back ( cChar );

        return cBoardTypeString;

    }

    //void BeBoardFWInterface::getBoardInfo()
    //{
    //LOG(INFO) << "FMC1 present : " << ReadReg( "status.fmc1_present" ) ;
    //LOG(INFO) << "FMC2 present : " << ReadReg( "status.fmc2_present" ) ;
    //LOG(INFO) << "FW version : " << ReadReg( "firm_id.firmware_major" ) << "." << ReadReg( "firm_id.firmware_minor" ) << "." << ReadReg( "firm_id.firmware_build" ) ;

    //uhal::ValWord<uint32_t> cBoardType = ReadReg( "board_id" );

    //LOG(INFO) << "BoardType : ";

    //char cChar = ( ( cBoardType & cMask4 ) >> 24 );
    //LOG(INFO) << cChar;

    //cChar = ( ( cBoardType & cMask3 ) >> 16 );
    //LOG(INFO) << cChar;

    //cChar = ( ( cBoardType & cMask2 ) >> 8 );
    //LOG(INFO) << cChar;

    //cChar = ( cBoardType & cMask1 );
    //LOG(INFO) << cChar ;

    //LOG(INFO) << "FMC User Board ID : " << ReadReg( "user_wb_ttc_fmc_regs.user_board_id" ) ;
    //LOG(INFO) << "FMC User System ID : " << ReadReg( "user_wb_ttc_fmc_regs.user_sys_id" ) ;
    //LOG(INFO) << "FMC User Version : " << ReadReg( "user_wb_ttc_fmc_regs.user_version" ) ;

    //}


    //void BeBoardFWInterface::StopThread()
    //{
    //if ( runningAcquisition )
    //{
    //runningAcquisition = false;

    //try
    //{
    //thrAcq.join();
    //}
    //catch ( std::exception& e )
    //{
    //std::cerr << "Death to Stop in BeBoardFWInterface::StopThread()" << e.what() ;
    //}
    //catch ( ... )
    //{
    //std::cerr << "Death to Stop in BeBoardFWInterface::StopThread(). failed to perform thrAcq.join()" ;
    //}
    //}

    //}

    //int BeBoardFWInterface::getNumAcqThread()
    //{
    //return ( int ) numAcq;
    //}
}
