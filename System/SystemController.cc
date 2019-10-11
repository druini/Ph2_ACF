/*

        FileName :                    SystemController.cc
        Content :                     Controller of the System, overall wrapper of the framework
        Programmer :                  Nicolas PIERRE
        Version :                     1.0
        Date of creation :            10/08/14
        Support :                     mail to : nicolas.pierre@cern.ch

 */

#include "SystemController.h"
#include "../HWInterface/CbcInterface.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace Ph2_System {

    SystemController::SystemController()
    : fBeBoardInterface    (nullptr)
    , fReadoutChipInterface(nullptr)
    , fChipInterface       (nullptr)
    , fDetectorContainer   (nullptr)
    , fBoardVector         ()
    , fSettingsMap         ()
    , fFileHandler         (nullptr)
    , fRawFileName         ("")
    , fWriteHandlerEnabled (false)
    , fStreamerEnabled     (false)
    , fNetworkStreamer     (nullptr)//This is the server listening port
    , fData                (nullptr)
    {
    //      bool fStreamData = true;
    //      if(fStreamData && !fNetworkStreamer->accept(10, 0))
    //      {
    //          std::cout << "NOBODY IS LISTENING FOR MY OCCUPANCY DATA!!!!!!!! CRASHING!" << std::endl;
    //          abort();
    //      }

    }

    SystemController::~SystemController()
    {
    }

    void SystemController::Inherit (SystemController* pController)
    {
        fBeBoardInterface = pController->fBeBoardInterface;
        fReadoutChipInterface = pController->fReadoutChipInterface;
        fChipInterface = pController->fChipInterface;
        fBoardVector = pController->fBoardVector;
        fBeBoardFWMap = pController->fBeBoardFWMap;
        fSettingsMap = pController->fSettingsMap;
        fFileHandler = pController->fFileHandler;
        fStreamerEnabled = pController->fStreamerEnabled;
        fNetworkStreamer = pController->fNetworkStreamer;
    }

    void SystemController::Destroy()
    {
        if (fFileHandler)
        {
            if (fFileHandler->file_open() ) fFileHandler->closeFile();

            if (fFileHandler!=nullptr) delete fFileHandler;
        }

        if (fBeBoardInterface!=nullptr) delete fBeBoardInterface;

        if (fReadoutChipInterface!=nullptr)  delete fReadoutChipInterface;
        if (fChipInterface!=nullptr)  delete fChipInterface;
        if (fMPAInterface!=nullptr)  delete fMPAInterface;
        if(fDetectorContainer!=nullptr) delete fDetectorContainer;

        // It crash if I try to delete them !!!!!!!!!!
        // for (auto& it : fBeBoardFWMap) {
        //     if (it.second)
        //         delete it.second;
        // }
        fBeBoardFWMap.clear();

        fSettingsMap.clear();
        if(fNetworkStreamer!=nullptr) delete fNetworkStreamer;
    // for ( auto& el : fBoardVector )
    //  if (el) delete el;

    // fBoardVector.clear();

        if (fData!=nullptr) delete fData;
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
        if (fData!=nullptr) delete fData;

        fData = new Data();

    //pass data by reference to set and let it know what board we are dealing with
        fData->Set (pBoard, pData, pNEvents, pBoard->getBoardType () );
    //return the packet size
    }

    void SystemController::InitializeHw ( const std::string& pFilename, std::ostream& os, bool pIsFile , bool streamData)
    {
        fStreamerEnabled = streamData;
        if(streamData)
        {
            fNetworkStreamer = new TCPPublishServer(6000,1);
        }
    // this->fParser.parseHW (pFilename, fBeBoardFWMap, fBoardVector, os, pIsFile );
        fDetectorContainer = new DetectorContainer;
        this->fParser.parseHW (pFilename, fBeBoardFWMap, fBoardVector, fDetectorContainer, os, pIsFile );

        fBeBoardInterface = new BeBoardInterface ( fBeBoardFWMap );
        if (fBoardVector[0]->getBoardType() != BoardType::FC7)
            fReadoutChipInterface = new CbcInterface  ( fBeBoardFWMap );
        else
            fReadoutChipInterface = new RD53Interface ( fBeBoardFWMap );
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
    LOG (INFO) << BOLDMAGENTA << "@@@ Configuring HW parsed from .xml file @@@" << RESET;

    for (auto& cBoard : fBoardVector)
      {
        // ######################################
        // # Configuring Outer Tracker hardware #
        // ######################################

        if (cBoard->getBoardType() != BoardType::FC7)
          {
            fBeBoardInterface->ConfigureBoard ( cBoard );

            LOG (INFO) << GREEN << "Successfully configured Board " << int ( cBoard->getBeId() ) << RESET;

            for (auto& cFe : cBoard->fModuleVector)
              {
                LOG (INFO) << "Configuring board.." << RESET;
                for (auto& cCbc : cFe->fReadoutChipVector)
                  {
                    if ( !bIgnoreI2c )
                      {
                        fReadoutChipInterface->ConfigureChip ( cCbc );
                        LOG (INFO) << GREEN <<  "Successfully configured Chip " << int ( cCbc->getChipId() ) << RESET;
                      }
                  }
              }
            fBeBoardInterface->ChipReSync ( cBoard );
            LOG (INFO) << BOLDGREEN << "Successfully sent resync." << RESET;
          }
        else
          {
            // ######################################
            // # Configuring Inner Tracker hardware #
            // ######################################
            LOG (INFO) << BOLDBLUE << "\t--> Found an Inner Tracker board" << RESET;
            LOG (INFO) << GREEN << "Configuring Board " << BOLDYELLOW << +cBoard->getBeId() << RESET;
            fBeBoardInterface->ConfigureBoard(cBoard);

            for (const auto& cModule : cBoard->fModuleVector)
              {
                LOG (INFO) << GREEN << "Initializing communication to Module " << BOLDYELLOW << +cModule->getModuleId() << RESET;
                for (const auto& cRD53 : cModule->fReadoutChipVector)
                  {
                    LOG (INFO) << GREEN << "Configuring RD53 " << BOLDYELLOW << +cRD53->getChipId() << RESET;
                    static_cast<RD53Interface*>(fReadoutChipInterface)->ConfigureChip(static_cast<RD53*>(cRD53));
                    LOG (INFO) << BOLDBLUE << "\t--> Number of masked pixels: " << BOLDYELLOW << static_cast<RD53*>(cRD53)->getNbMaskedPixels() << RESET;
                  }
              }

            LOG (INFO) << GREEN << "Checking status FW <---> RD53 communication" << RESET;
            bool commGood = static_cast<RD53FWInterface*>(this->fBeBoardFWMap[cBoard->getBeBoardId()])->InitChipCommunication();
            if (commGood == true) LOG (INFO) << BOLDBLUE << "\t--> Successfully initialized the communication to all chips"     << RESET;
            else                  LOG (INFO) << BOLDRED  << "\t--> I was not able to initialize the communication to all chips" << RESET;


            // ###################
            // # Configuring FSM #
            // ###################
            size_t nTRIGxEvent = SystemController::findValueInSettings("nTRIGxEvent");
            size_t injType     = SystemController::findValueInSettings("INJtype");
            static_cast<RD53FWInterface*>(this->fBeBoardFWMap[cBoard->getBeBoardId()])->SetAndConfigureFastCommands(cBoard, nTRIGxEvent, injType);
            LOG (INFO) << GREEN << "Configured FSM fast command block" << RESET;
          }
      }
  }


    void SystemController::initializeFileHandler()
    {
    // here would be the ideal position to fill the file Header and call openFile when in read mode
        for (const auto& cBoard : fBoardVector)
        {
            uint32_t cBeId = cBoard->getBeId();
            uint32_t cNChip = 0;

        //uint32_t cNFe = cBoard->getNFe();
            uint32_t cNEventSize32 = this->computeEventSize32 (cBoard);

            std::string cBoardTypeString;
            BoardType cBoardType = cBoard->getBoardType();

            for (const auto& cFe : cBoard->fModuleVector) cNChip += cFe->getNChip();

              if (cBoardType == BoardType::D19C)
                 cBoardTypeString = "D19C";
             else if (cBoardType == BoardType::FC7)
                 cBoardTypeString = "FC7";

             uint32_t cFWWord = fBeBoardInterface->getBoardInfo (cBoard);
             uint32_t cFWMajor = (cFWWord & 0xFFFF0000) >> 16;
             uint32_t cFWMinor = (cFWWord & 0x0000FFFF);

        //with the above info fill the header
             FileHeader cHeader (cBoardTypeString, cFWMajor, cFWMinor, cBeId, cNChip, cNEventSize32, cBoard->getEventType() );

        //construct a Handler
             std::stringstream cBeBoardString;
             cBeBoardString << "_Board" << std::setw (3) << std::setfill ('0') << cBeId;
             std::string cFilename = fRawFileName;

             if (fRawFileName.find (".raw") != std::string::npos)
                 cFilename.insert (fRawFileName.find (".raw"), cBeBoardString.str() );

             FileHandler* cHandler = new FileHandler (cFilename, 'w', cHeader);

        //finally set the handler
             fBeBoardInterface->SetFileHandler (cBoard, cHandler);
             LOG (INFO) << BOLDBLUE << "Saving binary raw data to: " << BOLDYELLOW << cFilename << RESET;
         }
     }
     uint32_t SystemController::computeEventSize32 (BeBoard* pBoard)
     {
        uint32_t cNEventSize32 = 0;
        uint32_t cNCbc = 0;

        for (const auto& cFe : pBoard->fModuleVector)
            cNCbc += cFe->getNChip();
        if (pBoard->getBoardType() == BoardType::D19C)
            cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32_CBC3 + cNCbc * D19C_EVENT_SIZE_32_CBC3;

        return cNEventSize32;
    }

    void SystemController::Start(int currentRun)
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

  void SystemController::ConfigureHardware(std::string cHWFile, bool enableStream)
  {
    std::stringstream outp;
    
    InitializeHw ( cHWFile, outp, true, enableStream );
    InitializeSettings ( cHWFile, outp );
    LOG (INFO) << outp.str();
    outp.str ("");
    ConfigureHw();
  }

    void SystemController::ConfigureCalibration()
    {
    }

    void SystemController::Configure(std::string cHWFile, bool enableStream)
    {
        ConfigureHardware(cHWFile, enableStream);
        ConfigureCalibration();
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

  double SystemController::findValueInSettings (const char* name)
  {
    auto setting = fSettingsMap.find(name);
    return ((setting != std::end(fSettingsMap)) ? setting->second : 0);
  }
}
