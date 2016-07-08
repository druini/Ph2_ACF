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

    SystemController::SystemController()
        : fFileHandler (nullptr)
    {
    }

    SystemController::~SystemController()
    {
    }
    void SystemController::Destroy()
    {
        if (fFileHandler)
        {
            if (fFileHandler->file_open() ) fFileHandler->closeFile();

            delete fFileHandler;
        }

        delete fBeBoardInterface;
        delete fCbcInterface;
        fBeBoardFWMap.clear();
        fSettingsMap.clear();

        for ( auto& el : fBoardVector )
            delete el;

        fBoardVector.clear();
    }

    void SystemController::addFileHandler ( const std::string& pFilename , char pOption )
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

    void SystemController::readFile ( std::vector<uint32_t>& pVec, uint32_t pNWords32 )
    {
        if (pNWords32 == 0) pVec = fFileHandler->readFile( );
        else pVec = fFileHandler->readFileChunks (pNWords32);
    }

    void SystemController::InitializeHw ( const std::string& pFilename, std::ostream& os )
    {
        this->fParser.parseHW (pFilename, fBeBoardFWMap, fBoardVector, os);

        fBeBoardInterface = new BeBoardInterface ( fBeBoardFWMap );
        fCbcInterface = new CbcInterface ( fBeBoardFWMap );

        if (fWriteHandlerEnabled)
            this->initializeFileHandler();
    }

    void SystemController::InitializeSettings ( const std::string& pFilename, std::ostream& os )
    {
        this->fParser.parseSettings (pFilename, fSettingsMap, os);
    }

    void SystemController::ConfigureHw ( std::ostream& os , bool bIgnoreI2c )
    {
        os << std::endl << BOLDBLUE << "Configuring HW parsed from .xml file: " << RESET << std::endl;

        bool cHoleMode, cCheck;

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
            fBeBoardInterface->ConfigureBoard ( cBoard );
            fBeBoardInterface->CbcHardReset ( cBoard );

            if ( cCheck && cBoard->getBoardType() == "GLIB")
            {
                fBeBoardInterface->WriteBoardReg ( cBoard, "pc_commands2.negative_logic_CBC", ( ( cHoleMode ) ? 0 : 1 ) );
                os << GREEN << "Overriding GLIB register values for signal polarity with value from settings node!" << RESET << std::endl;
            }

            os << GREEN << "Successfully configured Board " << int ( cBoard->getBeId() ) << RESET << std::endl;

            for (auto& cFe : cBoard->fModuleVector)
            {
                for (auto& cCbc : cFe->fCbcVector)
                {
                    if ( !bIgnoreI2c )
                    {
                        fCbcInterface->ConfigureCbc ( cCbc );
                        os << GREEN <<  "Successfully configured Cbc " << int ( cCbc->getCbcId() ) << RESET << std::endl;
                    }
                }
            }

            //CbcFastReset as per recommendation of Mark Raymond
            fBeBoardInterface->CbcFastReset ( cBoard );
        }
    }

    void SystemController::initializeFileHandler()
    {
        std::cout << BOLDBLUE << "Saving binary raw data to: " << fRawFileName << ".fedId" << RESET << std::endl;

        // here would be the ideal position to fill the file Header and call openFile when in read mode
        for (const auto& cBoard : fBoardVector)
        {
            std::string cBoardTypeString = cBoard->getBoardType();
            uint32_t cBeId = cBoard->getBeId();
            uint32_t cNCbc = 0;

            for (const auto& cFe : cBoard->fModuleVector)
                cNCbc += cFe->getNCbc();

            //nCbcDataSize is set to 0 if not explicitly set
            uint32_t cNEventSize32 = (cBoard->getNCbcDataSize() == 0 ) ? EVENT_HEADER_TDC_SIZE_32 + cNCbc * CBC_EVENT_SIZE_32 : EVENT_HEADER_TDC_SIZE_32 + cBoard->getNCbcDataSize() * CBC_EVENT_SIZE_32;

            uint32_t cFWWord = fBeBoardInterface->getBoardInfo (cBoard);
            uint32_t cFWMajor = (cFWWord & 0xFFFF0000) >> 16;
            uint32_t cFWMinor = (cFWWord & 0x0000FFFF);

            //with the above info fill the header
            FileHeader cHeader (cBoardTypeString, cFWMajor, cFWMinor, cBeId, cNCbc, cNEventSize32);

            //construct a Handler
            std::stringstream cBeBoardString;
            cBeBoardString << "_fed" << std::setw (3) << std::setfill ('0') << cBeId;
            std::string cFilename = fRawFileName;

            if (fRawFileName.find (".raw") != std::string::npos)
                cFilename.insert (fRawFileName.find (".raw"), cBeBoardString.str() );

            FileHandler* cHandler = new FileHandler (cFilename, 'w', cHeader);

            //finally set the handler
            fBeBoardInterface->SetFileHandler (cBoard, cHandler);
        }
    }
}
