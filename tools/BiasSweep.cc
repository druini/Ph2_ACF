#include "BiasSweep.h"

void BiasSweep::InitializeAmuxMap()
{
    fAmuxSettings.clear();

    if (fType == ChipType::CBC2)
    {
        // key(BiasSweep, reg name, amux code, bit mask, bit shift
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("none"), std::make_tuple ("", 0x00, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vplus"), std::make_tuple ( "Vplus", 0x01, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VCth"), std::make_tuple ("VCth", 0x02, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ihyst"), std::make_tuple ("FrontEndControl", 0x03, 0x3C, 2) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Icomp"), std::make_tuple ("Icomp", 0x04, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_Vcas"), std::make_tuple ("TestPulseChargeMirrCascodeVolt", 0x05, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ibias"), std::make_tuple ("", 0x06, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Bandgap"), std::make_tuple ("", 0x07, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_I"), std::make_tuple ("TestPulseChargePumpCurrent", 0x08, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre1"), std::make_tuple ("Ipre1", 0x09, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre2"), std::make_tuple ("Ipre2", 0x0A, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpc"), std::make_tuple ("Vpc", 0x0B, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipsf"), std::make_tuple ("Ipsf", 0x0C, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipaos"), std::make_tuple ("Ipaos", 0x0D, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipa"), std::make_tuple ("Ipa", 0x0E, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpasf"), std::make_tuple ("", 0x0F, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpafb"), std::make_tuple ("Vpafb", 0x10, 0xFF, 0) );
    }
    else if (fType == ChipType::CBC3)
    {
        // key(BiasSweep, reg name, amux code, bit mask, bit shift
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("none"),   std::make_tuple ("", 0x00, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipa"),    std::make_tuple ("Ipa", 0x01, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre2"),  std::make_tuple ("Ipre2", 0x02, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_I"),  std::make_tuple ("CALIbias", 0x03, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ibias"),  std::make_tuple ("Ibias", 0x04, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VCth"),    std::make_tuple ("", 0x05, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VBGbias"), std::make_tuple ("BandgapFuse", 0x06, 0x3F, 0) );//read this on the VDDA line?
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VBG_LDO"), std::make_tuple ("", 0x07, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Vpafb"),  std::make_tuple ("", 0x08, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Nc50"),   std::make_tuple ("", 0x09, 0x00, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipre1"),  std::make_tuple ("Ipre1", 0x0A, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipsf"),   std::make_tuple ("Ipsf", 0x0B, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ipaos"),  std::make_tuple ("Ipaos", 0x0C, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Icomp"),  std::make_tuple ("Icomp", 0x0D, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("Ihyst"),  std::make_tuple ("FeCtrl&TrgLat2", 0x0E, 0x3C, 2) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("CAL_Vcasc"), std::make_tuple ("CALVcasc", 0x0F, 0xFF, 0) );
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VPLUS2"), std::make_tuple ("Vplus1&2", 0x10, 0xF0, 4) ) ;
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VPLUS1"), std::make_tuple ("Vplus1&2", 0x11, 0x0F, 0) ) ;
        fAmuxSettings.emplace (std::piecewise_construct, std::make_tuple ("VDDA"), std::make_tuple ("", 0x00, 0x00, 0) ) ;
    }
}

//cases: 1)VCth 2)Voltage & sweepable -> DMM 3) not sweepable: old code 4) current & sweepable: 1 reading on dmm with default setting, then sweep using PS

BiasSweep::BiasSweep (HMP4040Client* pClient, Ke2110Controller* pController)
{
#ifdef __USBINST__

    fKeController = pController;

    fHMPClient = pClient;

    fArdNanoController = nullptr;

#endif
}

BiasSweep::~BiasSweep()
{
#ifdef __USBINST__

    //if (fKeController) delete fKeController;

    //if (fHMPClient) delete fHMPClient;

    if (fArdNanoController) delete fArdNanoController;

#endif
}

void BiasSweep::Initialize()
{
    // now read the settings from the map
    auto cSetting = fSettingsMap.find ( "SweepTimeout" );
    fSweepTimeout = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    cSetting = fSettingsMap.find ( "KePort" );
    fKePort = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 8083;
    cSetting = fSettingsMap.find ( "HMPPort" );
    fHMPPort = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 8082;
    cSetting = fSettingsMap.find ( "StepSize" );
    fStepSize = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;

    LOG (INFO) << "\tSettings for BiasSweep parsed:";
    LOG (INFO) << "\t\tSweepTimeout: " << fSweepTimeout;
    LOG (INFO) << "\t\tStepSize    : " << fStepSize;
    LOG (INFO) << "\t\tHMPPort     : " << fHMPPort;
    LOG (INFO) << "\t\tKePort      : " << fKePort;

#ifdef __USBINST__

    //create a controller
    if (fKeController == nullptr) fKeController = new Ke2110Controller ();

    if (!fKeController->ClientInitialized() ) fKeController->InitializeClient ("localhost", fKePort);

    //initialize the HMP4040Client to connect to the server
    if (fHMPClient == nullptr) fHMPClient = new HMP4040Client ("localhost", fHMPPort);

    // first is async, second is multex??
    LOG (INFO) << YELLOW << "Attempting to connect to arduino nano!" << RESET;
    fArdNanoController = new ArdNanoController (false, false);

    int cCounter = 0;

    while (fArdNanoController->Write ("1") != 0 )
    {
        if (cCounter++ > 5)
        {
            LOG (ERROR) << RED << "Failed to initialize ArduinoNano Controller - quitting!" << RESET;
            this->cleanup();
            exit (1);
        }
    }

#endif

    gROOT->ProcessLine ("#include <vector>");
    TString cName = "c_BiasSweep";
    TObject* cObj = gROOT->FindObject ( cName );

    if ( cObj ) delete cObj;

    fSweepCanvas = new TCanvas (cName, "Bias Sweep", 10, 0, 500, 500 );
    fSweepCanvas->SetGrid();
    fSweepCanvas->cd();
#ifdef __HTTP__

    if (fHttpServer) fHttpServer->Register ("/", fSweepCanvas);

#endif
    LOG (INFO) << "Created Canvas for Bias sweeps";

    //initialize empty bias sweep object
    fData = new BiasSweepData();

    for (auto cBoard : fBoardVector)
    {
        for (auto cFe : cBoard->fModuleVector)
        {
            fType = cFe->getChipType();

            for (auto cCbc : cFe->fCbcVector)
            {
                cName = Form ("BiasSweep_Fe%d_Cbc%d", cCbc->getFeId(), cCbc->getCbcId() );
                cObj = gROOT->FindObject (cName);

                if (cObj) delete cObj;

                TTree* cTmpTree = new TTree (cName, cName);
                //cTmpTree->Branch (Form ("BiasSweepData_Fe%d_Cbc%d", cCbc->getFeId(), cCbc->getCbcId() ), "BiasSweepData", &fData);
                cTmpTree->Branch ("Bias", &fData->fBias);
                cTmpTree->Branch ("Fe", &fData->fFeId, "Fe/s" );
                cTmpTree->Branch ("Cbc", &fData->fCbcId, "Cbc/s" );
                cTmpTree->Branch ("Time", &fData->fTimestamp, "Time/l" );
                cTmpTree->Branch ("Unit", &fData->fUnit, "Unit/C" );
                cTmpTree->Branch ("InitialBiasValue", &fData->fInitialXValue, "InitialDAC/s");
                cTmpTree->Branch ("InitialDMMValue", &fData->fInitialYValue, "InitialDMM/F");
                cTmpTree->Branch ("BiasValues", &fData->fXValues);
                cTmpTree->Branch ("DMMValues", &fData->fYValues);

                this->bookHistogram (cCbc, "DataTree", cTmpTree);

                LOG (INFO) << "TTree for BiasSweep data for Fe " << +cCbc->getFeId() << " Cbc " << +cCbc->getCbcId() << " created!";
            }
        }

    }

    //initialize the Amux Setting map by calling the corresponding method
    this->InitializeAmuxMap();
}


void BiasSweep::SweepBias (std::string pBias, Cbc* pCbc)
{
    auto cAmuxValue = fAmuxSettings.find (pBias);

    if (cAmuxValue == std::end (fAmuxSettings) )
    {
        LOG (ERROR) << "Error: the bias " << pBias << " is not part of the known Amux settings - check spelling! - value will be left at the original";
        exit (1);
    }
    else
    {
        LOG (INFO) << std::endl;
        LOG (INFO) << BOLDBLUE << "*****************************************" << RESET;
        LOG (INFO) << BOLDBLUE << "Measuring bias: " << BOLDRED << pBias << RESET;
        LOG (INFO) << BOLDBLUE << "*****************************************" << RESET;
        //boolean variable to find out if current or not
        bool cCurrent = (pBias.find ("I") != std::string::npos) ? true : false;

        if (pBias == "CAL_I") cCurrent = false;

        //since I want to have a simple class to just sweep a bias on 1 CBC, I create the Graph inside the method
        //just create objects, sweep and fill and forget about them again!

        std::time_t cTime = std::time (nullptr);
        TString cName = Form ("g_BiasSweep_%s_Fe%d_Cbc%d_TS%d", pBias.c_str(), pCbc->getFeId(), pCbc->getCbcId(), cTime );

        TObject* cObj = gROOT->FindObject (cName);

        if (cObj) delete cObj;

        TGraph* cGraph = new TGraph ();
        cGraph->SetName (cName);
        std::string cYAxis = (cCurrent) ? "I [A]" : "V [V]";
        std::string cXAxis = pBias + " [DAC]";
        cGraph->SetTitle (Form ("Bias Sweep %s; %s ; %s", pBias.c_str(), cXAxis.c_str(), cYAxis.c_str() ) );
        cGraph->SetLineWidth ( 2 );
        cGraph->SetLineColor (2);
        cGraph->SetMarkerColor (2);
        //cGraph->GetXaxis()->SetTitle (pBias.c_str() );
        bookHistogram ( pCbc, pBias, cGraph );

        LOG (INFO) << "Created Graph for Sweep of " << pBias;

        //now get the TTree for this CBC and fill the already known fields
        TTree* cTmpTree = static_cast<TTree*> (getHist ( pCbc, "DataTree" ) );
        fData->fBias = pBias.c_str();
        fData->fTimestamp = static_cast<long int> (cTime);
        fData->fFeId = pCbc->getFeId();
        fData->fCbcId = pCbc->getCbcId();
        fData->fUnit[0] = (cCurrent) ? 'I' : 'V';
        fData->fUnit[1] = 0;
        fData->fInitialXValue = 0;
        fData->fInitialYValue = 0;
        fData->fXValues.clear();
        fData->fYValues.clear();

#ifdef __USBINST__
        std::string cLogFile = fDirectoryName + "/DMM_log.txt";
        //send pause command to any running Ke server
        LOG (INFO) << YELLOW << "Sending request to pause Temperature monitoring" << RESET;

        //the underlying client has already n retries built in, so no need to repeat here!
        if (!fKeController->SendPause() )
        {
            LOG (ERROR) << RED << "Failed to pause the Temperature monitoring - quitting!" << RESET;
            this->cleanup();
            exit (1);
        }
        else
            LOG (INFO) << YELLOW << "Temperature Monitoring Pause request sent successfully!" << RESET;

        //then set up for local operation
        fKeController->SetLogFileName (cLogFile);

        fKeController->openLogFile();

        fKeController->Reset();

        std::string cConfString = "VOLTAGE:DC";

        //set up to either measure Voltage DC - params: configstring, range, resolution, autorange
        fKeController->Configure (cConfString, 0, 0.0001, true);

        fKeController->Autozero();

        if (cCurrent )
        {
            LOG (INFO) << YELLOW <<  "Trying to pause monitoring with HMP4040!" << RESET;

            if (!fHMPClient->PauseMonitoring() )
            {
                LOG (ERROR) << RED << "HMP4040 Monitoring pause failed - qutting!" << RESET;
                this->cleanup();
                exit (1);
            }
            else
                LOG (INFO) << YELLOW << "HMP4040 Monitoring Pause request sent successfully!" << RESET;
        }

#endif

        //ok, now set the Analogmux to the value required to see the bias there
        //in order to do this, read the current value and store it for later
        uint8_t cOriginalAmuxValue = this->configureAmux (cAmuxValue, pCbc, fSweepTimeout);

        if (cAmuxValue->first == "VCth")
            this->sweepVCth (cGraph, pCbc);
        // the bias is not VCth
        else
        {
            // here start sweeping the bias!
            bool cChangeReg = (cAmuxValue->second.fBitMask != 0x00) ? true : false;

            // the bias is sweepable
            if (cChangeReg)
                // if the bias is sweepable, save the original value, do a full sweep, update the canvas etc;
                this->sweep8Bit (cAmuxValue, cGraph, pCbc, cCurrent);
            //else, the bias is not sweepable just measure what is on the amux output and put it in the initial value in the tree
            else
                this->measureSingle (cAmuxValue, pCbc);
        }

        //to clean up, save everything
        this->resetAmux (cOriginalAmuxValue, pCbc, fSweepTimeout);

#ifdef __USBINST__
        //close the log file
        fKeController->closeLogFile();

        //tell any server to resume the monitoring except for when it is a current, because then the sweep already sent resume
        if (!cCurrent)
        {
            LOG (INFO) << YELLOW << "Sending request to resume Temperatue monitoring" << RESET;

            if (!fKeController->SendResume() )
            {
                LOG (ERROR) << RED << "Resumine temperature monitoring failed - quitting!" << RESET;
                this->cleanup();
                exit (1);
            }
        }

        if (cCurrent)
        {
            LOG (INFO) << YELLOW << "Trying to resume monitoring with HMP4040!" << RESET;

            if (!fHMPClient->ResumeMonitoring() )
            {
                LOG (ERROR) << RED <<  "HMP4040 Monitoring resume failed - quitting!" << RESET;
                this->cleanup();
                exit (1);
            }
            else
                LOG (INFO) << YELLOW << "HMP4040 Monitoring Resume request sent successfully!" << RESET;
        }

#endif
        cTmpTree->Fill();
        this->writeObjects();
        LOG (INFO) << "Bias Sweep finished, results saved!";

    }
}

uint8_t BiasSweep::configureAmux (std::map<std::string, AmuxSetting>::iterator pAmuxValue, Cbc* pCbc, double pSettlingTime_s  )
{
    // first read original value in Amux register and save for later
    uint8_t cOriginalAmuxValue = fCbcInterface->ReadCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux");
    LOG (INFO) << "Analog mux originally set to: " << std::hex << (cOriginalAmuxValue & 0x1F) << std::dec << " (full register is 0x" << std::hex << +cOriginalAmuxValue << std::dec << ") (the Test pulse bits are not changed!)";

    //ok, now set the Analogmux to the value required to see the bias there
    if (pAmuxValue->first == "VDDA" )
    {
        LOG (INFO) << "Bias setting is " << pAmuxValue->first << " -this is not routed via the Amux, thus leaving settings at original value!";
        //need to switch the Arduino nano controller to VDDA
#ifdef __USBINST__
        fArdNanoController->ControlRelay (1);
        LOG (INFO) << "Setting Arduino Nano relay to " << pAmuxValue->first;
#endif
        return cOriginalAmuxValue;
    }
    else
    {
#ifdef __USBINST__
        fArdNanoController->ControlRelay (0);
        LOG (INFO) << "Setting Arduino Nano relay to Amux (default)";
#endif

        uint8_t cNewValue = (cOriginalAmuxValue & 0xE0) | (pAmuxValue->second.fAmuxCode & 0x1F);
        fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", cNewValue);
        LOG (INFO) << "Analog MUX setting modified to connect " <<  pAmuxValue->first << " (setting to 0x" << std::hex << +cNewValue << std::dec << ")";

        for ( unsigned int i = 0 ; i < pSettlingTime_s; i++)
            std::this_thread::sleep_for (std::chrono::milliseconds ( 1000 ) );

        return cOriginalAmuxValue;
    }
}
void BiasSweep::resetAmux (uint8_t pAmuxValue, Cbc* pCbc, double pSettlingTime_s  )
{
#ifdef __USBINST__
    fArdNanoController->ControlRelay (0);
    LOG (INFO) << "Setting Arduino Nano relay to Amux (default)";
#endif
    LOG (INFO) << "Reseting Amux settings back to original value of 0x" << std::hex << +pAmuxValue << std::dec;
    fCbcInterface->WriteCbcReg (pCbc, "MiscTestPulseCtrl&AnalogMux", pAmuxValue);

    for ( unsigned int i = 0 ; i < pSettlingTime_s; i++)
        std::this_thread::sleep_for (std::chrono::milliseconds ( 1000 ) );
}

void BiasSweep::sweep8Bit (std::map<std::string, AmuxSetting>::iterator pAmuxValue, TGraph* pGraph, Cbc* pCbc, bool pCurrent)
{

    uint8_t cOriginalBiasValue = fCbcInterface->ReadCbcReg (pCbc, pAmuxValue->second.fRegName);
    LOG (INFO) << "Origainal Register Value for bias " << pAmuxValue->first << "(" << pAmuxValue->second.fRegName << ") read to be 0x" << std::hex << +cOriginalBiasValue << std::dec << " - saving for later!";

    //take one initial reading on the dmm and save as the inital value in the tree
    float cInitialReading = -999;
#ifdef __USBINST__

    while (cInitialReading == -999)
    {
        fKeController->Measure();
        cInitialReading = fKeController->GetLatestReadValue();
    }

    LOG (INFO) << GREEN << "Initial Reading on AMUX with original bias value " << +cOriginalBiasValue << " : " << cInitialReading << RESET;

    if (pCurrent)
    {
        if (!fKeController->SendResume() )
        {
            LOG (ERROR) << RED << "Send resume monitoring failed - quitting!" << RESET;
            this->cleanup();
            exit (1);
        }
    }

#endif
    fData->fInitialXValue = cOriginalBiasValue;
    fData->fInitialYValue = cInitialReading;

    uint8_t cBits = __builtin_popcount (pAmuxValue->second.fBitMask);
    uint16_t cRange;

    if (cBits == 4) cRange = 16;
    //VBG_bias
    else if (cBits == 6) cRange = 64;
    else cRange = 255;

    std::this_thread::sleep_for (std::chrono::milliseconds (100) );

    for (uint8_t cBias = 0; cBias < cRange; cBias += fStepSize)
    {
        //make map string, pair<string, uint8_t> and use the string in pair for the bias
        uint8_t cRegValue = (cBias) << pAmuxValue->second.fBitShift;

        //VBG_bias
        if (cBits == 6) cRegValue |= (0x2 << 6);

        fCbcInterface->WriteCbcReg (pCbc, pAmuxValue->second.fRegName, cRegValue );

        double cReading = -999;
#ifdef __USBINST__

        if (!pCurrent)
        {
            while (cReading == -999)
            {
                fKeController->Measure();
                cReading = fKeController->GetLatestReadValue();
            }

            if (cBias % 10 == 0) LOG (INFO) << "Set bias to " << +cBias << " (0x" << std::hex << +cBias << std::dec << ") DAC units and read " << cReading << " V on the DMM";
        }
        else if (pCurrent)
        {
            //read the LV PS instead
            int cTimeoutCounter = 0;

            bool cSuccess = fHMPClient->MeasureValues();

            if (cSuccess)
            {
                // request was successfull, so proceed
                cReading = fHMPClient->fValues.fCurrents.at (2);

                if (cBias % 10 == 0) LOG (INFO) << "Set bias to " << +cBias << " (0x" << std::hex << +cBias << std::dec << ") DAC units and read " << cReading << " A on the HMP4040";
            }
            else
                LOG (ERROR) << RED << "Could not retreive the measurement values from the HMP4040!" << RESET;
        }

#endif


        //now I have the value I set and the reading from the DMM
        pGraph->SetPoint (pGraph->GetN(), cBias, cReading);

        //update the canvas
        if (cBias == fStepSize) pGraph->Draw ("APL");
        else if (cBias % 10 == 0 || static_cast<uint16_t> (cBias) + fStepSize > 255)
        {
            fSweepCanvas->Modified();
            fSweepCanvas->Update();
        }

        //now set the values for the ttree
        fData->fXValues.push_back (cBias);
        fData->fYValues.push_back (cReading);

        if (static_cast<uint16_t> (cBias) + fStepSize > 255)
            break;
    }

    // set the bias back to the original value
    fCbcInterface->WriteCbcReg (pCbc, pAmuxValue->second.fRegName, cOriginalBiasValue );
    LOG (INFO) << "Re-setting " << pAmuxValue->second.fRegName << " to original value of 0x" << std::hex << +cOriginalBiasValue << std::dec;
}

void BiasSweep::measureSingle (std::map<std::string, AmuxSetting>::iterator pAmuxValue, Cbc* pCbc)
{
    LOG (INFO) << "Not an Amux setting that requires a sweep: " << pAmuxValue->first;
    double cReading = -999;
#ifdef __USBINST__

    while (cReading == -999)
    {
        fKeController->Measure();
        cReading = fKeController->GetLatestReadValue();
    }

    LOG (INFO) << "Measured bias " << pAmuxValue->first << " to be " << cReading;
#endif
    //now set the values for the ttree
    fData->fInitialXValue = 0;
    fData->fInitialYValue = cReading;
}

void BiasSweep::sweepVCth (TGraph* pGraph, Cbc* pCbc)
{
    ThresholdVisitor cThresholdVisitor (fCbcInterface);
    pCbc->accept (cThresholdVisitor);
    uint16_t cOriginalThreshold = cThresholdVisitor.getThreshold();
    LOG (INFO) << "Original threshold set to " << cOriginalThreshold << " (0x" << std::hex << cOriginalThreshold << std::dec << ") - saving for later!";
    cThresholdVisitor.setOption ('w');

    //take one initial reading on the dmm
    float cInitialReading = -999;
#ifdef __USBINST__

    while (cInitialReading == -999)
    {
        fKeController->Measure();
        cInitialReading = fKeController->GetLatestReadValue();
    }

#endif
    LOG (INFO) << GREEN << "Initial Reading on AMUX with original threshold " << +cOriginalThreshold << " : " << cInitialReading << RESET;
    fData->fInitialXValue = cOriginalThreshold;
    fData->fInitialYValue = cInitialReading;

    std::this_thread::sleep_for (std::chrono::milliseconds (100) );

    for (uint16_t cThreshold = 0; cThreshold < 1023; cThreshold += fStepSize)
    {
        cThresholdVisitor.setThreshold (cThreshold);
        pCbc->accept (cThresholdVisitor);

        double cReading = -999;
#ifdef __USBINST__

        while (cReading == -999)
        {
            fKeController->Measure();
            cReading = fKeController->GetLatestReadValue();
        }

#endif

        //now I have the value I set and the reading from the DMM
        pGraph->SetPoint (pGraph->GetN(), cThreshold, cReading);

        if (cThreshold % 10 == 0) LOG (INFO) << "Set bias to " << cThreshold << " (0x" << std::hex << cThreshold << std::dec << ") DAC units and read " << cReading << " on the DMM";

        //update the canvas
        if (cThreshold == fStepSize) pGraph->Draw ("APL");
        else if (cThreshold % 10 == 0 || cThreshold == 1022)
        {
            fSweepCanvas->Modified();
            fSweepCanvas->Update();
        }

        //now set the values for the ttree
        fData->fXValues.push_back (cThreshold);
        fData->fYValues.push_back (cReading);
    }

    //now set back the original value
    LOG (INFO) << "Re-setting original Threshold value of " << cOriginalThreshold << "(0x" << std::hex << cOriginalThreshold << std::dec << ")";
    cThresholdVisitor.setThreshold (cOriginalThreshold);
    pCbc->accept (cThresholdVisitor);
}

void BiasSweep::writeObjects()
{
    this->SaveResults();
    //save the canvas too!
    fResultFile->cd();
    fSweepCanvas->Write ( fSweepCanvas->GetName(), TObject::kOverwrite );
    //to clean up, just use Tool::SaveResults in here!
    fResultFile->Flush();
}

void BiasSweep::cleanup()
{
    if (fKeController)
    {
        fKeController->SendQuit();
        delete fKeController;
    }

    if (fHMPClient)
    {
        fHMPClient->Quit();
        delete fHMPClient;
    }

    if (fArdNanoController) delete fArdNanoController;
}
