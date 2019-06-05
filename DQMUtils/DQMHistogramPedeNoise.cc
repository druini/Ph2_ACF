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
#include "../RootUtils/TH1FContainer.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../System/FileParser.h"
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
void DQMHistogramPedeNoise::book(std::string configurationFileName)
{
	Ph2_System::FileParser fParser;
    std::map<uint16_t, Ph2_HwInterface::BeBoardFWInterface*> fBeBoardFWMap;
    std::vector<Ph2_HwDescription::BeBoard*> fBoardVector;
    std::stringstream out;
    fParser.parseHW (configurationFileName, fBeBoardFWMap, fBoardVector, &fDetectorStructure, out, true );
    std::cout << out.str() << std::endl;
    ContainerFactory   theDetectorFactory;
    EmptyContainer theEmptyContainer;

    //Pedestal
    TH1FContainer theTH1FPedestalContainer("", "", 2048, -0.5, 1023.5);
    theDetectorFactory.copyAndInitStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(fDetectorStructure, fDetectorPedestalHistograms, theEmptyContainer, theTH1FPedestalContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );

    //Noise
    TH1FContainer theTH1FNoiseContainer("", "", 200, 0., 20.);
    theDetectorFactory.copyAndInitStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(fDetectorStructure, fDetectorNoiseHistograms, theEmptyContainer, theTH1FNoiseContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );

    //Validation
    TH1FContainer theTH1FValidationContainer("", "", 254, -0.5, 253.5);
    theDetectorFactory.copyAndInitStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(fDetectorStructure, fDetectorValidationHistograms, theEmptyContainer, theTH1FValidationContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );

    //Data container initialization
    theDetectorFactory.copyStructure(fDetectorStructure, fDetectorData);
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
        // fillOccupancy(fDetectorData);
        
		for(auto board : fDetectorData)
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
		//If I want to keep the data then I just copy them in another container
        fDetectorData.cleanDataStored();
	}
    else if(theThresholdAndNoiseStream.attachBuffer(&dataBuffer))
    {
        std::cout<<"Matched ThresholdAndNoise!!!!!\n";
        theThresholdAndNoiseStream.decodeChipData(fDetectorData);
        for(auto board : fDetectorData)
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
        fDetectorData.cleanDataStored();
    }

}

//========================================================================================================================
void DQMHistogramPedeNoise::save(const std::string& outFile)
{
    TFile output(outFile.data(), "RECREATE");
    //@TMP
    TCanvas *cValidation = new TCanvas();

    for(auto board : fDetectorStructure)
    {
        for(auto module: *board)
        {
            cValidation->Divide(module->size());
            int padId = 1;
            for(auto chip: *module)
            {
                cValidation->cd(padId++);
                TH1F *chipValidationHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorValidationHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;
                std::string cHistname = Form ( "Fe%dCBC%d_Occupancy", module->getId(), chip->getId() );
                chipValidationHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                chipValidationHistogram->Write();
                chipValidationHistogram->Draw();
            }
        }
    }

    TCanvas *cPedeNoise = new TCanvas();

    for(auto board : fDetectorStructure)
    {
        for(auto module: *board)
        {
            cPedeNoise->Divide(module->size(),2);
            int padId = 1;
            for(auto chip: *module)
            {
                cPedeNoise->cd(padId++);
                TH1F *chipPedestalHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorPedestalHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;

                std::string cHistname = Form ( "Fe%dCBC%d_Pedestal", module->getId(), chip->getId() );
                chipPedestalHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                chipPedestalHistogram->Write();
                chipPedestalHistogram->Draw();

                cPedeNoise->cd(padId++);
                TH1F *chipNoiseHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorNoiseHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;

                cHistname = Form ( "Fe%dCBC%d_Noise", module->getId(), chip->getId() );
                chipNoiseHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                chipNoiseHistogram->Write();
                chipNoiseHistogram->Draw();


            }
        }
    }

    cValidation->Write();
    cPedeNoise->Write();
    //delete c1;
    //output.Close();
}

//========================================================================================================================
void DQMHistogramPedeNoise::reset(void)
{

}
