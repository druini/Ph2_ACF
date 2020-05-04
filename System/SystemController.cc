/*!
  \file                  SystemController.cc
  \brief                 Controller of the System, overall wrapper of the framework
  \author                Mauro DINARDO
  \version               2.0
  \date                  01/01/20
  Support:               email to mauro.dinardo@cern.ch
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
    , flpGBTInterface      (nullptr)
    , fDetectorContainer   (nullptr)
    , fSettingsMap         ()
    , fFileHandler         (nullptr)
    , fRawFileName         ("")
    , fWriteHandlerEnabled (false)
    , fStreamerEnabled     (false)
    , fNetworkStreamer     (nullptr) // This is the server listening port
  {}

  SystemController::~SystemController() {}

  void SystemController::Inherit (const SystemController* pController)
  {
    fBeBoardInterface     = pController->fBeBoardInterface;
    fReadoutChipInterface = pController->fReadoutChipInterface;
    fChipInterface        = pController->fChipInterface;
    flpGBTInterface       = pController->flpGBTInterface;
    fBeBoardFWMap         = pController->fBeBoardFWMap;
    fSettingsMap          = pController->fSettingsMap;
    fFileHandler          = pController->fFileHandler;
    fStreamerEnabled      = pController->fStreamerEnabled;
    fNetworkStreamer      = pController->fNetworkStreamer;
    fDetectorContainer    = pController->fDetectorContainer;
    fCicInterface         = pController->fCicInterface;
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
    delete flpGBTInterface;
    flpGBTInterface = nullptr;
    delete fDetectorContainer;
    fDetectorContainer = nullptr;

    delete fCicInterface;
    fCicInterface = nullptr;
    delete fMPAInterface;
    fMPAInterface = nullptr;

    fBeBoardFWMap.clear();
    fSettingsMap.clear();

    delete fNetworkStreamer;
    fNetworkStreamer = nullptr;
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
    else                pVec = fFileHandler->readFileChunks(pNWords32);
  }

  void SystemController::InitializeHw (const std::string& pFilename, std::ostream& os, bool pIsFile , bool streamData)
  {
    fStreamerEnabled = streamData;
    if (streamData == true) fNetworkStreamer = new TCPPublishServer(6000,1);

    fDetectorContainer = new DetectorContainer;
    this->fParser.parseHW(pFilename, fBeBoardFWMap, fDetectorContainer, os, pIsFile);

    fBeBoardInterface = new BeBoardInterface(fBeBoardFWMap);
    const BeBoard* theFirstBoard = fDetectorContainer->at(0);

    flpGBTInterface = new lpGBTInterface(fBeBoardFWMap);

    if (theFirstBoard->getBoardType() != BoardType::RD53)
      {
        OuterTrackerModule* theOuterTrackerModule = static_cast<OuterTrackerModule*>((theFirstBoard->at(0))->at(0));
        auto cChipType = (static_cast<ReadoutChip*>(theOuterTrackerModule->at(0))->getFrontEndType());
        if (cChipType == FrontEndType::CBC3)

          fReadoutChipInterface = new CbcInterface(fBeBoardFWMap);
        else if(cChipType == FrontEndType::SSA)
          fReadoutChipInterface = new SSAInterface(fBeBoardFWMap);
        fCicInterface = new CicInterface(fBeBoardFWMap);
        fMPAInterface = new MPAInterface(fBeBoardFWMap);
      }
    else
      fReadoutChipInterface = new RD53Interface(fBeBoardFWMap);

    if (fWriteHandlerEnabled == true) this->initializeFileHandler();
  }

  void SystemController::InitializeSettings (const std::string& pFilename, std::ostream& os, bool pIsFile)
  {
    this->fParser.parseSettings(pFilename, fSettingsMap, os, pIsFile);
  }

  void SystemController::ConfigureHw (bool bIgnoreI2c)
  {
    LOG (INFO) << BOLDMAGENTA << "@@@ Configuring HW parsed from .xml file @@@" << RESET;

    for (auto cBoard : *fDetectorContainer)
      {
        if (cBoard->getBoardType() != BoardType::RD53)
          {
            //setting up back-end board
            fBeBoardInterface->ConfigureBoard(cBoard);
            LOG (INFO) << GREEN << "Successfully configured Board " << int ( cBoard->getId() ) << RESET;
            LOG (INFO) << BOLDBLUE << "Now going to configure chips on Board " << int ( cBoard->getId() ) << RESET;

            // CIC start-up
            for(auto cOpticalGroup : *cBoard)
              {
                if (cOpticalGroup->flpGBT != nullptr) flpGBTInterface->ConfigureChip(cOpticalGroup->flpGBT);
                uint8_t cLinkId = cOpticalGroup->getId();
                LOG (INFO) << BOLDMAGENTA << "CIC start-up seqeunce for hybrids on link " << +cLinkId << RESET;
                for (auto cHybrid : *cOpticalGroup)
                  {
                    OuterTrackerModule* theOuterTrackerModule = static_cast<OuterTrackerModule*>(cHybrid);
                    if( theOuterTrackerModule->fCic != NULL )
                      {
                        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cLinkId);
                        auto& cCic = theOuterTrackerModule->fCic;

                        // read CIC sparsification setting
                        bool cSparsified = (fBeBoardInterface->ReadBoardReg(cBoard,"fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable") == 1);
                        cBoard->setSparsification( cSparsified );

                        LOG (INFO) << BOLDBLUE << "Configuring CIC" << +(theOuterTrackerModule->getFeId()%2) << " on link " << +theOuterTrackerModule->getLinkId() << " on hybrid " << +theOuterTrackerModule->getFeId() << RESET;
                        fCicInterface->ConfigureChip( cCic);

                        // CIC start-up
                        uint8_t cModeSelect = (static_cast<ReadoutChip*>(theOuterTrackerModule->at(0))->getFrontEndType() != FrontEndType::CBC3); // 0 --> CBC , 1 --> MPA
                        // select CIC mode
                        bool cSuccess = fCicInterface->SelectMode( cCic, cModeSelect );
                        if(!cSuccess)
                          {
                            LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << " to configure CIC mode.." << RESET;
                            exit(0);
                          }
                        LOG (INFO) << BOLDMAGENTA << "CIC configured for "
                                   << ((cModeSelect==0)? "2S" : "PS")
                                   << " readout."
                                   << RESET;
                        // CIC start-up sequence
                        uint8_t cDriveStrength = 5;
                        cSuccess = fCicInterface->StartUp(cCic , cDriveStrength);
                        for (auto cChip : *cHybrid)
                          {
                            fReadoutChipInterface->WriteChipReg ( static_cast<ReadoutChip*>(cChip), "EnableSLVS", 1);
                          }
                        fBeBoardInterface->ChipReSync ( cBoard );
                        LOG (INFO) << BOLDGREEN << "SUCCESSFULLY " << BOLDBLUE << " performed start-up sequence on CIC" << +(theOuterTrackerModule->getId()%2) << " connected to link " << +theOuterTrackerModule->getLinkId() <<  RESET ;
                        LOG (INFO) << BOLDGREEN << "####################################################################################" << RESET;
                      }
                    // Configure readout-chips [CBCs, MPAs, SSAs]
                    for (auto cReadoutChip : *cHybrid)
                      {
                        ReadoutChip* theReadoutChip = static_cast<ReadoutChip*>(cReadoutChip);
                        if ( !bIgnoreI2c )
                          {
                            LOG (INFO) << BOLDBLUE << "Configuring readout chip [CBC" << +cReadoutChip->getId() << " ]" << RESET;
                            fReadoutChipInterface->ConfigureChip ( theReadoutChip );
                          }
                      }
                  }
              }
          }
        else
          {
            // ######################################
            // # Configuring Inner Tracker hardware #
            // ######################################
            LOG (INFO) << BOLDBLUE << "\t--> Found an Inner Tracker board" << RESET;
            LOG (INFO) << GREEN << "Configuring Board: " << RESET << BOLDYELLOW << +cBoard->getId() << RESET;
            fBeBoardInterface->ConfigureBoard(cBoard);


            // ###################
            // # Configuring FSM #
            // ###################
            size_t nTRIGxEvent = SystemController::findValueInSettings("nTRIGxEvent");
            size_t injType     = SystemController::findValueInSettings("INJtype");
            size_t nClkDelays  = SystemController::findValueInSettings("nClkDelays");
            size_t colStart    = SystemController::findValueInSettings("COLstart");
            static_cast<RD53FWInterface*>(this->fBeBoardFWMap[cBoard->getId()])->SetAndConfigureFastCommands(cBoard, nTRIGxEvent, injType, nClkDelays, colStart < RD53::LIN.colStart);
            LOG (INFO) << GREEN << "Configured FSM fast command block" << RESET;


            // ########################
            // # Configuring from XML #
            // ########################
            static_cast<RD53FWInterface*>(this->fBeBoardFWMap[cBoard->getId()])->ConfigureFromXML(cBoard);


            // ###################
            // # Configure chips #
            // ###################
            for(auto cOpticalGroup : *cBoard)
              {
                if (cOpticalGroup->flpGBT != nullptr) flpGBTInterface->ConfigureChip(cOpticalGroup->flpGBT);
                for (auto cHybrid : *cOpticalGroup)
                  {
                    LOG (INFO) << GREEN << "Initializing communication to Module: " << RESET << BOLDYELLOW << +cHybrid->getId() << RESET;
                    for (const auto cRD53 : *cHybrid)
                      {
                        LOG (INFO) << GREEN << "Configuring RD53: " << RESET << BOLDYELLOW << +cRD53->getId() << RESET;
                        static_cast<RD53Interface*>(fReadoutChipInterface)->ConfigureChip(static_cast<RD53*>(cRD53));
                        LOG (INFO) << GREEN << "Number of masked pixels: " << RESET << BOLDYELLOW << static_cast<RD53*>(cRD53)->getNbMaskedPixels() << RESET;
                      }
                  }
              }
          }
      }
  }

  void SystemController::initializeFileHandler()
  {
    for (const auto* cBoard : *fDetectorContainer)
      {
        uint32_t cNChip        = 0;
        uint32_t cBeId         = cBoard->getId();
        uint32_t cNEventSize32 = this->computeEventSize32(cBoard);

        std::string cBoardTypeString;
        BoardType cBoardType = cBoard->getBoardType();

        for (const auto cOpticalGroup : *cBoard)
          for (const auto cHybrid : *cOpticalGroup)
            cNChip += cHybrid->size();

        if      (cBoardType == BoardType::D19C) cBoardTypeString = "D19C";
        else if (cBoardType == BoardType::RD53) cBoardTypeString = "RD53";

        uint32_t cFWWord  = fBeBoardInterface->getBoardInfo(cBoard);
        uint32_t cFWMajor = (cFWWord & 0xFFFF0000) >> 16;
        uint32_t cFWMinor = (cFWWord & 0x0000FFFF);

        FileHeader cHeader(cBoardTypeString, cFWMajor, cFWMinor, cBeId, cNChip, cNEventSize32, cBoard->getEventType());

        std::stringstream cBeBoardString;
        cBeBoardString << "_Board" << std::setw(3) << std::setfill ('0') << cBeId;
        std::string cFilename = fRawFileName;
        if (fRawFileName.find (".raw") != std::string::npos)
          cFilename.insert(fRawFileName.find(".raw"), cBeBoardString.str());

        fFileHandler = new FileHandler(cFilename, 'w', cHeader);

        fBeBoardInterface->SetFileHandler(cBoard, fFileHandler);
        LOG (INFO) << GREEN << "Saving binary data into: " << RESET << BOLDYELLOW << cFilename << RESET;
      }
  }

  uint32_t SystemController::computeEventSize32 (const BeBoard* pBoard)
  {
    uint32_t cNEventSize32 = 0;
    uint32_t cNChip = 0;

    for (const auto cOpticalGroup : *pBoard)
      for (const auto cHybrid : *cOpticalGroup)
        cNChip += cHybrid->size();

    if (pBoard->getBoardType() == BoardType::D19C)
      cNEventSize32 = D19C_EVENT_HEADER1_SIZE_32_CBC3 + cNChip * D19C_EVENT_SIZE_32_CBC3;

    return cNEventSize32;
  }

  void SystemController::Start(int currentRun)
  {
    for (auto cBoard : *fDetectorContainer)
      fBeBoardInterface->Start(cBoard);
  }

  void SystemController::Stop()
  {
    for (auto cBoard : *fDetectorContainer)
      fBeBoardInterface->Stop(cBoard);
  }

  void SystemController::Pause()
  {
    for (auto cBoard : *fDetectorContainer)
      fBeBoardInterface->Pause(cBoard);
  }

  void SystemController::Resume()
  {
    for (auto cBoard : *fDetectorContainer)
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

  void SystemController::Configure (std::string cHWFile, bool enableStream)
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
    for (auto cBoard : *fDetectorContainer)
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
    for (auto cBoard : *fDetectorContainer)
      this->ReadNEvents(cBoard, pNEvents);
  }

  void SystemController::ReadNEvents (BeBoard* pBoard, uint32_t pNEvents, std::vector<uint32_t>& pData, bool pWait)
  {
    fBeBoardInterface->ReadNEvents(pBoard, pNEvents, pData, pWait);

    uint32_t cMultiplicity = 0;
    if (fBeBoardInterface->getBoardType(pBoard) == BoardType::D19C)
      cMultiplicity = fBeBoardInterface->ReadBoardReg(pBoard,"fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
    pNEvents = pNEvents*(cMultiplicity+1);

    this->DecodeData(pBoard, pData, pNEvents, fBeBoardInterface->getBoardType(pBoard));
  }


  void SystemController::ReadASEvent (BeBoard* pBoard, uint32_t pNMsec,bool pulses)
  {
    std::vector<uint32_t> cData;
	if (pulses) static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->Send_pulses();
	else
      {
		static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PS_Open_shutter(0);
		std::this_thread::sleep_for (std::chrono::microseconds(pNMsec));
		static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PS_Close_shutter(0);
      }
    /* static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PS_Open_shutter(0);
       std::this_thread::sleep_for (std::chrono::microseconds(pNMsec));
       for (uint32_t i=0; i<pNpulse;i++)
       {
       static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ChipTestPulse();
       std::this_thread::sleep_for (std::chrono::microseconds(pNMsec));
       }
       static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ChipTestPulse();

       static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PS_Close_shutter(0);*/
	//static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ReadASEvent(pBoard,pNMsec, cData);

    for(auto cOpticalGroup : *pBoard)
      {
        for(auto cHybrid : *cOpticalGroup)
          {
            for(auto cChip : *cHybrid)
              {
                static_cast<SSAInterface*>(fReadoutChipInterface)->ReadASEvent(cChip, cData);
              }
          }
      }

    this->DecodeData(pBoard, cData, 1, fBeBoardInterface->getBoardType(pBoard));
  }

  double SystemController::findValueInSettings (const std::string name, double defaultValue) const
  {
    auto setting = fSettingsMap.find(name);
    return (setting != std::end(fSettingsMap) ? setting->second : defaultValue);
  }


  // #################
  // # Data decoding #
  // #################
  void SystemController::SetFuture (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType)
  {
    if (pData.size() != 0) fFuture = std::async(&SystemController::DecodeData, this, pBoard, pData, pNevents, pType);
  }

  void SystemController::DecodeData (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType)
  {
    if (pType == BoardType::RD53)
      {
        fEventList.clear();
        if (RD53FWInterface::decodedEvents.size() == 0) RD53FWInterface::DecodeEvents(pData, RD53FWInterface::decodedEvents);
        RD53FWInterface::Event::addBoardInfo2Events(pBoard, RD53FWInterface::decodedEvents);
        for (auto i = 0u; i < RD53FWInterface::decodedEvents.size(); i++) fEventList.push_back(&RD53FWInterface::decodedEvents[i]);
      }
    else if (pType == BoardType::D19C)
      {
        for (auto &pevt : fEventList) delete pevt;
        fEventList.clear();
        fCurrentEvent = 0;

        EventType fEventType = pBoard->getEventType();
        uint32_t fNFe = pBoard->getNFe();
        uint32_t cBlockSize = 0x0000FFFF & pData.at(0) ;
        LOG (DEBUG) << BOLDBLUE << "Reading events from " << +fNFe << " FEs connected to uDTC...[ " << +cBlockSize*4 << " 32 bit words to decode]" << RESET;

        if (fEventType == EventType::SSAAS)
          {
            uint16_t nSSA = 2;
            fEventList.push_back(new D19cSSAEventAS(pBoard, nSSA, fNFe, pData));
          }
        else if (fEventType != EventType::ZS)
          {
            size_t cEventIndex=0;
            auto cEventIterator = pData.begin();
            do
              {
                uint32_t cEventSize = (0x0000FFFF & (*cEventIterator))*4 ; // event size is given in 128 bit words
                auto cEnd = ( (cEventIterator+cEventSize) > pData.end() ) ? pData.end() : (cEventIterator + cEventSize) ;
                // retrieve chunck of data vector belonging to this event
                if( cEnd - cEventIterator == cEventSize )
                  {
                    std::vector<uint32_t> cEvent(cEventIterator, cEnd);
                    //some useful debug information
                    LOG (DEBUG) << BOLDGREEN << "Event" << +cEventIndex << " .. Data word that should be event header ..  " << std::bitset<32>(*cEventIterator) << ". Event is made up of " << +cEventSize <<  " 32 bit words..." << RESET;
                    if( pBoard->getFrontEndType() == FrontEndType::CBC3 )
                      {
                        fEventSize = static_cast<uint32_t>(cEventSize);
                        fNCbc = (fEventSize - D19C_EVENT_HEADER1_SIZE_32_CBC3) / D19C_EVENT_SIZE_32_CBC3 / fNFe;
                        fEventList.push_back ( new D19cCbc3Event ( pBoard, fNCbc, fNFe , cEvent ) );
                      }
                    else if( pBoard->getFrontEndType() == FrontEndType::CIC )
                      {
                        fNCbc = 8;
                        fNFe = 8*2; // maximum of 8 links x 2 FEHs per link
                        fEventList.push_back ( new D19cCicEvent ( pBoard, fNCbc , fNFe, cEvent ) );
                      }
                    else if(pBoard->getFrontEndType() == FrontEndType::CIC2   )
                      {
                        fNCbc = 8;
                        fNFe = 8*2; // maximum of 8 links x 2 FEHs per link
                        // check if the board is reading sparsified or unsparsified data
                        fEventList.push_back ( new D19cCic2Event ( pBoard, fNCbc , fNFe, cEvent ) );
                      }
                    else if( pBoard->getFrontEndType() == FrontEndType::SSA )
                      {
                        size_t nSSA = 2 ;
                        size_t cNFe = 1;
                        fEventList.push_back(new D19cSSAEvent(pBoard, nSSA, cNFe, cEvent));
                      }
                    cEventIndex++;
                  }
                cEventIterator += cEventSize;
              }while( cEventIterator < pData.end());
          }
      }
  }
}
