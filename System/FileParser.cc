#include "FileParser.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Cic.h"
#include "../HWDescription/RD53.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/OuterTrackerModule.h"

namespace Ph2_System
{
  void FileParser::parseHW(const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, DetectorContainer* pDetectorContainer, std::ostream& os, bool pIsFile)
  {
    //FIXME-FR
    if (pIsFile && pFilename.find ( ".xml" ) != std::string::npos )
      {
        parseHWxml ( pFilename, pBeBoardFWMap, pBoardVector, pDetectorContainer, os, pIsFile );
      }
    else if (!pIsFile)
      {
        parseHWxml ( pFilename, pBeBoardFWMap, pBoardVector, pDetectorContainer, os, pIsFile );
      }
    else
      {
        LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xml!" ;
      }
  }

  void FileParser::parseSettings ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os, bool pIsFile)
  {
    //FIXME-FR
    if ( pIsFile && pFilename.find ( ".xml" ) != std::string::npos )
      parseSettingsxml ( pFilename, pSettingsMap, os, pIsFile );
    else if (!pIsFile)
      parseSettingsxml ( pFilename, pSettingsMap, os, pIsFile );
    else
      LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xm!" ;
  }

  void FileParser::parseHWxml ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, DetectorContainer* pDetectorContainer, std::ostream& os, bool pIsFile )
  {
    // uint32_t cNBeBoard = 0;
    int i, j;

    pugi::xml_document doc;
    pugi::xml_parse_result result;

    if (pIsFile)
      result = doc.load_file ( pFilename.c_str() );
    else
      result = doc.load (pFilename.c_str() );


    if ( !result )
      {
        os << BOLDRED << "ERROR :\n Unable to open the file : " << RESET << pFilename << std::endl;
        os << BOLDRED << "Error description : " << RED << result.description() << RESET << std::endl;

        if (!pIsFile) os << "Error offset: " << result.offset << " (error at [..." << (pFilename.c_str() + result.offset) << "]\n" << std::endl;

        throw Exception ("Unable to parse XML source!");
        return;
      }

    os << RESET << "\n\n";

    for (i = 0; i < 80; i++) os << "*";
    os << "\n";

    for (j = 0; j < 40; j++) os << " ";
    os << BOLDRED << "HW SUMMARY" << RESET << std::endl;

    for (i = 0; i < 80; i++) os << "*";
    os << "\n";

    const std::string strUhalConfig = expandEnvironmentVariables (doc.child ( "HwDescription" ).child ( "Connections" ).attribute ( "name" ).value() );

    // Iterate over the BeBoard Nodes
    for ( pugi::xml_node cBeBoardNode = doc.child ( "HwDescription" ).child ( "BeBoard" ); cBeBoardNode; cBeBoardNode = cBeBoardNode.next_sibling() )
      {
        if (static_cast<std::string> (cBeBoardNode.name() ) == "BeBoard")
          {
            this->parseBeBoard (cBeBoardNode, pBeBoardFWMap, pBoardVector, pDetectorContainer, os);
          }
        // cNBeBoard++;
      }

    for ( i = 0; i < 80; i++ )
      os << "*";

    os << "\n";

    for ( j = 0; j < 40; j++ )
      os << " ";

    os << BOLDRED << "END OF HW SUMMARY" << RESET << std::endl;

    for ( i = 0; i < 80; i++ )
      os << "*";
  }

  void FileParser::parseBeBoard (pugi::xml_node pBeBordNode, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, DetectorContainer* pDetectorContainer, std::ostream& os )
  {


    uint32_t cBeId = pBeBordNode.attribute ( "Id" ).as_int();
    BeBoard* cBeBoard = pDetectorContainer->addBoardContainer(cBeId, new BeBoard ( cBeId ));//FIX Change it to Reference!!!!
    pBoardVector.emplace_back ( cBeBoard );

    pugi::xml_attribute cBoardTypeAttribute = pBeBordNode.attribute ("boardType");

    if (cBoardTypeAttribute == nullptr)
      {
        LOG (ERROR) << BOLDRED << "Error: Board Type not specified - aborting!" << RESET;
        exit (1);
      }

    //std::string cBoardType = pBeBordNode.attribute ( "boardType" ).value();
    std::string cBoardType = cBoardTypeAttribute.value();

    if      (cBoardType == "D19C") cBeBoard->setBoardType (BoardType::D19C);
    else if (cBoardType == "RD53") cBeBoard->setBoardType (BoardType::RD53);
    else
      {
        LOG (ERROR) << "Error: Unknown Board Type: " << cBoardType << " - aborting!";
        std::string errorstring = "Unknown Board Type " + cBoardType;
        throw Exception (errorstring.c_str() );
        exit (1);
      }

    pugi::xml_attribute cEventTypeAttribute = pBeBordNode.attribute ("eventType");
    std::string cEventTypeString;

    if (cEventTypeAttribute == nullptr)
      {
        //the HWDescription object does not have and EventType node, so assume EventType::VR
        cBeBoard->setEventType (EventType::VR);
        cEventTypeString = "VR";
      }
    else
      {
        cEventTypeString = cEventTypeAttribute.value();

        if (cEventTypeString == "ZS") cBeBoard->setEventType (EventType::ZS);
        else cBeBoard->setEventType (EventType::VR);
      }

    os << BOLDBLUE << "|" << "----" << pBeBordNode.name() << "  " << pBeBordNode.first_attribute().name() << ": " << BOLDYELLOW << pBeBordNode.attribute ( "Id" ).value()
       << BOLDBLUE << ", BoardType: " << BOLDYELLOW << cBoardType
       << BOLDBLUE << ", EventType: " << BOLDYELLOW << cEventTypeString
       << RESET << std:: endl;

    pugi::xml_node cBeBoardConnectionNode = pBeBordNode.child ("connection");

    std::string cId = cBeBoardConnectionNode.attribute ( "id" ).value();
    std::string cUri = cBeBoardConnectionNode.attribute ( "uri" ).value();
    std::string cAddressTable = expandEnvironmentVariables (cBeBoardConnectionNode.attribute ( "address_table" ).value() );

    if (cBeBoard->getBoardType() == BoardType::D19C)
      {
        pBeBoardFWMap[cBeBoard->getBeBoardId()] =  new D19cFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
      }
    else if (cBeBoard->getBoardType() == BoardType::RD53)
      {
        pBeBoardFWMap[cBeBoard->getBeBoardId()] = new RD53FWInterface (cId.c_str(), cUri.c_str(), cAddressTable.c_str());
      }

    os << BOLDCYAN << "|" << "       " <<  "|"  << "----" << "Board Id:      " << BOLDYELLOW << cId << std::endl
       << BOLDCYAN << "|" << "       " <<  "|"  << "----" << "URI:           " << BOLDYELLOW << cUri << std::endl
       << BOLDCYAN << "|" << "       " <<  "|"  << "----" << "Address Table: " << BOLDYELLOW << cAddressTable << RESET << std::endl;

    // Iterate over the BeBoardRegister Nodes
    for ( pugi::xml_node cBeBoardRegNode = pBeBordNode.child ( "Register" ); cBeBoardRegNode; cBeBoardRegNode = cBeBoardRegNode.next_sibling() )
      {
        if (std::string (cBeBoardRegNode.name() ) == "Register")
          {
            std::string cNameString;
            uint32_t cValue;
            this->parseRegister (cBeBoardRegNode, cNameString, cValue, cBeBoard, os);
          }
      }

    // Iterate the module node
    for ( pugi::xml_node pModuleNode = pBeBordNode.child ( "Module" ); pModuleNode; pModuleNode = pModuleNode.next_sibling() )
      {
        if ( static_cast<std::string> ( pModuleNode.name() ) == "Module" )
          {
            this->parseModuleContainer (pModuleNode, cBeBoard, os );
          }
      }

    //here parse the Slink Node
    pugi::xml_node cSLinkNode = pBeBordNode.child ("SLink");
    this->parseSLink (cSLinkNode, cBeBoard, os);

    // pBoardVector.emplace_back( cBeBoard );

    return;

  }

  void FileParser::parseRegister (pugi::xml_node pRegisterNode, std::string& pAttributeString, uint32_t& pValue, BeBoard* pBoard, std::ostream& os)
  {
    if (std::string (pRegisterNode.name() ) == "Register")
      {
        if (std::string (pRegisterNode.first_child().value() ).empty() ) // the node has no value associated and thus is just a container
          {
            if (!pAttributeString.empty() ) pAttributeString += ".";

            pAttributeString += pRegisterNode.attribute ("name").value();

            //ok, I have appended .name to the attribute string, now loop over all the children
            for ( pugi::xml_node cNode = pRegisterNode.child ( "Register" ); cNode; cNode = cNode.next_sibling() )
              {
                std::string cAttributeString = pAttributeString;
                this->parseRegister (cNode, cAttributeString, pValue, pBoard, os);
              }
          }
        else // this node has a value and thus is the last down the hierarchy - so I am terminate the attribute string, get the value and thats it
          {
            if (!pAttributeString.empty() ) // this is the first node in the hierarchy and thus no children
              pAttributeString += ".";

            pAttributeString += pRegisterNode.attribute ("name").value();
            pValue = convertAnyInt (pRegisterNode.first_child().value() );
            os << GREEN << "|\t|\t|" << "----" << pAttributeString << ": " << BOLDYELLOW << pValue << RESET << std:: endl;
            pBoard->setReg ( pAttributeString, pValue );
          }
      }

  }

  void FileParser::parseSLink (pugi::xml_node pSLinkNode, BeBoard* pBoard, std::ostream& os )
  {
    ConditionDataSet* cSet = new ConditionDataSet();

    if (pSLinkNode != nullptr && std::string (pSLinkNode.name() ) == "SLink")
      {
        os << BLUE << "|" << "  " << "|" << std::endl << "|" << "   " << "|" << "----" << pSLinkNode.name() << RESET << std::endl;

        pugi::xml_node cDebugModeNode = pSLinkNode.child ("DebugMode");
        std::string cDebugString;

        //the debug mode node exists
        if (cDebugModeNode != nullptr)
          {
            cDebugString = cDebugModeNode.attribute ("type").value();

            if (cDebugString == "FULL" )
              cSet->setDebugMode (SLinkDebugMode::FULL);
            else if (cDebugString == "SUMMARY")
              cSet->setDebugMode (SLinkDebugMode::SUMMARY);
            else if (cDebugString == "ERROR")
              cSet->setDebugMode (SLinkDebugMode::ERROR);

          }
        else
          {
            SLinkDebugMode pMode = cSet->getDebugMode();

            if (pMode == SLinkDebugMode::FULL) cDebugString = "FULL";
            else if (pMode == SLinkDebugMode::SUMMARY) cDebugString = "SUMMARY";
            else if (pMode == SLinkDebugMode::ERROR) cDebugString = "ERROR";
          }

        os << BLUE <<  "|" << " " << "|" << "       " << "|"  << "----"   << pSLinkNode.child ("DebugMode").name() << MAGENTA << " : SLinkDebugMode::" << cDebugString << RESET << std::endl;

        //now loop the condition data node
        for ( pugi::xml_node cNode = pSLinkNode.child ( "ConditionData" ); cNode; cNode = cNode.next_sibling() )
          {
            if (cNode != nullptr)
              {
                uint8_t cUID = 0;
                uint8_t cFeId = 0;
                uint8_t cCbcId = 0;
                uint8_t cPage = 0;
                uint8_t cAddress = 0;
                uint32_t cValue = 0;
                std::string cRegName;

                std::string cTypeString = cNode.attribute ("type").value();

                if (cTypeString == "HV")
                  {
                    cUID = 5;
                    cFeId = convertAnyInt (cNode.attribute ("FeId").value() );
                    cCbcId = convertAnyInt (cNode.attribute ("Sensor").value() );
                    cValue = convertAnyInt (cNode.first_child().value() );
                  }
                else if (cTypeString == "TDC")
                  {
                    cUID = 3;
                    cFeId = 0xFF;
                  }
                else if (cTypeString == "User")
                  {
                    cUID = convertAnyInt (cNode.attribute ("UID").value() );
                    cFeId = convertAnyInt (cNode.attribute ("FeId").value() );
                    cCbcId = convertAnyInt (cNode.attribute ("CbcId").value() );
                    cValue = convertAnyInt (cNode.first_child().value() );
                  }
                else if (cTypeString == "I2C")
                  {
                    //here is where it gets nasty
                    cUID = 1;
                    cRegName = cNode.attribute ("Register").value();
                    cFeId = convertAnyInt (cNode.attribute ("FeId").value() );
                    cCbcId = convertAnyInt (cNode.attribute ("CbcId").value() );

                    //ok, now I need to loop th CBCs to find page & address and the initial value
                    for (auto cFe : pBoard->fModuleVector )
                      {
                        if (cFe->getFeId() != cFeId) continue;

                        for (auto cCbc : cFe->fReadoutChipVector )
                          {
                            if (cCbc->getChipId() != cCbcId) continue;
                            else if (cCbc->getFeId() == cFeId && cCbc->getChipId() == cCbcId)
                              {
                                ChipRegItem cRegItem = cCbc->getRegItem ( cRegName );
                                cPage = cRegItem.fPage;
                                cAddress = cRegItem.fAddress;
                                cValue = cRegItem.fValue;
                              }
                            else
                              LOG (ERROR) << RED << "SLINK ERROR: no Chip with Id " << +cCbcId << " on Fe " << +cFeId << " - check your SLink Settings!";
                          }
                      }
                  }

                cSet->addCondData (cRegName, cUID, cFeId, cCbcId, cPage, cAddress, cValue);
                os << BLUE <<  "|" << " " << "|" << "       " << "|"  << "----"   <<  cNode.name() << ": Type " << RED << cTypeString << " " << cRegName << BLUE << ", UID " << RED << +cUID << BLUE << ", FeId " << RED << +cFeId << BLUE << ", CbcId " << RED << +cCbcId << std::hex << BLUE << ", Page " << RED << +cPage << BLUE << ", Address " << RED << +cAddress << BLUE << ", Value " << std::dec << MAGENTA << cValue << RESET << std::endl;
              }

          }
      }

    pBoard->addConditionDataSet (cSet);
  }

  void FileParser::parseModuleContainer (pugi::xml_node pModuleNode, BeBoard* pBoard, std::ostream& os )
  {

    bool cStatus = pModuleNode.attribute ( "Status" ).as_bool();

    if ( cStatus )
      {
        os << BOLDBLUE << "|" << "       " << "|" << "----" << pModuleNode.name() << "  "
           << BOLDBLUE << pModuleNode.first_attribute().name() << ": " << BOLDYELLOW << pModuleNode.attribute ( "FeId" ).value()
           << BOLDBLUE << ", FMCId: " << BOLDYELLOW << expandEnvironmentVariables (pModuleNode.attribute ("FMCId").value())
           << BOLDBLUE << ", ModuleId: " << BOLDYELLOW << expandEnvironmentVariables (pModuleNode.attribute ("ModuleId").value())
           << BOLDBLUE << ", Status: " << BOLDYELLOW << expandEnvironmentVariables (pModuleNode.attribute ("Status").value())
           << RESET << std:: endl;

        uint32_t cModuleId = pModuleNode.attribute ( "ModuleId" ).as_int();
        Module* cModule;
        if (pBoard->getBoardType() == BoardType::RD53)
          {
            cModule = pBoard->addModuleContainer(cModuleId, new Module(pBoard->getBeBoardId(), pModuleNode.attribute("FMCId").as_int(), pModuleNode.attribute("FeId").as_int(), cModuleId));
          }
        else
          {
            cModule = pBoard->addModuleContainer(cModuleId, new OuterTrackerModule(pBoard->getBeBoardId(), pModuleNode.attribute ( "FMCId" ).as_int(), pModuleNode.attribute ( "FeId" ).as_int(), cModuleId));
          }
        pBoard->addModule ( cModule );

        std::string cConfigFileDirectory;
        for (pugi::xml_node cChild: pModuleNode.children())
          {
            std::string cName = cChild.name();
            std::string cNextName = cChild.next_sibling().name();
            if (cName.find("CBC") != std::string::npos || cName.find("RD53") != std::string::npos || cName.find("CIC") != std::string::npos)
              {
                if(cName.find("_Files") != std::string::npos)
                  {
                    cConfigFileDirectory = expandEnvironmentVariables (static_cast<std::string> ( cChild.attribute ( "path" ).value() ) );
                  }
                else
                  {
                    int cChipId = cChild.attribute("Id").as_int();
                    std::string cFileName = expandEnvironmentVariables (static_cast<std::string> ( cChild.attribute ( "configfile" ).value() ) );
                    LOG (DEBUG) << BOLDBLUE << "Configuration file ... " << cName << " --- " << cConfigFileDirectory << RESET;
                    LOG (DEBUG) << GREEN << cName << " Id = " << +cChipId << " --- " << cFileName << RESET;

                    if (cName == "RD53")
                      {
                        this->parseRD53(cChild, cModule, cConfigFileDirectory, os);
                        if (cNextName.empty() || cNextName != cName)
                          {
                            this->parseGlobalRD53Settings (pModuleNode, cModule, os);
                          }
                      }
                    else if (cName == "CBC")
                      {
                        this->parseCbcContainer  (cChild, cModule, cConfigFileDirectory, os);
                        // check if this is the last node with this name
                        if( cNextName.empty() || cNextName!=cName )
                          {
                            // Parse the GlobalSettings so that Global regisers take precedence over Global settings which take precedence over specific settings
                            this->parseGlobalCbcSettings (pModuleNode, cModule, os);
                          }
                      }
                    else if (cName == "CIC")
                      {
                        if (!cConfigFileDirectory.empty())
                          {
                            if (cConfigFileDirectory.at (cConfigFileDirectory.length() - 1) != '/') cConfigFileDirectory.append ("/");
                            cFileName = cConfigFileDirectory + cFileName;
                          }
                        LOG (INFO) << BOLDBLUE << "Loading configuration for CIC from " << cFileName << RESET;
                        os << BOLDCYAN << "|" << "  " << "|" << "   " << "|" << "----" << cName << "  "
                           << "Id" << cChipId << " , File: " << cFileName << RESET << std::endl;
                        Cic* cCic = new Cic ( cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), cChipId , cFileName );
                        static_cast<OuterTrackerModule*>(cModule)->addCic (cCic);
                        FrontEndType cType = cCic->getFrontEndType();
                        os << GREEN << "|\t|\t|\t|----FrontEndType: ";
                        os << GREEN << "|\t|\t|\t|----FrontEndType: ";
                        if (cType == FrontEndType::CIC)
                          os << RED << "CIC";
                        os << RESET << std::endl;

                      }
                  }
              }
          }
      }
    return;
  }

  void FileParser::parseCbcContainer (pugi::xml_node pCbcNode, Module* cModule, std::string cFilePrefix, std::ostream& os )
  {

    os << BOLDCYAN << "|" << "  " << "|" << "   " << "|" << "----" << pCbcNode.name() << "  "
       << pCbcNode.first_attribute().name() << " :" << pCbcNode.attribute ( "Id" ).value()
       << ", File: " << expandEnvironmentVariables (pCbcNode.attribute ( "configfile" ).value() ) << RESET << std:: endl;

    std::string cFileName;

    if ( !cFilePrefix.empty() )
      {
        if (cFilePrefix.at (cFilePrefix.length() - 1) != '/')
          cFilePrefix.append ("/");

        cFileName = cFilePrefix + expandEnvironmentVariables (pCbcNode.attribute ( "configfile" ).value() );
      }
    else cFileName = expandEnvironmentVariables (pCbcNode.attribute ( "configfile" ).value() );

    //ReadoutChip* cCbc = new Cbc ( cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), pCbcNode.attribute ( "Id" ).as_int(), cFileName );
    //cModule->addReadoutChip (cCbc);
    //ReadoutChip* cCbc = new Cbc ( cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), pCbcNode.attribute ( "Id" ).as_int(), cFileName );

    uint32_t cChipId = pCbcNode.attribute ( "Id" ).as_int();
    ReadoutChip* cCbc = cModule->addChipContainer(cChipId, new Cbc ( cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), cChipId, cFileName ));
    cModule->addReadoutChip (cCbc);
    cCbc->setNumberOfChannels(254);

    // parse the specific CBC settings so that Registers take precedence
    this->parseCbcSettings (pCbcNode, cCbc, os);

    for ( pugi::xml_node cCbcRegisterNode = pCbcNode.child ( "Register" ); cCbcRegisterNode; cCbcRegisterNode = cCbcRegisterNode.next_sibling() )
      {
        cCbc->setReg ( std::string ( cCbcRegisterNode.attribute ( "name" ).value() ), convertAnyInt ( cCbcRegisterNode.first_child().value() ) );
        os << BLUE << "|\t|\t|\t|----Register: " << std::string ( cCbcRegisterNode.attribute ( "name" ).value() ) << " : " << RED << std::hex << "0x" <<  convertAnyInt ( cCbcRegisterNode.first_child().value() ) << RESET << std::dec << std::endl;
      }

  }

  void FileParser::parseGlobalCbcSettings (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os)
  {
    LOG (INFO) << BOLDBLUE << "Now I'm parsing global..." << RESET;
    //use this to parse GlobalCBCRegisters and the Global CBC settings
    //i deliberately pass the Module object so I can loop the CBCs of the Module inside this method
    //this has to be called at the end of the parseCBC() method
    //Global_CBC_Register takes precedence over Global
    pugi::xml_node cGlobalCbcSettingsNode = pModuleNode.child ("Global");

    if (cGlobalCbcSettingsNode != nullptr)
      {
        os << BOLDCYAN << "|\t|\t|----Global CBC Settings: " << RESET <<  std::endl;

        int cCounter = 0;

        for (auto cCbc : pModule->fReadoutChipVector)
          {
            if (cCounter == 0)
              this->parseCbcSettings (cGlobalCbcSettingsNode, cCbc, os);
            else
              {
                std::ofstream cDummy;
                this->parseCbcSettings (cGlobalCbcSettingsNode, cCbc, cDummy);
              }

            cCounter++;
          }
      }

    // now that global has been applied to each CBC, handle the GlobalCBCRegisters
    for ( pugi::xml_node cCbcGlobalNode = pModuleNode.child ( "Global_CBC_Register" ); cCbcGlobalNode != pModuleNode.child ( "CBC" ) && cCbcGlobalNode != pModuleNode.child ( "CBC_Files" ) && cCbcGlobalNode != nullptr; cCbcGlobalNode = cCbcGlobalNode.next_sibling() )
      {
        if ( cCbcGlobalNode != nullptr )
          {
            std::string regname = std::string ( cCbcGlobalNode.attribute ( "name" ).value() );
            uint32_t regvalue = convertAnyInt ( cCbcGlobalNode.first_child().value() ) ;

            for (auto cCbc : pModule->fReadoutChipVector)
              cCbc->setReg ( regname, uint8_t ( regvalue ) ) ;

            os << GREEN << "|" << " " << "|" << "   " << "|" << "----" << cCbcGlobalNode.name()
               << "  " << cCbcGlobalNode.first_attribute().name() << " :"
               << regname << " =  0x" << std::hex << std::setw ( 2 ) << std::setfill ( '0' ) << RED << regvalue << std::dec << RESET << std:: endl;
          }
      }
  }

  void FileParser::parseCbcSettings (pugi::xml_node pCbcNode, ReadoutChip* pCbc, std::ostream& os)
  {
    //parse the cbc settings here and put them in the corresponding registers of the Chip object
    //call this for every CBC, Register nodes should take precedence over specific settings??
    FrontEndType cType = pCbc->getFrontEndType();
    os << GREEN << "|\t|\t|\t|----FrontEndType: ";
    os << GREEN << "|\t|\t|\t|----FrontEndType: ";

    if (cType == FrontEndType::CBC3)
      os << RED << "CBC3";

    os << RESET << std::endl;

    //THRESHOLD & LATENCY
    pugi::xml_node cThresholdNode = pCbcNode.child ( "Settings" );

    if (cThresholdNode != nullptr)
      {
        uint16_t cThreshold = convertAnyInt (cThresholdNode.attribute ("threshold").value() ) ;
        uint16_t cLatency = convertAnyInt (cThresholdNode.attribute ("latency").value() );

        //the moment the cbc object is constructed, it knows which chip type it is
        if (cType == FrontEndType::CBC3)
          {
            pCbc->setReg ("VCth1", (cThreshold & 0x00FF) );
            pCbc->setReg ("VCth2", (cThreshold & 0x0300) >> 8);
            pCbc->setReg ("TriggerLatency1", (cLatency & 0x00FF) );
            uint8_t cLatReadValue = pCbc->getReg ("FeCtrl&TrgLat2") & 0xFE;
            pCbc->setReg ("FeCtrl&TrgLat2", (cLatReadValue | ( (cLatency & 0x0100) >> 8) ) );
          }

        os << GREEN << "|\t|\t|\t|----VCth: " << RED << std::hex << "0x" << cThreshold << std::dec << " (" << cThreshold << ")" << RESET << std::endl;
        os << GREEN << "|\t|\t|\t|----TriggerLatency: " << RED << std::hex << "0x" << cLatency << std::dec << " (" << cLatency << ")" << RESET << std::endl;
      }

    //TEST PULSE
    pugi::xml_node cTPNode = pCbcNode.child ("TestPulse");

    if (cTPNode != nullptr)
      {
        //pugi::xml_node cAmuxNode = pCbcNode.child ("Misc");
        uint8_t cEnable, cPolarity, cGroundOthers;
        uint8_t cAmplitude, cChanGroup, cDelay;

        cEnable = convertAnyInt (cTPNode.attribute ("enable").value() );
        cPolarity = convertAnyInt (cTPNode.attribute ("polarity").value() );
        cAmplitude = convertAnyInt (cTPNode.attribute ("amplitude").value() );
        cChanGroup = convertAnyInt (cTPNode.attribute ("channelgroup").value() );
        cDelay = convertAnyInt (cTPNode.attribute ("delay").value() );
        cGroundOthers = convertAnyInt (cTPNode.attribute ("groundothers").value() );

        if (cType == FrontEndType::CBC3)
          {
            pCbc->setReg ("TestPulsePotNodeSel", cAmplitude );
            pCbc->setReg ("TestPulseDel&ChanGroup", reverseBits ( (cChanGroup & 0x07) << 5 | (cDelay & 0x1F) ) );
            uint8_t cAmuxValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
            pCbc->setReg ("MiscTestPulseCtrl&AnalogMux", ( ( (cPolarity & 0x01) << 7) | ( (cEnable & 0x01) << 6) | ( (cGroundOthers & 0x01) << 5) | (cAmuxValue & 0x1F) ) );
          }

        os << GREEN << "|\t|\t|\t|----TestPulse: " << "enabled: " << RED << +cEnable << GREEN << ", polarity: " << RED << +cPolarity << GREEN << ", amplitude: " << RED << +cAmplitude << GREEN << " (0x" << std::hex << +cAmplitude << std::dec << ")" << RESET << std::endl;
        os << GREEN << "|\t|\t|\t|               channelgroup: " << RED << +cChanGroup << GREEN << ", delay: " << RED << +cDelay << GREEN << ", groundohters: " << RED << +cGroundOthers << RESET << std::endl;
      }


    //CLUSTERS & STUBS
    pugi::xml_node cStubNode = pCbcNode.child ("ClusterStub");

    if (cStubNode != nullptr)
      {
        uint8_t cCluWidth, cPtWidth, cLayerswap, cOffset1, cOffset2, cOffset3, cOffset4;

        cCluWidth = convertAnyInt (cStubNode.attribute ("clusterwidth").value() );
        cPtWidth = convertAnyInt (cStubNode.attribute ("ptwidth").value() );
        cLayerswap = convertAnyInt (cStubNode.attribute ("layerswap").value() );
        cOffset1 = convertAnyInt (cStubNode.attribute ("off1").value() );
        cOffset2 = convertAnyInt (cStubNode.attribute ("off2").value() );
        cOffset3 = convertAnyInt (cStubNode.attribute ("off3").value() );
        cOffset4 = convertAnyInt (cStubNode.attribute ("off4").value() );

        if (cType == FrontEndType::CBC3)
          {
            uint8_t cLogicSel = pCbc->getReg ("Pipe&StubInpSel&Ptwidth");
            pCbc->setReg ("Pipe&StubInpSel&Ptwidth", ( (cLogicSel & 0xF0) | (cPtWidth & 0x0F) ) );
            pCbc->setReg ("LayerSwap&CluWidth", ( ( (cLayerswap & 0x01) << 3) | (cCluWidth & 0x07) ) );
            pCbc->setReg ("CoincWind&Offset34", ( ( (cOffset4 & 0x0F) << 4) | (cOffset3 & 0x0F) ) );
            pCbc->setReg ("CoincWind&Offset12", ( ( (cOffset2 & 0x0F) << 4) | (cOffset1 & 0x0F) ) );

            os << GREEN << "|\t|\t|\t|----Cluster & Stub Logic: " << "ClusterWidthDiscrimination: " << RED << +cCluWidth << GREEN << ", PtWidth: " << RED << +cPtWidth << GREEN << ", Layerswap: " << RED << +cLayerswap << RESET << std::endl;
            os << GREEN << "|\t|\t|\t|                          Offset1: " << RED << +cOffset1 << GREEN << ", Offset2: " << RED << +cOffset2 << GREEN << ", Offset3: " << RED << +cOffset3 << GREEN << ", Offset4: " << RED << +cOffset4 << RESET << std::endl;
          }
      }

    //MISC
    pugi::xml_node cMiscNode = pCbcNode.child ("Misc");

    if (cMiscNode != nullptr)
      {
        uint8_t cPipeLogic, cStubLogic, cOr254, cTestClock, cTpgClock, cDll;
        uint8_t cAmuxValue;

        cPipeLogic = convertAnyInt (cMiscNode.attribute ("pipelogic").value() );
        cStubLogic = convertAnyInt (cMiscNode.attribute ("stublogic").value() );
        cOr254 = convertAnyInt (cMiscNode.attribute ("or254").value() );
        cDll = reverseBits (static_cast<uint8_t> (convertAnyInt (cMiscNode.attribute ("dll").value() ) ) & 0x1F ) >> 3;
        //LOG (DEBUG) << convertAnyInt (cMiscNode.attribute ("dll").value() ) << " " << +cDll << " " << std::bitset<5> (cDll);
        cTpgClock = convertAnyInt (cMiscNode.attribute ("tpgclock").value() );
        cTestClock = convertAnyInt (cMiscNode.attribute ("testclock").value() );
        cAmuxValue = convertAnyInt (cMiscNode.attribute ("analogmux").value() );

        if (cType == FrontEndType::CBC3)
          {
            pCbc->setReg ("40MhzClk&Or254", ( ( (cTpgClock & 0x01) << 7) | ( (cOr254 & 0x01) << 6) | (cTestClock & 0x01) << 5 | (cDll & 0x1F) ) );
            //LOG (DEBUG) << BOLDRED << std::bitset<8> (pCbc->getReg ("40MhzClk&Or254") ) << RESET;
            uint8_t cPtWidthRead = pCbc->getReg ("Pipe&StubInpSel&Ptwidth");
            pCbc->setReg ("Pipe&StubInpSel&Ptwidth", ( ( (cPipeLogic & 0x03) << 6) | ( (cStubLogic & 0x03) << 4) | (cPtWidthRead & 0x0F) ) );

            uint8_t cAmuxRead = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
            pCbc->setReg ("MiscTestPulseCtrl&AnalogMux", ( (cAmuxRead & 0xE0) | (cAmuxValue & 0x1F) ) );

            os << GREEN << "|\t|\t|\t|----Misc Settings: " << " PipelineLogicSource: " << RED << +cPipeLogic << GREEN << ", StubLogicSource: " << RED << +cStubLogic << GREEN << ", OR254: " << RED << +cOr254 << GREEN << ", TPG Clock: " << RED << +cTpgClock << GREEN  << ", Test Clock 40: " << RED << +cTestClock << GREEN << ", DLL: " << RED << convertAnyInt (cMiscNode.attribute ("dll").value() ) << RESET << std::endl;
          }

        os << GREEN << "|\t|\t|\t|----Analog Mux " << "value: " << RED << +cAmuxValue << " (0x" << std::hex << +cAmuxValue << std::dec << ", 0b" << std::bitset<5> (cAmuxValue) << ")" << RESET << std::endl;
      }


    // CHANNEL MASK
    pugi::xml_node cDisableNode = pCbcNode.child ("ChannelMask");

    if (cDisableNode != nullptr)
      {
        std::string cList = std::string (cDisableNode.attribute ("disable").value() );
        std::string ctoken;
        std::stringstream cStr (cList);
        os << GREEN << "|\t|\t|\t|----List of disabled Channels: ";

        //std::vector<uint8_t> cDisableVec;

        int cIndex = 0;

        while (std::getline (cStr, ctoken, ',') )
          {
            if (cIndex != 0) os << GREEN << ", ";

            uint8_t cChannel = convertAnyInt (ctoken.c_str() );
            //cDisableVec.push_back (cChannel);

            if (cChannel == 0 || cChannel > 254) LOG (ERROR) << "Error: channels for mask have to be between 1 and 254!";
            else
              {
                //get the reigister string name from the map in Definition.h
                uint8_t cRegisterIndex = (cChannel - 1) / 8;
                //get the index of the bit to shift
                uint8_t cBitShift = (cChannel - 1) % 8;
                //get the original value of the register
                uint8_t cReadValue;

                if (cType == FrontEndType::CBC3)
                  {
                    //get the original value of the register
                    cReadValue = pCbc->getReg (ChannelMaskMapCBC3[cRegisterIndex]);
                    //clear bit cBitShift
                    cReadValue &= ~ (1 << cBitShift);
                    //write the new value
                    pCbc->setReg (ChannelMaskMapCBC3[cRegisterIndex], cReadValue);
                    LOG (DEBUG) << ChannelMaskMapCBC3[cRegisterIndex] << " " << std::bitset<8> (cReadValue);
                  }

                os << BOLDCYAN <<  +cChannel;
              }

            cIndex++;
          }

        os << RESET << std::endl;
      }
  }

  void FileParser::parseSettingsxml ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os, bool pIsFile)
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result;

    if (pIsFile)
      result = doc.load_file ( pFilename.c_str() );
    else
      result = doc.load (pFilename.c_str() );


    if ( !result )
      {
        os << BOLDRED << "ERROR :\n Unable to open the file : " << RESET << pFilename << std::endl;
        os << BOLDRED << "Error description : " << RED << result.description() << RESET << std::endl;

        if (!pIsFile) os << "Error offset: " << result.offset << " (error at [..." << (pFilename.c_str() + result.offset) << "]\n" << std::endl;

        throw Exception ("Unable to parse XML source!");
        return;
      }

    for ( pugi::xml_node nSettings = doc.child ( "HwDescription" ).child ("Settings"); nSettings; nSettings = nSettings.next_sibling() )
      // for ( pugi::xml_node nSettings = doc.child ("Settings"); nSettings; nSettings = nSettings.next_sibling() )
      {
        os << "\n" << std::endl;

        for ( pugi::xml_node nSetting = nSettings.child ( "Setting" ); nSetting; nSetting = nSetting.next_sibling() )
          {
            pSettingsMap[nSetting.attribute ( "name" ).value()] = convertAnyDouble ( nSetting.first_child().value() );
            os << BOLDRED << "Setting" << RESET << " -- " << BOLDCYAN << nSetting.attribute ( "name" ).value() << RESET << ":" << BOLDYELLOW << convertAnyDouble ( nSetting.first_child().value() ) << RESET << std::endl;
          }
      }
  }


  // ########################
  // # RD53 specific parser #
  // ########################
  void FileParser::parseRD53 (pugi::xml_node theChipNode, Module* cModule, std::string cFilePrefix, std::ostream& os)
  {
    os << BOLDBLUE   << "|\t|\t|----" << theChipNode.name()               << "  Id: "
       << BOLDYELLOW << theChipNode.attribute("Id").value()   << BOLDBLUE << ",  Lane: "
       << BOLDYELLOW << theChipNode.attribute("Lane").value() << BOLDBLUE << ",  File: "
       << BOLDYELLOW << expandEnvironmentVariables (theChipNode.attribute ("configfile").value() ) << RESET << std::endl;

    std::string cFileName;

    if (!cFilePrefix.empty())
      {
        if (cFilePrefix.at (cFilePrefix.length() - 1) != '/') cFilePrefix.append ("/");
        cFileName = cFilePrefix + expandEnvironmentVariables (theChipNode.attribute ("configfile").value());
      }
    else cFileName = expandEnvironmentVariables (theChipNode.attribute ("configfile").value());

    uint32_t chipId      = theChipNode.attribute("Id").as_int();
    uint32_t chipLane    = theChipNode.attribute("Lane").as_int();
    ReadoutChip* theChip = cModule->addChipContainer(chipId, new RD53 (cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), chipId, chipLane, cFileName));
    theChip->setNumberOfChannels(RD53::nRows,RD53::nCols);

    this->parseRD53Settings (theChipNode, theChip, os);

    cModule->addReadoutChip(theChip);
  }

  void FileParser::parseGlobalRD53Settings (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os)
  {
    pugi::xml_node cGlobalChipSettings = pModuleNode.child("Global");
    if (cGlobalChipSettings != nullptr)
      {
        os << BOLDCYAN << "|\t|\t|----Global RD53 Settings:" << RESET << std::endl;

        for (const pugi::xml_attribute& attr : cGlobalChipSettings.attributes())
          {
            std::string regname = attr.name();
            uint16_t regvalue   = convertAnyInt (attr.value());
            os << GREEN << "|\t|\t|\t|----" << regname << ": " << BOLDYELLOW << std::hex << "0x" << regvalue << std::dec << " (" << regvalue << ")" << RESET << std::endl;

            for (auto theChip : pModule->fReadoutChipVector) theChip->setReg(regname,regvalue,true);
          }
      }
  }

  void FileParser::parseRD53Settings (pugi::xml_node theChipNode, ReadoutChip* theChip, std::ostream& os)
  {
    pugi::xml_node cLocalChipSettings = theChipNode.child("Settings");
    if (cLocalChipSettings != nullptr)
      {
        os << BOLDCYAN << "|\t|\t|----FrontEndType: " << BOLDYELLOW << "RD53" << RESET << std::endl;

        for (const pugi::xml_attribute& attr : cLocalChipSettings.attributes())
          {
            std::string regname = attr.name();
            uint16_t regvalue   = convertAnyInt(attr.value());
            theChip->setReg(regname,regvalue,true);
            os << GREEN << "|\t|\t|\t|----" << regname << ": " << BOLDYELLOW << std::hex << "0x" << regvalue << std::dec << " (" << regvalue << ")" << RESET << std::endl;
          }
      }
  }
  // ########################
}
