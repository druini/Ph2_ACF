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
    , fDetectorContainer   (nullptr)
    , fBoardVector         ()
    , fSettingsMap         ()
    , fFileHandler         (nullptr)
    , fRawFileName         ("")
    , fWriteHandlerEnabled (false)
    , fStreamerEnabled     (false)
    , fNetworkStreamer     (nullptr) // This is the server listening port
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
    delete fCicInterface;
    fCicInterface = nullptr;
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
    this->fParser.parseHW(pFilename, fBeBoardFWMap, fBoardVector, fDetectorContainer, os, pIsFile);

    fBeBoardInterface = new BeBoardInterface(fBeBoardFWMap);
    if (fBoardVector[0]->getBoardType() != BoardType::RD53)
    {
        if (fBoardVector[0]->getEventType() != EventType::SSA) fReadoutChipInterface = new CbcInterface(fBeBoardFWMap);
        else                                                   fReadoutChipInterface = new SSAInterface(fBeBoardFWMap);
        fCicInterface = new CicInterface(fBeBoardFWMap);
        fMPAInterface = new MPAInterface(fBeBoardFWMap);
    }
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
          //setting up back-end board
          fBeBoardInterface->ConfigureBoard ( cBoard );
          LOG (INFO) << GREEN << "Successfully configured Board " << int ( cBoard->getBeId() ) << RESET;
          LOG (INFO) << BOLDBLUE << "Now going to configure chips on Board " << int ( cBoard->getBeId() ) << RESET;

          // CIC start-up
          bool cWithCIC=false;
          for (auto& cFe : cBoard->fModuleVector)
          {
              if( static_cast<OuterTrackerModule*>(cFe)->fCic == NULL )
                continue;

              cWithCIC = true;
              static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
              auto& cCic = static_cast<OuterTrackerModule*>(cFe)->fCic;

              // read CIC sparsification setting 
              bool cSparsified = (fBeBoardInterface->ReadBoardReg(cBoard,"fc7_daq_cnfg.physical_interface_block.cic.2s_sparsified_enable") == 1);
              cBoard->setSparsification( cSparsified );

              LOG (INFO) << BOLDBLUE << "Configuring CIC" << +(cFe->getFeId()%2) << " on link " << +cFe->getLinkId() << " on hybrid " << +cFe->getFeId() << RESET;
              fCicInterface->ConfigureChip( static_cast<OuterTrackerModule*>(cFe)->fCic);
              auto cRegisterMap = cCic->getRegMap();
              uint16_t cRegValue = fReadoutChipInterface->ReadChipReg( cCic, cRegisterMap.begin()->first );
              LOG (INFO) << BOLDGREEN <<  "Successfully configured CIC : " << cRegisterMap.begin()->first << " set to 0x" << std::hex << cRegValue << std::dec <<  "." << RESET;

              // CIC start-up
              uint8_t cModeSelect = (cFe->fReadoutChipVector[0]->getFrontEndType() != FrontEndType::CBC3); // 0 --> CBC , 1 --> MPA
              // select CIC mode
              bool cSuccess = fCicInterface->SelectMode( static_cast<OuterTrackerModule*>(cFe)->fCic, cModeSelect );
              if(!cSuccess)
              {
                  LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << " to configure CIC mode.." << RESET;
                  exit(0);
              }
              // CIC start-up sequence
              uint8_t cDriveStrength = 5;
              cSuccess = fCicInterface->StartUp(cCic , cDriveStrength);
              if(!cSuccess)
              {
                  LOG (INFO) << BOLDRED << "FAILED " << BOLDBLUE << " CIC start-up sequence.." << RESET;
                  exit(0);
              }
              fBeBoardInterface->ChipReSync ( cBoard );
              LOG (INFO) << BOLDGREEN << "SUCCESSFULLY " << BOLDBLUE << " performed start-up sequence on CIC" << +(cFe->getFeId()%2) << " connected to link " << +cFe->getLinkId() <<  RESET ;
              LOG (INFO) << BOLDGREEN << "####################################################################################" << RESET;
          }

          // hard reset to all readout chips
          //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ReadoutChipReset();
          // Configure readout-chips [CBCs, MPAs, SSAs]
          for (auto& cFe : cBoard->fModuleVector)
          {
              static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->selectLink (cFe->getLinkId());
              for (auto& cReadoutChip : cFe->fReadoutChipVector)
              {
                  if ( !bIgnoreI2c && cReadoutChip->getFrontEndType() == FrontEndType::CBC3 )
                  {
                    LOG (INFO) << BOLDBLUE << "Configuring readout chip [CBC" << +cReadoutChip->getChipId() << " ]" << RESET;
                    fReadoutChipInterface->ConfigureChip ( cReadoutChip );
                    // // check the first value in the register map
                    // auto cRegisterMap = cReadoutChip->getRegMap();
                    // uint16_t cRegValue = fReadoutChipInterface->ReadChipReg( cReadoutChip, cRegisterMap.begin()->first );
                    // LOG (INFO) << BOLDGREEN <<  "Successfully configured Chip " << int ( cReadoutChip->getChipId() ) << " [ " << cRegisterMap.begin()->first << " set to 0x" << std::hex << cRegValue << std::dec <<  " ]." << RESET;

                    // // if there is no CICs then do the back-end alignment here ...
                    // if( static_cast<OuterTrackerModule*>(cFe)->fCic != NULL )
                    //   continue;

                    // // original threshold
                    // uint16_t cThreshold = static_cast<CbcInterface*>(fReadoutChipInterface)->ReadChipReg(cReadoutChip, "VCth" );
                    // LOG (INFO) << BOLDBLUE << "Running phase tuning and word alignment on FE" << +cFe->getFeId() << " CBC" << +cReadoutChip->getId() << "..." << RESET;
                    // std::vector<uint8_t> cSeeds{0x35,0x6A,0xD5};
                    // uint8_t cBendCode_phAlign = 10;
                    // std::vector<uint8_t> cBendLUT = static_cast<CbcInterface*>(fReadoutChipInterface)->readLUT( cReadoutChip );
                    // auto cIterator = std::find(cBendLUT.begin(), cBendLUT.end(), cBendCode_phAlign);
                    // // if bend code isn't there ... quit
                    // if( cIterator == cBendLUT.end() )
                    //     continue;

                    // int cPosition = std::distance( cBendLUT.begin(), cIterator);
                    // double cBend_strips = -7. + 0.5*cPosition;
                    // std::vector<int> cBends ( cSeeds.size() , static_cast<int>( cBend_strips*2));
                    // static_cast<CbcInterface*>(fReadoutChipInterface)->injectStubs( cReadoutChip , cSeeds , cBends);
                    // // make sure pT width is set to maximum
                    // static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cReadoutChip , "PtCut" , 14);
                    // // and that hit OR is turned off
                    // static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg( cReadoutChip , "HitOr" , 0);
                    // LOG (INFO) << BOLDBLUE << "\t.... configured CBC" << +cReadoutChip->getId() << " to disable HitOr and set PtCut to maximum value [14 half-strips]" << RESET;
                    // bool cPhaseTuningSuccess = false;
                    // uint8_t cLineId=1;
                    // for( auto& cSeed : cSeeds )
                    // {
                    //     cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( cBoard, cFe->getFeId(), cReadoutChip->getChipId() , cLineId++ , cSeed , 8);
                    //     if (!cPhaseTuningSuccess )
                    //     {
                    //         LOG (INFO) << BOLDRED << "Hybrid " << +cFe->getId() << " CBC " << +cReadoutChip->getChipId() << " FAILED phase tuning on SLVS line " << +(cLineId-1) << RESET;
                    //         exit(0);
                    //     }
                    // }
                    // // line with bend codes
                    // cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( cBoard, cFe->getFeId(), cReadoutChip->getChipId() , 4 , ( cBendCode_phAlign << 4) | (cBendCode_phAlign << 0) , 8);
                    // if (!cPhaseTuningSuccess )
                    // {
                    //     LOG (INFO) << BOLDRED << "Hybrid " << +cFe->getId() << " CBC " << +cReadoutChip->getChipId() << " FAILED phase tuning on last SLVS line!" << RESET;
                    //     exit(0);
                    // }
                    // // final line
                    // fBeBoardInterface->ChipReSync ( cBoard );
                    // cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( cBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, (1 << 7) | (cBendCode_phAlign << 0)  , 8);
                    // if( !cPhaseTuningSuccess )
                    // {
                    //     // if it doen't work then try all possible
                    //     // combinations of error bits/SoF flags on
                    //     LOG (INFO) << BOLDRED << "Phase Tuning/Word Alignment failed on SLVS5 : going to try and see if this is because error flags/SoF bits are high!" << RESET;
                    //     cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( cBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, ( 1 << 7) | (1 <<6) | (cBendCode_phAlign <<0)  , 8);
                    //     if(!cPhaseTuningSuccess)
                    //     {
                    //         cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( cBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, ( 1 << 7) | (1 <<6) | (1<<4) | (cBendCode_phAlign <<0)  , 8);
                    //         if(! cPhaseTuningSuccess )
                    //         {
                    //             cPhaseTuningSuccess = static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning( cBoard, cFe->getFeId(), cReadoutChip->getChipId() , 5, ( 1 << 7) | (1 <<4) | (cBendCode_phAlign <<0)  , 8);
                    //             if (!cPhaseTuningSuccess )
                    //             {
                    //                 LOG (INFO) << BOLDRED << "Hybrid " << +cFe->getId() << " CBC " << +cReadoutChip->getChipId() << " FAILED phase tuning on last SLVS line!" << RESET;
                    //                 exit(0);
                    //             }
                    //         }
                    //     }
                    // }
                    // //now unmask all channels
                    // static_cast<CbcInterface*>(fReadoutChipInterface)->MaskAllChannels( cReadoutChip, false);
                    // std::this_thread::sleep_for (std::chrono::milliseconds (50) );
                    // LOG (INFO) << BOLDBLUE << "Setting threshold back to orginal value [ " << +cThreshold << " ] DAC units." << RESET;
                    // //static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg(cReadoutChip, "VCth" , 0);
                    // //fBeBoardInterface->WriteBoardReg(cBoard, "fc7_daq_cnfg.physical_interface_block.cic.debug_select" , cReadoutChip->getChipId() ) ;
                    // //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1ADebug();
                    // //static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg(cReadoutChip, "VCth" , 900);
                    // //static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->L1ADebug();
                    // static_cast<CbcInterface*>(fReadoutChipInterface)->WriteChipReg(cReadoutChip, "VCth" , cThreshold);
                  }
              }
          }
          static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PhaseTuning (cBoard);
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

            // ########################
            // # Configuring from XML #
            // ########################
            static_cast<RD53FWInterface*>(this->fBeBoardFWMap[cBoard->getBeBoardId()])->ConfigureFromXML(cBoard);



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
        uint32_t cNChip        = 0;
        uint32_t cBeId         = cBoard->getBeId();
        uint32_t cNEventSize32 = this->computeEventSize32(cBoard);

        std::string cBoardTypeString;
        BoardType cBoardType = cBoard->getBoardType();

        for (const auto& cFe : cBoard->fModuleVector) cNChip += cFe->getNChip();

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
        LOG (INFO) << GREEN << "Saving binary data into: " << BOLDYELLOW << cFilename << RESET;
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
    uint32_t cMultiplicity=0;
    if (fBeBoardInterface->getBoardType(pBoard) == BoardType::D19C)
      cMultiplicity = fBeBoardInterface->ReadBoardReg(pBoard,"fc7_daq_cnfg.fast_command_block.misc.trigger_multiplicity");
    pNEvents = pNEvents*(cMultiplicity+1);
    this->DecodeData(pBoard, pData, pNEvents, fBeBoardInterface->getBoardType(pBoard));
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

  // void SystemController::DecodeData (const BeBoard* pBoard, const std::vector<uint32_t>& pData, uint32_t pNevents, BoardType pType)
  // {
  //   if (pType == BoardType::RD53)
  //   {
  //     fEventList.clear();
  //     if (RD53FWInterface::decodedEvents.size() == 0) RD53FWInterface::DecodeEvents(pData, RD53FWInterface::decodedEvents);
  //     RD53FWInterface::Event::addBoardInfo2Events(pBoard, RD53FWInterface::decodedEvents);
  //     for (auto i = 0u; i < RD53FWInterface::decodedEvents.size(); i++) fEventList.push_back(&RD53FWInterface::decodedEvents[i]);
  //   }
  //   else if (pType == BoardType::D19C)
  //   {
  //       for (auto &pevt : fEventList) delete pevt;
  //       fEventList.clear();
  //       fCurrentEvent = 0;

  //       EventType fEventType = pBoard->getEventType();
  //       uint32_t fNFe = pBoard->getNFe();
  //       uint32_t cBlockSize = 0x0000FFFF & pData.at(0) ;
  //       LOG (DEBUG) << BOLDBLUE << "Reading events from " << +fNFe << " FEs connected to uDTC...[ " << +cBlockSize*4 << " 32 bit words to decode]" << RESET;

  //       if (fEventType == EventType::SSA)
  //       {
  //         fNevents   = static_cast<uint32_t>(pNevents);
  //         uint32_t fEventSize = static_cast<uint32_t>((pData.size()) / fNevents);
  //         uint16_t nSSA = (fEventSize - D19C_EVENT_HEADER1_SIZE_32_SSA) / D19C_EVENT_SIZE_32_SSA / fNFe;
  //         fEventList.push_back(new D19cSSAEvent(pBoard, nSSA, fNFe, pData));
  //         return;
  //       }

  //       if (fEventType != EventType::ZS)
  //       {
  //           auto it = pData.begin();
  //           size_t cEventIndex=0;
  //           bool cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
  //           while(cCondition)
  //           {
  //               cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
  //               uint32_t cEventSize = (0x0000FFFF & (*it))*4 ; // event size is given in 128 bit words
  //               auto cEnd = ( (it+cEventSize) > pData.end() ) ? pData.end() : (it + cEventSize) ;
  //               bool cCondition2 = (pNevents > 0 ) ?  (cEnd <= pData.end() && cEventIndex < pNevents) : (cEnd <= pData.end());
  //               if( cCondition2 )
  //               {
  //                 // retrieve chunck of data vector belonging to this event
  //                 std::vector<uint32_t> cEvent(it, cEnd);
  //                 // some useful debug information
  //                 LOG (DEBUG) << BOLDGREEN << "Event" << +cEventIndex << " : " << std::bitset<32>(cEvent[0]) << " : " << +cEventSize <<  " 32 bit words..." << RESET;
  //                 // currently some problem with the dummy words..
  //                 // push back event into event list
  //                 if( pBoard->getFrontEndType() == FrontEndType::CBC3 )
  //                 {
  //                   fNevents   = static_cast<uint32_t>(pNevents);
  //                   fEventSize = static_cast<uint32_t>((pData.size()) / fNevents);

  //                   LOG (DEBUG) << BOLDBLUE << "CBC3 events..." << RESET;
  //                   fNCbc = (fEventSize - D19C_EVENT_HEADER1_SIZE_32_CBC3) / D19C_EVENT_SIZE_32_CBC3 / fNFe;
  //                   fEventList.push_back ( new D19cCbc3Event ( pBoard, fNCbc, fNFe , cEvent ) );
  //                 }
  //                 else if( pBoard->getFrontEndType() == FrontEndType::CIC )
  //                 {
  //                   fNCbc = 8;
  //                   fNFe = 8*2; // maximum of 8 links x 2 FEHs per link
  //                   fEventList.push_back ( new D19cCicEvent ( pBoard, fNCbc , fNFe, cEvent ) );
  //                 }
  //                 else if(pBoard->getFrontEndType() == FrontEndType::CIC2 )
  //                 {
  //                   fNCbc = 8;
  //                   fNFe = 8*2; // maximum of 8 links x 2 FEHs per link
  //                   // check if the board is reading sparsified or unsparsified data 
  //                   fEventList.push_back ( new D19cCic2Event ( pBoard, fNCbc , fNFe, cEvent ) );
  //                 }
  //                 cEventIndex++;
  //               }
  //               it = cEnd;
  //           }
  //       }
  //       // moved the ZS case to here ... to be tackled afterwards
  //       else
  //       {
  //         fNCbc = 0;
  //         if( pBoard->getFrontEndType() == FrontEndType::CIC || pBoard->getFrontEndType() == FrontEndType::CIC2 )
  //         {
  //           auto it = pData.begin();
  //           size_t cEventIndex=0;
  //           bool cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
  //           while(cCondition)
  //           {
  //               cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
  //               uint32_t cEventSize = (0x0000FFFF & (*it))*4 ; // event size is given in 128 bit words
  //               auto cEnd = ( (it+cEventSize) > pData.end() ) ? pData.end() : (it + cEventSize) ;
  //               bool cCondition2 = (pNevents > 0 ) ?  (cEnd <= pData.end() && cEventIndex < pNevents) : (cEnd <= pData.end());
  //               if( cCondition2 )
  //               {
  //                 // retrieve chunck of data vector belonging to this event
  //                 std::vector<uint32_t> cEvent(it, cEnd);

  //                 // some useful debug information
  //                 LOG (INFO) << BOLDGREEN << "Event" << +cEventIndex << " : " << std::bitset<32>(cEvent[0]) << " : " << +cEventSize <<  " 32 bit words." << RESET;
  //                 // push back event into event list
  //                 LOG (DEBUG) << BOLDBLUE << "CIC events..." << RESET;
  //                 for( auto cWord : cEvent )
  //                 {
  //                   LOG (INFO) << BOLDBLUE << "\t.... " << std::bitset<32>(cWord) << RESET;

  //                 }
  //                 //fEventList.push_back ( new D19cCicEvent ( pBoard, fNCbc , fNFe, cEvent ) );
  //                 cEventIndex++;
  //               }
  //               it = cEnd;
  //           }
  //         }
  //         else
  //         {
  //           fNCbc = 0;
  //           //use a SwapIndex to decide wether to swap a word or not
  //           //use a WordIndex to pick events apart
  //           uint32_t cWordIndex = 0;
  //           uint32_t cSwapIndex = 0;
  //           // index of the word inside the event (ZS)
  //           uint32_t fZSEventSize = 0;
  //           uint32_t cZSWordIndex = 0;

  //           // to fill fEventList
  //           std::vector<uint32_t> lvec;
  //           for ( auto word : pData )
  //           {
  //               //if the SwapIndex is greater than 0 and a multiple of the event size in 32 bit words, reset SwapIndex to 0
  //               if (cSwapIndex > 0 && cSwapIndex % fEventSize == 0) cSwapIndex = 0;

  //                 #ifdef __CBCDAQ_DEV__
  //                           //TODO
  //                           LOG (DEBUG) << std::setw (3) << "Original " << cWordIndex << " ### " << std::bitset<32> (pData.at (cWordIndex) );
  //                           if ( (cWordIndex + 1) % fEventSize == 0 && cWordIndex > 0 ) LOG (DEBUG) << std::endl << std::endl;
  //                 #endif
  //               lvec.push_back ( word );
  //               if ( cZSWordIndex == fZSEventSize - 1 )
  //               {
  //                   //LOG(INFO) << "Packing event # " << fEventList.size() << ", Event size is " << fZSEventSize << " words";
  //                   if (pType == BoardType::D19C)
  //                       fEventList.push_back ( new D19cCbc3EventZS ( pBoard, fZSEventSize, lvec ) );
  //                   lvec.clear();
  //                   if (fEventList.size() >= fNevents) break;
  //               }
  //               else if ( cZSWordIndex == fZSEventSize )
  //               {
  //                   // get next event size
  //                   cZSWordIndex = 0;
  //                   if (pType == BoardType::D19C)
  //                       fZSEventSize = (0x0000FFFF & word);
  //                   if (fZSEventSize > pData.size() )
  //                   {
  //                       LOG (ERROR) << "Missaligned data, not accepted";
  //                       break;
  //                   }
  //               }
  //               cWordIndex++;
  //               cSwapIndex++;
  //               cZSWordIndex++;
  //           }
  //         }
  //       }
  //   }
  // }

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
        if (fEventType != EventType::ZS)
        {
            auto it = pData.begin();
            size_t cEventIndex=0;
            bool cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
            while(cCondition)
            {
                cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
                uint32_t cEventSize = (0x0000FFFF & (*it))*4 ; // event size is given in 128 bit words
                auto cEnd = ( (it+cEventSize) > pData.end() ) ? pData.end() : (it + cEventSize) ;
                bool cCondition2 = (pNevents > 0 ) ?  (cEnd <= pData.end() && cEventIndex < pNevents) : (cEnd <= pData.end());
                if( cCondition2 )
                {
                  // retrieve chunck of data vector belonging to this event 
                  std::vector<uint32_t> cEvent(it, cEnd);
                  // some useful debug information 
                  LOG (DEBUG) << BOLDGREEN << "Event" << +cEventIndex << " : " << std::bitset<32>(cEvent[0]) << " : " << +cEventSize <<  " 32 bit words..." << RESET;
                  // currently some problem with the dummy words..
                  // push back event into event list
                  if( pBoard->getFrontEndType() == FrontEndType::CBC3 ) 
                  {
                    fNevents   = static_cast<uint32_t>(pNevents);
                    fEventSize = static_cast<uint32_t>((pData.size()) / fNevents);

                    LOG (DEBUG) << BOLDBLUE << "CBC3 events..." << RESET;
                    fNCbc = (fEventSize - D19C_EVENT_HEADER1_SIZE_32_CBC3) / D19C_EVENT_SIZE_32_CBC3 / fNFe;
                    fEventList.push_back ( new D19cCbc3Event ( pBoard, fNCbc, fNFe , cEvent ) );
                  }
                  else if( pBoard->getFrontEndType() == FrontEndType::CIC )
                  {
                    fNCbc = 8; 
                    fNFe = 8*2; // maximum of 8 links x 2 FEHs per link
                    fEventList.push_back ( new D19cCicEvent ( pBoard, fNCbc , fNFe, cEvent ) );
                  }
                  else if(pBoard->getFrontEndType() == FrontEndType::CIC2 ) 
                  {
                    fNCbc = 8; 
                    fNFe = 8*2; // maximum of 8 links x 2 FEHs per link
                    // check if the board is reading sparsified or unsparsified data 
                    fEventList.push_back ( new D19cCic2Event ( pBoard, fNCbc , fNFe, cEvent ) );
                  }
                  cEventIndex++;
                }
                it = cEnd;
            }
        }
        // moved the ZS case to here ... to be tackled afterwards
        else
        {
          fNCbc = 0;
          if( pBoard->getFrontEndType() == FrontEndType::CIC || pBoard->getFrontEndType() == FrontEndType::CIC2 )
          {
            auto it = pData.begin();
            size_t cEventIndex=0;
            bool cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
            while(cCondition)
            {
                cCondition = (pNevents > 0 ) ? (it != pData.end() && cEventIndex < pNevents) : (it != pData.end()) ;
                uint32_t cEventSize = (0x0000FFFF & (*it))*4 ; // event size is given in 128 bit words
                auto cEnd = ( (it+cEventSize) > pData.end() ) ? pData.end() : (it + cEventSize) ;
                bool cCondition2 = (pNevents > 0 ) ?  (cEnd <= pData.end() && cEventIndex < pNevents) : (cEnd <= pData.end());
                if( cCondition2 )
                {
                  // retrieve chunck of data vector belonging to this event 
                  std::vector<uint32_t> cEvent(it, cEnd);
                  
                  // some useful debug information 
                  LOG (INFO) << BOLDGREEN << "Event" << +cEventIndex << " : " << std::bitset<32>(cEvent[0]) << " : " << +cEventSize <<  " 32 bit words." << RESET;  
                  // push back event into event list
                  LOG (DEBUG) << BOLDBLUE << "CIC events..." << RESET;
                  for( auto cWord : cEvent )
                  {
                    LOG (INFO) << BOLDBLUE << "\t.... " << std::bitset<32>(cWord) << RESET;
                    
                  }
                  //fEventList.push_back ( new D19cCicEvent ( pBoard, fNCbc , fNFe, cEvent ) );
                  cEventIndex++;
                }
                it = cEnd;
            }
          } 
          else
          {
            fNCbc = 0;
            //use a SwapIndex to decide wether to swap a word or not
            //use a WordIndex to pick events apart
            uint32_t cWordIndex = 0;
            uint32_t cSwapIndex = 0;
            // index of the word inside the event (ZS)
            uint32_t fZSEventSize = 0;
            uint32_t cZSWordIndex = 0;

            // to fill fEventList
            std::vector<uint32_t> lvec;
            for ( auto word : pData )
            {
                //if the SwapIndex is greater than 0 and a multiple of the event size in 32 bit words, reset SwapIndex to 0
                if (cSwapIndex > 0 && cSwapIndex % fEventSize == 0) cSwapIndex = 0;

                  #ifdef __CBCDAQ_DEV__
                            //TODO
                            LOG (DEBUG) << std::setw (3) << "Original " << cWordIndex << " ### " << std::bitset<32> (pData.at (cWordIndex) );
                            if ( (cWordIndex + 1) % fEventSize == 0 && cWordIndex > 0 ) LOG (DEBUG) << std::endl << std::endl;
                  #endif
                lvec.push_back ( word );
                if ( cZSWordIndex == fZSEventSize - 1 )
                {
                    //LOG(INFO) << "Packing event # " << fEventList.size() << ", Event size is " << fZSEventSize << " words";
                    if (pType == BoardType::D19C)
                        fEventList.push_back ( new D19cCbc3EventZS ( pBoard, fZSEventSize, lvec ) );
                    lvec.clear();
                    if (fEventList.size() >= fNevents) break;
                }
                else if ( cZSWordIndex == fZSEventSize )
                {
                    // get next event size
                    cZSWordIndex = 0;
                    if (pType == BoardType::D19C) 
                        fZSEventSize = (0x0000FFFF & word);
                    if (fZSEventSize > pData.size() )
                    {
                        LOG (ERROR) << "Missaligned data, not accepted";
                        break;
                    }
                }
                cWordIndex++;
                cSwapIndex++;
                cZSWordIndex++;
            }
          }
        }
    }
  }
}
