/*

        FileName :                    SystemController.cc
        Content :                     Controller of the System, overall wrapper of the framework
        Programmer :                  Nicolas PIERRE
        Version :                     1.0
        Date of creation :            10/08/14
        Support :                     mail to : nicolas.pierre@cern.ch

 */

#include "SystemController.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace Ph2_System {

    SystemController::SystemController() :
        fBeBoardInterface (nullptr),
        fCbcInterface (nullptr),
        fBoardVector(),
        fSettingsMap(),
        fFileHandler (nullptr),
        fRawFileName (""),
        fWriteHandlerEnabled (false),
        fData (nullptr)
    {
    }

    SystemController::~SystemController()
    {
    }

    void SystemController::Inherit (SystemController* pController)
    {
        fBeBoardInterface = pController->fBeBoardInterface;
        fCbcInterface = pController->fCbcInterface;
        fBoardVector = pController->fBoardVector;
        fBeBoardFWMap = pController->fBeBoardFWMap;
        fSettingsMap = pController->fSettingsMap;
        fFileHandler = pController->fFileHandler;
    }

    void SystemController::Destroy()
    {
        if (fFileHandler)
        {
            if (fFileHandler->file_open() ) fFileHandler->closeFile();

            if (fFileHandler) delete fFileHandler;
        }

        if (fBeBoardInterface) delete fBeBoardInterface;

        if (fCbcInterface) delete fCbcInterface;

        fBeBoardFWMap.clear();
        fSettingsMap.clear();

        for ( auto& el : fBoardVector )
            if (el) delete el;

        fBoardVector.clear();

        if (fData) delete fData;
    }

    void SystemController::addFileHandler ( const std::string& pFilename, char pOption )
    {
        //if the opion is read, create a handler object and use it to read the
        //file in the method below!

        if (pOption == 'r')
            fFileHandler = new FileHandler ( pFilename, pOption );
        //if the option is w, remember the filename and construct a new
        //fileHandler for every Interface
        else if (pOption == 'w')
        {
            fRawFileName = pFilename;
            fWriteHandlerEnabled = true;
        }

    }

    void SystemController::closeFileHandler()
    {
        if (fFileHandler)
        {
            if (fFileHandler->file_open() ) fFileHandler->closeFile();

            if (fFileHandler) delete fFileHandler;

            fFileHandler = nullptr;
        }
    }

    void SystemController::readFile ( std::vector<uint32_t>& pVec, uint32_t pNWords32 )
    {
        if (pNWords32 == 0) pVec = fFileHandler->readFile( );
        else pVec = fFileHandler->readFileChunks (pNWords32);
    }

    void SystemController::setData (BeBoard* pBoard, std::vector<uint32_t>& pData, uint32_t pNEvents)
    {
        //reset the data object
        if (fData) delete fData;

        fData = new Data();

        //pass data by reference to set and let it know what board we are dealing with
        fData->Set (pBoard, pData, pNEvents, pBoard->getBoardType () );
        //return the packet size
    }

    void SystemController::InitializeHw ( const std::string& pFilename, std::ostream& os, bool pIsFile )
    {
        this->fParser.parseHW (pFilename, fBeBoardFWMap, fBoardVector, os, pIsFile );

        fBeBoardInterface = new BeBoardInterface ( fBeBoardFWMap );
        fCbcInterface = new CbcInterface ( fBeBoardFWMap );
        fMPAInterface = new MPAInterface ( fBeBoardFWMap );

        if (fWriteHandlerEnabled)
            this->initializeFileHandler();
    }

    void SystemController::InitializeSettings ( const std::string& pFilename, std::ostream& os, bool pIsFile )
    {
        this->fParser.parseSettings (pFilename, fSettingsMap, os, pIsFile );
    }

    void SystemController::ConfigureHw ( bool bIgnoreI2c )
    {
        LOG (INFO) << BOLDBLUE << "Configuring HW parsed from .xml file: " << RESET;

        bool cHoleMode = false;
        bool cCheck = false;

        if ( !fSettingsMap.empty() )
        {
            SettingsMap::iterator cSetting = fSettingsMap.find ( "HoleMode" );

            if ( cSetting != fSettingsMap.end() )
                cHoleMode = cSetting->second;

            cCheck = true;
        }
        else cCheck = false;

        for (auto& cBoard : fBoardVector)
        {
            //fBeBoardInterface->CbcHardReset ( cBoard );
            fBeBoardInterface->ConfigureBoard ( cBoard );
            //fBeBoardInterface->CbcFastReset ( cBoard );

            if ( cCheck && cBoard->getBoardType() == BoardType::GLIB)
            {
                fBeBoardInterface->WriteBoardReg ( cBoard, "pc_commands2.negative_logic_CBC", ( ( cHoleMode ) ? 0 : 1 ) );
                LOG (INFO) << GREEN << "Overriding GLIB register values for signal polarity with value from settings node!" << RESET;
            }

            LOG (INFO) << GREEN << "Successfully configured Board " << int ( cBoard->getBeId() ) << RESET;

            for (auto& cFe : cBoard->fModuleVector)
            {
                for (auto& cCbc : cFe->fCbcVector)
                {
                    if ( !bIgnoreI2c )
                    {
                        fCbcInterface->ConfigureCbc ( cCbc );
                        LOG (INFO) << GREEN <<  "Successfully configured Cbc " << int ( cCbc->getCbcId() ) << RESET;
                    }
                }
            }

            //CbcFastReset as per recommendation of Mark Raymond
            fBeBoardInterface->CbcFastReset ( cBoard );
        }
    }

    void SystemController::initializeFileHandler()
    {
        // here would be the ideal position to fill the file Header and call openFile when in read mode
        for (const auto& cBoard : fBoardVector)
        {
            uint32_t cBeId = cBoard->getBeId();
            uint32_t cNCbc = 0;

            //uint32_t cNFe = cBoard->getNFe();
            uint32_t cNEventSize32 = this->computeEventSize32 (cBoard);

            for (const auto& cFe : cBoard->fModuleVector)
                cNCbc += cFe->getNCbc();

            std::string cBoardTypeString;
            BoardType cBoardType = cBoard->getBoardType();

            if (cBoardType == BoardType::GLIB)
                cBoardTypeString = "GLIB";
            else if (cBoardType == BoardType::MPAGLIB)
                cBoardTypeString = "MPAGLIB";
            else if (cBoardType == BoardType::CTA)
                cBoardTypeString = "CTA";
            else if (cBoardType == BoardType::ICGLIB)
                cBoardTypeString = "ICGLIB";
            else if (cBoardType == BoardType::ICFC7)
                cBoardTypeString = "ICFC7";
            else if (cBoardType == BoardType::CBC3FC7)
                cBoardTypeString = "CBC3FC7";
            else if (cBoardType == BoardType::D19C)
                cBoardTypeString = "D19C";

            uint32_t cFWWord = fBeBoardInterface->getBoardInfo (cBoard);
            uint32_t cFWMajor = (cFWWord & 0xFFFF0000) >> 16;
            uint32_t cFWMinor = (cFWWord & 0x0000FFFF);

            //with the above info fill the header
            FileHeader cHeader (cBoardTypeString, cFWMajor, cFWMinor, cBeId, cNCbc, cNEventSize32, cBoard->getEventType() );

            //construct a Handler
            std::stringstream cBeBoardString;
            cBeBoardString << "_fed" << std::setw (3) << std::setfill ('0') << cBeId;
            std::string cFilename = fRawFileName;

            if (fRawFileName.find (".raw") != std::string::npos)
                cFilename.insert (fRawFileName.find (".raw"), cBeBoardString.str() );

            FileHandler* cHandler = new FileHandler (cFilename, 'w', cHeader);

            //finally set the handler
            fBeBoardInterface->SetFileHandler (cBoard, cHandler);
            LOG (INFO) << BOLDBLUE << "Saving binary raw data to: " << fRawFileName << ".fedId" << RESET ;
        }
    }
    uint32_t SystemController::computeEventSize32 (BeBoard* pBoard)
    {
        uint32_t cNEventSize32 = 0;
        uint32_t cNCbc = 0;
        uint32_t cNFe = pBoard->getNFe();

        for (const auto& cFe : pBoard->fModuleVector)
            cNCbc += cFe->getNCbc();

        if (pBoard->getBoardType() == BoardType::GLIB)
        {
            //this is legacy as the fNCbcDataSize is not used any more
            //cNEventSize32 = (cBoard->getNCbcDataSize() == 0 ) ? EVENT_HEADER_TDC_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32 : EVENT_HEADER_TDC_SIZE_32 + cBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32;
            if (cNCbc <= 4)
                cNEventSize32 = EVENT_HEADER_TDC_SIZE_32 + 4 * CBC_EVENT_SIZE_32;
            else if (cNCbc > 4 && cNCbc <= 8)
                cNEventSize32 = EVENT_HEADER_TDC_SIZE_32 + 8 * CBC_EVENT_SIZE_32;
            else if (cNCbc > 8 && cNCbc <= 16)
                cNEventSize32 = EVENT_HEADER_SIZE_32 + 16 * CBC_EVENT_SIZE_32;
        }

        if (pBoard->getBoardType() == BoardType::MPAGLIB)
            cNEventSize32 = MPA_HEADER_SIZE_32 + 6 * MPA_EVENT_SIZE_32;

        else if (pBoard->getBoardType() == BoardType::CTA)
        {
            //this is legacy as the fNCbcDataSize is not used any more
            //cNEventSize32 = (cBoard->getNCbcDataSize() == 0 ) ? EVENT_HEADER_TDC_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32 : EVENT_HEADER_TDC_SIZE_32 + cBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32;
            if (cNCbc <= 4)
                cNEventSize32 = EVENT_HEADER_TDC_SIZE_32 + 4 * CBC_EVENT_SIZE_32;
            else if (cNCbc > 4 && cNCbc <= 8)
                cNEventSize32 = EVENT_HEADER_TDC_SIZE_32 + 8 * CBC_EVENT_SIZE_32;
            else if (cNCbc > 8 && cNCbc <= 16)
                cNEventSize32 = EVENT_HEADER_SIZE_32 + 16 * CBC_EVENT_SIZE_32;
        }
        else if (pBoard->getBoardType() == BoardType::ICGLIB)
            cNEventSize32 = EVENT_HEADER_TDC_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32;
        else if (pBoard->getBoardType() == BoardType::ICFC7)
            cNEventSize32 = EVENT_HEADER_TDC_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32;
        else if (pBoard->getBoardType() == BoardType::CBC3FC7)
            cNEventSize32 = EVENT_HEADER_TDC_SIZE_32_CBC3 + cNCbc * CBC_EVENT_SIZE_32_CBC3;
        else if (pBoard->getBoardType() == BoardType::D19C)
            cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32_CBC3 + cNCbc * D19C_EVENT_SIZE_32_CBC3;

        return cNEventSize32;
    }

    void SystemController::Start()
    {
        for (auto& cBoard : fBoardVector)
            fBeBoardInterface->Start (cBoard);
    }
    void SystemController::Stop()
    {
        for (auto& cBoard : fBoardVector)
            fBeBoardInterface->Stop (cBoard);
    }
    void SystemController::Pause()
    {
        for (auto& cBoard : fBoardVector)
            fBeBoardInterface->Pause (cBoard);
    }
    void SystemController::Resume()
    {
        for (auto& cBoard : fBoardVector)
            fBeBoardInterface->Resume (cBoard);
    }

    void SystemController::Start (BeBoard* pBoard)
    {
        fBeBoardInterface->Start (pBoard);
    }
    void SystemController::Stop (BeBoard* pBoard)
    {
        fBeBoardInterface->Stop (pBoard);
    }
    void SystemController::Pause (BeBoard* pBoard)
    {
        fBeBoardInterface->Pause (pBoard);
    }
    void SystemController::Resume (BeBoard* pBoard)
    {
        fBeBoardInterface->Resume (pBoard);
    }

    //method to read data standalone
    uint32_t SystemController::ReadData (BeBoard* pBoard, bool pWait)
    {
        //reset the data object
        //if (fData) delete fData;

        //fData = new Data();

        //std::vector<uint32_t> cData;
        //read the data and get it by reference
        //uint32_t cNPackets = fBeBoardInterface->ReadData (pBoard, false, cData);
        //pass data by reference to set and let it know what board we are dealing with
        //fData->Set (pBoard, cData, cNPackets, fBeBoardInterface->getBoardType (pBoard) );
        //return the packet size
        //return cNPackets;
        std::vector<uint32_t> cData;
        return this->ReadData (pBoard, cData, pWait);
    }

    // for OTSDAQ
    uint32_t SystemController::ReadData (BeBoard* pBoard, std::vector<uint32_t>& pData, bool pWait)
    {
        //reset the data object
        if (fData) delete fData;

        fData = new Data();

        //read the data and get it by reference
        uint32_t cNPackets = fBeBoardInterface->ReadData (pBoard, false, pData, pWait);
        //pass data by reference to set and let it know what board we are dealing with
        fData->Set (pBoard, pData, cNPackets, fBeBoardInterface->getBoardType (pBoard) );
        //return the packet size
        return cNPackets;
    }

    void SystemController::ReadData (bool pWait)
    {
        for (auto cBoard : fBoardVector)
            this->ReadData (cBoard, pWait);
    }

    //standalone
    void SystemController::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents)
    {
        std::vector<uint32_t> cData;
        return this->ReadNEvents (pBoard, pNEvents, cData, true);
    }

    //for OTSDAQ
    void SystemController::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
    {
        //reset the data object
        if (fData) delete fData;

        fData = new Data();
        //read the data and get it by reference
        fBeBoardInterface->ReadNEvents (pBoard, pNEvents, pData, pWait);
        //pass data by reference to set and let it know what board we are dealing with
        fData->Set (pBoard, pData, pNEvents, fBeBoardInterface->getBoardType (pBoard) );
        //return the packet size
    }

    void SystemController::ReadNEvents (uint32_t pNEvents)
    {
        for (auto cBoard : fBoardVector)
            this->ReadNEvents (cBoard, pNEvents);
    }
}
