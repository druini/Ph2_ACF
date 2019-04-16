#include "Tool.h"
#include <TSystem.h>
#include "../Utils/ObjectStreamer.h"
#include "../Utils/ChannelGroupHandler.h"

Tool::Tool() :
    SystemController(),
    fCanvasMap(),
    fChipHistMap(),
    fModuleHistMap(),
    fType(),
    fTestGroupChannelMap(),
    fDirectoryName (""),
    fResultFile (nullptr),
    fSkipMaskedChannels(false),
    fAllChan(false),
    fMaskChannelsFromOtherGroups(false),
    fTestPulse(false),
    fChannelGroupHandler(nullptr)
{
#ifdef __HTTP__
    fHttpServer = nullptr;
#endif
}

#ifdef __HTTP__
Tool::Tool (THttpServer* pHttpServer)
: SystemController            ()
, fCanvasMap                  ()
, fChipHistMap                ()
, fModuleHistMap              ()
, fType                       ()
, fTestGroupChannelMap        ()
, fDirectoryName              ("")
, fResultFile                 (nullptr)
, fHttpServer                 (pHttpServer)
, fSkipMaskedChannels         (false)
, fAllChan                    (false)
, fMaskChannelsFromOtherGroups(false)
, fTestPulse                  (false)
{
}
#endif

Tool::Tool (const Tool& pTool)
{
	fDetectorContainer   = pTool.fDetectorContainer;
    fBeBoardInterface    = pTool.fBeBoardInterface;
    fChipInterface       = pTool.fChipInterface;
    fBoardVector         = pTool.fBoardVector;
    fBeBoardFWMap        = pTool.fBeBoardFWMap;
    fSettingsMap         = pTool.fSettingsMap;
    fFileHandler         = pTool.fFileHandler;

    fDirectoryName       = pTool.fDirectoryName;             /*< the Directoryname for the Root file with results */
    fResultFile          = pTool.fResultFile;                /*< the Name for the Root file with results */
    fType                = pTool.fType;
    fCanvasMap           = pTool.fCanvasMap;
    fChipHistMap         = pTool.fChipHistMap;
    fModuleHistMap       = pTool.fModuleHistMap;
    fBeBoardHistMap      = pTool.fBeBoardHistMap;
    fTestGroupChannelMap = pTool.fTestGroupChannelMap;

#ifdef __HTTP__
    fHttpServer          = pTool.fHttpServer;
#endif
}

Tool::~Tool()
{
}

void Tool::Inherit (Tool* pTool)
{
	fDetectorContainer   = pTool->fDetectorContainer;//IS THIS RIGHT?????? HERE WE ARE COPYING THE OBJECTS!!!!!
	fBeBoardInterface    = pTool->fBeBoardInterface;
    fChipInterface       = pTool->fChipInterface;
    fBoardVector         = pTool->fBoardVector;
    fBeBoardFWMap        = pTool->fBeBoardFWMap;
    fSettingsMap         = pTool->fSettingsMap;
    fFileHandler         = pTool->fFileHandler;
    fDirectoryName       = pTool->fDirectoryName;
    fResultFile          = pTool->fResultFile;
    fType                = pTool->fType;
    fCanvasMap           = pTool->fCanvasMap;
    fChipHistMap         = pTool->fChipHistMap;
    fModuleHistMap       = pTool->fModuleHistMap;
    fBeBoardHistMap      = pTool->fBeBoardHistMap;
    fTestGroupChannelMap = pTool->fTestGroupChannelMap;
    fNetworkStreamer     = pTool->fNetworkStreamer;
#ifdef __HTTP__
    fHttpServer          = pTool->fHttpServer;
#endif
}

void Tool::Inherit (SystemController* pSystemController)
{
	fDetectorContainer = pSystemController->fDetectorContainer; //IS THIS RIGHT?????? HERE WE ARE COPYING THE OBJECTS!!!!!
    fBeBoardInterface  = pSystemController->fBeBoardInterface;
    fChipInterface     = pSystemController->fChipInterface;
    fBoardVector       = pSystemController->fBoardVector;
    fBeBoardFWMap      = pSystemController->fBeBoardFWMap;
    fSettingsMap       = pSystemController->fSettingsMap;
    fFileHandler       = pSystemController->fFileHandler;
    fNetworkStreamer   = pSystemController->fNetworkStreamer;
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
    fChipHistMap.clear();
    fModuleHistMap.clear();
    fBeBoardHistMap.clear();
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
    fChipHistMap.clear();
    fModuleHistMap.clear();
    fBeBoardHistMap.clear();
    fTestGroupChannelMap.clear();
}

void Tool::bookHistogram ( Chip* pChip, std::string pName, TObject* pObject )
{
    // find or create map<string,TOBject> for specific CBC
    auto cChipHistMap = fChipHistMap.find ( pChip );

    if ( cChipHistMap == std::end ( fChipHistMap ) )
    {
        //Fabio: CBC specific -> to be moved out from Tool
        LOG (INFO) << "Histo Map for CBC " << int ( pChip->getChipId() ) <<  " (FE " << int ( pChip->getFeId() ) << ") does not exist - creating " ;
        std::map<std::string, TObject*> cTempChipMap;

        fChipHistMap[pChip] = cTempChipMap;
        cChipHistMap = fChipHistMap.find ( pChip );
    }

    // find histogram with given name: if it exists, delete the object, if not create
    auto cHisto = cChipHistMap->second.find ( pName );

    if ( cHisto != std::end ( cChipHistMap->second ) ) cChipHistMap->second.erase ( cHisto );

    cChipHistMap->second[pName] = pObject;
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

void Tool::bookHistogram ( BeBoard* pBeBoard, std::string pName, TObject* pObject )
{
    // find or create map<string,TOBject> for specific CBC
    auto cBeBoardHistMap = fBeBoardHistMap.find ( pBeBoard );

    if ( cBeBoardHistMap == std::end ( fBeBoardHistMap ) )
    {
        LOG (INFO) << "Histo Map for Module " << int ( pBeBoard->getBeId() ) << " does not exist - creating " ;
        std::map<std::string, TObject*> cTempModuleMap;

        fBeBoardHistMap[pBeBoard] = cTempModuleMap;
        cBeBoardHistMap = fBeBoardHistMap.find ( pBeBoard );
    }

    // find histogram with given name: if it exists, delete the object, if not create
    auto cHisto = cBeBoardHistMap->second.find ( pName );

    if ( cHisto != std::end ( cBeBoardHistMap->second ) ) cBeBoardHistMap->second.erase ( cHisto );

    cBeBoardHistMap->second[pName] = pObject;
#ifdef __HTTP__

    if (fHttpServer) fHttpServer->Register ("/Histograms", pObject);

#endif
}

TObject* Tool::getHist ( Chip* pChip, std::string pName )
{
    auto cChipHistMap = fChipHistMap.find ( pChip );

    if ( cChipHistMap == std::end ( fChipHistMap ) )
    {
        //Fabio: CBC specific -> to be moved out from Tool
        LOG (ERROR) << RED << "Error: could not find the Histograms for CBC " << int ( pChip->getChipId() ) <<  " (FE " << int ( pChip->getFeId() ) << ")" << RESET ;
        return nullptr;
    }
    else
    {
        auto cHisto = cChipHistMap->second.find ( pName );

        if ( cHisto == std::end ( cChipHistMap->second ) )
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

TObject* Tool::getHist ( BeBoard* pBeBoard, std::string pName )
{
    auto cBeBoardHistMap = fBeBoardHistMap.find ( pBeBoard );

    if ( cBeBoardHistMap == std::end ( fBeBoardHistMap ) )
    {
        LOG (ERROR) << RED << "Error: could not find the Histograms for Module " << int ( pBeBoard->getBeId() ) << RESET ;
        return nullptr;
    }
    else
    {
        auto cHisto = cBeBoardHistMap->second.find ( pName );

        if ( cHisto == std::end ( cBeBoardHistMap->second ) )
        {
            LOG (ERROR) << RED << "Error: could not find the Histogram with the name " << pName << RESET ;
            return nullptr;
        }
        else return cHisto->second;
    }
}

void Tool::SaveResults()
{
    for ( const auto& cBeBoard : fBeBoardHistMap )
    {

        fResultFile->cd();

        for ( const auto& cHist : cBeBoard.second )
            cHist.second->Write ( cHist.second->GetName(), TObject::kOverwrite );

        fResultFile->cd();
    }

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

    for ( const auto& cChip : fChipHistMap )
    {
        //Fabio: CBC specific -> to be moved out from Tool
        TString cDirName = Form ( "FE%dCBC%d", cChip.first->getFeId(), cChip.first->getChipId() );
        TObject* cObj = gROOT->FindObject ( cDirName );

        //if ( cObj ) delete cObj;

        if (!cObj) fResultFile->mkdir ( cDirName );

        fResultFile->cd ( cDirName );

        for ( const auto& cHist : cChip.second )
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
    //Fabio: CBC specific -> to be moved out from Tool - BEGIN
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
    //Fabio: CBC specific -> to be moved out from Tool - END

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
    // visitor to call dumpRegFile on each Chip
    struct RegMapDumper : public HwDescriptionVisitor
    {
        std::string fDirectoryName;
        RegMapDumper ( std::string pDirectoryName ) : fDirectoryName ( pDirectoryName ) {};
        void visit ( Chip& pChip )
        {
            if ( !fDirectoryName.empty() )
            {
                //Fabio: CBC specific -> to be moved out from Tool
                TString cFilename = fDirectoryName + Form ( "/FE%dCBC%d.txt", pChip.getFeId(), pChip.getChipId() );
                // cFilename += Form( "/FE%dCBC%d.txt", pChip.getFeId(), pChip.getChipId() );
                pChip.saveRegMap ( cFilename.Data() );
            }
            else LOG (INFO) << "Error: no results Directory initialized! "  ;
        }
    };

    RegMapDumper cDumper ( fDirectoryName );
    accept ( cDumper );

    LOG (INFO) << BOLDBLUE << "Configfiles for all Chips written to " << fDirectoryName << RESET ;
}


void Tool::setSystemTestPulse ( uint8_t pTPAmplitude, uint8_t pTestGroup, bool pTPState, bool pHoleMode )
{

    for (auto cBoard : this->fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cChip : cFe->fChipVector)
            {
                //Fabio: CBC specific but not used by common scans - BEGIN
                //first, get the Amux Value
                uint8_t cOriginalAmuxValue;
                cOriginalAmuxValue = cChip->getReg ("MiscTestPulseCtrl&AnalogMux" );
                //uint8_t cOriginalHitDetectSLVSValue = cChip->getReg ("HitDetectSLVS" );

                std::vector<std::pair<std::string, uint16_t>> cRegVec;
                uint8_t cRegValue =  to_reg ( 0, pTestGroup );

                if (cChip->getFrontEndType() == FrontEndType::CBC3)
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

                this->fChipInterface->WriteChipMultReg (cChip, cRegVec);
                //Fabio: CBC specific but not used by common scans - END

            }
        }
    }
}

void Tool::enableTestPulse(bool enableTP)
{

    for (auto cBoard : this->fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            for (auto cChip : cFe->fChipVector)
            {
                switch(cChip->getFrontEndType())
                {
                    case FrontEndType::CBC3 :
                    {
                        uint8_t cOriginalAmuxValue;
                        cOriginalAmuxValue = cChip->getReg ("MiscTestPulseCtrl&AnalogMux" );
                        
                        uint8_t cTPRegValue;

                        if (enableTP) cTPRegValue  = (cOriginalAmuxValue |  0x1 << 6);
                        else cTPRegValue = (cOriginalAmuxValue & ~ (0x1 << 6) );

                        this->fChipInterface->WriteChipReg ( cChip, "MiscTestPulseCtrl&AnalogMux",  cTPRegValue );
                        break;
                    }

                    default :
                    {
                        LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized for Bebord "<< 
                            cBoard->getBeId() << " Module " << cFe->getModuleId() << " Chip " << cChip->getChipId() << ", aborting" << RESET;
                        throw ("[Tool::enableTestPulse]\tError, FrontEnd type not found");
                        break;
                    }
                }
                
            }
        }
    }

    return;
}


void Tool::selectGroupTestPulse(Chip* cChip, uint8_t pTestGroup)
{
    
    switch(cChip->getFrontEndType())
    {
        case FrontEndType::CBC3 :
        {
            uint8_t cRegValue =  to_reg ( 0, pTestGroup );
            this->fChipInterface->WriteChipReg ( cChip, "TestPulseDel&ChanGroup",  cRegValue );
            break;
        }

        default :
        {
            LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized for Bebord "<< 
                cChip->getBeId() << " Module " << cChip->getFeId() << " Chip " << cChip->getChipId() << ", aborting" << RESET;
            throw ("[Tool::selectGroupTestPulse]\tError, FrontEnd type not found");
            break;
        }
    }
    
    return;
}

void Tool::setFWTestPulse()
{
    for (auto& cBoard : fBoardVector)
    {
        std::vector<std::pair<std::string, uint32_t> > cRegVec;
        
        switch(cBoard->getBoardType())
        {
            case BoardType::D19C :
            {
                cRegVec.push_back ({"fc7_daq_cnfg.fast_command_block.trigger_source", 6});
                cRegVec.push_back ({"fc7_daq_ctrl.fast_command_block.control.load_config", 0x1});
                break;
            }
            
            default :
            {
                LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " BeBoard type not recognized for Bebord "<< cBoard->getBeId() << ", aborting" << RESET;
                throw ("[Tool::setFWTestPulse]\tError, BeBoard type not found");
                break;
            }

        }

        fBeBoardInterface->WriteBoardMultReg (cBoard, cRegVec);
    }
}

void Tool::MakeTestGroups (FrontEndType theFrontEndType)
{
    switch(theFrontEndType)
    {
        case FrontEndType::CBC3 :
        {
            for ( int cGId = 0; cGId < 8; cGId++ )
            {
                std::vector<uint8_t> tempchannelVec;
                std::vector<uint8_t> tempGroupMaskVec(32,0x00);

                for ( int idx = 0; idx < 16; idx++ )
                {
                    int ctemp1 = idx * 16 + cGId * 2;
                    int ctemp2 = ctemp1 + 1;

                    if ( ctemp1 < 254 ){
                        tempchannelVec.push_back ( ctemp1 );
                        tempGroupMaskVec[ctemp1>>3] |= (1<<(ctemp1 & 0x7));
                    }

                    if ( ctemp2 < 254 ){
                        tempchannelVec.push_back ( ctemp2 );
                        tempGroupMaskVec[ctemp2>>3] |= (1<<(ctemp2 & 0x7));
                    }

                }

                fTestGroupChannelMap[cGId] = tempchannelVec;
                fMaskForTestGroupChannelMap[cGId] = tempGroupMaskVec;

            }

            int cGId = -1;
            std::vector<uint8_t> tempchannelVec;

            for ( int idx = 0; idx < 254; idx++ )
                tempchannelVec.push_back ( idx );
            
            fTestGroupChannelMap[cGId] = tempchannelVec;
            break;
        }

        default :
        {
            LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized, aborting" << RESET;
            throw ("[Tool::MakeTestGroups]\tError, BeBoard type not found");
            break;
        }

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
std::map<uint8_t, double> Tool::decodeBendLUT(Chip* pChip)
{
    //Fabio: CBC specific but not used by common scans - BEGIN

    std::map<uint8_t, double> cLUT;

    double cBend=-7.0; 
    LOG (DEBUG) << BOLDGREEN << "Decoding bend LUT for CBC" << +pChip->getChipId() << " ." << RESET; 
    for( int i = 0 ; i <= 14 ; i++ )
    {
        TString cRegName = Form ( "Bend%d", i  );
        uint8_t cRegValue = fChipInterface->ReadChipReg (pChip, cRegName.Data() ); 
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
    //Fabio: CBC specific but not used by common scans - END

}


//method to mask a channel list
void Tool::maskChannelFromOtherGroups (Chip* pChip, int pTestGroup){

    std::vector<uint8_t> chipMask;
    bool chipHasMaskedChannels = pChip->hasMaskedChannels();
    if(chipHasMaskedChannels) chipMask = pChip->getChipMask();
    const std::vector<uint8_t> &groupMask = fMaskForTestGroupChannelMap[pTestGroup];

    RegisterVector cRegVec; 
    cRegVec.clear(); 
    
    switch(pChip->getFrontEndType())
    {
        case FrontEndType::CBC3 :
        {   
            for(uint8_t i=0; i<chipMask.size(); ++i){
                if(chipHasMaskedChannels) cRegVec.push_back ( {fChannelMaskMapCBC3[i], chipMask[i] & groupMask[i] } );
                else cRegVec.push_back ( {fChannelMaskMapCBC3[i], groupMask[i] } );
            }
            break;
        }

        default :
        {
            LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " FrontEnd type not recognized for Bebord "<< 
                pChip->getBeId() << " Module " << pChip->getFeId() << " Chip " << pChip->getChipId() << ", aborting" << RESET;
            throw ("[Tool::SetMaskAllChannels]\tError, FrontEnd type not found");
            break;
        }
    }

    fChipInterface->WriteChipMultReg ( pChip , cRegVec );

    return;
}



// then a method to un-mask pairs of channels on a given CBC
void Tool::unmaskPair(Chip* cChip ,  std::pair<uint8_t,uint8_t> pPair)
{
    //Fabio: CBC specific but not used by common scans - BEGIN

    FrontEndType cFrontEndType; 
    for ( BeBoard* pBoard : fBoardVector )
    {
        for (auto cFe : pBoard->fModuleVector)
        {
            cFrontEndType = cFe->getFrontEndType();
        }
    }

    // get ready to mask/un-mask channels in pairs... 
    MaskedChannelsList cMaskedList; 
    MaskedChannels cMaskedChannels; cMaskedChannels.clear(); cMaskedChannels.push_back(pPair.first);
    
    uint8_t cRegisterIndex = pPair.first >> 3;
    std::string cMaskRegName = (cFrontEndType == FrontEndType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
    cMaskedList.insert ( std::pair<std::string , MaskedChannels>(cMaskRegName.c_str()  ,cMaskedChannels ) );
    
    cRegisterIndex = pPair.second >> 3;
    cMaskRegName = (cFrontEndType == FrontEndType::CBC2) ? fChannelMaskMapCBC2[cRegisterIndex] : fChannelMaskMapCBC3[cRegisterIndex];
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
        uint8_t cRegValue = 0; //cChip->getReg (cMasked.first);
        std::string cOutput = "";  
        for(auto cMaskedChannel : cMasked.second )
        {
            uint8_t cBitShift = (cMaskedChannel) & 0x7;
            cRegValue |=  (1 << cBitShift);
            std::string cChType =  ( (+cMaskedChannel & 0x1) == 0 ) ? "seed" : "correlation"; 
            TString cOut; cOut.Form("Channel %d in the %s layer\t", (int)cMaskedChannel, cChType.c_str() ); 
            cOutput += cOut.Data(); 
        }
        //LOG (INFO) << GREEN << "\t Writing " << std::bitset<8> (cRegValue) <<  " to " << cMasked.first << " to UNMASK channels for stub sweep : " << cOutput.c_str() << RESET ; 
        fChipInterface->WriteChipReg ( cChip, cMasked.first ,  cRegValue  );
    }
    //Fabio: CBC specific but not used by common scans - END

}


// Two dimensional dac scan
void Tool::scanDacDac(const std::string &dac1Name, const std::vector<uint16_t> &dac1List, const std::string &dac2Name, const std::vector<uint16_t> &dac2List, const uint16_t &numberOfEvents, std::map<uint16_t, std::map<uint16_t, std::map<uint16_t, ModuleOccupancyPerChannelMap> > > &backEndOccupancyPerChannelMap, std::map<uint16_t, std::map<uint16_t, std::map<uint16_t, ModuleGlobalOccupancyMap > > > &backEndChipOccupanyMap)
{

    for (auto& cBoard : fBoardVector)
    {
        std::map<uint16_t, std::map<uint16_t, ModuleOccupancyPerChannelMap> > moduleOccupancyPerChannelMap;
        std::map<uint16_t, std::map<uint16_t, ModuleGlobalOccupancyMap > > moduleGlobalOccupancyMap;
        
        scanBeBoardDacDac(cBoard, dac1Name, dac1List, dac2Name, dac2List, numberOfEvents, moduleOccupancyPerChannelMap, moduleGlobalOccupancyMap);

        backEndOccupancyPerChannelMap[cBoard->getBeId()] = moduleOccupancyPerChannelMap;
        backEndChipOccupanyMap[cBoard->getBeId()] = moduleGlobalOccupancyMap;
    }

    return;
}


// Two dimensional dac scan per BeBoard
void Tool::scanBeBoardDacDac(BeBoard* pBoard, const std::string &dac1Name, const std::vector<uint16_t> &dac1List, const std::string &dac2Name, const std::vector<uint16_t> &dac2List, const uint16_t &numberOfEvents, std::map<uint16_t, std::map<uint16_t, ModuleOccupancyPerChannelMap> > &moduleOccupancyPerChannelMap, std::map<uint16_t, std::map<uint16_t, ModuleGlobalOccupancyMap > > &backEndChipOccupanyMap)
{

    for(const auto & dac1Value : dac1List){
        std::map<uint16_t, ModuleOccupancyPerChannelMap> moduleOccupancyPerChannelDac1Map;
        std::map<uint16_t, ModuleGlobalOccupancyMap > moduleChipOccupanyDac1Map;
        setSameDacBeBoard(pBoard, dac1Name, dac1Value);

        scanBeBoardDac(pBoard, dac2Name, dac2List, numberOfEvents, moduleOccupancyPerChannelDac1Map, moduleChipOccupanyDac1Map);

        moduleOccupancyPerChannelMap[dac1Value] = moduleOccupancyPerChannelDac1Map;
        backEndChipOccupanyMap[dac1Value] = moduleChipOccupanyDac1Map;
    }

    return;
}


// One dimensional dac scan
void Tool::scanDac(const std::string &dacName, const std::vector<uint16_t> &dacList, const uint16_t &numberOfEvents, std::map<uint16_t, std::map<uint16_t, ModuleOccupancyPerChannelMap> > &backEndOccupancyPerChannelMap, std::map<uint16_t, std::map<uint16_t, ModuleGlobalOccupancyMap > > &backEndChipOccupanyMap)
{

    for (auto& cBoard : fBoardVector)
    {
        std::map<uint16_t, ModuleOccupancyPerChannelMap> moduleOccupancyPerChannelMap;
        std::map<uint16_t, ModuleGlobalOccupancyMap > moduleChipOccupanyMap;

        scanBeBoardDac(cBoard, dacName, dacList, numberOfEvents, moduleOccupancyPerChannelMap, moduleChipOccupanyMap);

        backEndOccupancyPerChannelMap[cBoard->getBeId()] = moduleOccupancyPerChannelMap;
        backEndChipOccupanyMap[cBoard->getBeId()] = moduleChipOccupanyMap;

    }

    return;
}


// One dimensional dac scan per BeBoard
void Tool::scanBeBoardDac(BeBoard* pBoard, const std::string &dacName, const std::vector<uint16_t> &dacList, const uint16_t &numberOfEvents, std::map<uint16_t, ModuleOccupancyPerChannelMap> &moduleOccupancyPerChannelMap, std::map<uint16_t, ModuleGlobalOccupancyMap > &moduleChipOccupanyMap)
{

    for(const auto & dacValue : dacList){
        float globalOccupancy=0;
        ModuleOccupancyPerChannelMap moduleOccupancyDacMap;
        ModuleGlobalOccupancyMap moduleGlobalOccupancyMap;

        setDacAndMeasureBeBoardOccupancy(pBoard, dacName, dacValue, numberOfEvents, moduleOccupancyDacMap, moduleGlobalOccupancyMap, globalOccupancy);
        
        moduleOccupancyPerChannelMap[dacValue] = moduleOccupancyDacMap;
        moduleChipOccupanyMap[dacValue] = moduleGlobalOccupancyMap;
    }

    return;
}


// bit wise scan
void Tool::bitWiseScan(const std::string &dacName, const uint16_t &numberOfEvents, const float &targetOccupancy, bool isOccupancyTheMaximumAccepted, std::map<uint16_t, ModuleOccupancyPerChannelMap> &backEndOccupanyPerChannelAtTargetMap, std::map<uint16_t, ModuleGlobalOccupancyMap> &backEndOccupanyAtTargetMap)
{

    for (auto& cBoard : fBoardVector)
    {
        ModuleOccupancyPerChannelMap moduleOccupancyPerChannelMap;
        ModuleGlobalOccupancyMap moduleOccupancyMap;
        bitWiseScanBeBoard(cBoard, dacName, numberOfEvents, targetOccupancy, isOccupancyTheMaximumAccepted, moduleOccupancyPerChannelMap, moduleOccupancyMap);
        backEndOccupanyPerChannelAtTargetMap[cBoard->getBeId()] = moduleOccupancyPerChannelMap;
        backEndOccupanyAtTargetMap[cBoard->getBeId()] = moduleOccupancyMap;
    }

    return;

}


// bit wise scan per BeBoard
void Tool::bitWiseScanBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t &numberOfEvents, const float &targetOccupancy, bool &isOccupancyTheMaximumAccepted, ModuleOccupancyPerChannelMap &moduleOccupancyPerChannelMap, ModuleGlobalOccupancyMap &moduleOccupancyMap)
{

    if(!isOccupancyTheMaximumAccepted){
        LOG (INFO) << BOLDRED << "Tool::bitWiseScanBeBoard: isOccupancyTheMaximumAccepted = false not supported, using true" << RESET;
        isOccupancyTheMaximumAccepted=true;
    }

    float globalOccupancy = 0.;
    Chip *cChip = pBoard->fModuleVector.at(0)->fChipVector.at(0); //assumption: one BeBoard has only one type of chip;
    
    bool localDAC = cChip->isDACLocal(dacName);
    uint8_t numberOfBits = cChip->getNumberOfBits(dacName);
    bool occupanyDirectlyProportionalToDAC;

    //Maps for local DAC scans
    ModuleOccupancyPerChannelMap moduleOccupancyPerChannelMapPreviousStep;
    ModuleOccupancyPerChannelMap moduleOccupancyPerChannelMapCurrentStep;
    std::map<uint8_t, std::map<uint8_t, std::vector<uint16_t> > > previousLocalDacListPerBoard;
    std::map<uint8_t, std::map<uint8_t, std::vector<uint16_t> > > currentLocalDacListPerBoard;
    
    //Maps for global DAC scans
    ModuleGlobalOccupancyMap moduleOccupancyMapPreviousStep;
    ModuleGlobalOccupancyMap moduleOccupancyMapCurrentStep;
    std::map<uint8_t, std::map<uint8_t, uint16_t> > previousGlobalDacListPerBoard;
    std::map<uint8_t, std::map<uint8_t, uint16_t> > currentGlobalDacListPerBoard;


    float globalOccupancyPreviousStep;
    float globalOccupancyCurrentStep;

 
    // Starting point: all bits to 0
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cChip : cFe->fChipVector )
        {
            if(localDAC)
            {
                previousLocalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = std::vector<uint16_t>(cChip->getNumberOfChannels(),0);
            }
            else
            {
                previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = 0;
            }
        }
    }
    

    if(localDAC) setAllLocalDacBeBoard(pBoard, dacName, previousLocalDacListPerBoard);
    else setGlobalDacBeBoard(pBoard, dacName, previousGlobalDacListPerBoard);

    measureBeBoardOccupancy(pBoard, numberOfEvents, moduleOccupancyPerChannelMapPreviousStep, moduleOccupancyMapPreviousStep, globalOccupancyPreviousStep);
    



    //Determine Occupancy vs DAC proportionality setting all bits to 1
    // if(localDAC)
    // {
    //     setSameLocalDacBeBoard(pBoard, dacName, (0xFFFF>>(16-numberOfBits)) );//trick to set n bits to 1 without using power of 2
    // }
    // else{
    //     setSameGlobalDacBeBoard(pBoard, dacName, (0xFFFF>>(16-numberOfBits)) );//trick to set n bits to 1 without using power of 2
    // }


    // Starting point: all bits to 0
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cChip : cFe->fChipVector )
        {
            if(localDAC)
            {
                currentLocalDacListPerBoard [cFe->getModuleId()][cChip->getChipId()] = std::vector<uint16_t>(cChip->getNumberOfChannels(),0xFFFF>>(16-numberOfBits)); //trick to set n bits to 1 without using power of 2
            }
            else
            {
                currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = 0xFFFF>>(16-numberOfBits); //trick to set n bits to 1 without using power of 2
            }
        }
    }
    
    if(localDAC) setAllLocalDacBeBoard(pBoard, dacName, currentLocalDacListPerBoard);
    else setGlobalDacBeBoard(pBoard, dacName, currentGlobalDacListPerBoard);

    measureBeBoardOccupancy(pBoard, numberOfEvents, moduleOccupancyPerChannelMapCurrentStep, moduleOccupancyMapCurrentStep, globalOccupancyCurrentStep);

    occupanyDirectlyProportionalToDAC = globalOccupancyCurrentStep > globalOccupancyPreviousStep;
    
    if(!occupanyDirectlyProportionalToDAC)
    {
        if(localDAC) previousLocalDacListPerBoard = currentLocalDacListPerBoard;
        else previousGlobalDacListPerBoard = currentGlobalDacListPerBoard;
    }


    for(int iBit = numberOfBits-1; iBit>=0; --iBit)
    {
        // std::cout<<iBit<<std::endl;

        for ( auto cFe : pBoard->fModuleVector )
        {
            for ( auto cChip : cFe->fChipVector )
            {
                if(localDAC)
                {
                    for(uint8_t iChannel=0; iChannel<cChip->getNumberOfChannels(); ++iChannel)
                    {
                        if(occupanyDirectlyProportionalToDAC) currentLocalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()][iChannel] = previousLocalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()][iChannel] + (1<<iBit);
                        else currentLocalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()][iChannel] = previousLocalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()][iChannel] & (0xFFFF - (1<<iBit));
                    }
                }
                else
                {
                    if(occupanyDirectlyProportionalToDAC) currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] + (1<<iBit);
                    else currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] & (0xFFFF - (1<<iBit));
                }
            }
        }

        if(localDAC) setAllLocalDacBeBoard(pBoard, dacName, currentLocalDacListPerBoard);
        else setGlobalDacBeBoard(pBoard, dacName, currentGlobalDacListPerBoard);

        measureBeBoardOccupancy(pBoard, numberOfEvents, moduleOccupancyPerChannelMapCurrentStep, moduleOccupancyMapCurrentStep, globalOccupancyCurrentStep);

        //Determine if it is better or not
        for ( auto cFe : pBoard->fModuleVector )
        {
            for ( auto cChip : cFe->fChipVector )
            {
                // std::cout<<"CBC "<<(int16_t)cChip->getChipId()<<std::endl;
                if(localDAC)
                {
                    // if(cChip->getChipId()==0) std::cout<<"Current DAC: "<<std::bitset<10>(currentLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId()).at(10))
                    //     <<" Current occupancy: "<<moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId()).at(10)
                    //     <<" Previous Occupancy: "<<moduleOccupancyPerChannelMapPreviousStep.at(cFe->getModuleId()).at(cChip->getChipId()).at(10);
                    for(uint8_t iChannel=0; iChannel<cChip->getNumberOfChannels(); ++iChannel)
                    {

                        
                        if(isOccupancyTheMaximumAccepted){
                            if( moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] <= targetOccupancy )
                            {
                                previousLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = currentLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                                moduleOccupancyPerChannelMapPreviousStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                            }
                            // if( occupanyDirectlyProportionalToDAC && moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] <= targetOccupancy)
                            // {
                            //     previousLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = currentLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                            //     moduleOccupancyPerChannelMapPreviousStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                            // }
                            // else if( !occupanyDirectlyProportionalToDAC && moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] >= targetOccupancy)
                            // {
                            //     previousLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = currentLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                            //     moduleOccupancyPerChannelMapPreviousStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                            // }
                        }
                        else{
                            if( abs( ( (moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] - targetOccupancy))/ (moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] - targetOccupancy) ) <= 1. 
                                && abs( (moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] - targetOccupancy)/
                                    ( (moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] - moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] ) / (0xFFFF>>(16-iBit)) ) ) < 1.  )
                            {
                                previousLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = currentLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                                moduleOccupancyPerChannelMapPreviousStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel] = moduleOccupancyPerChannelMapCurrentStep.at(cFe->getModuleId()).at(cChip->getChipId())[iChannel];
                            }
                        }

                    }
                    // if(cChip->getChipId()==0) std::cout<<" Chosen Occupancy: "<<moduleOccupancyPerChannelMapPreviousStep.at(cFe->getModuleId()).at(cChip->getChipId()).at(10)
                    //     <<" DAC: "<<std::bitset<10>(previousLocalDacListPerBoard.at(cFe->getModuleId()).at(cChip->getChipId()).at(10))<<std::endl;
                }
                else
                {
    
                    // if(cChip->getChipId()==0) std::cout<<"Current DAC: "<<std::bitset<10>(currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()])<<" Current occupancy: "<<moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()]<<" Previous Occupancy: "<<moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()];
                    if(isOccupancyTheMaximumAccepted){
                        if( moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()] <= targetOccupancy ){
                            previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()];
                            moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()] = moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()];
                        }
                        // if(occupanyDirectlyProportionalToDAC && moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()] <= targetOccupancy){
                        //     previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()];
                        //     moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()] = moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()];
                        // }
                        // else if(!occupanyDirectlyProportionalToDAC && moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()] >= targetOccupancy){
                        //     previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()];
                        //     moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()] = moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()];
                        // }
                    }
                    else{
                        if( abs( (moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()] - targetOccupancy)/(moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()] - targetOccupancy) ) <= 1. ){
                            previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()] = currentGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()];
                            moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()] = moduleOccupancyMapCurrentStep[cFe->getModuleId()][cChip->getChipId()];
                        }
                    }
                    // if(cChip->getChipId()==0) std::cout<<" Chosen Occupancy: "<<moduleOccupancyMapPreviousStep[cFe->getModuleId()][cChip->getChipId()]<<" DAC: "<<std::bitset<10>(previousGlobalDacListPerBoard[cFe->getModuleId()][cChip->getChipId()])<<std::endl;

                }
            }
        }
    }


    if(localDAC){
        setAllLocalDacBeBoard(pBoard, dacName, previousLocalDacListPerBoard);
    }
    else setGlobalDacBeBoard(pBoard, dacName, previousGlobalDacListPerBoard);
    
    
    measureBeBoardOccupancy(pBoard, numberOfEvents, moduleOccupancyPerChannelMap, moduleOccupancyMap, globalOccupancy);

    dumpConfigFiles();

    return;
}


// set dac and measure occupancy
void Tool::setDacAndMeasureOccupancy(const std::string &dacName, const uint16_t &dacValue, const uint16_t &numberOfEvents, std::map<uint16_t, ModuleOccupancyPerChannelMap> &backEndOccupancyPerChannelMap, std::map<uint16_t, ModuleGlobalOccupancyMap > &backEndChipOccupanyMap, float &globalOccupancy)
{

    float globalBeBoardOccupancy = 0.;
    for (auto& cBoard : fBoardVector)
    {
    	globalBeBoardOccupancy = 0.;
    	backEndOccupancyPerChannelMap[cBoard->getBeId()] = ModuleOccupancyPerChannelMap();
        backEndChipOccupanyMap       [cBoard->getBeId()] = ModuleGlobalOccupancyMap();
        setDacAndMeasureBeBoardOccupancy(cBoard, dacName, dacValue, numberOfEvents, backEndOccupancyPerChannelMap[cBoard->getBeId()], backEndChipOccupanyMap[cBoard->getBeId()], globalOccupancy);
        globalOccupancy += globalBeBoardOccupancy;
    }

    globalOccupancy/=fBoardVector.size();

    return;
}


// set dac and measure occupancy per BeBoard
void Tool::setDacAndMeasureBeBoardOccupancy(BeBoard* pBoard, const std::string &dacName, const uint16_t &dacValue, const uint16_t &numberOfEvents, ModuleOccupancyPerChannelMap &moduleOccupancyPerChannelMap, ModuleGlobalOccupancyMap &cbcOccupanyMap, float &globalOccupancy)
{

    setSameDacBeBoard(pBoard, dacName, dacValue);

    measureBeBoardOccupancy(pBoard, numberOfEvents, moduleOccupancyPerChannelMap, cbcOccupanyMap, globalOccupancy);

    return;
}

// measure occupancy
void Tool::measureOccupancy(const uint16_t &numberOfEvents, std::map<uint16_t, ModuleOccupancyPerChannelMap> &backEndOccupancyPerChannelMap, std::map<uint16_t, ModuleGlobalOccupancyMap > &backEndChipOccupanyMap, float &globalOccupancy)
{

    float globalBeBoardOccupancy = 0.;
    for (auto& cBoard : fBoardVector)
    {
    	globalBeBoardOccupancy = 0.;
        backEndOccupancyPerChannelMap[cBoard->getBeId()] = ModuleOccupancyPerChannelMap();
        backEndChipOccupanyMap       [cBoard->getBeId()] = ModuleGlobalOccupancyMap();
        measureBeBoardOccupancy(cBoard, numberOfEvents, backEndOccupancyPerChannelMap[cBoard->getBeId()], backEndChipOccupanyMap[cBoard->getBeId()], globalOccupancy);
        globalOccupancy += globalBeBoardOccupancy;

    }

    globalOccupancy /= fBoardVector.size();

    return;
}


// measure occupancy
void Tool::measureBeBoardOccupancy(BeBoard* pBoard, const uint16_t &numberOfEvents, ModuleOccupancyPerChannelMap &moduleOccupancyPerChannelMap, ModuleGlobalOccupancyMap &chipOccupanyMap, float &globalOccupancy)
{
    
    uint32_t normalization=0;
    uint32_t numberOfHits=0;

    if(!fAllChan)
    {
        for(const auto & group : fTestGroupChannelMap)
        {
            if(group.first == -1) continue;

            if(fMaskChannelsFromOtherGroups || fTestPulse)
            {
                for ( auto cFe : pBoard->fModuleVector )
                {
                    for ( auto cChip : cFe->fChipVector )
                    {
                        if(fMaskChannelsFromOtherGroups)
                        {
                            maskChannelFromOtherGroups (cChip, group.first);
                        }
                        if(fTestPulse)
                        {
                            selectGroupTestPulse(cChip, group.first); // check
                        }
                    }
                }
            }

            measureBeBoardOccupancyPerGroup(group.second, pBoard, numberOfEvents, moduleOccupancyPerChannelMap);      
        }

        if(fMaskChannelsFromOtherGroups)//re-enable all the channels and evaluate
        {
            for ( auto cFe : pBoard->fModuleVector )
            {
                for ( auto cChip : cFe->fChipVector )
                {
                    fChipInterface->ConfigureChipOriginalMask ( cChip );
                }
            }
        }
    }
    else
    {
        measureBeBoardOccupancyPerGroup(fTestGroupChannelMap[-1], pBoard, numberOfEvents, moduleOccupancyPerChannelMap);
    }

    //Evaluate module and BeBoard Occupancy
    for ( auto cFe : pBoard->fModuleVector )
    {

        ChipOccupancyPerChannelMap *chipChannelOccupancy = &moduleOccupancyPerChannelMap[cFe->getModuleId()];
        std::map<uint8_t,float> *chipNumberOfHitsMap = &chipOccupanyMap[cFe->getModuleId()];


        for ( auto cChip : cFe->fChipVector )
        {

            ChannelOccupancy *channelOccupancy = &chipChannelOccupancy->at(cChip->getChipId());

            (*chipNumberOfHitsMap)[cChip->getChipId()] =0;
            
            std::vector<uint8_t> chipMask;
            bool chipHasMaskedChannels = cChip->hasMaskedChannels();
            if(chipHasMaskedChannels) chipMask = cChip->getChipMask();

            for ( uint8_t cChan=0; cChan<cChip->getNumberOfChannels(); ++cChan)
            {
                if(fSkipMaskedChannels && chipHasMaskedChannels)
                {
                    if(cChip->IsChannelUnMasked(cChan))
                    {
                        (*chipNumberOfHitsMap)[cChip->getChipId()]+=channelOccupancy->at(cChan);
                    }
                }
                else
                {
                    (*chipNumberOfHitsMap)[cChip->getChipId()]+=channelOccupancy->at(cChan);
                }
                channelOccupancy->at(cChan)/=numberOfEvents;
            }

            uint32_t chipNormalization = 0;
            if(fSkipMaskedChannels && chipHasMaskedChannels)
            {
                for(const auto & mask : cChip->getChipMask())
                {
                    chipNormalization += mask;
                }
                chipNormalization *= numberOfEvents;
            }
            else
            {
                chipNormalization = numberOfEvents * cChip->getNumberOfChannels();
            }

            numberOfHits  += (*chipNumberOfHitsMap)[cChip->getChipId()];
            normalization += chipNormalization;
            (*chipNumberOfHitsMap)[cChip->getChipId()] /= chipNormalization;
        }
    }

    globalOccupancy = (float)numberOfHits/normalization;

    return;
}


// measure occupancy per group
void Tool::measureBeBoardOccupancyPerGroup(const std::vector<uint8_t> &cTestGrpChannelVec, BeBoard* pBoard, const uint16_t &numberOfEvents, ModuleOccupancyPerChannelMap &moduleOccupancyPerChannelMap)
{

    ReadNEvents ( pBoard, numberOfEvents );

    const std::vector<Event*>& events = GetEvents ( pBoard );

    // Loop over Events from this Acquisition
    for ( auto& ev : events )
    {

        for ( auto cFe : pBoard->fModuleVector )
        {

            if(moduleOccupancyPerChannelMap.find(cFe->getModuleId())==moduleOccupancyPerChannelMap.end())
            {
                moduleOccupancyPerChannelMap[cFe->getModuleId()]=ChipOccupancyPerChannelMap();
            }
            ChipOccupancyPerChannelMap *chipOccupancy = &moduleOccupancyPerChannelMap[cFe->getModuleId()];
 
            for ( auto cChip : cFe->fChipVector )
            {

                if(chipOccupancy->find(cChip->getChipId())==chipOccupancy->end()){
                    (*chipOccupancy)[cChip->getChipId()]=ChannelOccupancy(cChip->getNumberOfChannels(),0);
                }
                ChannelOccupancy *stripOccupancy = &chipOccupancy->at(cChip->getChipId());

                for ( auto& cChan : cTestGrpChannelVec )
                {

                    if ( ev->DataBit ( cFe->getFeId(), cChip->getChipId(), cChan) )
                    {
                        ++stripOccupancy->at(cChan);
                    }
                }
            }
        }
    }

    return;
}

// measure occupancy
void Tool::measureOccupancy(const uint16_t &numberOfEvents)
{
	for(unsigned int boardIndex=0; boardIndex<fDetectorContainer.size(); boardIndex++)
	{
        measureBeBoardOccupancy(boardIndex, numberOfEvents);
        fObjectStream->streamBoard(fDetectorDataContainer->at(boardIndex));
        //sendstuff
        fNetworkStreamer->sendMessage(fObjectStream->encodeStringStream());
	}
}
// measure occupancy
void Tool::measureBeBoardOccupancy(unsigned int boardIndex, const uint16_t numberOfEvents)
{

    uint32_t normalization=0;
    uint32_t numberOfHits=0;

    if(!fAllChan)
    {
        for(auto group : *fChannelGroupHandler)
        {
            
            if(fMaskChannelsFromOtherGroups || fTestPulse)
            {
                for ( auto cFe : *(fDetectorContainer.at(boardIndex)))
                {
                    for ( auto cChip : *cFe )
                    {
                        if(fMaskChannelsFromOtherGroups)
                        {
                            std::cout << __PRETTY_FUNCTION__ << " NOT IMPLEMENTED!!!";
                            // maskChannelFromOtherGroups (static_cast<Chip*>(cChip), group.first);//FIX MAYBE NO NEED TO STATIC CAST
                        }
                        if(fTestPulse)
                        {
                            std::cout << __PRETTY_FUNCTION__ << " NOT IMPLEMENTED!!!";
                            // selectGroupTestPulse(static_cast<Chip*>(cChip), group.first); // check
                        }
                    }
                }
            }

            measureBeBoardOccupancyPerGroup(boardIndex, numberOfEvents, group);
        }

        if(fMaskChannelsFromOtherGroups)//re-enable all the channels and evaluate
        {
            for ( auto cFe : *(fDetectorContainer.at(boardIndex)) )
            {
                for ( auto cChip : *cFe )
                {
                    fChipInterface->ConfigureChipOriginalMask ( static_cast<Chip*>(cChip) );
                }
            }
        }
    }
    else
    {
    	measureBeBoardOccupancyPerGroup(boardIndex, numberOfEvents, fChannelGroupHandler->allChannelGroup());
    }
/*
    //Evaluate module and BeBoard Occupancy
    for ( auto cFe : pBoard->fModuleVector )
    {

        ChipOccupancyPerChannelMap *chipChannelOccupancy = &moduleOccupancyPerChannelMap[cFe->getModuleId()];
        std::map<uint8_t,float> *chipNumberOfHitsMap = &chipOccupanyMap[cFe->getModuleId()];


        for ( auto cChip : cFe->fChipVector )
        {

            ChannelOccupancy *channelOccupancy = &chipChannelOccupancy->at(cChip->getChipId());

            (*chipNumberOfHitsMap)[cChip->getChipId()] =0;

            std::vector<uint8_t> chipMask;
            bool chipHasMaskedChannels = cChip->hasMaskedChannels();
            if(chipHasMaskedChannels) chipMask = cChip->getChipMask();

            for ( uint8_t cChan=0; cChan<cChip->getNumberOfChannels(); ++cChan)
            {
                if(fSkipMaskedChannels && chipHasMaskedChannels)
                {
                    if(cChip->IsChannelUnMasked(cChan))
                    {
                        (*chipNumberOfHitsMap)[cChip->getChipId()]+=channelOccupancy->at(cChan);
                    }
                }
                else
                {
                    (*chipNumberOfHitsMap)[cChip->getChipId()]+=channelOccupancy->at(cChan);
                }
                channelOccupancy->at(cChan)/=numberOfEvents;
            }

            uint32_t chipNormalization = 0;
            if(fSkipMaskedChannels && chipHasMaskedChannels)
            {
                for(const auto & mask : cChip->getChipMask())
                {
                    chipNormalization += mask;
                }
                chipNormalization *= numberOfEvents;
            }
            else
            {
                chipNormalization = numberOfEvents * cChip->getNumberOfChannels();
            }

            numberOfHits  += (*chipNumberOfHitsMap)[cChip->getChipId()];
            normalization += chipNormalization;
            (*chipNumberOfHitsMap)[cChip->getChipId()] /= chipNormalization;
        }
    }

    globalOccupancy = (float)numberOfHits/normalization;
*/
}

void Tool::measureBeBoardOccupancyPerGroup(unsigned int boardIndex, const uint16_t numberOfEvents, const ChannelGroupBase *cTestChannelGroup)
{
    ReadNEvents ( static_cast<BeBoard*>(fDetectorContainer.at(boardIndex)), numberOfEvents );
    // Loop over Events from this Acquisition
    const std::vector<Event*>& events = GetEvents ( static_cast<BeBoard*>(fDetectorContainer.at(boardIndex)) );
    for ( auto& event : events )
    	event->fillOccupancy((fDetectorDataContainer->at(boardIndex)), cTestChannelGroup);

}

//Set global DAC for all CBCs in the BeBoard
void Tool::setGlobalDacBeBoard(BeBoard* pBoard, const std::string &dacName, const std::map<uint8_t, std::map<uint8_t, uint16_t> > &dacList)
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cChip : cFe->fChipVector )
        {
            fChipInterface->WriteChipReg ( cChip, dacName, dacList.at(cFe->getModuleId()).at(cChip->getChipId()) );
        }
    }
    return;
}

//Set same global DAC for all CBCs
void Tool::setSameGlobalDac(const std::string &dacName, const uint16_t &dacValue){

    for (auto& cBoard : fBoardVector)
    {
        setSameGlobalDacBeBoard(cBoard, dacName, dacValue);
    }

    return;
}


//Set same global DAC for all CBCs in the BeBoard
void Tool::setSameGlobalDacBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t &dacValue)
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cChip : cFe->fChipVector )
        {
            fChipInterface->WriteChipReg ( cChip, dacName, dacValue );
        }
    }
    return;
}


// set local dac per BeBoard
void Tool::setAllLocalDacBeBoard(BeBoard* pBoard, const std::string &dacName, const std::map<uint8_t, std::map<uint8_t, std::vector<uint16_t> > > &dacList)
{   
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cChip : cFe->fChipVector )
        {
            std::vector<uint16_t> dacVector = dacList.at(cFe->getModuleId()).at(cChip->getChipId());
            fChipInterface->WriteChipAllLocalReg ( cChip, dacName, dacVector);
        }
    } 
    return;
}


// set same local dac for all BeBoard
void Tool::setSameLocalDac(const std::string &dacName, const uint16_t &dacValue)
{

    for (auto& cBoard : fBoardVector)
    {
        setSameLocalDacBeBoard(cBoard, dacName, dacValue);
    }

    return;
}


// set same local dac per BeBoard
void Tool::setSameLocalDacBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t &dacValue)
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        for ( auto cChip : cFe->fChipVector )
        {
            std::vector<uint16_t> dacVector(cChip->getNumberOfChannels(),dacValue);
            fChipInterface->WriteChipAllLocalReg ( cChip, dacName, dacVector);
        }
    } 
    return;
}

void Tool::setSameDacBeBoard(BeBoard* pBoard, const std::string &dacName, const uint16_t &dacValue)
{
    //Assumption: 1 BeBoard has only 1 chip flavor:
    if(pBoard->fModuleVector.at(0)->fChipVector.at(0)->isDACLocal(dacName))
    {
        setSameLocalDacBeBoard(pBoard, dacName, dacValue);
    }
    else
    {
        setSameGlobalDacBeBoard(pBoard, dacName, dacValue);
    }
}

void Tool::setSameDac(const std::string &dacName, const uint16_t &dacValue)
{

    for (auto& cBoard : fBoardVector)
    {
        setSameDacBeBoard(cBoard, dacName, dacValue);
    }

    return;

}

