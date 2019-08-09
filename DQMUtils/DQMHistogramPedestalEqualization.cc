/*!
        \file                DQMHistogramPedestalEqualization.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
 */

#include "../DQMUtils/DQMHistogramPedestalEqualization.h"
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
DQMHistogramPedestalEqualization::DQMHistogramPedestalEqualization ()
{
}

//========================================================================================================================
DQMHistogramPedestalEqualization::~DQMHistogramPedestalEqualization ()
{

}


//========================================================================================================================
void DQMHistogramPedestalEqualization::book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure, std::map<std::string, uint32_t> pSettingsMap)
{
    ContainerFactory::copyStructure(theDetectorStructure, fDetectorData);

    HistContainer<TH1I> hVplus("VplusValue","Vplus Value",1, 0, 1);
    RootContainerFactory::bookChipHistograms(theOutputFile,theDetectorStructure,fDetectorVplusHistograms,hVplus);

    HistContainer<TH1I> hOffset("OffsetValues","Offset Values",254, -.5, 253.5 );
    RootContainerFactory::bookChipHistograms(theOutputFile,theDetectorStructure,fDetectorOffsetHistograms,hOffset);

    HistContainer<TH1F> hOccupancy("OccupancyAfterOffsetEqualization","Occupancy After Offset Equalization",254, -.5, 253.5 );
    RootContainerFactory::bookChipHistograms(theOutputFile,theDetectorStructure,fDetectorOccupancyHistograms,hOccupancy);



}

//========================================================================================================================
bool DQMHistogramPedestalEqualization::fill(std::vector<char>& dataBuffer)
{
        return false;
}

//========================================================================================================================
void DQMHistogramPedestalEqualization::process()
{
    TCanvas* offsetCanvas    = new TCanvas ( "Offset", "Offset", 10, 0, 500, 500 );
    TCanvas* occupancyCanvas = new TCanvas ( "Occupancy", "Occupancy", 10, 525, 500, 500 );


    for(auto board : fDetectorOffsetHistograms)
    { 
        for(auto module: *board)
        {
            TCanvas* offsetCanvas    = new TCanvas ( "Offset", "Offset", 10, 0, 500, 500 );
            TCanvas* occupancyCanvas = new TCanvas ( "Occupancy", "Occupancy", 10, 525, 500, 500 );
            
        }

            // for(auto chip: *module)
            // {
            //     cValidation->cd(chip->getIndex()+1 +module->size()*0);
            //     TH1F *validationHistogram = fDetectorValidationHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<TH1FContainer>().fTheHistogram;
            //     validationHistogram->DrawCopy();
            // }
    
    }

}

//========================================================================================================================

void DQMHistogramPedestalEqualization::reset(void)
{

}

//========================================================================================================================
void DQMHistogramPedestalEqualization::fillVplusPlots(DetectorDataContainer &theVthr)
{
    for(auto board : theVthr)
    {
        for(auto module: *board)
        {
            for(auto chip: *module)
            {
                TH1I *chipVplusHistogram = fDetectorVplusHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<HistContainer<TH1I>>().fTheHistogram;
                chipVplusHistogram->SetBinContent(254, chip->getSummary<uint16_t>());
            }
        }
    }
}

//========================================================================================================================

void DQMHistogramPedestalEqualization::fillOccupancyPlots(DetectorDataContainer &theOccupancy)
{
    for(auto board : theOccupancy)
    {
        for(auto module: *board)
        {
            for(auto chip: *module)
            {
                TH1F *chipOccupancyHistogram = fDetectorOccupancyHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<HistContainer<TH1F>>().fTheHistogram;
                if(chip->getChannelContainer<Occupancy>() == nullptr ) continue;
                uint channelBin=1;
                for(auto channel : *chip->getChannelContainer<Occupancy>())
                {
                    chipOccupancyHistogram->SetBinContent(channelBin  ,channel.fOccupancy     );
                    chipOccupancyHistogram->SetBinError  (channelBin++,channel.fOccupancyError);
                }
            }

        }
    }
}

//========================================================================================================================

void DQMHistogramPedestalEqualization::fillOffsetPlots(DetectorDataContainer &theOffsets)
{
    for(auto board : theOffsets)
    {
        for(auto module: *board)
        {
            for(auto chip: *module)
            {
                TH1I *chipOffsetHistogram = fDetectorOffsetHistograms.at(board->getIndex())->at(module->getIndex())->at(chip->getIndex())->getSummary<HistContainer<TH1I>>().fTheHistogram;
                if(chip->getChannelContainer<uint8_t>() == nullptr ) continue;
                uint channelBin=1;
                for(auto channel : *chip->getChannelContainer<uint8_t>())
                {
                    chipOffsetHistogram->SetBinContent(channelBin++,channel );
                }
            }
        }
    }
}

//========================================================================================================================