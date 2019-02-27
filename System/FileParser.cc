#include "FileParser.h"


namespace Ph2_System {

    void FileParser::parseHW ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os, bool pIsFile )
    {

        //FIXME-FR
        if (pIsFile && pFilename.find ( ".xml" ) != std::string::npos )
            parseHWxml ( pFilename, pBeBoardFWMap, pBoardVector, os, pIsFile );
        else if (!pIsFile)
            parseHWxml ( pFilename, pBeBoardFWMap, pBoardVector, os, pIsFile );
        else
            LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xml!" ;
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

    void FileParser::parseHWxml ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os, bool pIsFile )
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

        os << "\n\n\n";

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";

        for ( j = 0; j < 40; j++ )
            os << " ";

        os << BOLDRED << "HW SUMMARY: " << RESET << std::endl;

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";
        os << "\n";
        const std::string strUhalConfig = expandEnvironmentVariables (doc.child ( "HwDescription" ).child ( "Connections" ).attribute ( "name" ).value() );

        // Iterate over the BeBoard Nodes
        for ( pugi::xml_node cBeBoardNode = doc.child ( "HwDescription" ).child ( "BeBoard" ); cBeBoardNode; cBeBoardNode = cBeBoardNode.next_sibling() )
        {
            if (static_cast<std::string> (cBeBoardNode.name() ) == "BeBoard")
            {
                this->parseBeBoard (cBeBoardNode, pBeBoardFWMap, pBoardVector, os);
            }
            // cNBeBoard++;
        }


        os << "\n";
        os << "\n";

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";

        for ( j = 0; j < 40; j++ )
            os << " ";

        os << BOLDRED << "END OF HW SUMMARY: " << RESET << std::endl;

        for ( i = 0; i < 80; i++ )
            os << "*";

        os << "\n";
        os << "\n";
    }

    void FileParser::parseBeBoard (pugi::xml_node pBeBordNode, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector,  std::ostream& os)
    {



        uint32_t cBeId = pBeBordNode.attribute ( "Id" ).as_int();
        BeBoard* cBeBoard = new BeBoard ( cBeId );

        pugi::xml_attribute cBoardTypeAttribute = pBeBordNode.attribute ("boardType");

        if (cBoardTypeAttribute == nullptr)
        {
            LOG (ERROR) << BOLDRED << "Error: Board Type not specified - aborting!" << RESET;
            exit (1);
        }

        //std::string cBoardType = pBeBordNode.attribute ( "boardType" ).value();
        std::string cBoardType = cBoardTypeAttribute.value();

        if (cBoardType == "GLIB")         cBeBoard->setBoardType (BoardType::GLIB);
        else if (cBoardType == "CTA")     cBeBoard->setBoardType (BoardType::CTA);
        else if (cBoardType == "ICGLIB")  cBeBoard->setBoardType (BoardType::ICGLIB);
        else if (cBoardType == "ICFC7")   cBeBoard->setBoardType (BoardType::ICFC7);
        else if (cBoardType == "CBC3FC7") cBeBoard->setBoardType (BoardType::CBC3FC7);
        else if (cBoardType == "D19C")    cBeBoard->setBoardType (BoardType::D19C);
        else if (cBoardType == "MPAGLIB") cBeBoard->setBoardType (BoardType::MPAGLIB);
    	else if (cBoardType == "FC7")     cBeBoard->setBoardType (BoardType::FC7);
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

        os << BOLDCYAN << "|" << "----" << pBeBordNode.name() << "  " << pBeBordNode.first_attribute().name() << " :" << BOLDBLUE << pBeBordNode.attribute ( "Id" ).value() << BOLDCYAN << " BoardType: " << BOLDBLUE << cBoardType << BOLDCYAN << " EventType: " << BOLDRED << cEventTypeString << RESET << std:: endl;

        pugi::xml_node cBeBoardConnectionNode = pBeBordNode.child ("connection");

        std::string cId = cBeBoardConnectionNode.attribute ( "id" ).value();
        std::string cUri = cBeBoardConnectionNode.attribute ( "uri" ).value();
        std::string cAddressTable = expandEnvironmentVariables (cBeBoardConnectionNode.attribute ( "address_table" ).value() );

        if (cBeBoard->getBoardType() == BoardType::D19C)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new D19cFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
        else if (cBeBoard->getBoardType() == BoardType::FC7)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()]   =  new FC7FWInterface (cId.c_str(), cUri.c_str(), cAddressTable.c_str());
        /*else if (cBeBoard->getBoardType() == BoardType::GLIB)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new GlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
        else if (cBeBoard->getBoardType() == BoardType::ICGLIB)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new ICGlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
        else if (cBeBoard->getBoardType() == BoardType::CTA)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new CtaFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
        else if (cBeBoard->getBoardType() == BoardType::ICFC7)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new ICFc7FWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
        else if (cBeBoard->getBoardType() == BoardType::CBC3FC7)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new Cbc3Fc7FWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
        else if (cBeBoard->getBoardType() == BoardType::MPAGLIB)
            pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new MPAGlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
    */

        os << BOLDBLUE << "|" << "       " <<  "|"  << "----" << "Board Id:      " << BOLDYELLOW << cId << std::endl << BOLDBLUE <<  "|" << "       " <<  "|"  << "----" << "URI:           " << BOLDYELLOW << cUri << std::endl << BOLDBLUE <<  "|" << "       " <<  "|"  << "----" << "Address Table: " << BOLDYELLOW << cAddressTable << std::endl << BOLDBLUE << "|" << "       " <<  "|" << RESET << std::endl;

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

        os << BLUE <<  "|\t|" << RESET << std::endl;

        // Iterate the module node
        for ( pugi::xml_node pModuleNode = pBeBordNode.child ( "Module" ); pModuleNode; pModuleNode = pModuleNode.next_sibling() )
        {
            if ( static_cast<std::string> ( pModuleNode.name() ) == "Module" )
            {
                this->parseModule (pModuleNode, cBeBoard, os );
            }
        }

        //here parse the Slink Node
        pugi::xml_node cSLinkNode = pBeBordNode.child ("SLink");
        this->parseSLink (cSLinkNode, cBeBoard, os);

        pBoardVector.push_back ( cBeBoard );

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
                os << BLUE << "|" << "\t" << "|" << "----" << pRegisterNode.name() << "  " << pAttributeString << ": " << BOLDYELLOW << pValue << RESET << std:: endl;
                pBoard->setReg ( pAttributeString, pValue );
            }
        }

    }

    void FileParser::parseSLink (pugi::xml_node pSLinkNode, BeBoard* pBoard, std::ostream& os )
    {
        ConditionDataSet* cSet = new ConditionDataSet();

        if (pSLinkNode != nullptr && std::string (pSLinkNode.name() ) == "SLink")
        {
            os << BLUE << "|" << "	" << "|" << std::endl << "|" << "	" << "|" << "----" << pSLinkNode.name() << RESET << std::endl;

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

            os << BLUE <<  "|" << "	" << "|" << "       " << "|"  << "----"   << pSLinkNode.child ("DebugMode").name() << MAGENTA << " : SLinkDebugMode::" << cDebugString << RESET << std::endl;

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

                            for (auto cCbc : cFe->fChipVector )
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
                    os << BLUE <<  "|" << "	" << "|" << "       " << "|"  << "----"   <<  cNode.name() << ": Type " << RED << cTypeString << " " << cRegName << BLUE << ", UID " << RED << +cUID << BLUE << ", FeId " << RED << +cFeId << BLUE << ", CbcId " << RED << +cCbcId << std::hex << BLUE << ", Page " << RED << +cPage << BLUE << ", Address " << RED << +cAddress << BLUE << ", Value " << std::dec << MAGENTA << cValue << RESET << std::endl;
                }

            }

            //only add if there is condition data defined
            //pBoard->addConditionDataSet (cSet);
        }

        //else
        //{

        //}

        //LOG (ERROR) << "No Slink node found for Board " << +pBoard->getBeId() << " - continuing with default debug mode!";
        //add ConditionDataSet to pBoard in any case, even if there is no SLink node in the xml, that way at least
        //an SLinkDebugMode property is set for this board (SUMMARY)
        pBoard->addConditionDataSet (cSet);
    }


    void FileParser::parseModule (pugi::xml_node pModuleNode, BeBoard* pBoard, std::ostream& os )
    {

        bool cStatus = pModuleNode.attribute ( "Status" ).as_bool();

        //LOG(INFO) << cStatus ;
        if ( cStatus )
        {
            os << BOLDCYAN << "|" << "  " << "|" << "----" << pModuleNode.name() << "  "
               << pModuleNode.first_attribute().name() << " :" << pModuleNode.attribute ( "ModuleId" ).value() << RESET << std:: endl;

            uint32_t cModuleId = pModuleNode.attribute ( "ModuleId" ).as_int();

            Module* cModule = new Module ( pBoard->getBeBoardIdentifier(), pModuleNode.attribute ( "FMCId" ).as_int(), pModuleNode.attribute ( "FeId" ).as_int(), cModuleId );
            pBoard->addModule ( cModule );

            pugi::xml_node cChipPathPrefixNode;
            // Iterate the CBC node
            if (pBoard->getBoardType() == BoardType::FC7)
                cChipPathPrefixNode = pModuleNode.child ( "RD53_Files" );
            else
                cChipPathPrefixNode = pModuleNode.child ( "CBC_Files" );

            std::string cFilePrefix = expandEnvironmentVariables (static_cast<std::string> ( cChipPathPrefixNode.attribute ( "path" ).value() ) );

            if ( !cFilePrefix.empty() ) os << GREEN << "|" << " " << "|" << "   " << "|" << "----" << "Chip Files Path : " << cFilePrefix << RESET << std::endl;

            // Iterate the Chip node
            if (pBoard->getBoardType() == BoardType::FC7)
            {
                for (pugi::xml_node pRD53Node = pModuleNode.child ("RD53"); pRD53Node, pRD53Node.name() == std::string("RD53"); pRD53Node = pRD53Node.next_sibling())
                  this->parseRD53 (pRD53Node, cModule, cFilePrefix, os);
        
                // Parse the GlobalSettings so that Global regisers take precedence over Global settings which take precedence over specific settings
                this->parseGlobalRD53Settings (pModuleNode, cModule, os);
	      }
            else
	      {
                for ( pugi::xml_node pCbcNode = pModuleNode.child ( "CBC" ); pCbcNode; pCbcNode = pCbcNode.next_sibling() )
		  this->parseCbc  (pCbcNode, cModule, cFilePrefix, os);
		
                // parse the GlobalCbcSettings so that Global CBC regisers take precedence over Global CBC settings which take precedence over CBC specific settings
                this->parseGlobalCbcSettings (pModuleNode, cModule, os);
	      }
        }
	
        return;
    }

    void FileParser::parseCbc (pugi::xml_node pCbcNode, Module* cModule, std::string cFilePrefix, std::ostream& os )
    {

        os << BOLDCYAN << "|" << "	" << "|" << "	" << "|" << "----" << pCbcNode.name() << "  "
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

        Chip* cCbc = new Chip ( cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), pCbcNode.attribute ( "Id" ).as_int(), cFileName );

        // parse the specific CBC settings so that Registers take precedence
        this->parseCbcSettings (pCbcNode, cCbc, os);

        for ( pugi::xml_node cCbcRegisterNode = pCbcNode.child ( "Register" ); cCbcRegisterNode; cCbcRegisterNode = cCbcRegisterNode.next_sibling() )
        {
            cCbc->setReg ( std::string ( cCbcRegisterNode.attribute ( "name" ).value() ), convertAnyInt ( cCbcRegisterNode.first_child().value() ) );
            os << BLUE << "|\t|\t|\t|----Register: " << std::string ( cCbcRegisterNode.attribute ( "name" ).value() ) << " : " << RED << std::hex << "0x" <<  convertAnyInt ( cCbcRegisterNode.first_child().value() ) << RESET << std::dec << std::endl;
        }

        cModule->addChip (cCbc);
    }


    void FileParser::parseGlobalCbcSettings (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os)
    {
        //use this to parse GlobalCBCRegisters and the Global CBC settings
        //i deliberately pass the Module object so I can loop the CBCs of the Module inside this method
        //this has to be called at the end of the parseCBC() method
        //Global_CBC_Register takes precedence over Global
        pugi::xml_node cGlobalCbcSettingsNode = pModuleNode.child ("Global");

        if (cGlobalCbcSettingsNode != nullptr)
        {
            os << BOLDCYAN << "|\t|\t|----Global CBC Settings: " << RESET <<  std::endl;

            int cCounter = 0;

            for (auto cCbc : pModule->fChipVector)
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

                for (auto cCbc : pModule->fChipVector)
                    cCbc->setReg ( regname, uint8_t ( regvalue ) ) ;

                os << BOLDGREEN << "|" << "	" << "|" << "	" << "|" << "----" << cCbcGlobalNode.name()
                   << "  " << cCbcGlobalNode.first_attribute().name() << " :"
                   << regname << " =  0x" << std::hex << std::setw ( 2 ) << std::setfill ( '0' ) << RED << regvalue << std::dec << RESET << std:: endl;
            }
        }
    }

    void FileParser::parseCbcSettings (pugi::xml_node pCbcNode, Chip* pCbc, std::ostream& os)
    {
        //parse the cbc settings here and put them in the corresponding registers of the Chip object
        //call this for every CBC, Register nodes should take precedence over specific settings??
        ChipType cType = pCbc->getChipType();
        os << GREEN << "|\t|\t|\t|----ChipType: ";
        os << GREEN << "|\t|\t|\t|----ChipType: ";

        if (cType == ChipType::CBC2)
            os << RED << "CBC2";
        else if (cType == ChipType::CBC3)
            os << RED << "CBC3";

        os << RESET << std::endl;

        //THRESHOLD & LATENCY
        pugi::xml_node cThresholdNode = pCbcNode.child ( "Settings" );

        if (cThresholdNode != nullptr)
        {
            uint16_t cThreshold = convertAnyInt (cThresholdNode.attribute ("threshold").value() ) ;
            uint16_t cLatency = convertAnyInt (cThresholdNode.attribute ("latency").value() );

            //the moment the cbc object is constructed, it knows which chip type it is
            if (cType == ChipType::CBC2)
            {
                pCbc->setReg ("VCth", uint8_t (cThreshold) );
                pCbc->setReg ("TriggerLatency", uint8_t (cLatency) );
            }
            else if (cType == ChipType::CBC3)
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

            if (cType == ChipType::CBC2)
            {
                pCbc->setReg ("TestPulsePot", cAmplitude );
                pCbc->setReg ("SelTestPulseDel&ChanGroup", reverseBits ( (cChanGroup & 0x07) << 5 | (cDelay & 0x1F) ) );
                uint8_t cAmuxValue = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
                pCbc->setReg ("MiscTestPulseCtrl&AnalogMux", ( ( (cPolarity & 0x01) << 7) | ( (cEnable & 0x01) << 6) | ( (cGroundOthers & 0x01) << 5) | (cAmuxValue & 0x1F) ) );

            }
            else if (cType == ChipType::CBC3)
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

            if (cType == ChipType::CBC2)
            {
                pCbc->setReg ("CwdWindow&Coincid", ( ( (cCluWidth & 0x03) << 6) | (cOffset1 & 0x3F) ) );
                uint8_t cMiscStubLogic = pCbc->getReg ("MiscStubLogic");
                pCbc->setReg ("MiscStubLogic", ( (cPtWidth & 0x0F) << 4 | (cMiscStubLogic & 0x0F) ) );

                os << GREEN << "Cluster & Stub Logic: " << " ClusterWidthDiscrimination: " << RED << +cCluWidth << GREEN << ", PtWidth: " << RED << +cPtWidth << GREEN << ", Offset: " << RED << +cOffset1 << RESET << std::endl;
            }
            else if (cType == ChipType::CBC3)
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

            if (cType == ChipType::CBC2)
            {
                uint8_t cAmuxRead = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
                pCbc->setReg ("MiscTestPulseCtrl&AnalogMux", ( (cAmuxRead & 0xE0) | (cAmuxValue & 0x1F) ) );
                os << RED << "|\t|\t|\t|----Other settengs than Amux curerntly not supported for CBC2, please set manually!" << RESET << std::endl;

            }
            else if (cType == ChipType::CBC3)
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

                    if (cType == ChipType::CBC2)
                    {
                        //get the original value of the register
                        cReadValue = pCbc->getReg (ChannelMaskMapCBC2[cRegisterIndex]);
                        //clear bit cBitShift
                        cReadValue &= ~ (1 << cBitShift);
                        //write the new value
                        pCbc->setReg (ChannelMaskMapCBC2[cRegisterIndex], cReadValue);
                        LOG (DEBUG) << ChannelMaskMapCBC2[cRegisterIndex] << " " << std::bitset<8> (cReadValue);
                    }
                    else if (cType == ChipType::CBC3)
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
        {
            os << std::endl;

            for ( pugi::xml_node nSetting = nSettings.child ( "Setting" ); nSetting; nSetting = nSetting.next_sibling() )
            {
                pSettingsMap[nSetting.attribute ( "name" ).value()] = convertAnyInt ( nSetting.first_child().value() );
                os <<  RED << "Setting" << RESET << " --" << BOLDCYAN << nSetting.attribute ( "name" ).value() << RESET << ":" << BOLDYELLOW << convertAnyInt ( nSetting.first_child().value() ) << RESET << std:: endl;
            }
        }
    }


  // ########################
  // # RD53 specific parser #
  // ########################
  void FileParser::parseRD53 (pugi::xml_node pRD53Node, Module* cModule, std::string cFilePrefix, std::ostream& os)
  {
    os << BOLDCYAN << "|" << "	" << "|" << "	" << "|" << "----" << pRD53Node.name() << "  "
       << pRD53Node.first_attribute().name() << ": " << pRD53Node.attribute ("Id").value()
       << ", File: " << expandEnvironmentVariables (pRD53Node.attribute ("configfile").value() ) << RESET << std::endl;

    std::string cFileName;
	    
    if (!cFilePrefix.empty())
      {
	if (cFilePrefix.at (cFilePrefix.length() - 1) != '/')
	  cFilePrefix.append ("/");
		
	cFileName = cFilePrefix + expandEnvironmentVariables (pRD53Node.attribute ("configfile").value());
      }
    else cFileName = expandEnvironmentVariables (pRD53Node.attribute ("configfile").value());

    std::cout<<cFileName<<std::endl;
        
    RD53* cRD53 = new RD53 ( cModule->getBeId(), cModule->getFMCId(), cModule->getFeId(), pRD53Node.attribute ( "Id" ).as_int(), cFileName );
std::cout<<cModule->getBeId()<<" " <<cModule->getFMCId()<<" " <<cModule->getFeId()<<" " <<pRD53Node.attribute ( "Id" ).as_int()<<" " <<cFileName <<std::endl;
        
    // Parse the specific RD53 settings so that Registers take precedence
    this->parseRD53Settings (pRD53Node, cRD53, os);
        
    for (pugi::xml_node cRD53RegisterNode = pRD53Node.child ("Register"); cRD53RegisterNode; cRD53RegisterNode = cRD53RegisterNode.next_sibling() )
      {
    cRD53->setReg (std::string(cRD53RegisterNode.attribute ("name").value() ), convertAnyInt (cRD53RegisterNode.first_child().value()));
    os << BLUE << "|\t|\t|\t|----Register: " << std::string (cRD53RegisterNode.attribute ("name").value()) << " : " << RED << std::hex << "0x" << convertAnyInt (cRD53RegisterNode.first_child().value()) << RESET << std::dec << std::endl;
      }

    cModule->addRD53 (cRD53);
  }

  void FileParser::parseGlobalRD53Settings (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os)
  {
    pugi::xml_node cGlobalRD53SettingsNode = pModuleNode.child ("Global");
    
    if (cGlobalRD53SettingsNode != nullptr)
      {
	os << BOLDCYAN << "|\t|\t|----Global RD53 Settings:" << RESET <<  std::endl;
	
	int cCounter = 0;
	
	for (auto cRD53 : pModule->fRD53Vector)
	  {
	    if (cCounter == 0)
	      this->parseRD53Settings (cGlobalRD53SettingsNode, cRD53, os);
	    else
	      {
		std::ofstream cDummy;
		this->parseRD53Settings (cGlobalRD53SettingsNode, cRD53, cDummy);
	      }
	    
	    cCounter++;
	  }
      }
    
    // Now that global has been applied to each RD53, handle the GlobalRD53Registers
    for (pugi::xml_node cRD53GlobalNode = pModuleNode.child ("Global_RD53_Register"); cRD53GlobalNode != pModuleNode.child ("RD53") && cRD53GlobalNode != pModuleNode.child ("RD53_Files") && cRD53GlobalNode != nullptr; cRD53GlobalNode = cRD53GlobalNode.next_sibling() )
      {
	if (cRD53GlobalNode != nullptr)
	  {
	    std::string regname = std::string ( cRD53GlobalNode.attribute ( "name" ).value() );
	    uint32_t regvalue = convertAnyInt ( cRD53GlobalNode.first_child().value() ) ;
	    
	    for (auto cRD53 : pModule->fRD53Vector)
	      cRD53->setReg (regname, uint8_t (regvalue)) ;
	    
	    os << BOLDGREEN << "|" << "	" << "|" << "	" << "|" << "----" << cRD53GlobalNode.name()
	       << "  " << cRD53GlobalNode.first_attribute().name() << " :"
	       << regname << " =  0x" << std::hex << std::setw (2) << std::setfill ('0') << RED << regvalue << std::dec << RESET << std:: endl;
	  }
      }
  }
 
  void FileParser::parseRD53Settings (pugi::xml_node pRD53Node, RD53* pRD53, std::ostream& os)
  {
    // Parse the RD53 settings here and put them in the corresponding registers of the RD53 object
    os << BOLDBLUE << "|\t|\t|\t|----ChipType: " << RED << "RD53" << RESET << std::endl;

    pugi::xml_node cSettingsChild = pRD53Node.child ("Settings");
    if (cSettingsChild != nullptr)
      {
	uint16_t value = convertAnyInt (cSettingsChild.attribute ("GP_LVDS_ROUTE").value());

    pRD53->setReg("GP_LVDS_ROUTE",value,true);
	os << GREEN << "|\t|\t|\t|----GP_LVDS_ROUTE: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("VTH_SYNC").value());
	pRD53->setReg("VTH_SYNC",value,true);
	os << GREEN << "|\t|\t|\t|----VTH_SYNC: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("Vthreshold_LIN").value());
	pRD53->setReg("Vthreshold_LIN",value,true);
	os << GREEN << "|\t|\t|\t|----Vthreshold_LIN: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("VTH1_DIFF").value());
	pRD53->setReg("VTH1_DIFF",value,true);
	os << GREEN << "|\t|\t|\t|----VTH1_DIFF: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("VTH2_DIFF").value());
	pRD53->setReg("VTH2_DIFF",value,true);
	os << GREEN << "|\t|\t|\t|----VTH2_DIFF: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("LATENCY_CONFIG").value());
	pRD53->setReg("LATENCY_CONFIG",value,true);
	os << GREEN << "|\t|\t|\t|----LATENCY_CONFIG: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("MONITOR_SELECT").value());
	pRD53->setReg("MONITOR_SELECT",value,true);
	os << GREEN << "|\t|\t|\t|----MONITOR_SELECT: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("EN_CORE_COL_SYNC").value());
	pRD53->setReg("EN_CORE_COL_SYNC",value,true);
	os << GREEN << "|\t|\t|\t|----EN_CORE_COL_SYNC: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("EN_CORE_COL_LIN_1").value());
	pRD53->setReg("EN_CORE_COL_LIN_1",value,true);
	os << GREEN << "|\t|\t|\t|----EN_CORE_COL_LIN_1: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("EN_CORE_COL_LIN_2").value());
	pRD53->setReg("EN_CORE_COL_LIN_2",value,true);
	os << GREEN << "|\t|\t|\t|----EN_CORE_COL_LIN_2: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("EN_CORE_COL_DIFF_1").value());
	pRD53->setReg("EN_CORE_COL_DIFF_1",value,true);
	os << GREEN << "|\t|\t|\t|----EN_CORE_COL_DIFF_1: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;

	value = convertAnyInt (cSettingsChild.attribute ("EN_CORE_COL_DIFF_2").value());
	pRD53->setReg("EN_CORE_COL_DIFF_2",value,true);
	os << GREEN << "|\t|\t|\t|----EN_CORE_COL_DIFF_2: " << BOLDRED << std::hex << "0x" << value << std::dec << " (" << value << ")" << RESET << std::endl;
      }
  }
  // ########################
}
