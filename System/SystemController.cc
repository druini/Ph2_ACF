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

namespace Ph2_System
{
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
    , fNetworkStreamer     (nullptr) // This is the server listening port
    // , fData                (nullptr)
  {}

  SystemController::~SystemController() {}

  void SystemController::Inherit (SystemController* pController)
  {
    fBeBoardInterface     = pController->fBeBoardInterface;
    fReadoutChipInterface = pController->fReadoutChipInterface;
    fChipInterface        = pController->fChipInterface;
    fBoardVector          = pController->fBoardVector;
    fBeBoardFWMap         = pController->fBeBoardFWMap;
    fSettingsMap          = pController->fSettingsMap;
    fFileHandler          = pController->fFileHandler;
    fStreamerEnabled      = pController->fStreamerEnabled;
    fNetworkStreamer      = pController->fNetworkStreamer;
  }

  void SystemController::Destroy()
  {
    this->closeFileHandler();

    delete fBeBoardInterface;
    fBeBoardInterface = nullptr;
    delete fReadoutChipInterface;
    fReadoutChipInterface = nullptr;
    delete fChipInterface;
    fChipInterface = nullptr;
    delete fMPAInterface;
    fMPAInterface = nullptr;
    delete fDetectorContainer;
    fDetectorContainer = nullptr;

    fBeBoardFWMap.clear();
    fSettingsMap.clear();

    delete fNetworkStreamer;
    fNetworkStreamer = nullptr;
    // delete fData;
    // fData = nullptr;
  }

  void SystemController::addFileHandler (const std::string& pFilename, char pOption)
  {
    if (pOption == 'r') fFileHandler = new FileHandler(pFilename, pOption);
    else if (pOption == 'w')
      {
        fRawFileName = pFilename;
        fWriteHandlerEnabled = true;
      }
  }

  void SystemController::closeFileHandler()
  {
    if (fFileHandler != nullptr)
      {
        if (fFileHandler->isFileOpen() == true) fFileHandler->closeFile();
        delete fFileHandler;
        fFileHandler = nullptr;
      }
  }

  void SystemController::readFile (std::vector<uint32_t>& pVec, uint32_t pNWords32)
  {
    if (pNWords32 == 0) pVec = fFileHandler->readFile();
    else pVec = fFileHandler->readFileChunks(pNWords32);
  }

  void SystemController::setData (BeBoard* pBoard, std::vector<uint32_t>& pData, uint32_t pNEvents)
  {
    this->DecodeData(pBoard, pData, pNEvents, pBoard->getBoardType());
  }

  void SystemController::InitializeHw (const std::string& pFilename, std::ostream& os, bool pIsFile , bool streamData)
  {
    fStreamerEnabled = streamData;
    if (streamData) fNetworkStreamer = new TCPPublishServer(6000,1);

    fDetectorContainer = new DetectorContainer;
    this->fParser.parseHW(pFilename, fBeBoardFWMap, fBoardVector, fDetectorContainer, os, pIsFile);

    fBeBoardInterface = new BeBoardInterface(fBeBoardFWMap);
    if (fBoardVector[0]->getBoardType() != BoardType::RD53)
      fReadoutChipInterface = new CbcInterface(fBeBoardFWMap);
    else
      fReadoutChipInterface = new RD53Interface(fBeBoardFWMap);
    fMPAInterface = new MPAInterface(fBeBoardFWMap);

    if (fWriteHandlerEnabled) this->initializeFileHandler();
  }

  void SystemController::InitializeSettings (const std::string& pFilename, std::ostream& os, bool pIsFile)
  {
    this->fParser.parseSettings(pFilename, fSettingsMap, os, pIsFile);
  }

  void SystemController::ConfigureHw (bool bIgnoreI2c)
  {
    LOG (INFO) << BOLDMAGENTA << "@@@ Configuring HW parsed from .xml file @@@" << RESET;

    for (auto& cBoard : fBoardVector)
      {
        if (cBoard->getBoardType() != BoardType::RD53)
          {
            // ######################################
            // # Configuring Outer Tracker hardware #
            // ######################################
            fBeBoardInterface->ConfigureBoard(cBoard);

            LOG (INFO) << GREEN << "Successfully configured Board " << int ( cBoard->getBeId() ) << RESET;

            for (auto& cFe : cBoard->fModuleVector)
              {
                LOG (INFO) << "Configuring board.." << RESET;
                for (auto& cCbc : cFe->fReadoutChipVector)
                  {
                    if (bIgnoreI2c == false)
                      {
                        fReadoutChipInterface->ConfigureChip(cCbc);
                        LOG (INFO) << GREEN <<  "Successfully configured Chip " << int(cCbc->getChipId()) << RESET;
                      }
                  }
              }
            fBeBoardInterface->ChipReSync(cBoard);
            LOG (INFO) << GREEN << "Successfully sent resync." << RESET;
          }
        else
          {
            // ######################################
            // # Configuring Inner Tracker hardware #
            // ######################################
            LOG (INFO) << BOLDBLUE << "\t--> Found an Inner Tracker board" << RESET;
            LOG (INFO) << GREEN << "Configuring Board: " << BOLDYELLOW << +cBoard->getBeId() << RESET;
            fBeBoardInterface->ConfigureBoard(cBoard);


            // ###################
            // # Configuring FSM #
            // ###################
            size_t nTRIGxEvent = SystemController::findValueInSettings("nTRIGxEvent");
            size_t injType     = SystemController::findValueInSettings("INJtype");
            size_t nClkDelays  = SystemController::findValueInSettings("nClkDelays");
            static_cast<RD53FWInterface*>(this->fBeBoardFWMap[cBoard->getBeBoardId()])->SetAndConfigureFastCommands(cBoard, nTRIGxEvent, injType, nClkDelays);
            LOG (INFO) << GREEN << "Configured FSM fast command block" << RESET;


            // ###################
            // # Configure chips #
            // ###################
            for (const auto& cModule : cBoard->fModuleVector)
              {
                LOG (INFO) << GREEN << "Initializing communication to Module: " << BOLDYELLOW << +cModule->getModuleId() << RESET;
                for (const auto& cRD53 : cModule->fReadoutChipVector)
                  {
                    LOG (INFO) << GREEN << "Configuring RD53: " << BOLDYELLOW << +cRD53->getChipId() << RESET;
                    static_cast<RD53Interface*>(fReadoutChipInterface)->ConfigureChip(static_cast<RD53*>(cRD53));
                    LOG (INFO) << GREEN << "Number of masked pixels: " << BOLDYELLOW << static_cast<RD53*>(cRD53)->getNbMaskedPixels() << RESET;
                  }
              }
          }
      }
  }

  void SystemController::initializeFileHandler()
  {
    for (const auto& cBoard : fBoardVector)
      {
        uint32_t cBeId = cBoard->getBeId();
        uint32_t cNChip = 0;

        uint32_t cNEventSize32 = this->computeEventSize32 (cBoard);

        std::string cBoardTypeString;
        BoardType cBoardType = cBoard->getBoardType();

        for (const auto& cFe : cBoard->fModuleVector) cNChip += cFe->getNChip();

        if      (cBoardType == BoardType::D19C) cBoardTypeString = "D19C";
        else if (cBoardType == BoardType::RD53) cBoardTypeString = "RD53";

        uint32_t cFWWord  = fBeBoardInterface->getBoardInfo (cBoard);
        uint32_t cFWMajor = (cFWWord & 0xFFFF0000) >> 16;
        uint32_t cFWMinor = (cFWWord & 0x0000FFFF);

        FileHeader cHeader(cBoardTypeString, cFWMajor, cFWMinor, cBeId, cNChip, cNEventSize32, cBoard->getEventType());

        std::stringstream cBeBoardString;
        cBeBoardString << "_Board" << std::setw (3) << std::setfill ('0') << cBeId;
        std::string cFilename = fRawFileName;
        if (fRawFileName.find (".raw") != std::string::npos)
          cFilename.insert(fRawFileName.find(".raw"), cBeBoardString.str());

        FileHandler* cHandler = new FileHandler(cFilename, 'w', cHeader);

        fBeBoardInterface->SetFileHandler(cBoard, cHandler);
        LOG (INFO) << BOLDBLUE << "Saving binary data into: " << BOLDYELLOW << cFilename << RESET;
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
      fBeBoardInterface->Start(cBoard);
  }

  void SystemController::Stop()
  {
    for (auto& cBoard : fBoardVector)
      fBeBoardInterface->Stop(cBoard);
  }

  void SystemController::Pause()
  {
    for (auto& cBoard : fBoardVector)
      fBeBoardInterface->Pause(cBoard);
  }

  void SystemController::Resume()
  {
    for (auto& cBoard : fBoardVector)
      fBeBoardInterface->Resume(cBoard);
  }

  void SystemController::ConfigureHardware(std::string cHWFile, bool enableStream)
  {
    std::stringstream outp;

    InitializeHw(cHWFile, outp, true, enableStream);
    InitializeSettings(cHWFile, outp);
    std::cout << outp.str() << std::endl;
    outp.str("");
    ConfigureHw();
  }

  void SystemController::ConfigureCalibration() {}

  void SystemController::Configure(std::string cHWFile, bool enableStream)
  {
    ConfigureHardware(cHWFile, enableStream);
    ConfigureCalibration();
  }

  void SystemController::Start (BeBoard* pBoard)
  {
    fBeBoardInterface->Start(pBoard);
  }

  void SystemController::Stop (BeBoard* pBoard)
  {
    fBeBoardInterface->Stop(pBoard);
  }
  void SystemController::Pause (BeBoard* pBoard)
  {
    fBeBoardInterface->Pause(pBoard);
  }
  void SystemController::Resume (BeBoard* pBoard)
  {
    fBeBoardInterface->Resume(pBoard);
  }

  uint32_t SystemController::ReadData (BeBoard* pBoard, bool pWait)
  {
    std::vector<uint32_t> cData;
    return this->ReadData(pBoard, cData, pWait);
  }

  void SystemController::ReadData (bool pWait)
  {
    for (auto cBoard : fBoardVector)
      this->ReadData(cBoard, pWait);
  }

  uint32_t SystemController::ReadData (BeBoard* pBoard, std::vector<uint32_t>& pData, bool pWait)
  {
    uint32_t cNPackets = fBeBoardInterface->ReadData(pBoard, false, pData, pWait);
    this->DecodeData(pBoard, pData, cNPackets, fBeBoardInterface->getBoardType(pBoard));

    return cNPackets;
  }

  void SystemController::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents)
  {
    std::vector<uint32_t> cData;
    return this->ReadNEvents(pBoard, pNEvents, cData, true);
  }

  void SystemController::ReadNEvents (uint32_t pNEvents)
  {
    for (auto cBoard : fBoardVector)
      this->ReadNEvents(cBoard, pNEvents);
  }

  void SystemController::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
  {
    fBeBoardInterface->ReadNEvents(pBoard, pNEvents, pData, pWait);
    this->DecodeData(pBoard, pData, pNEvents, fBeBoardInterface->getBoardType(pBoard));
  }

  double SystemController::findValueInSettings (const char* name)
  {
    auto setting = fSettingsMap.find(name);
    return ((setting != std::end(fSettingsMap)) ? setting->second : 0);
  }


  void SystemController::SetFuture (const BeBoard *pBoard, const std::vector<uint32_t> &pData, uint32_t pNevents, BoardType pType)
  {
    if (pData.size() != 0) fFuture = std::async(&SystemController::DecodeData, this, pBoard, pData, pNevents, pType);
  }

  void SystemController::DecodeData (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType)
  {
    this->ResetEventList();

    if (pType == BoardType::RD53)
      {
        uint16_t status;
        if (RD53decodedEvents.size() == 0) RD53FWInterface::DecodeEvents(pData, status, RD53decodedEvents);

        for (const auto& evt : RD53decodedEvents)
          {
            std::vector<std::pair<size_t,size_t>> moduleAndChipIDs;

            for (const auto& chip_frame : evt.chip_frames)
              {
                int chip_id = RD53FWInterface::lane2chipId(pBoard, chip_frame.module_id, chip_frame.chip_lane);
                if (chip_id != -1) moduleAndChipIDs.push_back(std::pair<size_t,size_t>(chip_frame.module_id, chip_id));
              }

            if (moduleAndChipIDs.size() != 0)
              fEventList.push_back(new RD53Event(std::move(moduleAndChipIDs), evt.chip_events));
          }
      }
    else
      {
        fNevents   = static_cast<uint32_t>(pNevents);
        fEventSize = static_cast<uint32_t>((pData.size()) / fNevents);

        EventType fEventType = pBoard->getEventType();
        uint32_t fNFe        = pBoard->getNFe();

        if (fEventType == EventType::ZS) fNCbc = 0;
        else fNCbc = (fEventSize - D19C_EVENT_HEADER1_SIZE_32_CBC3) / D19C_EVENT_SIZE_32_CBC3 / fNFe;

        // to fill fEventList
        std::vector<uint32_t> lvec;

        //use a SwapIndex to decide wether to swap a word or not
        //use a WordIndex to pick events apart
        uint32_t cWordIndex = 0;
        uint32_t cSwapIndex = 0;
        // index of the word inside the event (ZS)
        uint32_t fZSEventSize = 0;
        uint32_t cZSWordIndex = 0;

        for (auto word : pData)
          {
            //if the SwapIndex is greater than 0 and a multiple of the event size in 32 bit words, reset SwapIndex to 0
            if (cSwapIndex > 0 && cSwapIndex % fEventSize == 0)
              cSwapIndex = 0;

#ifdef __CBCDAQ_DEV__
            //TODO
            LOG(DEBUG) << std::setw(3) << "Original " << cWordIndex << " ### " << std::bitset<32>(pData.at(cWordIndex));
            //LOG (DEBUG) << std::setw (3) << "Treated  " << cWordIndex << " ### " << std::bitset<32> (word);

            if ((cWordIndex + 1) % fEventSize == 0 && cWordIndex > 0)
              LOG(DEBUG) << std::endl
                         << std::endl;

#endif

            lvec.push_back(word);

            if (fEventType == EventType::ZS)
              {
                if (cZSWordIndex == fZSEventSize - 1)
                  {
                    if (pType == BoardType::D19C)
                      fEventList.push_back(new D19cCbc3EventZS(pBoard, fZSEventSize, lvec));

                    lvec.clear();

                    if (fEventList.size() >= fNevents)
                      break;
                  }
                else if (cZSWordIndex == fZSEventSize)
                  {
                    cZSWordIndex = 0;

                    if (pType == BoardType::D19C)
                      fZSEventSize = (0x0000FFFF & word);

                    if (fZSEventSize > pData.size())
                      {
                        LOG(ERROR) << "Missaligned data, not accepted";
                        break;
                      }
                  }
              }
            else
              {
                if (cWordIndex > 0 && (cWordIndex + 1) % fEventSize == 0)
                  {
                    if (pType == BoardType::D19C)
                      fEventList.push_back(new D19cCbc3Event(pBoard, fNCbc, fNFe, lvec));

                    lvec.clear();

                    if (fEventList.size() >= fNevents)
                      break;
                  }
              }

            cWordIndex++;
            cSwapIndex++;
            cZSWordIndex++;
          }
      }
  }

  void SystemController::ResetEventList ()
  {
    for (auto &pevt : fEventList) delete pevt;
    fEventList.clear();
    fCurrentEvent = 0;
  }
}
