#include "FileParser.h"


namespace Ph2_System {

    void FileParser::parseHW ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os )
    {
        if ( pFilename.find ( ".xml" ) != std::string::npos )
            parseHWxml ( pFilename, pBeBoardFWMap, pBoardVector, os );
        else
            LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xml!" ;
    }

    void FileParser::parseSettings ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os)
    {
        if ( pFilename.find ( ".xml" ) != std::string::npos )
            parseSettingsxml ( pFilename, pSettingsMap, os );
        else
            LOG (ERROR) << "Could not parse settings file " << pFilename << " - it is not .xm!" ;
    }


    void FileParser::parseHWxml ( const std::string& pFilename, BeBoardFWMap& pBeBoardFWMap, BeBoardVec& pBoardVector, std::ostream& os )
    {
        pugi::xml_document doc;
        uint32_t cBeId, cModuleId;//, cCbcId;
        // Basil: initialize these ID's?
        cBeId = cModuleId = 0;
        uint32_t cNBeBoard = 0;
        int i, j;

        pugi::xml_parse_result result = doc.load_file ( pFilename.c_str() );

        if ( !result )
        {
            os << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
            os << "Error description : " << result.description() << std::endl;
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
            BeBoard* cBeBoard = this->parseBeBoard (cBeBoardNode, pBoardVector, os);
            std::string cBoardType = cBeBoardNode.attribute ( "boardType" ).value();

            if (cBoardType == "GLIB") cBeBoard->setBoardType (BoardType::GLIB);
            else if (cBoardType == "CTA") cBeBoard->setBoardType (BoardType::CTA);
            else if (cBoardType == "ICGLIB") cBeBoard->setBoardType (BoardType::ICGLIB);
            else if (cBoardType == "ICFC7") cBeBoard->setBoardType (BoardType::ICFC7);
            else if (cBoardType == "CBC3FC7") cBeBoard->setBoardType (BoardType::CBC3FC7);
            else if (cBoardType == "D19C") cBeBoard->setBoardType (BoardType::D19C);
            else
            {
                LOG (ERROR) << "Error: Unknown Board Type: " << cBoardType << " - aborting!";
                exit (1);
            }

            pugi::xml_node cBeBoardConnectionNode = cBeBoardNode.child ("connection");

            std::string cId = cBeBoardConnectionNode.attribute ( "id" ).value();
            std::string cUri = cBeBoardConnectionNode.attribute ( "uri" ).value();
            std::string cAddressTable = expandEnvironmentVariables (cBeBoardConnectionNode.attribute ( "address_table" ).value() );

            if (!strUhalConfig.empty() )
                RegManager::setDummyXml (strUhalConfig);

            os << BOLDBLUE << "	" <<  "|"  << "----" << "Board Id: " << BOLDYELLOW << cId << BOLDBLUE << " URI: " << BOLDYELLOW << cUri << BOLDBLUE << " Address Table: " << BOLDYELLOW << cAddressTable << std::endl;
            os << BOLDBLUE << "|\t|----Type: " << BOLDYELLOW << cBoardType << RESET << std::endl << BLUE << "|\t|" << RESET << std::endl;

            //else LOG(INFO) << BOLDBLUE << "   " <<  "|"  << "----" << "Board Id: " << BOLDYELLOW << cId << BOLDBLUE << " Type: " << BOLDYELLOW << cBoardType << RESET ;

            // Iterate over the BeBoardRegister Nodes
            for ( pugi::xml_node cBeBoardRegNode = cBeBoardNode.child ( "Register" ); cBeBoardRegNode; cBeBoardRegNode = cBeBoardRegNode.next_sibling() )
            {
                if (std::string (cBeBoardRegNode.name() ) == "Register")
                {
                    std::string cNameString;
                    uint32_t cValue;
                    this->parseRegister (cBeBoardRegNode, cNameString, cValue, cBeBoard, os);
                }
            }

            os << BLUE <<  "|\t|" << RESET << std::endl;

            if (cBeBoard->getBoardType() == BoardType::GLIB)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new GlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::ICGLIB)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new ICGlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::CTA)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new CtaFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::ICFC7)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new ICFc7FWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            //else if ( !cBoardType.compare ( std::string ( "MPAGLIB" ) ) )
            //    pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new MPAGlibFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::CBC3FC7)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new Cbc3Fc7FWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );
            else if (cBeBoard->getBoardType() == BoardType::D19C)
                pBeBoardFWMap[cBeBoard->getBeBoardIdentifier()] =  new D19cFWInterface ( cId.c_str(), cUri.c_str(), cAddressTable.c_str() );

            //else
            //cBeBoardFWInterface = new OtherFWInterface();

            // Iterate the module node
            for ( pugi::xml_node cModuleNode = cBeBoardNode.child ( "Module" ); cModuleNode; cModuleNode = cModuleNode.next_sibling() )
            {
                if ( static_cast<std::string> ( cModuleNode.name() ) == "Module" )
                {
                    bool cStatus = cModuleNode.attribute ( "Status" ).as_bool();

                    //LOG(INFO) << cStatus ;
                    if ( cStatus )
                    {
                        os << BOLDCYAN << "|" << "	" << "|" << "----" << cModuleNode.name() << "  "
                           << cModuleNode.first_attribute().name() << " :" << cModuleNode.attribute ( "ModuleId" ).value() << RESET << std:: endl;

                        cModuleId = cModuleNode.attribute ( "ModuleId" ).as_int();

                        Module* cModule = new Module ( cBeId, cModuleNode.attribute ( "FMCId" ).as_int(), cModuleNode.attribute ( "FeId" ).as_int(), cModuleId );
                        cBeBoard->addModule ( cModule );

                        this->parseCbc (cModuleNode, cModule, os);
                    }
                }
            }

        }

        cNBeBoard++;

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

    BeBoard* FileParser::parseBeBoard (pugi::xml_node pNode, BeBoardVec& pBoardVector,  std::ostream& os)
    {
        uint32_t cBeId = pNode.attribute ( "Id" ).as_int();
        BeBoard* cBeBoard = new BeBoard ( cBeId );

        os << BOLDCYAN << "|" << "----" << pNode.name() << "  " << pNode.first_attribute().name() << " :" << pNode.attribute ( "Id" ).value() << RESET << std:: endl;

        pugi::xml_node cBeBoardFWVersionNode = pNode.child ( "FW_Version" );
        uint16_t cNCbcDataSize = 0;
        cNCbcDataSize = static_cast<uint16_t> ( cBeBoardFWVersionNode.attribute ( "NCbcDataSize" ).as_int() );

        if ( cNCbcDataSize != 0 ) os << BOLDCYAN << "|" << "	" << "|" << "----" << cBeBoardFWVersionNode.name() << " NCbcDataSize: " << cNCbcDataSize  <<  RESET << std:: endl;

        cBeBoard->setNCbcDataSize ( cNCbcDataSize );
        pBoardVector.push_back ( cBeBoard );
        return cBeBoard;
    }

    void FileParser::parseRegister (pugi::xml_node pNode, std::string& pAttributeString, uint32_t& pValue, BeBoard* pBoard, std::ostream& os)
    {
        if (std::string (pNode.name() ) == "Register")
        {
            if (std::string (pNode.first_child().value() ).empty() ) // the node has no value associated and thus is just a container
            {
                if (!pAttributeString.empty() ) pAttributeString += ".";

                pAttributeString += pNode.attribute ("name").value();

                //ok, I have appended .name to the attribute string, now loop over all the children
                for ( pugi::xml_node cNode = pNode.child ( "Register" ); cNode; cNode = cNode.next_sibling() )
                {
                    std::string cAttributeString = pAttributeString;
                    this->parseRegister (cNode, cAttributeString, pValue, pBoard, os);
                }
            }
            else // this node has a value and thus is the last down the hierarchy - so I am terminate the attribute string, get the value and thats it
            {
                if (!pAttributeString.empty() ) // this is the first node in the hierarchy and thus no children
                    pAttributeString += ".";

                pAttributeString += pNode.attribute ("name").value();
                pValue = convertAnyInt (pNode.first_child().value() );
                os << BLUE << "|" << "\t" << "|" << "----" << pNode.name() << "  " << pAttributeString << ": " << BOLDYELLOW << pValue << RESET << std:: endl;
                pBoard->setReg ( pAttributeString, pValue );
            }
        }

    }

    void FileParser::parseCbc (pugi::xml_node pModuleNode, Module* pModule, std::ostream& os )
    {
        pugi::xml_node cCbcPathPrefixNode = pModuleNode.child ( "CBC_Files" );
        std::string cFilePrefix = expandEnvironmentVariables (static_cast<std::string> ( cCbcPathPrefixNode.attribute ( "path" ).value() ) );

        if ( !cFilePrefix.empty() ) os << GREEN << "|" << "	" << "|" << "	" << "|" << "----" << "CBC Files Path : " << cFilePrefix << RESET << std::endl;

        // Iterate the CBC node
        for ( pugi::xml_node cCbcNode = pModuleNode.child ( "CBC" ); cCbcNode; cCbcNode = cCbcNode.next_sibling() )
        {
            os << BOLDCYAN << "|" << "	" << "|" << "	" << "|" << "----" << cCbcNode.name() << "  "
               << cCbcNode.first_attribute().name() << " :" << cCbcNode.attribute ( "Id" ).value()
               << ", File: " << expandEnvironmentVariables (cCbcNode.attribute ( "configfile" ).value() ) << RESET << std:: endl;


            std::string cFileName;

            if ( !cFilePrefix.empty() ){
                if (cFilePrefix.at(cFilePrefix.length()-1) != '/')
                    cFilePrefix.append("/");
                cFileName = cFilePrefix + expandEnvironmentVariables (cCbcNode.attribute ( "configfile" ).value() );
            } else cFileName = expandEnvironmentVariables (cCbcNode.attribute ( "configfile" ).value() );

            Cbc* cCbc = new Cbc ( pModule->getBeId(), pModuleNode.attribute ( "FMCId" ).as_int(), pModuleNode.attribute ( "FeId" ).as_int(), cCbcNode.attribute ( "Id" ).as_int(), cFileName );

            //here parse the specific CBC settings so that Registers take precedence
            this->parseCbcSettings (pModuleNode.child ("CBC"), cCbc, os);

            for ( pugi::xml_node cCbcRegisterNode = cCbcNode.child ( "Register" ); cCbcRegisterNode; cCbcRegisterNode = cCbcRegisterNode.next_sibling() )
            {
                cCbc->setReg ( std::string ( cCbcRegisterNode.attribute ( "name" ).value() ), convertAnyInt ( cCbcRegisterNode.first_child().value() ) );
                os << BLUE << "|\t|\t|\t|----Register: " << std::string ( cCbcRegisterNode.attribute ( "name" ).value() ) << " : " << RED << std::hex << "0x" <<  convertAnyInt ( cCbcRegisterNode.first_child().value() ) << RESET << std::dec << std::endl;
            }

            // here parese the GlobalCbcSettings so that Global CBC regisers take precedence over Global CBC settings which take precedence over CBC specific settings
            //for ( pugi::xml_node cCbcGlobalNode = pModuleNode.child ( "Global_CBC_Register" ); cCbcGlobalNode != pModuleNode.child ( "CBC" ) && cCbcGlobalNode != pModuleNode.child ( "CBC_Files" ) && cCbcGlobalNode != nullptr; cCbcGlobalNode = cCbcGlobalNode.next_sibling() )
            //{

            //if ( cCbcGlobalNode != nullptr )
            //{
            //std::string regname = std::string ( cCbcGlobalNode.attribute ( "name" ).value() );
            //uint32_t regvalue = convertAnyInt ( cCbcGlobalNode.first_child().value() ) ;
            //cCbc->setReg ( regname, uint8_t ( regvalue ) ) ;

            //os << GREEN << "|" << "   " << "|" << "   " << "|" << "----" << cCbcGlobalNode.name()
            //<< "  " << cCbcGlobalNode.first_attribute().name() << " :"
            //<< regname << " =  0x" << std::hex << std::setw ( 2 ) << std::setfill ( '0' ) << regvalue << std::dec << RESET << std:: endl;
            //}
            //}

            pModule->addCbc (cCbc);
        }

        // here parese the GlobalCbcSettings so that Global CBC regisers take precedence over Global CBC settings which take precedence over CBC specific settings
        this->parseGlobalCbcSettings (pModuleNode, pModule, os);
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
            for (auto cCbc : pModule->fCbcVector)
                this->parseCbcSettings (cGlobalCbcSettingsNode, cCbc, os);
        }

        // now that global has been applied to each CBC, handle the GlobalCBCRegisters
        for ( pugi::xml_node cCbcGlobalNode = pModuleNode.child ( "Global_CBC_Register" ); cCbcGlobalNode != pModuleNode.child ( "CBC" ) && cCbcGlobalNode != pModuleNode.child ( "CBC_Files" ) && cCbcGlobalNode != nullptr; cCbcGlobalNode = cCbcGlobalNode.next_sibling() )
        {
            if ( cCbcGlobalNode != nullptr )
            {
                std::string regname = std::string ( cCbcGlobalNode.attribute ( "name" ).value() );
                uint32_t regvalue = convertAnyInt ( cCbcGlobalNode.first_child().value() ) ;

                for (auto cCbc : pModule->fCbcVector)
                    cCbc->setReg ( regname, uint8_t ( regvalue ) ) ;

                os << BOLDGREEN << "|" << "	" << "|" << "	" << "|" << "----" << cCbcGlobalNode.name()
                   << "  " << cCbcGlobalNode.first_attribute().name() << " :"
                   << regname << " =  0x" << std::hex << std::setw ( 2 ) << std::setfill ( '0' ) << RED << regvalue << std::dec << RESET << std:: endl;
            }
        }
    }

    void FileParser::parseCbcSettings (pugi::xml_node pCbcNode, Cbc* pCbc, std::ostream& os)
    {
        //parse the cbc settings here and put them in the corresponding registers of the Cbc object
        //call this for every CBC, Register nodes should take precedence over specific settings??
        ChipType cType = pCbc->getChipType();
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
                pCbc->setReg ("MiscStubLogic", ( (cPtWidth & 0x0F) << 4 | (cMiscStubLogic & 0x0F)) );

                os << GREEN << "Cluster & Stub Logic: " << " ClusterWidthDiscrimination: " << RED << +cCluWidth << GREEN << ", PtWidth: " << RED << +cPtWidth << GREEN << ", Offset: " << RED << +cOffset1 << RESET << std::endl;
            }
            else if (cType == ChipType::CBC3)
            {
                uint8_t cLogicSel = pCbc->getReg ("Pipe&StubInpSel&Ptwidth");
                pCbc->setReg ("Pipe&StubInpSel&Ptwidth", ( (cLogicSel & 0xF0) | (cPtWidth & 0x0F) ) );
                pCbc->setReg ("LayerSwap&CluWidth", ( ( (cLayerswap & 0x01) << 3) | (cCluWidth & 0x07)) );
                pCbc->setReg ("CoincWind&Offset34", ( ( (cOffset4 & 0x0F) << 4) | (cOffset3 & 0x0F)) );
                pCbc->setReg ("CoincWind&Offset12", ( ( (cOffset2 & 0x0F) << 4) | (cOffset1 & 0x0F)) );

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
                pCbc->setReg ("MiscTestPulseCtrl&AnalogMux", ((cAmuxRead & 0xE0) | (cAmuxValue & 0x1F) ) );
                os << RED << "|\t|\t|\t|----Other settengs than Amux curerntly not supported for CBC2, please set manually!" << RESET << std::endl;

            }
            else if (cType == ChipType::CBC3)
            {
                pCbc->setReg ("40MhzClk&Or254", ( ( (cTpgClock & 0x01) << 7) | ( (cOr254 & 0x01) << 6) | (cTestClock & 0x01) << 5 | (cDll & 0x1F)) );
                //LOG (DEBUG) << BOLDRED << std::bitset<8> (pCbc->getReg ("40MhzClk&Or254") ) << RESET;
                uint8_t cPtWidthRead = pCbc->getReg ("Pipe&StubInpSel&Ptwidth");
                pCbc->setReg ("Pipe&StubInpSel&Ptwidth", ( ( (cPipeLogic & 0x03) << 6) | ( (cStubLogic & 0x03) << 4) | (cPtWidthRead & 0x0F) ) );

                uint8_t cAmuxRead = pCbc->getReg ("MiscTestPulseCtrl&AnalogMux");
                pCbc->setReg ("MiscTestPulseCtrl&AnalogMux", ((cAmuxRead & 0xE0) | (cAmuxValue & 0x1F) ) );

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

    void FileParser::parseSettingsxml ( const std::string& pFilename, SettingsMap& pSettingsMap,  std::ostream& os)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_file ( pFilename.c_str() );

        if ( !result )
        {
            os << "ERROR :\n Unable to open the file : " << pFilename << std::endl;
            os << "Error description : " << result.description() << std::endl;
            return;
        }

        for ( pugi::xml_node nSettings = doc.child ( "Settings" ); nSettings; nSettings = nSettings.next_sibling() )
        {
            os << std::endl;

            for ( pugi::xml_node nSetting = nSettings.child ( "Setting" ); nSetting; nSetting = nSetting.next_sibling() )
            {
                pSettingsMap[nSetting.attribute ( "name" ).value()] = convertAnyInt ( nSetting.first_child().value() );
                os <<  RED << "Setting" << RESET << " --" << BOLDCYAN << nSetting.attribute ( "name" ).value() << RESET << ":" << BOLDYELLOW << convertAnyInt ( nSetting.first_child().value() ) << RESET << std:: endl;
            }
        }
    }

    std::string FileParser::expandEnvironmentVariables ( std::string s )
    {
        if ( s.find ( "${" ) == std::string::npos ) return s;

        std::string pre  = s.substr ( 0, s.find ( "${" ) );
        std::string post = s.substr ( s.find ( "${" ) + 2 );

        if ( post.find ( '}' ) == std::string::npos ) return s;

        std::string variable = post.substr ( 0, post.find ( '}' ) );
        std::string value    = "";

        post = post.substr ( post.find ( '}' ) + 1 );

        if ( getenv ( variable.c_str() ) != NULL ) value = std::string ( getenv ( variable.c_str() ) );

        return expandEnvironmentVariables ( pre + value + post );
    }
}
