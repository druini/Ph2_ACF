#include "PedeNoise.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/ThresholdAndNoiseStream.h"
#include "../Utils/OccupancyStream.h"
#include "../Utils/CBCChannelGroupHandler.h"
#include <math.h>

PedeNoise::PedeNoise() :
    Tool(),
    fNoiseCanvas (nullptr),
    fPedestalCanvas (nullptr),
    fFeSummaryCanvas (nullptr),
    fNCbc (0),
    fNFe (0),
    fHoleMode (false),
    fFitted (false),
    fTestPulseAmplitude (0),
    fEventsPerPoint (0)
    // fSkipMaskedChannels(0)
{
}

PedeNoise::~PedeNoise()
{
    delete fChannelGroupHandler;
    for(auto container : fSCurveOccupancyMap) delete container.second;
    fSCurveOccupancyMap.clear();
    // delete fPedestalCanvas;
    // delete fNoiseCanvas;
}

void PedeNoise::Initialise (bool pAllChan, bool pDisableStubLogic)
{
    fDisableStubLogic = pDisableStubLogic;
    this->MakeTestGroups(FrontEndType::CBC3);
    fChannelGroupHandler = new CBCChannelGroupHandler();
    fChannelGroupHandler->setChannelGroupParameters(16, 2);
    fAllChan = pAllChan;

    auto cSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : false;
    cSetting = fSettingsMap.find ( "MaskChannelsFromOtherGroups" );
    fMaskChannelsFromOtherGroups = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;
    cSetting = fSettingsMap.find ( "SkipMaskedChannels" );
    fSkipMaskedChannels = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 1;

    this->SetSkipMaskedChannels( fSkipMaskedChannels );


    //is to be called after system controller::InitialiseHW, InitialiseSettings
    // populates all the maps
    // create the canvases


    fPedestalCanvas = new TCanvas ( "Pedestal & Noise", "Pedestal & Noise", 670, 0, 650, 650 );
    //fFeSummaryCanvas = new TCanvas ( "Noise for each FE", "Noise for each FE", 0, 670, 650, 650 );
    fNoiseCanvas = new TCanvas ( "Final SCurves, Strip Noise", "Final SCurves, Noise", 0, 0, 650, 650 );


    uint16_t cStartValue = 0x000;

    uint32_t cCbcCount = 0;
    uint32_t cFeCount = 0;
    uint32_t cCbcIdMax = 0;

    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();
            cFeCount++;
            fType = cFe->getFrontEndType();

            for ( auto cCbc : cFe->fChipVector )
            {

                //if it is a CBC3, disable the stub logic for this procedure
                if (cCbc->getFrontEndType() == FrontEndType::CBC3 && fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - thus disabling Stub logic for offset tuning" << RESET ;
                    fStubLogicValue[cCbc] = fChipInterface->ReadChipReg (cCbc, "Pipe&StubInpSel&Ptwidth");
                    fHIPCountValue[cCbc] = fChipInterface->ReadChipReg (cCbc, "HIP&TestMode");
                    fChipInterface->WriteChipReg (cCbc, "Pipe&StubInpSel&Ptwidth", 0x23);
                    fChipInterface->WriteChipReg (cCbc, "HIP&TestMode", 0x08);
                }

                uint32_t cCbcId = cCbc->getChipId();
                cCbcCount++;

                if ( cCbcId > cCbcIdMax ) cCbcIdMax = cCbcId;

                TString cHistname;
                TH1F* cHist;

                cHistname = Form ( "Fe%dCBC%d_Noise", cFe->getFeId(), cCbc->getChipId() );
                cHist = new TH1F ( cHistname, cHistname, 200, 0, 20 );
                bookHistogram ( cCbc, "Cbc_Noise", cHist );

                cHistname = Form ( "Fe%dCBC%d_StripNoise", cFe->getFeId(), cCbc->getChipId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5 );
                cHist->SetMaximum (10);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_Stripnoise", cHist );

                cHistname = Form ( "Fe%dCBC%d_Pedestal", cFe->getFeId(), cCbc->getChipId() );
                cHist = new TH1F ( cHistname, cHistname, 2048, -0.5, 1023.5 );
                bookHistogram ( cCbc, "Cbc_Pedestal", cHist );

                cHistname = Form ( "Fe%dCBC%d_Noise_even", cFe->getFeId(), cCbc->getChipId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS / 2, -0.5, 126.5 );
                cHist->SetMaximum (10);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_Noise_even", cHist );

                cHistname = Form ( "Fe%dCBC%d_Noise_odd", cFe->getFeId(), cCbc->getChipId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS / 2, -0.5, 126.5 );
                cHist->SetLineColor ( 2 );
                cHist->SetMaximum (10);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_noise_odd", cHist );

                cHistname = Form ( "Fe%dCBC%d_Occupancy", cFe->getFeId(), cCbc->getChipId() );
                cHist = new TH1F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5 );
                cHist->SetLineColor ( 31 );
                cHist->SetMaximum (1);
                cHist->SetMinimum (0);
                bookHistogram ( cCbc, "Cbc_occupancy", cHist );

                // initialize the hitcount and threshold map
            }

            TString cNoisehistname =  Form ( "Fe%d_Noise", cFeId );
            TH1F* cNoise = new TH1F ( cNoisehistname, cNoisehistname, 200, 0, 20 );
            bookHistogram ( cFe, "Module_noisehist", cNoise );

            cNoisehistname = Form ( "Fe%d_StripNoise", cFeId );
            TProfile* cStripnoise = new TProfile ( cNoisehistname, cNoisehistname, ( NCHANNELS * cCbcCount ), -0.5, cCbcCount * NCHANNELS - .5 );
            cStripnoise->SetMinimum (0);
            cStripnoise->SetMaximum (15);
            bookHistogram ( cFe, "Module_Stripnoise", cStripnoise );
        }

        fNCbc = cCbcCount;
        fNFe = cFeCount;
    }

    uint32_t cPads = ( cCbcIdMax > cCbcCount ) ? cCbcIdMax : cCbcCount;

    fNoiseCanvas->DivideSquare ( 2 * cPads );
    fPedestalCanvas->DivideSquare ( 2 * cPads );
    
    // now read the settings from the map
    cSetting = fSettingsMap.find ( "Nevents" );
    fEventsPerPoint = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 10;
    cSetting = fSettingsMap.find ( "FitSCurves" );
    fFitted = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;
    //cSetting = fSettingsMap.find ( "TestPulseAmplitude" );
    //fTestPulseAmplitude = ( cSetting != std::end ( fSettingsMap ) ) ? cSetting->second : 0;


    LOG (INFO) << "Created Object Maps and parsed settings:" ;
    LOG (INFO) << " Nevents = " << fEventsPerPoint ;
    LOG (INFO) << " FitSCurves = " << int ( fFitted ) ;

    if (fType == FrontEndType::CBC3)
        LOG (INFO) << BOLDBLUE << "Chip Type determined to be " << BOLDRED << "CBC3" << RESET;
    else
        LOG (INFO) << BOLDBLUE << "Chip Type determined to be " << BOLDRED << "CBC2" << RESET;

    DetectorContainer         theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);

    // std::map<uint16_t, ModuleOccupancyPerChannelMap> backEndOccupancyPerChannelMap;
    // std::map<uint16_t, ModuleGlobalOccupancyMap > backEndCbcOccupanyMap;
    // float globalOccupancy=0;
    
    bool originalAllChannelFlag = this->fAllChan;
    this->SetTestAllChannels(false);

    this->setDacAndMeasureData("VCth", cStartValue, fEventsPerPoint);

    // this->setDacAndMeasureOccupancy("VCth", cStartValue, fEventsPerPoint, backEndOccupancyPerChannelMap, backEndCbcOccupanyMap, globalOccupancy);
    
    this->SetTestAllChannels(originalAllChannelFlag);


    cSetting = fSettingsMap.find ("HoleMode");

    if ( cSetting != std::end (fSettingsMap) )
    {
        bool cHoleModeFromSettings = cSetting->second;
        bool cHoleModeFromOccupancy = true;

        fHoleMode = cHoleModeFromSettings;
        std::string cMode = (fHoleMode) ? "Hole Mode" : "Electron Mode";

        for (auto& cBoard : fBoardVector)
        {
            for ( auto cFe : cBoard->fModuleVector )
            {
                for ( auto cCbc : cFe->fChipVector )
                {
                    std::stringstream ss;
                    
                    float cOccupancy = static_cast<Summary<Occupancy,Occupancy>*>(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->summary_)->theSummary_.fOccupancy;
                    // float cOccupancy = backEndCbcOccupanyMap[cBoard->getBeId()][cFe->getFMCId()][cCbc->getChipId()];
                    cHoleModeFromOccupancy = (cOccupancy == 0) ? false :  true;

                    if (cHoleModeFromOccupancy != cHoleModeFromSettings)
                        ss << BOLDRED << "Be careful: " << RESET << "operation mode from settings does not correspond to the one found by measuring occupancy. Using the one from settings (" << BOLDYELLOW << cMode << RESET << ")";
                    else
                        ss << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << (unsigned int)cCbc->getChipId() << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming " << BOLDYELLOW << cMode << RESET << " (consistent with the settings file)";

                    LOG (INFO) << ss.str();
                }
            }
        }
    }
    else
    {
        for (auto& cBoard : fBoardVector)
        {
            for ( auto cFe : cBoard->fModuleVector )
            {
                for ( auto cCbc : cFe->fChipVector )
                {
                    float cOccupancy = static_cast<Summary<Occupancy,Occupancy>*>(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->summary_)->theSummary_.fOccupancy;
                    // float cOccupancy = backEndCbcOccupanyMap[cBoard->getBeId()][cFe->getFMCId()][cCbc->getChipId()];
                    fHoleMode = (cOccupancy == 0) ? false :  true;
                    std::string cMode = (fHoleMode) ? "Hole Mode" : "Electron Mode";
                    std::stringstream ss;
                    ss << BOLDBLUE << "Measuring Occupancy @ Threshold " << BOLDRED << (unsigned int)cCbc->getChipId() << BOLDBLUE << ": " << BOLDRED << cOccupancy << BOLDBLUE << ", thus assuming " << BOLDYELLOW << cMode << RESET;
                    LOG (INFO) << ss.str();
                }
            }
        }
    }
}


std::string PedeNoise::sweepSCurves (uint8_t pTPAmplitude)
{
    uint16_t cStartValue = 0;
    bool originalAllChannelFlag = this->fAllChan;

    if(pTPAmplitude != 0 && originalAllChannelFlag){
        this->SetTestAllChannels(false);
        LOG (INFO) << RED <<  "Cannot inject pulse for all channels, test in groups enabled. " << RESET ;
    }


    if(pTPAmplitude != 0){
        this->SetTestPulse( true );
        fTestPulseAmplitude = pTPAmplitude;
        setFWTestPulse();
        setSameGlobalDac("TestPulsePotNodeSel",  pTPAmplitude);
        LOG (INFO) << BLUE <<  "Enabled test pulse. " << RESET ;
        cStartValue = this->findPedestal ();
    }
    else
    {
        fTestPulseAmplitude = pTPAmplitude;
        this->SetTestPulse( false );
        cStartValue = this->findPedestal (true);
    }

    // now initialize the Scurve histogram
    std::string cHistogramname = Form ("SCurves_TP%d", fTestPulseAmplitude);

    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fChipVector )
            {
                TString cHistname = Form ( "Fe%dCBC%d_Scurves_TP%d", cCbc->getFeId(), cCbc->getChipId(), fTestPulseAmplitude );
                TH2F* cHist = new TH2F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5, 1024, -0.5, 1023.5 );
                cHist->Sumw2();
                bookHistogram ( cCbc, cHistogramname, cHist );

                fNoiseCanvas->cd ( cCbc->getChipId() + 1 );
                cHist->Draw ( "colz2" );
            }
        }
    }


    measureSCurves ( cHistogramname, cStartValue );

    //filling histograms and re-enable stub logic
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fChipVector )
            {
               
                TH2F* cSCurveHist = dynamic_cast<TH2F*> (this->getHist (cCbc, cHistogramname) );
                fNoiseCanvas->cd (cCbc->getChipId() + 1);
                TH1D* cTmp = cSCurveHist->ProjectionY();
                cSCurveHist->GetYaxis()->SetRangeUser ( cTmp->GetBinCenter (cTmp->FindFirstBinAbove (0) ) - 10, cTmp->GetBinCenter (cTmp->FindLastBinAbove (0.99) ) + 10 );
                cSCurveHist->Draw ("colz2");

                RegisterVector cRegVec;

                if (fDisableStubLogic)
                {
                    LOG (INFO) << BOLDBLUE << "Chip Type = CBC3 - re-enabling stub logic to original value!" << RESET;
                    cRegVec.push_back ({"Pipe&StubInpSel&Ptwidth", fStubLogicValue[cCbc]});
                    cRegVec.push_back ({"HIP&TestMode", fHIPCountValue[cCbc]});
                }

                fChipInterface->WriteChipMultReg (cCbc, cRegVec);
            }
        }
    }

    fNoiseCanvas->Modified();
    fNoiseCanvas->Update();

    this->SetTestAllChannels(originalAllChannelFlag);
    if(pTPAmplitude != 0){
        this->SetTestPulse( false );
        setSameGlobalDac("TestPulsePotNodeSel",  0);
        LOG (INFO) << BLUE <<  "Disabled test pulse. " << RESET ;

    }

    processSCurves (cHistogramname);
    LOG (INFO) << BOLDBLUE << "Finished sweeping SCurves..."  << RESET ;
    return cHistogramname;

}

void PedeNoise::measureNoise (uint8_t pTPAmplitude)
{
    std::string cHistName = this->sweepSCurves (pTPAmplitude);
    this->extractPedeNoise (cHistName);
    this->extractPedeNoise ();
}

void PedeNoise::Validate ( uint32_t pNoiseStripThreshold, uint32_t pMultiple )
{
    LOG (INFO) << "Validation: Taking Data with " << fEventsPerPoint* pMultiple << " random triggers!" ;

    for ( auto cBoard : fBoardVector )
    {
        //increase threshold to supress noise
        setThresholdtoNSigma (cBoard, 5);
    }
    DetectorContainer         theOccupancyContainer;
	fDetectorDataContainer = &theOccupancyContainer;
	OccupancyBoardStream      theOccupancyStream;
    fObjectStream          = &theOccupancyStream;

    ContainerFactory   theDetectorFactory;
	theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);

    bool originalAllChannelFlag = this->fAllChan;

    this->SetTestAllChannels(true);

    this->measureData(fEventsPerPoint*pMultiple);
    this->SetTestAllChannels(originalAllChannelFlag);
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fChipVector )
            {
                //get the histogram for the occupancy
                TH1F* cHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_occupancy" ) );
                TLine* line = new TLine (0, pNoiseStripThreshold * 0.001, NCHANNELS, pNoiseStripThreshold * 0.001);
                RegisterVector cRegVec;

            	for (uint32_t iChan = 0; iChan < NCHANNELS; iChan++)
                {
                    float occupancy = theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->getChannel<Occupancy>(iChan).fOccupancy;
                    cHist->SetBinContent(iChan+1,occupancy);
                    cHist->SetBinError  (iChan+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->getChannel<Occupancy>(iChan).fOccupancyError);

                    if( occupancy > float ( pNoiseStripThreshold * 0.001 ) )
                    {
                        TString cRegName = Form ( "Channel%03d", iChan + 1 );
                        cRegVec.push_back ({cRegName.Data(), 0xFF });
                        LOG (INFO) << RED << "Found a noisy channel on CBC " << +cCbc->getChipId() << " Channel " << iChan  << " with an occupancy of " << cHist->GetBinContent (iChan) << "; setting offset to " << +0xFF << RESET ;
                    }
                }

                fNoiseCanvas->cd ( cCbc->getChipId() + 1 );
                cHist->Scale (1.);
                gPad->SetLogy (1);
                cHist->DrawCopy();
                line->Draw ("same");
                fNoiseCanvas->Modified();
                fNoiseCanvas->Update();
                
                fChipInterface->WriteChipMultReg (cCbc, cRegVec);

            }
        }

        setThresholdtoNSigma (cBoard, 0);
        this->HttpServerProcess();
    }

}

double PedeNoise::getPedestal (Chip* pCbc)
{
    TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( pCbc, "Cbc_Pedestal" ) );
    LOG (INFO) << "Pedestal on CBC " << +pCbc->getChipId() << " is " << cPedeHist->GetMean() << " VCth units.";
    return cPedeHist->GetMean();
}
double PedeNoise::getPedestal (Module* pFe)
{
    double cPedestal = 0;

    for (auto cCbc : pFe->fChipVector)
    {
        TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );
        cPedestal += cPedeHist->GetMean();
        LOG (INFO) << "Pedestal on CBC " << +cCbc->getChipId() << " is " << cPedeHist->GetMean() << " VCth units.";
    }

    cPedestal /= pFe->fChipVector.size();

    LOG (INFO) << "Pedestal on Module " << +pFe->getFeId() << " is " << cPedestal << " VCth units.";
    return cPedestal;
}

double PedeNoise::getNoise (Chip* pCbc)
{
    TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( pCbc, "Cbc_Noise" ) );
    return cNoiseHist->GetMean();
}
double PedeNoise::getNoise (Module* pFe)
{
    TH1F* cNoiseHist = dynamic_cast<TH1F*> (getHist (pFe, "Module_noisehist") );
    return cNoiseHist->GetMean();
}

//////////////////////////////////////      PRIVATE METHODS     /////////////////////////////////////////////

uint16_t PedeNoise::findPedestal (bool forceAllChannels)
{

    bool originalAllChannelFlag = this->fAllChan;
    if(forceAllChannels) this->SetTestAllChannels(true);
    // std::map<uint16_t, Tool::ModuleOccupancyPerChannelMap> backEndOccupanyPerChannelAtTargetMap;
    // std::map<uint16_t, Tool::ModuleGlobalOccupancyMap> backEndOccupanyAtTargetMap;

    // bitWiseScan("VCth", fEventsPerPoint, 0.56, true, backEndOccupanyPerChannelAtTargetMap, backEndOccupanyAtTargetMap);

    DetectorContainer         theOccupancyContainer;
    fDetectorDataContainer = &theOccupancyContainer;
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
    this->bitWiseScan("VCth", fEventsPerPoint, 0.56);

    if(forceAllChannels) this->SetTestAllChannels(originalAllChannelFlag);
    
    float cMean = 0.;
    uint32_t nCbc = 0;

    for (auto& cBoard : fBoardVector)
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fChipVector )
            {
                uint16_t tmpVthr = (cCbc->getReg("VCth1") + (cCbc->getReg("VCth2")<<8));
                cMean+=tmpVthr;
                ++nCbc;
            }
        }
    }

    cMean /= nCbc;
    
    LOG (INFO) << BOLDBLUE << "Found Pedestals to be around " << BOLDRED << cMean << " (0x" << std::hex << cMean << std::dec << ", 0b" << std::bitset<10> (cMean) << ")" << BOLDBLUE << RESET;

    return cMean;

}

void PedeNoise::measureSCurves (std::string pHistName, uint16_t pStartValue)
{

    int cMinBreakCount = 10;

    // if (pStartValue == 0) pStartValue = this->findPedestal();

    bool     cAllZero        = false;
    bool     cAllOne         = false;
    int      cAllZeroCounter = 0;
    int      cAllOneCounter  = 0;
    uint16_t cValue          = pStartValue;
    int      cSign           = 1;
    int      cIncrement      = 0;
    uint16_t cMaxValue       = (1 << 10) - 1;

    //start with the threshold value found above
    // ThresholdVisitor cVisitor (fChipInterface, cValue);

    while (! (cAllZero && cAllOne) )
    {

        // std::map<uint16_t, ModuleOccupancyPerChannelMap> backEndOccupancyPerChannelMap;
        // std::map<uint16_t, ModuleGlobalOccupancyMap > backEndCbcOccupanyMap;
        // float globalOccupancy=0;

        // this->setDacAndMeasureOccupancy("VCth", cValue, fEventsPerPoint, backEndOccupancyPerChannelMap, backEndCbcOccupanyMap, globalOccupancy);
        

        DetectorContainer *theOccupancyContainer = new DetectorContainer();
        fDetectorDataContainer = theOccupancyContainer;
        ContainerFactory   theDetectorFactory;
        theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
        fSCurveOccupancyMap[cValue] = theOccupancyContainer;

        this->setDacAndMeasureData("VCth", cValue, fEventsPerPoint);


        //filling histograms
        for ( auto cBoard : fBoardVector )
        {
             // std::cout << "Board occupancy = " << static_cast<Summary<Occupancy,Occupancy>*>(theOccupancyContainer.at(cBoard->getBeId())->summary_)->theSummary_.fOccupancy << std::endl;

            for ( auto cFe : cBoard->fModuleVector )
            {
                 // std::cout << "Module occupancy = " << static_cast<Summary<Occupancy,Occupancy>*>(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->summary_)->theSummary_.fOccupancy << std::endl;
                for ( auto cCbc : cFe->fChipVector )
                {
                    // std::cout << "Chip occupancy = " << static_cast<Summary<Occupancy,Occupancy>*>(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->summary_)->theSummary_.fOccupancy << std::endl;
                    TH2F* cSCurveHist = dynamic_cast<TH2F*> (this->getHist (cCbc, pHistName) );
                    for (uint32_t cChannel = 0; cChannel < NCHANNELS; cChannel++)
                    // for (int cChannel=0; cChannel<=backEndOccupancyPerChannelMap[cBoard->getBeId()][cFe->getModuleId()][cCbc->getChipId()].size(); ++cChannel)
                    {
                        float tmpOccupancy = theOccupancyContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cCbc->getChipId())->getChannel<Occupancy>(cChannel).fOccupancy;
                        // float tmpOccupancy = backEndOccupancyPerChannelMap[cBoard->getBeId()][cFe->getModuleId()][cCbc->getChipId()][cChannel];
                        cSCurveHist->SetBinContent ( cChannel+1, cValue+1, tmpOccupancy);
                        cSCurveHist->SetBinError   ( cChannel+1, cValue+1, sqrt(tmpOccupancy*(1.-tmpOccupancy)/fEventsPerPoint));
                    }
                }
            }
        }

        float globalOccupancy = static_cast<Summary<Occupancy,Occupancy>*>(theOccupancyContainer->summary_)->theSummary_.fOccupancy;
        // std::cout<<globalOccupancy<<std::endl;
        //now establish if I'm zero or one
        if (globalOccupancy == 0) ++cAllZeroCounter;

        if (globalOccupancy > 0.98) ++cAllOneCounter;

        //it will either find one or the other extreme first and thus these will be mutually exclusive
        //if any of the two conditions is true, just revert the sign and go the opposite direction starting from startvalue+1
        //check that cAllZero is not yet set, otherwise I'll be reversing signs a lot because once i switch direction, the statement stays true
        if (!cAllZero && cAllZeroCounter == cMinBreakCount )
        {
            cAllZero = true;
            cSign = fHoleMode ? -1 : 1;
            cIncrement = 0;
        }

        if (!cAllOne && cAllOneCounter == cMinBreakCount)
        {
            cAllOne = true;
            cSign = fHoleMode ? 1 : -1;
            cIncrement = 0;
        }

        cIncrement++;
        // following checks if we're not going out of bounds
        if (cSign == 1 && (pStartValue + (cIncrement * cSign) > cMaxValue) )
        {
            if (fHoleMode) cAllZero = true;
            else cAllOne = true;

            cIncrement = 1;
            cSign = -1 * cSign;
        }

        if (cSign == -1 && (pStartValue + (cIncrement * cSign) < 0) )
        {
            if (fHoleMode) cAllOne = true;
            else cAllZero = true;

            cIncrement = 1;
            cSign = -1 * cSign;
        }


        LOG (DEBUG) << "All 0: " << cAllZero << " | All 1: " << cAllOne << " current value: " << cValue << " | next value: " << pStartValue + (cIncrement * cSign) << " | Sign: " << cSign << " | Increment: " << cIncrement << " Occupancy: " << globalOccupancy << RESET;
        cValue = pStartValue + (cIncrement * cSign);
    }

    this->HttpServerProcess();
    LOG (INFO) << YELLOW << "Found minimal and maximal occupancy " << cMinBreakCount << " times, SCurves finished! " << RESET ;

}

void PedeNoise::processSCurves (std::string pHistName)
{
    
    //filling histograms
    for ( auto cBoard : fBoardVector )
    {
        for ( auto cFe : cBoard->fModuleVector )
        {
            for ( auto cCbc : cFe->fChipVector )
            {

                // TH2F* cHist = dynamic_cast<TH2F*> ( getHist ( cCbc, pHistName) );
                // //in order to have proper binomial errors
                // // cHist->Divide (cHist, fNormHist, 1, 1, "B");
                // cHist->Divide (cHist, fNormHist, 1, 1, "B");

                this->differentiateHist (cCbc, pHistName);

                if (fFitted)
                    this->fitHist (cCbc, pHistName);
                
            }
        }
    }
    //end of CBC loop
}

void PedeNoise::differentiateHist (Chip* pCbc, std::string pHistName)
{
    //first get the SCurveHisto and create a differential histo
    TH2F* cHist = dynamic_cast<TH2F*> ( getHist ( pCbc, pHistName) );
    TString cHistname = Form ( "Fe%dCBC%d_Differential_TP%d", pCbc->getFeId(), pCbc->getChipId(), fTestPulseAmplitude );
    TH2F* cDerivative = new TH2F ( cHistname, cHistname, NCHANNELS, -0.5, 253.5, 1024, 0, 1024 );
    cDerivative->Sumw2();
    bookHistogram ( pCbc, pHistName + "_Diff", cDerivative );

    for (uint16_t cChan = 0; cChan < NCHANNELS; cChan++)
    {
        //get a projection
        int iBin = cHist->GetXaxis()->FindBin (cChan);
        TH1D* cProjection = cHist->ProjectionY ("_py", iBin, iBin);

        double_t cDiff;
        double_t cCurrent;
        double_t cPrev;
        bool cActive; // indicates existence of data points
        int cStep = 1;
        int cDiffCounter = 0;

        double cBin = 0;

        if ( fHoleMode )
        {
            cPrev = cProjection->GetBinContent ( cProjection->FindBin ( 0 ) );
            cActive = false;

            for ( cBin = cProjection->FindBin (0); cBin <= cProjection->FindBin (1023); cBin++ )
            {
                //veify that this happens exactly 1023
                cCurrent = cProjection->GetBinContent (cBin);
                cDiff = cPrev - cCurrent;

                if ( cPrev > 0.75 ) cActive = true; // sampling begins

                int iBinDerivative = cDerivative->FindBin (cChan, (cProjection->GetBinCenter (cBin + 1) + cProjection->GetBinCenter (cBin) ) / 2.0);

                if ( cActive ) cDerivative->SetBinContent ( iBinDerivative, cDiff  );

                if ( cActive && cDiff == 0 && cCurrent == 0 ) cDiffCounter++;

                if ( cDiffCounter == 8 ) break;

                cPrev = cCurrent;
            }
        }
        else
        {
            cPrev = cProjection->GetBinContent ( cProjection->FindBin ( 1023 ) );
            cActive = false;

            for ( cBin = cProjection->FindBin (1023); cBin >= cProjection->FindBin ( 0); cBin-- )
            {
                cCurrent = cProjection->GetBinContent (cBin);
                cDiff = cPrev - cCurrent;

                if ( cPrev > 0.75 ) cActive = true; // sampling begins

                int iBinDerivative = cDerivative->FindBin ( cChan, (cProjection->GetBinCenter (cBin - 1 ) + cProjection->GetBinCenter (cBin ) ) / 2.0);

                if ( cActive ) cDerivative->SetBinContent ( iBinDerivative, cDiff  );

                if ( cActive && cDiff == 0 && cCurrent == 0 ) cDiffCounter++;

                if ( cDiffCounter == 8 ) break;

                cPrev = cCurrent;
            }
        }

        //end of channel loop
    }

    //end of CBC loop
}

void PedeNoise::fitHist (Chip* pCbc, std::string pHistName)
{
    TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( pCbc, "Cbc_Noise" ) );
    TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( pCbc, "Cbc_Pedestal" ) );
    
    //first get the SCurveHisto and create a differential histo
    TH2F* cHist = dynamic_cast<TH2F*> ( getHist ( pCbc, pHistName) );
    TString cDirName = Form ("FE%dCBC%d/%s_Fits", pCbc->getFeId(), pCbc->getChipId(), pHistName.c_str() );
    TDirectory* cDir = dynamic_cast<TDirectory*> (gROOT->FindObject (cDirName) );


    if (!cDir) fResultFile->mkdir (cDirName);

    fResultFile->cd (cDirName);
    //since this is a bit of a special situation I need to create a directory for the SCurves and their fits inside the FExCBCx direcotry and make sure they are saved here

    for (uint16_t cChan = 0; cChan < NCHANNELS; cChan++)
    {
        //get a projection
        TString cProjectionName = Form ("%s_Channel%d", cHist->GetName(), cChan);
        int iBin = cHist->GetXaxis()->FindBin (cChan);
        TH1D* cProjection = cHist->ProjectionY (cProjectionName, iBin, iBin);
        double cFirstNon0 ( 0 );
        double cFirst1 ( 0 );
        std::string cFitname = "SCurveFit";
        TF1* cFit = dynamic_cast<TF1*> (gROOT->FindObject (cFitname.c_str() ) );

        if (cFit) delete cFit;

        // Not Hole Mode
        if ( !fHoleMode )
        {
            for ( Int_t cBin = 1; cBin < cProjection->GetNbinsX() - 1; cBin++ )
            {
                double cContent = cProjection->GetBinContent ( cBin );

                if ( !cFirstNon0 )
                {
                    if ( cContent ) cFirstNon0 = cProjection->GetBinCenter ( cBin );
                }
                else if ( cContent > 0.85 )
                {
                    cFirst1 = cProjection->GetBinCenter ( cBin );
                    break;
                }
            }

            cFit = new TF1 ( "SCurveFit", MyErf, cFirstNon0 - 10, cFirst1 + 10, 2 );
        }
        // Hole mode
        else
        {
            for ( Int_t cBin = cProjection->GetNbinsX() - 1; cBin > 1; cBin-- )
            {
                double cContent = cProjection->GetBinContent ( cBin );

                if ( !cFirstNon0 )
                {
                    if ( cContent ) cFirstNon0 = cProjection->GetBinCenter ( cBin );
                }
                else if ( cContent > 0.85 )
                {
                    cFirst1 = cProjection->GetBinCenter ( cBin );
                    break;
                }
            }

            cFit = new TF1 (cFitname.c_str(), MyErf, cFirst1 - 10, cFirstNon0 + 10, 2 );
        }

        // Get rough midpoint & width
        double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
        double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

        cFit->SetParameter ( 0, cMid );
        cFit->SetParameter ( 1, cWidth );

        // Fit
        cProjection->Fit ( cFit, "RQ+" );
        cPedeHist->Fill ( cFit->GetParameter(0) );
        cNoiseHist->Fill ( cFit->GetParameter(1) );

        cProjection->SetDirectory (cDir);
        cProjection->Write (cProjection->GetName(), TObject::kOverwrite);
    }
}

void PedeNoise::extractPedeNoise ()
{

    //MEGATMP!!!!
    //Threshold = average
    //fThresholdError = normalization
    //Noise = rms

    ContainerFactory   theDetectorFactory;
    DetectorContainer theDifferentialContainer;
    theDetectorFactory.copyAndInitStructure<ThresholdAndNoise,EmptyContainer>(*fDetectorContainer, theDifferentialContainer);
    
    uint16_t counter = 0;
    std::map<uint16_t, DetectorContainer*>::reverse_iterator previousIterator = fSCurveOccupancyMap.rend();
    for(std::map<uint16_t, DetectorContainer*>::reverse_iterator mIt=fSCurveOccupancyMap.rbegin(); mIt!=fSCurveOccupancyMap.rend(); ++mIt)
    {
        if(previousIterator == fSCurveOccupancyMap.rend())
        {
            previousIterator = mIt;
            continue;
        }
        if(fSCurveOccupancyMap.size()-1 == counter) break;

        for ( auto board : *fDetectorContainer)
        {
            for ( auto module : *board)
            {
                for ( auto chip : *module )
                {
                    for(uint8_t iChannel=0; iChannel<chip->size(); ++iChannel)
                    {
                        float previousOccupancy = mIt->second->at(board->getId())->at(module->getId())->at(chip->getId())->getChannel<Occupancy>(iChannel).fOccupancy;
                        float currentOccupancy = (previousIterator)->second->at(board->getId())->at(module->getId())->at(chip->getId())->getChannel<Occupancy>(iChannel).fOccupancy;
                        float binCenter = (mIt->first + (previousIterator)->first)/2.;

                        theDifferentialContainer.at(board->getId())->at(module->getId())->at(chip->getId())->getChannel<ThresholdAndNoise>(iChannel).fThreshold +=
                            binCenter * (previousOccupancy - currentOccupancy);

                        theDifferentialContainer.at(board->getId())->at(module->getId())->at(chip->getId())->getChannel<ThresholdAndNoise>(iChannel).fNoise +=
                            binCenter * binCenter * (previousOccupancy - currentOccupancy) * (previousOccupancy - currentOccupancy);

                        theDifferentialContainer.at(board->getId())->at(module->getId())->at(chip->getId())->getChannel<ThresholdAndNoise>(iChannel).fThresholdError +=
                            previousOccupancy - currentOccupancy;

                    }
                }
            }
        }

        previousIterator = mIt;
        ++counter;
    }

    //calculate the averages and ship
    ThresholdAndNoiseBoardStream  theThresholdAndNoiseStream;
    
    for ( auto board : theDifferentialContainer)
    {
        for ( auto module : *board)
        {
            for ( auto chip : *module )
            {
                for(uint8_t iChannel=0; iChannel<chip->size(); ++iChannel)
                {
                    chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold/=chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError;
                    chip->getChannel<ThresholdAndNoise>(iChannel).fNoise/=chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError;
                    chip->getChannel<ThresholdAndNoise>(iChannel).fNoise = sqrt(chip->getChannel<ThresholdAndNoise>(iChannel).fNoise - (chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold * chip->getChannel<ThresholdAndNoise>(iChannel).fThreshold));
                    chip->getChannel<ThresholdAndNoise>(iChannel).fThresholdError = 0;
                }
            }
        }
        if(fStreamerEnabled) theThresholdAndNoiseStream.streamAndSendBoard(board, fNetworkStreamer);

    }


}

void PedeNoise::extractPedeNoise (std::string pHistName)
{
    // instead of looping over the Histograms and finding everything according to the CBC from the map, just loop the CBCs

    for ( auto cBoard : fBoardVector )
    {
        uint32_t cBoardId = cBoard->getBeId();

        for ( auto cFe : cBoard->fModuleVector )
        {
            uint32_t cFeId = cFe->getFeId();

            // here get the per FE histograms
            TH1F* cTmpHist = dynamic_cast<TH1F*> ( getHist ( cFe, "Module_noisehist" ) );
            TProfile* cTmpProfile = dynamic_cast<TProfile*> ( getHist ( cFe, "Module_Stripnoise" ) );

            for ( auto cCbc : cFe->fChipVector )
            {
                uint32_t cCbcId = static_cast<int> ( cCbc->getChipId() );

                // here get the per-CBC histograms
                // first the derivative of the scurves
                TH2F* cDerivative = dynamic_cast<TH2F*> (getHist (cCbc, pHistName + "_Diff") );
                //and then everything else
                TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise" ) );
                TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );
                TH1F* cStripHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Stripnoise" ) );
                TH1F* cEvenHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise_even" ) );
                TH1F* cOddHist   = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_noise_odd" ) );

                // now fill the various histograms
                for (uint16_t cChan = 0; cChan < NCHANNELS; cChan++)
                {
                    //get a projection to contain the derivative of the scurve
                    TH1D* cProjection = cDerivative->ProjectionY ("_py", cChan + 1, cChan + 1);

                    if ( cProjection->GetRMS() == 0 || cProjection->GetRMS() > 1023 ) LOG (INFO) << RED << "Error, SCurve Fit for Fe " << int ( cCbc->getFeId() ) << " Chip " << int ( cCbc->getChipId() ) << " Channel " << cChan << " did not work correctly! Noise " << cProjection->GetRMS() << RESET ;

                    if (!fFitted)
                    {
                        cNoiseHist->Fill ( cProjection->GetRMS() );
                        cPedeHist->Fill ( cProjection->GetMean() );
                    }

                    // Even and odd channel noise
                    if ( ( int (cChan) % 2 ) == 0 )
                        cEvenHist->Fill ( int ( cChan / 2 ), cProjection->GetRMS() );
                    else
                        cOddHist->Fill ( int ( cChan / 2.0 ), cProjection->GetRMS() );

                    cStripHist->Fill ( cChan, cProjection->GetRMS() );
                }

                LOG (INFO) << BOLDRED << "Average noise on FE " << +cCbc->getFeId() << " CBC " << +cCbc->getChipId() << " : " << cNoiseHist->GetMean() << " ; RMS : " << cNoiseHist->GetRMS() << " ; Pedestal : " << cPedeHist->GetMean() << " VCth units." << RESET ;

                fNoiseCanvas->cd ( fNCbc + cCbc->getChipId() + 1 );
                //cStripHist->DrawCopy();
                cEvenHist->DrawCopy();
                cOddHist->DrawCopy ( "same" );

                fPedestalCanvas->cd ( cCbc->getChipId() + 1 );
                cNoiseHist->DrawCopy();

                fPedestalCanvas->cd ( fNCbc + cCbc->getChipId() + 1 );
                cPedeHist->DrawCopy();
                fNoiseCanvas->Update();
                fPedestalCanvas->Update();
                // here add the CBC histos to the module histos
                cTmpHist->Add ( cNoiseHist );

                for ( int cChannel = 0; cChannel < NCHANNELS; cChannel++ )
                {
                    //edit suggested by B. Schneider
                    int iBin = cStripHist->FindBin (cChannel);

                    if ( cStripHist->GetBinContent ( iBin ) > 0 && cStripHist->GetBinContent ( iBin ) < 255 ) cTmpProfile->Fill ( cCbcId * 254 + cChannel, cStripHist->GetBinContent ( iBin ) );

                }
            }

            //end of cbc loop
        }

        //end of Fe loop
        this->HttpServerProcess();
    }

    //end of board loop
}

void PedeNoise::setThresholdtoNSigma (BeBoard* pBoard, uint32_t pNSigma)
{
    for ( auto cFe : pBoard->fModuleVector )
    {
        uint32_t cFeId = cFe->getFeId();

        for ( auto cCbc : cFe->fChipVector )
        {
            uint32_t cCbcId = cCbc->getChipId();
            TH1F* cNoiseHist = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Noise" ) );
            TH1F* cPedeHist  = dynamic_cast<TH1F*> ( getHist ( cCbc, "Cbc_Pedestal" ) );

            uint16_t cPedestal = round (cPedeHist->GetMean() );
            uint16_t cNoise =  round (cNoiseHist->GetMean() );
            int cDiff = fHoleMode ? pNSigma * cNoise : -pNSigma * cNoise;
            uint16_t cValue = cPedestal + cDiff;


            if (pNSigma > 0) LOG (INFO) << "Changing Threshold on CBC " << +cCbcId << " by " << cDiff << " to " << cPedestal + cDiff << " VCth units to supress noise!" ;
            else LOG (INFO) << "Changing Threshold on CBC " << +cCbcId << " back to the pedestal at " << +cPedestal ;

            ThresholdVisitor cThresholdVisitor (fChipInterface, cValue);
            cCbc->accept (cThresholdVisitor);
        }
    }
}

void PedeNoise::writeObjects()
{
    this->SaveResults();
    // just use auto iterators to write everything to disk
    // this is the old method before Tool class was cool
    fResultFile->cd();

    // Save canvasses too
    fNoiseCanvas->Write ( fNoiseCanvas->GetName(), TObject::kOverwrite );
    fPedestalCanvas->Write ( fPedestalCanvas->GetName(), TObject::kOverwrite );
    //fFeSummaryCanvas->Write ( fFeSummaryCanvas->GetName(), TObject::kOverwrite );
    fResultFile->Flush();
}


void PedeNoise::ConfigureCalibration()
{
    CreateResultDirectory ( "Results/Run_PedeNoise" );
    InitResultFile ( "PedeNoiseResults" );
}

void PedeNoise::Start(int currentRun)
{
	LOG (INFO) << "Starting noise measurement";
	Initialise ( true, true );
    measureNoise();
    Validate();
	LOG (INFO) << "Done with noise";
}

void PedeNoise::Stop()
{
    writeObjects();
    dumpConfigFiles();
    SaveResults();
    CloseResultFile();
    Destroy();
}

void PedeNoise::Pause()
{
}

void PedeNoise::Resume()
{
}

