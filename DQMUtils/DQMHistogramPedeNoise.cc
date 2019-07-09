/*!
        \file                DQMHistogramPedeNoise.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
 */

#include "../DQMUtils/DQMHistogramPedeNoise.h"
#include "../Utils/OccupancyStream.h"
#include "../Utils/ThresholdAndNoise.h"
#include "../Utils/ThresholdAndNoiseStream.h"
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
void DQMHistogramPedeNoise::book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure)
{
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyStructure(theDetectorStructure, fDetectorData);
    
    RootContainerFactory theRootFactory;
    EmptyContainer theEmptyContainer;
    
    //SCurve
    if(fPlotSCurve)
    {
        uint16_t nYbins = 1024;
        float    minY   = -0.5;
        float    maxY   = 1023.5;
        TH2FContainer theTH2FSCurve( "SCurve", "SCurve", 254, -0.5, 253.5, nYbins, minY, maxY );
        theRootFactory.bookChipHistrograms<TH2FContainer>(theOutputFile, theDetectorStructure, fDetectorSCurveHistograms, theTH2FSCurve);
        if(fFitSCurve)
        {
            TH1FContainer theTH1FSCurveContainer("SCurve", "SCurve", nYbins, minY, maxY);
            theRootFactory.bookChannelHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorChannelSCurveHistograms, theTH1FSCurveContainer);

            ContainerFactory      theDetectorFactory;
            theDetectorFactory.copyAndInitStructure<ThresholdAndNoise>(theDetectorStructure, fThresholdAndNoiseContainer);    
        }
    }

    //Pedestal
    TH1FContainer theTH1FPedestalContainer("PedestalDistribution", "Pedestal Distribution", 2048, -0.5, 1023.5);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorPedestalHistograms, theTH1FPedestalContainer);
    
    //Noise
    TH1FContainer theTH1FNoiseContainer("NoiseDistribution", "Noise Distribution", 200, 0., 20.);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorNoiseHistograms, theTH1FNoiseContainer);
    
    //Strip Noise
    TH1FContainer theTH1FStripNoiseContainer("StripNoiseDistribution", "Strip Noise", NCHANNELS, -0.5, 253.5);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripNoiseHistograms, theTH1FStripNoiseContainer);
    
    //Strip Pedestal
    TH1FContainer theTH1FStripPedestalContainer("StripPedestalDistribution", "Strip Pedestal", NCHANNELS, -0.5, 253.5);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripPedestalHistograms, theTH1FStripPedestalContainer);
    
    //Strip Noise Even
    TH1FContainer theTH1FStripNoiseEvenContainer("StripNoiseEvenDistribution", "Strip Noise Even", NCHANNELS / 2, -0.5, 126.5 );
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripNoiseEvenHistograms, theTH1FStripNoiseEvenContainer);
    
    //Strip Noise Odd
    TH1FContainer theTH1FStripNoiseOddContainer("StripNoiseOddDistribution", "Strip Noise Odd", NCHANNELS / 2, -0.5, 126.5 );
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorStripNoiseOddHistograms, theTH1FStripNoiseOddContainer);
    
    //Module Noise
    TH1FContainer theTH1FModuleNoiseContainer("ModuleNoiseDistribution", "Module Noise Distribution", 200, 0., 20.);
    theRootFactory.bookModuleHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorModuleNoiseHistograms, theTH1FModuleNoiseContainer);
    
    //Module Strip Noise
    TH1FContainer theTH1FModuleStripNoiseContainer("ModuleStripNoiseDistribution", "ModuleStrip Noise", NCHANNELS*8, -0.5, NCHANNELS*8 - 0.5);
    theRootFactory.bookModuleHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorModuleStripNoiseHistograms, theTH1FModuleStripNoiseContainer);
    
    //Validation
    TH1FContainer theTH1FValidationContainer("Occupancy", "Occupancy", 254, -0.5, 253.5);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorValidationHistograms, theTH1FValidationContainer);
    
}

//========================================================================================================================
void DQMHistogramPedeNoise::fill(std::vector<char>& dataBuffer)
{
	OccupancyBoardStream          theOccupancy;
    ThresholdAndNoiseBoardStream  theThresholdAndNoiseStream;

	if(theOccupancy.attachBuffer(&dataBuffer))
	{
		std::cout<<"Matched Occupancy!!!!!\n";
		theOccupancy.decodeChipData(fDetectorData);
        fillValidationPlots(fDetectorData);
        
	    fDetectorData.cleanDataStored();
	}
    else if(theThresholdAndNoiseStream.attachBuffer(&dataBuffer))
    {
        std::cout<<"Matched ThresholdAndNoise!!!!!\n";
        theThresholdAndNoiseStream.decodeChipData(fDetectorData);
        fillPedestalAndNoisePlots(fDetectorData);

        fDetectorData.cleanDataStored();
    }

}

//========================================================================================================================
void DQMHistogramPedeNoise::process()
{

    if(fFitSCurve) fitSCurves();

    for(auto board : fDetectorPedestalHistograms)
    {
        
        for(auto module: *board)
        {
            TCanvas *cValidation = new TCanvas(("Validation_" + std::to_string(module->getId())).data(),("Validation " + std::to_string(module->getId())).data());
            TCanvas *cPedeNoise = new TCanvas(("PedeNoise_" + std::to_string(module->getId())).data(),("PedeNoise " + std::to_string(module->getId())).data());

            cValidation->Divide(module->size());
            cPedeNoise->Divide(module->size(),2);
            int  validationPadId= 1;
            int  pedeNoisePadId= 1;

            for(auto chip: *module)
            {
                cValidation->cd(validationPadId++);
                fDetectorValidationHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->Draw();

                cPedeNoise->cd(pedeNoisePadId++);
                fDetectorPedestalHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->Draw();
                
                cPedeNoise->cd(pedeNoisePadId++);
                fDetectorNoiseHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram->Draw();

                if(fPlotSCurve)
                {
                    TH2F* cSCurveHist = fDetectorSCurveHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH2FContainer>().fTheHistogram;
                    TH1D* cTmp = cSCurveHist->ProjectionY();
                    cSCurveHist->GetYaxis()->SetRangeUser ( cTmp->GetBinCenter (cTmp->FindFirstBinAbove (0) ) - 10, cTmp->GetBinCenter (cTmp->FindLastBinAbove (0.99) ) + 10 );
                    delete cTmp;
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
                if(chip->getChannelContainer<ChannelContainer<Occupancy>>() == nullptr ) continue;
                for(auto channel : *chip->getChannelContainer<ChannelContainer<Occupancy>>())
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

                if(chip->getChannelContainer<ChannelContainer<ThresholdAndNoise>>() == nullptr ) continue;
                uint8_t channelNumber = 0;
                for(auto channel : *chip->getChannelContainer<ChannelContainer<ThresholdAndNoise>>())
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
void DQMHistogramPedeNoise::fillSCurvePlots(std::map<uint16_t, DetectorDataContainer*> fSCurveOccupancyMap)
{
    for ( auto board : fDetectorSCurveHistograms )
    {
        for ( auto module : *board )
        {
            for ( auto chip : *module )
            {
                TH2F *chipSCurve = fDetectorSCurveHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH2FContainer>().fTheHistogram;
    
                for( auto & scurveContainer : fSCurveOccupancyMap )
                {
                    for (uint32_t cChannel = 0; cChannel < NCHANNELS; cChannel++)
                    {
                        float tmpOccupancy      = scurveContainer.second->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<Occupancy>(cChannel).fOccupancy     ;
                        float tmpOccupancyError = scurveContainer.second->at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<Occupancy>(cChannel).fOccupancyError;
                        chipSCurve->SetBinContent(cChannel+1, scurveContainer.first+1, tmpOccupancy     );
                        chipSCurve->SetBinError  (cChannel+1, scurveContainer.first+1, tmpOccupancyError);

                        if(fFitSCurve)
                        {
                            TH1F *channelSCurve = fDetectorChannelSCurveHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getChannel<TH1FContainer>(cChannel).fTheHistogram;
                            channelSCurve->SetBinContent(scurveContainer.first+1, tmpOccupancy     );
                            channelSCurve->SetBinError  (scurveContainer.first+1, tmpOccupancyError);
                        }
                    }                    
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

                ChipDataContainer *theChipThresholdAndNoise = fThresholdAndNoiseContainer.at(board->getId())->at(module->getId())->at(chip->getId());

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
                    channelSCurve->Fit ( cFit, "RQ+" );

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
