/*!

        \file                   Tool.h
        \brief                                   Controller of the System, overall wrapper of the framework
        \author                                  Georg AUZINGER
        \version                 1.0
        \date                                    06/02/15
        Support :                                mail to : georg.auzinger@cern.ch

 */


#ifndef __TOOL_H__
#define __TOOL_H__

#include "../System/SystemController.h"
#include "TROOT.h"
#include "TFile.h"
#include "TObject.h"
#include "TCanvas.h"

#ifdef __HTTP__
#include "THttpServer.h"
#endif

using namespace Ph2_System;

using CbcHistogramMap = std::map<Cbc*, std::map<std::string, TObject*> >;
using ModuleHistogramMap = std::map<Module*, std::map<std::string, TObject*> >;
using CanvasMap = std::map<Ph2_HwDescription::FrontEndDescription*, TCanvas*>;
using TestGroupChannelMap =  std::map< int, std::vector<uint8_t> >;


/*!
 * \class Tool
 * \brief A base class for all kinds of applications that have to use ROOT that inherits from SystemController which does not have any dependence on ROOT
 */
class Tool : public SystemController
{
  public:
    std::string fDirectoryName;             /*< the Directoryname for the Root file with results */
    TFile* fResultFile;                /*< the Name for the Root file with results */
#ifdef __HTTP__
    THttpServer* fHttpServer;
#endif

    Tool()
    {
        fResultFile = nullptr;
#ifdef __HTTP__
        fHttpServer = nullptr;
#endif
    }

    ~Tool()
    {
    }

    void Destroy()
    {
        LOG (INFO) << BOLDRED << "Destroying memory objects" << RESET;
        SystemController::Destroy();
#ifdef __HTTP__

        if (fHttpServer) delete fHttpServer;

#endif

        if (fResultFile != nullptr)
        {
            if (fResultFile->IsOpen() ) fResultFile->Close();

            if (fResultFile) delete fResultFile;
        }
    }


  public:
    CanvasMap fCanvasMap;
    CbcHistogramMap fCbcHistMap;
    ModuleHistogramMap fModuleHistMap;
    ChipType fType;
    TestGroupChannelMap fTestGroupChannelMap;


    /*!
     * \brief Create a result directory at the specified path + ChargeMode + Timestamp
     * \param pDirectoryname : the name of the directory to create
     * \param pDate : apend the current date and time to the directoryname
     */
  public:
    Tool (const Tool& pTool);

    void Inherit (Tool* pTool)
    {
        fBeBoardInterface = pTool->fBeBoardInterface;
        fCbcInterface = pTool->fCbcInterface;
        fBoardVector = pTool->fBoardVector;
        fBeBoardFWMap = pTool->fBeBoardFWMap;
        fSettingsMap = pTool->fSettingsMap;
        fFileHandler = pTool->fFileHandler;
        fDirectoryName = pTool->fDirectoryName;
        fResultFile = pTool->fResultFile;
        fType = pTool->fType;
#ifdef __HTTP__
        fHttpServer = pTool->fHttpServer;
#endif
        fCanvasMap = pTool->fCanvasMap;
        fCbcHistMap = pTool->fCbcHistMap;
        fModuleHistMap = pTool->fModuleHistMap;
    }

    void CreateReport()
    {
        std::ofstream report;
        report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
        report.close();
    };
    void AmmendReport (std::string pString )
    {
        std::ofstream report;
        report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
        report << pString << std::endl;
        report.close();
    };

    void bookHistogram ( Cbc* pCbc, std::string pName, TObject* pObject );

    void bookHistogram ( Module* pModule, std::string pName, TObject* pObject );

    TObject* getHist ( Cbc* pCbc, std::string pName );

    TObject* getHist ( Module* pModule, std::string pName );

    void SaveResults();

    void CreateResultDirectory ( const std::string& pDirname, bool pMode = true, bool pDate = true );

    /*!
     * \brief Initialize the result Root file
     * \param pFilename : Root filename
     */
    void InitResultFile ( const std::string& pFilename );
    void CloseResultFile()
    {
        LOG (INFO) << BOLDRED << "closing result file!" << RESET;

        if (fResultFile)
            fResultFile->Close();
    }

    void StartHttpServer ( const int pPort = 8082, const int pRefreshTime = 100, bool pReadonly = true );
    //void StartHttpServer ( const int pPort = 8080, bool pReadonly = true );
    void ProcessRequests()
    {
#ifdef __HTTP__

        if (fHttpServer) fHttpServer->ProcessRequests();

#endif
    }
    void dumpConfigFiles();
    // general stuff that can be useful
    void setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState = false, bool pHoleMode = false );
    //enable commissioning loops and Test Pulse
    void setFWTestPulse();
    // make test groups for everything Test pulse or Calibration
    void MakeTestGroups ( bool pAllChan = false );
    // helper methods
    void setRegBit ( uint16_t& pRegValue, uint8_t pPos, bool pValue )
    {
        pRegValue ^= ( -pValue ^ pRegValue ) & ( 1 << pPos );
    }

    void toggleRegBit ( uint16_t& pRegValue, uint8_t pPos )
    {
        pRegValue ^= 1 << pPos;
    }

    bool getBit ( uint16_t& pRegValue, uint8_t pPos )
    {
        return ( pRegValue >> pPos ) & 1;
    }

    /*!
    * \brief reverse the endianess before writing in to the register
    * \param pDelay: the actual delay
    * \param pGroup: the actual group number
    * \return the reversed endianness
    */
    uint8_t to_reg ( uint8_t pDelay, uint8_t pGroup )
    {

        uint8_t cValue = ( ( reverse ( pDelay ) ) & 0xF8 ) |
                         ( ( reverse ( pGroup ) ) >> 5 );

        //LOG(DBUG) << std::bitset<8>( cValue ) << " cGroup " << +pGroup << " " << std::bitset<8>( pGroup ) << " pDelay " << +pDelay << " " << std::bitset<8>( pDelay ) ;
        return cValue;
    }
    /*!
    * \brief reverse the byte
    * \param n:the number to be reversed
    * \return the reversed number
    */
    uint8_t reverse ( uint8_t n )
    {
        // Reverse the top and bottom nibble then swap them.
        return ( fLookup[n & 0xF] << 4 ) | fLookup[n >> 4];
    }

    unsigned char fLookup[16] =
    {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    }; /*!< Lookup table for reverce the endianness */

    std::map<uint8_t, std::string> fChannelMaskMapCBC2 =
    {
        { 0, "MaskChannelFrom008downto001" },
        { 1, "MaskChannelFrom016downto009" },
        { 2, "MaskChannelFrom024downto017" },
        { 3, "MaskChannelFrom032downto025" },
        { 4, "MaskChannelFrom040downto033" },
        { 5, "MaskChannelFrom048downto041" },
        { 6, "MaskChannelFrom056downto049" },
        { 7, "MaskChannelFrom064downto057" },
        { 8, "MaskChannelFrom072downto065" },
        { 9, "MaskChannelFrom080downto073" },
        {10, "MaskChannelFrom088downto081" },
        {11, "MaskChannelFrom096downto089" },
        {12, "MaskChannelFrom104downto097" },
        {13, "MaskChannelFrom112downto105" },
        {14, "MaskChannelFrom120downto113" },
        {15, "MaskChannelFrom128downto121" },
        {16, "MaskChannelFrom136downto129" },
        {17, "MaskChannelFrom144downto137" },
        {18, "MaskChannelFrom152downto145" },
        {19, "MaskChannelFrom160downto153" },
        {20, "MaskChannelFrom168downto161" },
        {21, "MaskChannelFrom176downto169" },
        {22, "MaskChannelFrom184downto177" },
        {23, "MaskChannelFrom192downto185" },
        {24, "MaskChannelFrom200downto193" },
        {25, "MaskChannelFrom208downto201" },
        {26, "MaskChannelFrom216downto209" },
        {27, "MaskChannelFrom224downto217" },
        {28, "MaskChannelFrom232downto225" },
        {29, "MaskChannelFrom240downto233" },
        {30, "MaskChannelFrom248downto241" },
        {31, "MaskChannelFrom254downto249" }
    };

    std::map<uint8_t, std::string> fChannelMaskMapCBC3 =
    {
        { 0, "MaskChannel-008-to-001" },
        { 1, "MaskChannel-016-to-009" },
        { 2, "MaskChannel-024-to-017" },
        { 3, "MaskChannel-032-to-025" },
        { 4, "MaskChannel-040-to-033" },
        { 5, "MaskChannel-048-to-041" },
        { 6, "MaskChannel-056-to-049" },
        { 7, "MaskChannel-064-to-057" },
        { 8, "MaskChannel-072-to-065" },
        { 9, "MaskChannel-080-to-073" },
        {10, "MaskChannel-088-to-081" },
        {11, "MaskChannel-096-to-089" },
        {12, "MaskChannel-104-to-097" },
        {13, "MaskChannel-112-to-105" },
        {14, "MaskChannel-120-to-113" },
        {15, "MaskChannel-128-to-121" },
        {16, "MaskChannel-136-to-129" },
        {17, "MaskChannel-144-to-137" },
        {18, "MaskChannel-152-to-145" },
        {19, "MaskChannel-160-to-153" },
        {20, "MaskChannel-168-to-161" },
        {21, "MaskChannel-176-to-169" },
        {22, "MaskChannel-184-to-177" },
        {23, "MaskChannel-192-to-185" },
        {24, "MaskChannel-200-to-193" },
        {25, "MaskChannel-208-to-201" },
        {26, "MaskChannel-216-to-209" },
        {27, "MaskChannel-224-to-217" },
        {28, "MaskChannel-232-to-225" },
        {29, "MaskChannel-240-to-233" },
        {30, "MaskChannel-248-to-241" },
        {31, "MaskChannel-254-to-249" }
    };
};
#endif
