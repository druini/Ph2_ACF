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
void DQMHistogramPedeNoise::book(TFile *theOutputFile, DetectorContainer &theDetectorStructure)
{
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyStructure(theDetectorStructure,fDetectorStructure);
    //Data container initialization
    theDetectorFactory.copyStructure(fDetectorStructure, fDetectorData);
    
    RootContainerFactory theRootFactory;
    
    EmptyContainer theEmptyContainer;

    //Pedestal
    TH1FContainer theTH1FPedestalContainer("PedestalDistribution", "Pedestal Distribution", 2048, -0.5, 1023.5);
    theRootFactory.bookHistrogramsFromStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, fDetectorStructure, fDetectorPedestalHistograms, theEmptyContainer, theTH1FPedestalContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );

    //Noise
    TH1FContainer theTH1FNoiseContainer("NoiseDistribution", "Noise Distribution", 200, 0., 20.);
    theRootFactory.bookHistrogramsFromStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, fDetectorStructure, fDetectorNoiseHistograms, theEmptyContainer, theTH1FNoiseContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );

    //Validation
    TH1FContainer theTH1FValidationContainer("ThresholdDistribution", "Threshold Distribution", 254, -0.5, 253.5);
    theRootFactory.bookHistrogramsFromStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(theOutputFile, fDetectorStructure, fDetectorValidationHistograms, theEmptyContainer, theTH1FValidationContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );

}

//========================================================================================================================
void DQMHistogramPedeNoise::fill(std::vector<char>& dataBuffer)
{
	OccupancyBoardStream          theOccupancy;
    ThresholdAndNoiseBoardStream  theThresholdAndNoiseStream;

	//TODO Occupancy histos and Occupancy should be used and filled the same way so there should be no need to pass through the detector data
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

    for(auto board : fDetectorStructure)
    {
        std::string boardFolder = "Board_" + std::to_string(board->getId());
        // if(output.TDirectory::GetDirectory(boardFolder.data()) == nullptr) output.mkdir(boardFolder.data());
        // output.cd(boardFolder.data());

        for(auto module: *board)
        {
            // std::string moduleFolder = boardFolder + "/FE_" + std::to_string(module->getId());
            // if(output.TDirectory::GetDirectory(moduleFolder.data()) == nullptr) output.mkdir(moduleFolder.data());
            // output.cd(moduleFolder.data());

            TCanvas *cValidation = new TCanvas(("Validation_" + std::to_string(module->getId())).data(),("Validation " + std::to_string(module->getId())).data());
            TCanvas *cPedeNoise = new TCanvas(("PedeNoise_" + std::to_string(module->getId())).data(),("PedeNoise " + std::to_string(module->getId())).data());

            cValidation->Divide(module->size());
            cPedeNoise->Divide(module->size(),2);
            int  validationPadId= 1;
            int  pedeNoisePadId= 1;

            for(auto chip: *module)
            {
                // std::string chipFolder = moduleFolder + "/Chip_" + std::to_string(chip->getId());
                // if(output.TDirectory::GetDirectory(chipFolder.data()) == nullptr) output.mkdir(chipFolder.data());
                // output.cd(chipFolder.data());

                cValidation->cd(validationPadId++);
                TH1F *chipValidationHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorValidationHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;
                // std::string cHistname = Form ( "Fe%dCBC%d_Occupancy", module->getId(), chip->getId() );
                // chipValidationHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                // chipValidationHistogram->Write();
                chipValidationHistogram->Draw();

                cPedeNoise->cd(pedeNoisePadId++);
                TH1F *chipPedestalHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorPedestalHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;

                // cHistname = Form ( "Fe%dCBC%d_Pedestal", module->getId(), chip->getId() );
                // chipPedestalHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                // chipPedestalHistogram->Write();
                chipPedestalHistogram->Draw();

                cPedeNoise->cd(pedeNoisePadId++);
                TH1F *chipNoiseHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorNoiseHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;

                // cHistname = Form ( "Fe%dCBC%d_Noise", module->getId(), chip->getId() );
                // chipNoiseHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                // chipNoiseHistogram->Write();
                chipNoiseHistogram->Draw();
            }

            // output.cd(moduleFolder.data());

            // cValidation->Write();
            // cPedeNoise->Write();
        }
    }

    //delete c1;
    //output.Close();
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
                TH1F *chipValidationHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorValidationHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;
                
                uint channelBin=1;
                if(chip->getChannelContainer<ChannelContainer<Occupancy>>() == nullptr ) continue;
                for(auto channel : *chip->getChannelContainer<ChannelContainer<Occupancy>>())
                {
                    // fDetectorValidationHistograms.at(board->getId()).at(module->getId()).at(chip->getId()).fTheHistogram
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
                TH1F *chipPedestalHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorPedestalHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;
                
                TH1F *chipNoiseHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorNoiseHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;

                // uint channelBin=1;
                if(chip->getChannelContainer<ChannelContainer<ThresholdAndNoise>>() == nullptr ) continue;
                for(auto channel : *chip->getChannelContainer<ChannelContainer<ThresholdAndNoise>>())
                {
                    // fDetectorValidationHistograms.at(board->getId()).at(module->getId()).at(chip->getId()).fTheHistogram
                    chipPedestalHistogram->Fill(channel.fThreshold);
                    chipNoiseHistogram->Fill(channel.fNoise);
                }
            }
        }
    }
}
