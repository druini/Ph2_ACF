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
    DetectorContainer detectorContainer;
    std::map<uint16_t, Ph2_HwInterface::BeBoardFWInterface*> fBeBoardFWMap;
    std::vector<Ph2_HwDescription::BeBoard*> fBoardVector;
    std::stringstream out;
    fParser.parseHW (configurationFileName, fBeBoardFWMap, fBoardVector, &detectorContainer, out, true );
    std::cout << out.str() << std::endl;
    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyStructure(detectorContainer, fDetectorData);
    // theDetectorFactory.copyAndInitStructure<Occupancy>(detectorContainer, fDetectorData);
    EmptyContainer theEmptyContainer;
    TH1FContainer theTH1FContainer("","",254,-0.5,253.5);
    theDetectorFactory.copyAndInitStructure<EmptyContainer,TH1FContainer,EmptyContainer,EmptyContainer,EmptyContainer>(detectorContainer, fDetectorValidationHistograms, theEmptyContainer, theTH1FContainer, theEmptyContainer, theEmptyContainer, theEmptyContainer );
}

//========================================================================================================================
void DQMHistogramPedeNoise::fill(std::vector<char>& dataBuffer)
{
	std::cout << __PRETTY_FUNCTION__ << dataBuffer.size() << std::endl;
	OccupancyBoardStream theOccupancy;
	if(theOccupancy.attachBuffer(&dataBuffer))
	{
		std::cout<<"Matched!!!!!\n";
		theOccupancy.decodeChipData(fDetectorData);
		for(auto board : fDetectorData)
		{
			for(auto module: *board)
			{
				for(auto chip: *module)
				{
                    TH1F *chipHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                        fDetectorValidationHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                        )->theSummary_.fTheHistogram;
                    std::cout << __PRETTY_FUNCTION__ << __LINE__ << chipHistogram << std::endl;
                    std::cout << __PRETTY_FUNCTION__ << __LINE__ << chip->getChannelContainer<ChannelContainer<Occupancy>>() << std::endl;
                    uint channelBin=1;
                    if(chip->getChannelContainer<ChannelContainer<Occupancy>>() == nullptr ) continue;
					for(auto channel : *chip->getChannelContainer<ChannelContainer<Occupancy>>())
                    {
                        // fDetectorValidationHistograms.at(board->getId()).at(module->getId()).at(chip->getId()).fTheHistogram
                        chipHistogram->SetBinContent(channelBin++,channel.fOccupancy);
						std::cout << channel.fOccupancy << " ";
                    }
					std::cout << std::endl;
                    std::cout << __PRETTY_FUNCTION__ << __LINE__ << chipHistogram << std::endl;
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
    TCanvas *c1 = new TCanvas();
    for(auto board : fDetectorValidationHistograms)
    {
        for(auto module: *board)
        {
            c1->Divide(module->size());
            int padId = 1;
            for(auto chip: *module)
            {
                c1->cd(padId++);
                TH1F *chipHistogram = static_cast<Summary<TH1FContainer,EmptyContainer>*>(
                    fDetectorValidationHistograms.at(board->getId())->at(module->getId())->at(chip->getId())->summary_
                    )->theSummary_.fTheHistogram;
                std::string cHistname = Form ( "Fe%dCBC%d_Occupancy", module->getId(), chip->getId() );
                chipHistogram->SetNameTitle(cHistname.data(), cHistname.data());
                chipHistogram->Write();
                chipHistogram->Draw();
            }
        }
    }

    c1->Write();
    delete c1;
    output.Close();
}

//========================================================================================================================
void DQMHistogramPedeNoise::reset(void)
{

}
