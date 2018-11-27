#include "Tool.h"
#include <TSystem.h>

Tool::Tool() :
    SystemController(),
    fCanvasMap(),
    fCbcHistMap(),
    fModuleHistMap(),
    fType(),
    fTestGroupChannelMap(),
    fDirectoryName (""),
    fResultFile (nullptr),
    fSkipMaskedChannels(false),
    fAllChan(false),
    fMaskChannelsFromOtherGroups(true)
{
#ifdef __HTTP__
    fHttpServer = nullptr;
#endif
}

#ifdef __HTTP__
Tool::Tool (THttpServer* pHttpServer) :
    SystemController(),
    fCanvasMap(),
    fCbcHistMap(),
    fModuleHistMap(),
    fType(),
    fTestGroupChannelMap(),
    fDirectoryName (""),
    fResultFile (nullptr),
    fHttpServer (pHttpServer),
    fSkipMaskedChannels(false),
    fAllChan(false),
    fMaskChannelsFromOtherGroups(true)
{
}
#endif

Tool::Tool (const Tool& pTool)
{
    fBeBoardInterface = pTool.fBeBoardInterface;
    fCbcInterface = pTool.fCbcInterface;
    fBoardVector = pTool.fBoardVector;
    fBeBoardFWMap = pTool.fBeBoardFWMap;
    fSettingsMap = pTool.fSettingsMap;
    fFileHandler = pTool.fFileHandler;

    fDirectoryName = pTool.fDirectoryName;             /*< the Directoryname for the Root file with results */
    fResultFile = pTool.fResultFile;                /*< the Name for the Root file with results */
    fType = pTool.fType;
#ifdef __HTTP__
    fHttpServer = pTool.fHttpServer;
#endif
    fCanvasMap = pTool.fCanvasMap;
    fCbcHistMap = pTool.fCbcHistMap;
    fModuleHistMap = pTool.fModuleHistMap;
    fTestGroupChannelMap = pTool.fTestGroupChannelMap;
}

Tool::~Tool()
{
}

void Tool::Inherit (Tool* pTool)
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
    fTestGroupChannelMap = pTool->fTestGroupChannelMap;
}

void Tool::Inherit (SystemController* pSystemController)
{
    fBeBoardInterface = pSystemController->fBeBoardInterface;
    fCbcInterface = pSystemController->fCbcInterface;
    fBoardVector = pSystemController->fBoardVector;
    fBeBoardFWMap = pSystemController->fBeBoardFWMap;
    fSettingsMap = pSystemController->fSettingsMap;
    fFileHandler = pSystemController->fFileHandler;
}

void Tool::Destroy()
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

    fCanvasMap.clear();
    fCbcHistMap.clear();
    fModuleHistMap.clear();
    fTestGroupChannelMap.clear();
}

void Tool::SoftDestroy()
{
    LOG (INFO) << BOLDRED << "Destroying only tool memory objects" << RESET;

    if (fResultFile != nullptr)
    {
        if (fResultFile->IsOpen() ) fResultFile->Close();

        if (fResultFile) delete fResultFile;
    }

    fCanvasMap.clear();
    fCbcHistMap.clear();
    fModuleHistMap.clear();
    fTestGroupChannelMap.clear();
}

void Tool::bookHistogram ( Cbc* pCbc, std::string pName, TObject* pObject )
{
    // find or create map<string,TOBject> for specific CBC
    auto cCbcHistMap = fCbcHistMap.find ( pCbc );

    if ( cCbcHistMap == std::end ( fCbcHistMap ) )
    {
        LOG (INFO) << "Histo Map for CBC " << int ( pCbc->getCbcId() ) <<  " (FE " << int ( pCbc->getFeId() ) << ") does not exist - creating " ;
        std::map<std::string, TObject*> cTempCbcMap;

        fCbcHistMap[pCbc] = cTempCbcMap;
        cCbcHistMap = fCbcHistMap.find ( pCbc );
    }

    // find histogram with given name: if it exists, delete the object, if not create
    auto cHisto = cCbcHistMap->second.find ( pName );

    if ( cHisto != std::end ( cCbcHistMap->second ) ) cCbcHistMap->second.erase ( cHisto );

    cCbcHistMap->second[pName] = pObject;
#ifdef __HTTP__

    if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);

#endif
}

void Tool::bookHistogram ( Module* pModule, std::string pName, TObject* pObject )
{
    // find or create map<string,TOBject> for specific CBC
    auto cModuleHistMap = fModuleHistMap.find ( pModule );

    if ( cModuleHistMap == std::end ( fModuleHistMap ) )
    {
        LOG (INFO) << "Histo Map for Module " << int ( pModule->getFeId() ) << " does not exist - creating " ;
        std::map<std::string, TObject*> cTempModuleMap;

        fModuleHistMap[pModule] = cTempModuleMap;
        cModuleHistMap = fModuleHistMap.find ( pModule );
    }

    // find histogram with given name: if it exists, delete the object, if not create
    auto cHisto = cModuleHistMap->second.find ( pName );

    if ( cHisto != std::end ( cModuleHistMap->second ) ) cModuleHistMap->second.erase ( cHisto );

    cModuleHistMap->second[pName] = pObject;
#ifdef __HTTP__

    if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);

#endif
}

TObject* Tool::getHist ( Cbc* pCbc, std::string pName )
{
    auto cCbcHistMap = fCbcHistMap.find ( pCbc );

    if ( cCbcHistMap == std::end ( fCbcHistMap ) )
    {
        LOG (ERROR) << RED << "Error: could not find the Histograms for CBC " << int ( pCbc->getCbcId() ) <<  " (FE " << int ( pCbc->getFeId() ) << ")" << RESET ;
        return nullptr;
    }
    else
    {
        auto cHisto = cCbcHistMap->second.find ( pName );

        if ( cHisto == std::end ( cCbcHistMap->second ) )
        {
            LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
            return nullptr;
        }
        else
            return cHisto->second;
    }
}

TObject* Tool::getHist ( Module* pModule, std::string pName )
{
    auto cModuleHistMap = fModuleHistMap.find ( pModule );

    if ( cModuleHistMap == std::end ( fModuleHistMap ) )
    {
        LOG (ERROR) << RED << "Error: could not find the Histograms for Module " << int ( pModule->getFeId() ) << RESET ;
        return nullptr;
    }
    else
    {
        auto cHisto = cModuleHistMap->second.find ( pName );

        if ( cHisto == std::end ( cModuleHistMap->second ) )
        {
            LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
            return nullptr;
        }
        else return cHisto->second;
    }
}

void Tool::SaveResults()
{
    // Now per FE
    for ( const auto& cFe : fModuleHistMap )
    {
        TString cDirName = Form ( "FE%d", cFe.first->getFeId() );
        TObject* cObj = gROOT->FindObject ( cDirName );

        //if ( cObj ) delete cObj;

        if (!cObj) fResultFile->mkdir ( cDirName );

        fResultFile->cd ( cDirName );

        for ( const auto& cHist : cFe.second )
            cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

        fResultFile->cd();
    }

    for ( const auto& cCbc : fCbcHistMap )
    {
        TString cDirName = Form ( "FE%dCBC%d", cCbc.first->getFeId(), cCbc.first->getCbcId() );
        TObject* cObj = gROOT->FindObject ( cDirName );

        //if ( cObj ) delete cObj;

        if (!cObj) fResultFile->mkdir ( cDirName );

        fResultFile->cd ( cDirName );

        for ( const auto& cHist : cCbc.second )
            cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

        fResultFile->cd();
    }

    // Save Canvasses too
    for ( const auto& cCanvas : fCanvasMap )
    {
        cCanvas.second->Write ( cCanvas.second->GetName(), TObject::kOverwrite );
        std::string cPdfName = fDirectoryName + "/" + cCanvas.second->GetName() + ".pdf";
        cCanvas.second->SaveAs ( cPdfName.c_str() );
    }

    //fResultFile->Write();
    //fResultFile->Close();

    LOG (INFO) << "Results saved!" ;
}

void Tool::CreateResultDirectory ( const std::string& pDirname, bool pMode, bool pDate )
{
    bool cCheck;
    bool cHoleMode;
    auto cSetting = fSettingsMap.find ( "HoleMode" );

    if ( cSetting != std::end ( fSettingsMap ) )
    {
        cCheck = true;
        cHoleMode = ( cSetting->second == 1 ) ? true : false;
    }

    std::string cMode;

    if ( cCheck )
    {
        if ( cHoleMode ) cMode = "_Hole";
        else cMode = "_Electron";
    }

    std::string nDirname = pDirname;

    if ( cCheck && pMode ) nDirname +=  cMode;

    if ( pDate ) nDirname +=  currentDateTime();

    LOG (INFO)  << "Creating directory: " << nDirname  ;
    std::string cCommand = "mkdir -p " + nDirname;

    try
    {
        system ( cCommand.c_str() );
    }
    catch (std::exception& e)
    {
        LOG (ERROR) << "Exceptin when trying to create Result Directory: " << e.what();
    }

    fDirectoryName = nDirname;
}
/*!
 * \brief Initialize the result Root file
 * \param pFilename : Root filename
 */
void Tool::InitResultFile ( const std::string& pFilename )
{

    if ( !fDirectoryName.empty() )
    {
        std::string cFilename = fDirectoryName + "/" + pFilename + ".root";

        try
        {
            fResultFile = TFile::Open ( cFilename.c_str(), "RECREATE" );
            fResultFileName = cFilename;
        }
        catch (std::exception& e)
        {
            LOG (ERROR) << "Exceptin when trying to create Result File: " << e.what();
        }
    }
    else LOG (INFO) << RED << "ERROR: " << RESET << "No Result Directory initialized - not saving results!" ;
}

void Tool::CloseResultFile()
{
    LOG (INFO) << BOLDRED << "closing result file!" << RESET;

    if (fResultFile)
        fResultFile->Close();
}
void Tool::StartHttpServer ( const int pPort, bool pReadonly )
{
#ifdef __HTTP__

    if (fHttpServer)
        delete fHttpServer;

    char hostname[HOST_NAME_MAX];

    try
    {
        fHttpServer = new THttpServer ( Form ( "http:%d", pPort ) );
        fHttpServer->SetReadOnly ( pReadonly );
        //fHttpServer->SetTimer ( pRefreshTime, kTRUE );
        fHttpServer->SetTimer (0, kTRUE);
        fHttpServer->SetJSROOT ("https://root.cern.ch/js/latest/");

        //configure the server
        // see: https://root.cern.ch/gitweb/?p=root.git;a=blob_plain;f=tutorials/http/httpcontrol.C;hb=HEAD
        fHttpServer->SetItemField ("/", "_monitoring", "5000");
        fHttpServer->SetItemField ("/", "_layout", "grid2x2");

        gethostname (hostname, HOST_NAME_MAX);
    }
    catch (std::exception& e)
    {
        LOG (ERROR) << "Exception when trying to start THttpServer: " << e.what();
    }

    LOG (INFO) << "Opening THttpServer on port " << pPort << ". Point your browser to: " << BOLDGREEN << hostname << ":" << pPort << RESET ;
#else
    LOG (INFO) << "Error, ROOT version < 5.34 detected or not compiled with Http Server support!"  << " No THttpServer available! - The webgui will fail to show plots!" ;
    LOG (INFO) << "ROOT must be built with '--enable-http' flag to use this feature." ;
#endif
}

void Tool::HttpServerProcess()
{
#ifdef __HTTP__

    if (fHttpServer)
    {
        gSystem->ProcessEvents();
        fHttpServer->ProcessRequests();
    }

#endif
}

void Tool::dumpConfigFiles()
{
    // visitor to call dumpRegFile on each Cbc
    struct RegMapDumper : public HwDescriptionVisitor
    {
        std::string fDirectoryName;
        RegMapDumper ( std::string pDirectoryName ) : fDirectoryName ( pDirectoryName ) {};
        void visit ( Cbc& pCbc )
        {
            if ( !fDirectoryName.empty() )
            {
                TString cFilename = fDirectoryName + Form ( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
                // cFilename += Form( "/FE%dCBC%d.txt", pCbc.getFeId(), pCbc.getCbcId() );
                pCbc.saveRegMap ( cFilename.Data() );
            }
            else LOG (INFO) << "Error: no results Directory initialized! "  ;
        }
    };

    RegMapDumper cDumper ( fDirectoryName );
    accept ( cDumper );

    LOG (INFO) << BOLDBLUE << "Configfiles for all Cbcs written to " << fDirectoryName << RESET ;
}
void Tool::setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState, bool pHoleMode )
{

    for (auto cBoard : this->fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cCbc : cFe->fCbcVector)
            {
                //first, get the Amux Value
                uint8_t cOriginalAmuxValue;
                cOriginalAmuxValue = cCbc->getReg ("MiscTestPulseCtrl&AnalogMux" );
                //uint8_t cOriginalHitDetectSLVSValue = cCbc->getReg ("HitDetectSLVS" );

                std::vector<std::pair<std::string, uint8_t>> cRegVec;
                uint8_t cRegValue =  to_reg ( 0, pTestGroup );

                if (cCbc->getChipType() == ChipType::CBC3)
                {
                    uint8_t cTPRegValue;

                    if (pTPState) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
                    else if (!pTPState) cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

                    //uint8_t cHitDetectSLVSValue = (cOriginalHitDetectSLVSValue & ~(0x1 << 6));

                    //cRegVec.push_back ( std::make_pair ( "HitDetectSLVS", cHitDetectSLVSValue ) );
                    cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux",  cTPRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulseDel&ChanGroup",  cRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulsePotNodeSel", pTPAmplitude ) );
                    LOG (DEBUG) << BOLDBLUE << "Read original Amux Value to be: " << std::bitset<8> (cOriginalAmuxValue) << " and changed to " << std::bitset<8> (cTPRegValue) << " - the TP is bit 6!" RESET;
                }
                else
                {
                    //CBC2
                    cRegVec.push_back ( std::make_pair ( "SelTestPulseDel&ChanGroup",  cRegValue ) );

                    uint8_t cTPRegValue;

                    if (pTPState) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
                    else if (!pTPState) cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

                    //uint8_t cHitDetectSLVSValue = (cOriginalHitDetectSLVSValue & ~(0x1 << 6));

                    //cRegVec.push_back ( std::make_pair ( "HitDetectSLVS", cHitDetectSLVSValue ) );
                    cRegVec.push_back ( std::make_pair ( "MiscTestPulseCtrl&AnalogMux", cTPRegValue ) );
                    cRegVec.push_back ( std::make_pair ( "TestPulsePot", pTPAmplitude ) );
                }

                this->fCbcInterface->WriteCbcMultReg (cCbc, cRegVec);
            }
        }
    }
}

void Tool::setFWTestPulse()
{
    for (auto& cBoard : fBoardVector)
    {
        std::vector<std::pair<std::string, uint32_t> > cRegVec;
        BoardType cBoardType = cBoard->getBoardType();

        if (cBoardType == BoardType::GLIB || cBoardType == BoardType::CTA)
        {
            cRegVec.push_back ({"COMMISSIONNING_MODE_RQ", 1 });
            cRegVec.push_back ({"COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID", 1 });
        }
        else if (cBoardType == BoardType::ICGLIB || cBoardType == BoardType::ICFC7)
        {
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle.mode_flags.test_pulse_enable", 1 });
            cRegVec.push_back ({"cbc_daq_ctrl.commissioning_cycle_ctrl", 0x1 });
        }
        else if (cBoardType == BoardType::CBC3FC7)
        {
            cRegVec.push_back ({"cbc_system_cnfg.fast_signal_manager.fast_signal_generator.enable.test_pulse", 0x1});
        }
        else if(cBoardType == BoardType::D19C)
        {
            cRegVec.push_back ({"fc7_daq_cnfg.fast_command_block.trigger_source", 6});
            cRegVec.push_back ({"fc7_daq_ctrl.fast_command_block.control.load_config", 0x1});
        }

        fBeBoardInterface->WriteBoardMultReg (cBoard, cRegVec);
    }
}

void Tool::MakeTestGroups ( bool pAllChan )
{
    if ( !pAllChan )
    {
        for ( int cGId = 0; cGId < 8; cGId++ )
        {
            std::vector<uint8_t> tempchannelVec;

            for ( int idx = 0; idx < 16; idx++ )
            {
                int ctemp1 = idx * 16 + cGId * 2;
                int ctemp2 = ctemp1 + 1;

                if ( ctemp1 < 254 ) tempchannelVec.push_back ( ctemp1 );

                if ( ctemp2 < 254 )  tempchannelVec.push_back ( ctemp2 );

            }

            fTestGroupChannelMap[cGId] = tempchannelVec;

        }

        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;
    }
    else
    {
        int cGId = -1;
        std::vector<uint8_t> tempchannelVec;

        for ( int idx = 0; idx < 254; idx++ )
            tempchannelVec.push_back ( idx );

        fTestGroupChannelMap[cGId] = tempchannelVec;

    }
}

void Tool::CreateReport()
{
    std::ofstream report;
    report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
    report.close();
}
void Tool::AmmendReport (std::string pString )
{
    std::ofstream report;
    report.open (fDirectoryName + "/TestReport.txt", std::ofstream::out | std::ofstream::app);
    report << pString << std::endl;
    report.close();
}


// decode bend LUT for a given CBC
std::map<uint8_t, double> Tool::decodeBendLUT(Cbc* pCbc)
{

    std::map<uint8_t, double> cLUT;

    double cBend=-7.0; 
    uint8_t cRegAddress = 0x40; 
    LOG (DEBUG) << BOLDGREEN << "Decoding bend LUT for CBC" << +pCbc->getCbcId() << " ." << RESET; 
    for( int i = 0 ; i <= 14 ; i++ )
    {
        TString cRegName = Form ( "Bend%d", i  );
        uint8_t cRegValue = fCbcInterface->ReadCbcReg (pCbc, cRegName.Data() ); 
        //LOG (INFO) << BOLDGREEN << "Reading register " << cRegName.Data() << " - value of 0x" << std::hex <<  +cRegValue << " found [LUT entry for bend of " << cBend << " strips]" <<  RESET;

        uint8_t cLUTvalue_0 = (cRegValue >> 0) & 0x0F;
        uint8_t cLUTvalue_1 = (cRegValue >> 4) & 0x0F;

        LOG (DEBUG) << BOLDGREEN << "LUT entry for bend of " << cBend << " strips found to be " << std::bitset<4>(cLUTvalue_0) <<   RESET;
        cLUT[cLUTvalue_0] =  cBend ; 
        // just check if the bend code is already in the map 
        // and if it is ... do nothing 
        cBend += 0.5;
        if( cBend > 7) continue; 

        auto cItem = cLUT.find(cLUTvalue_1);
        if(cItem == cLUT.end()) 
        {
            LOG (DEBUG) << BOLDGREEN << "LUT entry for bend of " << cBend << " strips found to be " << std::bitset<4>(cLUTvalue_1) <<   RESET;
            cLUT[cLUTvalue_1] = cBend ; 
        }
        cBend += 0.5;
    }
    return cLUT;
}


// first a method to mask all channels in the CBC 
void Tool::maskAllChannels (Cbc* pCbc)
{
    uint8_t cRegValue ;
    std::string cRegName;
    // this doesn't seem like the smartest way to do this .... but ok might work for now 
    // TO-DO : figure out how to do this "properly" 
    ChipType cChipType; 
    for ( BeBoard* pBoard : fBoardVector )
    {
        for (auto cFe : pBoard->fModuleVector)
        {
            cChipType = cFe->getChipType();
        }
    }

    
    if (cChipType == ChipType::CBC2)
    {
        for ( unsigned int i = 0 ; i < fChannelMaskMapCBC2.size() ; i++ )
        {
            pCbc->setReg (fChannelMaskMapCBC2[i], 0);
            cRegValue = pCbc->getReg (fChannelMaskMapCBC2[i]);
            cRegName =  fChannelMaskMapCBC2[i];
            fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
        }
    }

    if (cChipType == ChipType::CBC3)
    {
        RegisterVector cRegVec; cRegVec.clear(); 
        for ( unsigned int i = 0 ; i < fChannelMaskMapCBC3.size() ; i++ )
        {
            cRegVec.push_back ( {fChannelMaskMapCBC3[i] ,0 } ); 
            //pCbc->setReg (fChannelMaskMapCBC3[i], 0);
            //cRegValue = pCbc->getReg (fChannelMaskMapCBC3[i]);
            //cRegName =  fChannelMaskMapCBC3[i];
            //fCbcInterface->WriteCbcReg ( pCbc, cRegName,  cRegValue  );
            LOG (DEBUG) << BOLDBLUE << fChannelMaskMapCBC3[i] << " " << std::bitset<8> (0);
        }
        fCbcInterface->WriteCbcMultReg ( pCbc , cRegVec );

    }
}


// then a method to un-mask pairs of channels on a given CBC
void Tool::unmaskPair(Cbc* cCbc ,  std::pair<uint16_t,uint16_t> pPair)
{
    ChipType cChipType; 
    for ( BeBoard* pBoard : fBoardVector )
    {
        for (auto cFe : pBoard->fModuleVector)
        {
            cChipType = cFe->getChipType();
        }
    }

    // get ready to mask/un-mask channels in pairs... 
    MaskedChannelsList cMaskedList; 
    MaskedChannels cMaskedChannels; cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.first);
    
    uint8_t cRegisterIndex = pPair.first/ 8;
    std::string cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
    cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
    
    cRegisterIndex = pPair.second/ 8;
    cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
    auto it = cMaskedList.find(cMaskRegName.c_str() );
    if (it != cMaskedList.end())
    {
        ( it->second ).push_back( pPair.second );
    }
    else
    {
        cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.second);
        cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
    }

    // do the actual channel un-masking
    //LOG (INFO) << GREEN << "\t ......... UNMASKing channels : " << RESET ;  
    for( auto cMasked : cMaskedList)
    {
        uint8_t cRegValue = 0; //cCbc->getReg (cMasked.first);
        std::string cOutput = "";  
        for(auto cMaskedChannel : cMasked.second )
        {
            uint8_t cBitShift = (cMaskedChannel) % 8;
            cRegValue |=  (1 << cBitShift);
            std::string cChType =  ( (+cMaskedChannel % 2) == 0 ) ? "seed" : "correlation"; 
            TString cOut; cOut.Form("Channel %d in the %s layer\t", (int)cMaskedChannel, cChType.c_str() ); 
            cOutput += cOut.Data(); 
        }
        //LOG (INFO) << GREEN << "\t Writing " << std::bitset<8> (cRegValue) <<  " to " << cMasked.first << " to UNMASK channels for stub sweep : " << cOutput.c_str() << RESET ; 
        fCbcInterface->WriteCbcReg ( cCbc, cMasked.first ,  cRegValue  );
    }

}



// and finally a method to un-mask a list of channels on a given CBC
void Tool::unmaskList(Cbc* cCbc , std::vector<uint16_t> pList )
{
    ChipType cChipType; 
    for ( BeBoard* pBoard : fBoardVector )
    {
        for (auto cFe : pBoard->fModuleVector)
        {
            cChipType = cFe->getChipType();
        }
    }

    // get ready to mask/un-mask channels in pairs... 
    uint16_t cChan = pList[0];
    MaskedChannelsList cMaskedList; 
    MaskedChannels cMaskedChannels; cMaskedChannels.clear(); cMaskedChannels.push_back(cChan);
    uint8_t cRegisterIndex = cChan/ 8;
    std::string cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
    cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  , cMaskedChannels ) );

    for( unsigned int cIndex = 1 ; cIndex < pList.size(); cIndex ++ )
    {
        cChan = pList[cIndex];
        cRegisterIndex = cChan/8;
        cMaskRegName = (cChipType == ChipType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
        auto it = cMaskedList.find(cMaskRegName.c_str() );
        if (it != cMaskedList.end())
        {
            ( it->second ).push_back( cChan );
        }
        else
        {
            cMaskedChannels.clear(); cMaskedChannels.push_back(cChan);
            cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
        }
    }

    // do the actual channel un-masking
    LOG (DEBUG) << GREEN << "\t ......... UNMASKing channels : " << RESET ;  
    for( auto cMasked : cMaskedList)
    {
        //LOG (INFO) << GREEN << "\t Writing to " << cMasked.first << " to un-mask " <<  (cMasked.second).size() << " channel(s) : " << RESET ; 
        // store original value of registe 
        uint8_t cRegValue = 0; //cCbc->getReg (cMasked.first);
        std::string cOutput = "";  
        for(auto cMaskedChannel : cMasked.second )
        {
            uint8_t cBitShift = (cMaskedChannel) % 8;
            cRegValue |=  (1 << cBitShift);
            std::string cChType =  ( (+cMaskedChannel % 2) == 0 ) ? "seed" : "correlation"; 
            TString cOut; cOut.Form("Channel %d in the %s layer\t", (int)cMaskedChannel, cChType.c_str() ); 
            //LOG (INFO) << cOut.Data();
            cOutput += cOut.Data(); 
        }
        LOG (DEBUG) << GREEN << "\t Writing " << std::bitset<8> (cRegValue) <<  " to " << cMasked.first << " to UNMASK channels for stub sweep : " << cOutput.c_str() << RESET ; 
        fCbcInterface->WriteCbcReg ( cCbc, cMasked.first ,  cRegValue  );
    }

}

// Two dimensional dac scan
void Tool::scanDacDac(const std::string &dac1Name, const std::vector<uint16_t> &dac1List, const std::string &dac2Name, const std::vector<uint16_t> &dac2List, const uint16_t &numberOfEvents, std::map<uint16_t, std::map<uint16_t, std::map<uint16_t, ModuleOccupancyMap> > > &backEndOccupancyMap){return;}

// Two dimensional dac scan per BeBoard
void Tool::scanBeBoardDacDac(BeBoard* pBoard, const std::string &dac1Name, const std::vector<uint16_t> &dac1List, const std::string &dac2Name, const std::vector<uint16_t> &dac2List, const uint16_t &numberOfEvents, std::map<uint16_t, std::map<uint16_t, ModuleOccupancyMap> > &moduleOccupancyMap){return;}

// One dimensional dac scan
void Tool::scanDac(const std::string &dacName, const std::vector<uint16_t> &dacList, const uint16_t &numberOfEvents, std::map<uint16_t, std::map<uint16_t, ModuleOccupancyMap> > &backEndOccupancyMap){return;}

// One dimensional dac scan per BeBoard
void Tool::scanBeBoardDac(BeBoard* pBoard, const std::string &dacName, const std::vector<uint16_t> &dacList, const uint16_t &numberOfEvents, std::map<uint16_t, ModuleOccupancyMap> &moduleOccupancyMap){return;}

// bit wise scan
void Tool::bitWiseScan(const std::string &dacName, const uint16_t &numberOfEvents, const float &targetOccupancy , std::map<uint16_t, ModuleOccupancyMap> &backEndOccupanyAtTargetMap){return;}

// set dac and measure occupancy
void Tool::setDacAndMeasureOccupancy(const std::string &dacName, const uint16_t &dacValue, const uint16_t &numberOfEvents, std::map<uint16_t, ModuleOccupancyMap> &backEndOccupancyMap){return;}

// set dac and measure occupancy per BeBoard
void Tool::setDacAndMeasureBeBoardOccupancy(BeBoard* pBoard, const std::string &dacName, const uint16_t &dacValue, const uint16_t &numberOfEvents, ModuleOccupancyMap &moduleOccupancyMap){return;}


// measure occupancy
void Tool::measureBeBoardOccupancy(BeBoard* pBoard, const uint16_t &numberOfEvents, ModuleOccupancyMap &moduleOccupancyMap){
    
    for(const auto & group : fTestGroupChannelMap){
        float groupOccupancy=0;

        if(!fAllChan && fMaskChannelsFromOtherGroups){// mask channel not scanned

        }
        measureBeBoardOccupancyPerGroup(group.second, pBoard, numberOfEvents, moduleOccupancyMap, groupOccupancy);
    }

    if(!fAllChan && fMaskChannelsFromOtherGroups){//re-enable all the channels

    }

    return;
}

// measure occupancy per group
void Tool::measureBeBoardOccupancyPerGroup(const std::vector<uint8_t> &cTestGrpChannelVec, BeBoard* pBoard, const uint16_t &numberOfEvents, ModuleOccupancyMap &moduleOccupancyMap, float &groupOccupancy){
   
    // const std::vector<uint8_t>& cTestGrpChannelVec = fTestGroupChannelMap[pTGrpId];

    ReadNEvents ( pBoard, numberOfEvents );


    const std::vector<Event*>& events = GetEvents ( pBoard );

    uint32_t cMaxHits=0;
    uint32_t cHitCounter = 0;

    // Loop over Events from this Acquisition
    for ( auto& ev : events )
    {

        for ( auto cFe : pBoard->fModuleVector )
        {
            if(moduleOccupancyMap.find(cFe->getModuleId())==moduleOccupancyMap.end()){
                moduleOccupancyMap[cFe->getModuleId()]=CbcOccupancyMap();
            }
            CbcOccupancyMap *cbcOccupancy = &moduleOccupancyMap[cFe->getModuleId()];

            for ( auto cCbc : cFe->fCbcVector )
            {
                // TH2F* cSCurveHist = dynamic_cast<TH2F*> (this->getHist (cCbc, pHistName) );
                 const uint32_t *cbcMask = cCbc->getCbcmask();

                 // std::cout<< (cbcMask[0]&0x0f) <<std::endl;

                if(cbcOccupancy->find(cCbc->getCbcId())==cbcOccupancy->end()){
                    cbcOccupancy->at(cCbc->getCbcId())=ChannelOccupancyMap();
                }
                ChannelOccupancyMap *stripOccupancy = &cbcOccupancy->at(cCbc->getCbcId());

                for ( auto& cChan : cTestGrpChannelVec )
                {

                    bool isChannelEnabled = true;
                    // if(true){
                    if(fSkipMaskedChannels){
                        if(!( (cbcMask[cChan>>5]>>(cChan&0x1F)) &0x1) ){
                            isChannelEnabled=false;
                        }
                    }

                    if(isChannelEnabled) cMaxHits++;

                    if ( ev->DataBit ( cFe->getFeId(), cCbc->getCbcId(), cChan) )
                    {
                        if(stripOccupancy->find(cChan)==stripOccupancy->end()){
                            stripOccupancy->at(cChan)=1;
                        }
                        else ++stripOccupancy->at(cChan);

                        //fill the strip number and the current threshold
                        // cSCurveHist->Fill (cChan, cValue);
                        if(isChannelEnabled) cHitCounter++;
                    }
                }
            }
        }
    }

    groupOccupancy = cHitCounter/cMaxHits;

    return;
}

