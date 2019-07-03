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
#include "../Utils/Occupancy.h"
#include "../Utils/EmptyContainer.h"
#include "../RootUtils/RootContainerFactory.h"
#include "../Utils/ContainerFactory.h"
#include "../RootUtils/TH1FContainer.h"
#include "../Utils/Container.h"
#include "TCanvas.h"
#include "TFile.h"

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
    
    //Pedestal
    TH1FContainer theTH1FPedestalContainer("PedestalDistribution", "Pedestal Distribution", 2048, -0.5, 1023.5);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorPedestalHistograms, theTH1FPedestalContainer);
    
    //Noise
    TH1FContainer theTH1FNoiseContainer("NoiseDistribution", "Noise Distribution", 200, 0., 20.);
    theRootFactory.bookChipHistrograms<TH1FContainer>(theOutputFile, theDetectorStructure, fDetectorNoiseHistograms, theTH1FNoiseContainer);
    
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
            }
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
            for(auto chip: *module)
            {
                TH1F *chipPedestalHistogram = fDetectorPedestalHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                TH1F *chipNoiseHistogram    = fDetectorNoiseHistograms   .at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
                
                if(chip->getChannelContainer<ChannelContainer<ThresholdAndNoise>>() == nullptr ) continue;
                for(auto channel : *chip->getChannelContainer<ChannelContainer<ThresholdAndNoise>>())
                {
                    chipPedestalHistogram->Fill(channel.fThreshold);
                    chipNoiseHistogram->Fill(channel.fNoise);
                }
            }
        }
    }
}
