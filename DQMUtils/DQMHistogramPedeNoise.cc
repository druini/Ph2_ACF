/*!
        \file                DQMHistogramPedeNoise.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
 */

#include "../DQMUtils/DQMHistogramPedeNoise.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/Utilities.h"
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../RootUtils/RootContainerFactory.h"
#include "../Utils/ContainerFactory.h"
#include "../RootUtils/TH1FContainer.h"
#include "../RootUtils/TH2FContainer.h"
#include "../Utils/Container.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TF1.h"

//========================================================================================================================
DQMHistogramPedeNoise::DQMHistogramPedeNoise ()
{
}

//========================================================================================================================
DQMHistogramPedeNoise::~DQMHistogramPedeNoise ()
{

}


//========================================================================================================================
void DQMHistogramPedeNoise::book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{

    auto cSetting = pSettingsMap.find ( "PlotSCurves" );
    fPlotSCurves = ( cSetting != std::end ( pSettingsMap ) ) ? cSetting->second : 0;
    cSetting = pSettingsMap.find ( "FitSCurves" );
    fFitSCurves = ( cSetting != std::end ( pSettingsMap ) ) ? cSetting->second : 0;
    if(fFitSCurves) fPlotSCurves = true;


    ContainerFactory::copyStructure(theDetectorStructure, fDetectorData);
    
    //SCurve
    if(fPlotSCurves)
    {
        uint16_t nYbins = 1024;
        float    minY   = -0.5;
        float    maxY   = 1023.5;
        TH2FContainer theTH2FSCurve( "SCurve", "SCurve", 254, -0.5, 253.5, nYbins, minY, maxY );
        RootContainerFactory::bookChipHistograms<TH2FContainer>(theOutputFile, theDetectorStructure, fDetectorSCurveHistograms, theTH2FSCurve);
        if(fFitSCurves)
        {
            TH1FContainer theTH1FSCurveContainer("SCurve", "SCurve", nYbins, minY, maxY);
            RootContainerFactory::bookChannelHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorChannelSCurveHistograms, theTH1FSCurveContainer);

            ContainerFactory::copyAndInitStructure<ThresholdAndNoise>(theDetectorStructure, fThresholdAndNoiseContainer);    
        }
    }

    //Pedestal
    TH1FContainer theTH1FPedestalContainer("PedestalDistribution", "Pedestal Distribution", 2048, -0.5, 1023.5);
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorPedestalHistograms, theTH1FPedestalContainer);
    
    //Noise
    TH1FContainer theTH1FNoiseContainer("NoiseDistribution", "Noise Distribution", 200, 0., 20.);
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorNoiseHistograms, theTH1FNoiseContainer);
    
    //Strip Noise
    TH1FContainer theTH1FStripNoiseContainer("StripNoiseDistribution", "Strip Noise", NCHANNELS, -0.5, 253.5);
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripNoiseHistograms, theTH1FStripNoiseContainer);
    
    //Strip Pedestal
    TH1FContainer theTH1FStripPedestalContainer("StripPedestalDistribution", "Strip Pedestal", NCHANNELS, -0.5, 253.5);
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripPedestalHistograms, theTH1FStripPedestalContainer);
    
    //Strip Noise Even
    TH1FContainer theTH1FStripNoiseEvenContainer("StripNoiseEvenDistribution", "Strip Noise Even", NCHANNELS / 2, -0.5, 126.5 );
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripNoiseEvenHistograms, theTH1FStripNoiseEvenContainer);
    
    //Strip Noise Odd
    TH1FContainer theTH1FStripNoiseOddContainer("StripNoiseOddDistribution", "Strip Noise Odd", NCHANNELS / 2, -0.5, 126.5 );
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripNoiseOddHistograms, theTH1FStripNoiseOddContainer);
    
    //Module Noise
    TH1FContainer theTH1FModuleNoiseContainer("ModuleNoiseDistribution", "Module Noise Distribution", 200, 0., 20.);
    RootContainerFactory::bookModuleHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorModuleNoiseHistograms, theTH1FModuleNoiseContainer);
    
    //Module Strip Noise
    TH1FContainer theTH1FModuleStripNoiseContainer("ModuleStripNoiseDistribution", "ModuleStrip Noise", NCHANNELS*8, -0.5, NCHANNELS*8 - 0.5);
    RootContainerFactory::bookModuleHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorModuleStripNoiseHistograms, theTH1FModuleStripNoiseContainer);
    
    //Validation
    TH1FContainer theTH1FValidationContainer("Occupancy", "Occupancy", 254, -0.5, 253.5);
    RootContainerFactory::bookChipHistograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorValidationHistograms, theTH1FValidationContainer);
    
}

//========================================================================================================================
bool DQMHistogramPedeNoise::fill(std::vector<char>& dataBuffer)
{
    ChannelContainerStream<Occupancy>          theOccupancy("PedeNoise");
    ChannelContainerStream<Occupancy,uint16_t> theSCurve("SCurve");
    ChannelContainerStream<ThresholdAndNoise>  theThresholdAndNoiseStream("PedeNoise");

	if(theOccupancy.attachBuffer(&dataBuffer))
	{
		std::cout<<"Matched Occupancy!!!!!\n";
		theOccupancy.decodeChipData(fDetectorData);
        fillValidationPlots(fDetectorData);
        
	    fDetectorData.cleanDataStored();
        return true;
	}
    if(theSCurve.attachBuffer(&dataBuffer))
	{
		std::cout<<"Matched SCurve!!!!!\n";
		theSCurve.decodeChipData(fDetectorData);
        fillSCurvePlots(theSCurve.getHeaderElement(),fDetectorData);
        
	    fDetectorData.cleanDataStored();
        return true;
	}
    else if(theThresholdAndNoiseStream.attachBuffer(&dataBuffer))
    {
        std::cout<<"Matched ThresholdAndNoise!!!!!\n";
        theThresholdAndNoiseStream.decodeChipData(fDetectorData);
        fillPedestalAndNoisePlots(fDetectorData);

        fDetectorData.cleanDataStored();
        return true;
    }

        return false;
}

//========================================================================================================================
void DQMHistogramPedeNoise::process()
{

    if(fFitSCurves) fitSCurves();

    for(auto board : fDetectorPedestalHistograms)
    {
        
        for(auto module: *board)
        {
            TCanvas *cValidation = new TCanvas(("Validation_module_" + std::to_string(module->getId())).data(),("Validation module " + std::to_string(module->getId())).data(),   0, 0, 650, fPlotSCurves ? 900 : 650 );
            TCanvas *cPedeNoise  = new TCanvas(("PedeNoise_module_"  + std::to_string(module->getId())).data(),("PedeNoise module "  + std::to_string(module->getId())).data(), 670, 0, 650, 650 );

            cValidation->Divide(module->size(),fPlotSCurves ? 3 : 2);
            cPedeNoise->Divide(module->size(),2);

            for(auto chip: *module)
            {
                cValidation->cd(chip->getIndex()+1 +module->size()*0);
                TH1F *validationHistogram = fDetectorValidationHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                validationHistogram->SetStats(false);
                validationHistogram->DrawCopy();
                gPad->SetLogy();

                cValidation->cd(chip->getIndex()+1 +module->size()*1);
                TH1F *chipStripNoiseEvenHistogram = fDetectorStripNoiseEvenHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipStripNoiseOddHistogram  = fDetectorStripNoiseOddHistograms .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                chipStripNoiseEvenHistogram->SetLineColor(kBlue);
                chipStripNoiseEvenHistogram->SetMaximum (10);
                chipStripNoiseEvenHistogram->SetMinimum (0);
                chipStripNoiseOddHistogram->SetLineColor(kRed);
                chipStripNoiseOddHistogram->SetMaximum (10);
                chipStripNoiseOddHistogram->SetMinimum (0);
                chipStripNoiseEvenHistogram->SetStats(false);
                chipStripNoiseOddHistogram->SetStats(false);
                chipStripNoiseEvenHistogram->DrawCopy();
                chipStripNoiseOddHistogram->DrawCopy("same");

                cPedeNoise->cd(chip->getIndex()+1 +module->size()*1);
                fDetectorPedestalHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->DrawCopy();
                
                cPedeNoise->cd(chip->getIndex()+1 +module->size()*0);
                fDetectorNoiseHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->DrawCopy();

                if(fPlotSCurves)
                {
                    TH2F* cSCurveHist = fDetectorSCurveHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH2FContainer>().fTheHistogram;
                    TH1D* cTmp = cSCurveHist->ProjectionY();
                    cSCurveHist->GetYaxis()->SetRangeUser ( cTmp->GetBinCenter (cTmp->FindFirstBinAbove (0) ) - 10, cTmp->GetBinCenter (cTmp->FindLastBinAbove (0.99) ) + 10 );
                    delete cTmp;
                    cValidation->cd(chip->getIndex()+1 +module->size()*2);
                    cSCurveHist->SetStats(false);
                    cSCurveHist->DrawCopy("colz");
                }

                fDetectorStripNoiseHistograms    .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->GetYaxis()->SetRangeUser(0.,10.);
                fDetectorStripNoiseEvenHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->GetYaxis()->SetRangeUser(0.,10.);
                fDetectorStripNoiseOddHistograms .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->GetYaxis()->SetRangeUser(0.,10.);

            }

            fDetectorModuleStripNoiseHistograms.at(board->getIndex())->at(module->getIndex())->getSummary<TH1FContainer>().fTheHistogram->GetXaxis()->SetRangeUser(-0.5, NCHANNELS*module->size() - 0.5);
            fDetectorModuleStripNoiseHistograms.at(board->getIndex())->at(module->getIndex())->getSummary<TH1FContainer>().fTheHistogram->GetYaxis()->SetRangeUser(0.,15.);
        
        }
    }
}

//========================================================================================================================
void DQMHistogramPedeNoise::reset(void)
{

}

//========================================================================================================================
void DQMHistogramPedeNoise::fillValidationPlots(DetectorDataContainer &theOccupancy)
{
    for(auto board : theOccupancy)
    {
        for(auto module: *board)
        {
            for(auto chip: *module)
            {
                TH1F *chipValidationHistogram = fDetectorValidationHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                uint channelBin=1;
                if(chip->getChannelContainer<Occupancy>() == nullptr ) continue;
                for(auto channel : *chip->getChannelContainer<Occupancy>())
                {
                    chipValidationHistogram->SetBinContent(channelBin  ,channel.fOccupancy     );
                    chipValidationHistogram->SetBinError  (channelBin++,channel.fOccupancyError);
                }
            }
        }
    }
}

//========================================================================================================================
void DQMHistogramPedeNoise::fillPedestalAndNoisePlots(DetectorDataContainer &thePedestalAndNoise)
{
    for(auto board : thePedestalAndNoise)
    {
        for(auto module: *board)
        {
            TH1F *moduleNoiseHistogram      = fDetectorModuleNoiseHistograms     .at(board->getIndex())->at(module->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
            TH1F *moduleStripNoiseHistogram = fDetectorModuleStripNoiseHistograms.at(board->getIndex())->at(module->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
            
            for(auto chip: *module)
            {
                TH1F *chipPedestalHistogram       = fDetectorPedestalHistograms      .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipNoiseHistogram          = fDetectorNoiseHistograms         .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipStripNoiseHistogram     = fDetectorStripNoiseHistograms    .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipStripPedestalHistogram  = fDetectorStripPedestalHistograms .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipStripNoiseEvenHistogram = fDetectorStripNoiseEvenHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipStripNoiseOddHistogram  = fDetectorStripNoiseOddHistograms .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;

                if(chip->getChannelContainer<ThresholdAndNoise>() == nullptr ) continue;
                uint8_t channelNumber = 0;
                for(auto channel : *chip->getChannelContainer<ThresholdAndNoise>())
                {
                    chipPedestalHistogram->Fill(channel.fThreshold);
                    chipNoiseHistogram->Fill(channel.fNoise);
                    moduleNoiseHistogram->Fill(channel.fNoise);

                    if ( ( int (channelNumber) % 2 ) == 0 )
                    {
                        chipStripNoiseEvenHistogram->SetBinContent( int ( channelNumber / 2 ) + 1, channel.fNoise     );
                        chipStripNoiseEvenHistogram->SetBinError  ( int ( channelNumber / 2 ) + 1, channel.fNoiseError);
                    }
                    else
                    {
                        chipStripNoiseOddHistogram->SetBinContent( int ( channelNumber / 2 ) + 1, channel.fNoise     );
                        chipStripNoiseOddHistogram->SetBinError  ( int ( channelNumber / 2 ) + 1, channel.fNoiseError);
                    }

                    chipStripNoiseHistogram   ->SetBinContent(channelNumber + 1                               , channel.fNoise        );
                    chipStripNoiseHistogram   ->SetBinError  (channelNumber + 1                               , channel.fNoiseError   );
                    chipStripPedestalHistogram->SetBinContent(channelNumber + 1                               , channel.fThreshold     );
                    chipStripPedestalHistogram->SetBinError  (channelNumber + 1                               , channel.fThresholdError);
                    moduleStripNoiseHistogram ->SetBinContent(NCHANNELS * chip->getIndex() + channelNumber + 1, channel.fNoise         );
                    moduleStripNoiseHistogram ->SetBinError  (NCHANNELS * chip->getIndex() + channelNumber + 1, channel.fNoiseError    );

                    ++channelNumber;
                }
            }
        }
    }
}

//========================================================================================================================
void DQMHistogramPedeNoise::fillSCurvePlots(uint16_t vcthr, DetectorDataContainer &fSCurveOccupancy)
{
    std::cout<<__PRETTY_FUNCTION__<<" vcthr = "<< vcthr << std::endl; 

    for ( auto board : fSCurveOccupancy )
    {
        for ( auto module : *board )
        {
            for ( auto chip : *module )
            {
                TH2F *chipSCurve = fDetectorSCurveHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH2FContainer>().fTheHistogram;
    
                if(chip->getChannelContainer<ThresholdAndNoise>() == nullptr ) continue;
                uint8_t channelNumber = 0;
                for(auto channel : *chip->getChannelContainer<Occupancy>())
                {
                    float tmpOccupancy      = channel.fOccupancy     ;
                    float tmpOccupancyError = channel.fOccupancyError;
                    chipSCurve->SetBinContent(channelNumber+1, vcthr+1, tmpOccupancy     );
                    chipSCurve->SetBinError  (channelNumber+1, vcthr+1, tmpOccupancyError);

                    if(fFitSCurves)
                    {
                        TH1F *channelSCurve = fDetectorChannelSCurveHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<TH1FContainer>(channelNumber).fTheHistogram;
                        channelSCurve->SetBinContent(vcthr+1, tmpOccupancy     );
                        channelSCurve->SetBinError  (vcthr+1, tmpOccupancyError);
                    }
                    ++channelNumber;
                }
            }
        }
    }
}

//========================================================================================================================
void DQMHistogramPedeNoise::fitSCurves ()
{

    for(auto board : fDetectorChannelSCurveHistograms)
    {
        for(auto module: *board)
        {
            for(auto chip: *module)
            {

                ChipDataContainer *theChipThresholdAndNoise = fThresholdAndNoiseContainer.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex());

                for (uint32_t cChannel = 0; cChannel < NCHANNELS; cChannel++)
                {
                    TH1F *channelSCurve = chip->getChannel<TH1FContainer>(cChannel).fTheHistogram;
                    
                    float cFirstNon0 ( 0 );
                    float cFirst1 ( 0 );
                    
                    for ( Int_t cBin = 1; cBin < channelSCurve->GetNbinsX() - 1; cBin++ )
                    {
                        double cContent = channelSCurve->GetBinContent ( cBin );

                        if ( !cFirstNon0 )
                        {
                            if ( cContent ) cFirstNon0 = channelSCurve->GetBinCenter ( cBin );
                        }
                        else if ( cContent > 0.85 )
                        {
                            cFirst1 = channelSCurve->GetBinCenter ( cBin );
                            break;
                        }
                    }

                    TF1 *cFit = new TF1 ( "SCurveFit", MyErf, cFirstNon0 - 10, cFirst1 + 10, 2 );
                    
                    // Get rough midpoint & width
                    double cMid = ( cFirst1 + cFirstNon0 ) * 0.5;
                    double cWidth = ( cFirst1 - cFirstNon0 ) * 0.5;

                    cFit->SetParameter ( 0, cMid );
                    cFit->SetParameter ( 1, cWidth );

                    // Fit
                    channelSCurve->Fit ( cFit, "RQ+0" );

                    theChipThresholdAndNoise->getChannel<ThresholdAndNoise>(cChannel).fThreshold      = cFit->GetParameter(0);
                    theChipThresholdAndNoise->getChannel<ThresholdAndNoise>(cChannel).fNoise          = cFit->GetParameter(1);
                    theChipThresholdAndNoise->getChannel<ThresholdAndNoise>(cChannel).fThresholdError = cFit->GetParError (0);
                    theChipThresholdAndNoise->getChannel<ThresholdAndNoise>(cChannel).fNoiseError     = cFit->GetParError (1);

                }    
            }
        }
    }

    fillPedestalAndNoisePlots(fThresholdAndNoiseContainer);
    
}
