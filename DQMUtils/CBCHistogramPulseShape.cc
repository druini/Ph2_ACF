/*!
        \file                CBCHistogramPulseShape.cc
        \brief               DQM class for Calibration example -> use it as a templare
        \author              Fabio Ravera
        \date                17/1/20
        Support :            mail to : fabio.ravera@cern.ch
*/
#include "../DQMUtils/CBCHistogramPulseShape.h"
#include "../Utils/Container.h"
#include "../Utils/ContainerFactory.h"
#include "../RootUtils/RootContainerFactory.h"
#include "../Utils/ContainerStream.h"
#include "../Utils/Occupancy.h"
#include "../RootUtils/HistContainer.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"

//========================================================================================================================
CBCHistogramPulseShape::CBCHistogramPulseShape ()
{
}

//========================================================================================================================
CBCHistogramPulseShape::~CBCHistogramPulseShape ()
{

}

//========================================================================================================================
void CBCHistogramPulseShape::book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure, const Ph2_System::SettingsMap& pSettingsMap)
{
    // SoC utilities only - BEGIN
    // THIS PART IT IS JUST TO SHOW HOW DATA ARE DECODED FROM THE TCP STREAM WHEN WE WILL GO ON THE SOC
    // IF YOU DO NOT WANT TO GO INTO THE SOC WITH YOUR CALIBRATION YOU DO NOT NEED THE FOLLOWING COMMENTED LINES
    // make fDetectorData ready to receive the information fromm the stream
    ContainerFactory::copyStructure(theDetectorStructure, fDetectorData);
    // SoC utilities only - END

    float initialVcth    = findValueInSettings(pSettingsMap, "PulseShapeInitialVcth"   , 250);
    float finalVcth      = findValueInSettings(pSettingsMap, "PulseShapeFinalVcth"     , 600);
    float vcthStep       = findValueInSettings(pSettingsMap, "PulseShapeVCthStep"      ,  10);
    float initialDelay   = findValueInSettings(pSettingsMap, "PulseShapeInitialDelay"  ,   0);
    float finalDelay     = findValueInSettings(pSettingsMap, "PulseShapeFinalDelay"    ,  25);
    float delayStep      = findValueInSettings(pSettingsMap, "PulseShapeDelayStep"     ,   1);

    float maxVCth = 1023;
    if(finalVcth>maxVCth) finalVcth = maxVCth;
    
    int delayNbins = (finalDelay - initialDelay)/delayStep + 1;
    int vcthNbins  = (finalVcth  - initialVcth )/vcthStep + 1;
    fEffectiveFinalDelay = (delayNbins-1) * delayStep + initialDelay;
    float effectiveFinalVcth  = (vcthNbins -1) * vcthStep  + initialVcth ;

    float delayHistogramMin = initialDelay - delayStep/2.;
    float vcthHistogramMin  = initialVcth  - vcthStep /2.;

    float delayHistogramMax = fEffectiveFinalDelay + delayStep/2.;
    float vcthHistogramMax  = effectiveFinalVcth   + vcthStep /2.;
    
    // auto theCanvasPulseShapeContainer = CanvasContainer<TH2F>("PulseShapePerChannel", "PulseShape Per Channel", delayNbins, delayHistogramMin, delayHistogramMax, vcthNbins, vcthHistogramMin, vcthHistogramMax);
    // RootContainerFactory::bookChannelHistograms(theOutputFile, theDetectorStructure, fDetectorChannelPulseShapeHistograms, theCanvasPulseShapeContainer);
    // theCanvasPulseShapeContainer.SetNameTitle("PulseShapePerChip", "PulseShape Per Chip");
    // RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fDetectorChipPulseShapeHistograms, theCanvasPulseShapeContainer);


    HistContainer<TH2F> theTH2FPulseShapeContainer("PulseShapePerChannel", "PulseShape Per Channel", delayNbins, delayHistogramMin, delayHistogramMax, vcthNbins, vcthHistogramMin, vcthHistogramMax);
    theTH2FPulseShapeContainer.fTheHistogram->GetXaxis()->SetTitle("delay");
    theTH2FPulseShapeContainer.fTheHistogram->GetYaxis()->SetTitle("Vcth");
    RootContainerFactory::bookChannelHistograms(theOutputFile, theDetectorStructure, fDetectorChannelPulseShapeHistograms, theTH2FPulseShapeContainer);
    
    theTH2FPulseShapeContainer.fTheHistogram->SetNameTitle("PulseShapePerChip", "PulseShape Per Chip");
    RootContainerFactory::bookChipHistograms(theOutputFile, theDetectorStructure, fDetectorChipPulseShapeHistograms, theTH2FPulseShapeContainer);
}

//========================================================================================================================
void CBCHistogramPulseShape::fillCBCPulseShapePlots(uint16_t threshold, uint16_t delay, DetectorDataContainer &theOccupancyContainer)
{

    float binCenterValue = fEffectiveFinalDelay - (delay/25)*25. - (24. - delay%25); 
    
    for(auto board : theOccupancyContainer) //for on boards - begin 
    {
        size_t boardIndex = board->getIndex();
        for(auto module: *board) //for on module - begin 
        {
            size_t moduleIndex = module->getIndex();

            for(auto chip: *module) //for on chip - begin 
            {
                size_t chipIndex = chip->getIndex();
                // Retreive the corresponging chip histogram:
                if(chip->getSummaryContainer<Occupancy,Occupancy>() == nullptr ) continue;
                TH2F *chipPulseShapeHistogram = fDetectorChipPulseShapeHistograms.at(boardIndex)->at(moduleIndex)->at(chipIndex)->getSummary<HistContainer<TH2F>>().fTheHistogram;
                chipPulseShapeHistogram->Fill(binCenterValue, threshold, chip->getSummary<Occupancy,Occupancy>().fOccupancy);
                // Check if the chip data are there (it is needed in the case of the SoC when data may be sent chip by chip and not in one shot)
                // Get channel data and fill the histogram
                uint8_t channelNumber = 0;
                for(auto channel : *chip->getChannelContainer<Occupancy>()) //for on channel - begin 
                {
                    TH2F *channelPulseShapeHistogram = fDetectorChannelPulseShapeHistograms.at(boardIndex)->at(moduleIndex)->at(chipIndex)->getChannel<HistContainer<TH2F>>(channelNumber).fTheHistogram;
                    channelPulseShapeHistogram->Fill(binCenterValue, threshold, channel.fOccupancy);
                    ++channelNumber;
                } //for on channel - end 
            } //for on chip - end 
        } //for on module - end 
    } //for on boards - end 
}

//========================================================================================================================
void CBCHistogramPulseShape::process()
{
    // This step it is not necessary, unless you want to format / draw histograms,
    // otherwise they will be automatically saved
    for(auto board : fDetectorChipPulseShapeHistograms) //for on boards - begin 
    {
        for(auto module: *board) //for on module - begin 
        {
            
            //Create a canvas do draw the plots
            TCanvas *cChipPulseShape = new TCanvas(("Hits_module_" + std::to_string(module->getId())).data(),("Hits module " + std::to_string(module->getId())).data(),   0, 0, 650, 650 );
            cChipPulseShape->Divide(0,module->size());

            for(auto chip: *module)  //for on chip - begin 
            {
                size_t chipIndex = chip->getIndex();
                cChipPulseShape->cd(chipIndex+1);
                // Retreive the corresponging chip histogram:
                TH2F *chipPulseShapeHistogram = chip->getSummary<HistContainer<TH2F>>().fTheHistogram;

                //Format the histogram (here you are outside from the SoC so you can use all the ROOT functions you need)
                chipPulseShapeHistogram->SetStats(false);
                chipPulseShapeHistogram->DrawCopy("colz");
            } //for on chip - end 
        } //for on module - end 
    } //for on boards - end 
}

//========================================================================================================================
void CBCHistogramPulseShape::reset(void)
{
    // Clear histograms if needed
}

//========================================================================================================================
bool CBCHistogramPulseShape::fill(std::vector<char>& dataBuffer)
{
    // SoC utilities only - BEGIN
    // THIS PART IT IS JUST TO SHOW HOW DATA ARE DECODED FROM THE TCP STREAM WHEN WE WILL GO ON THE SOC
    // IF YOU DO NOT WANT TO GO INTO THE SOC WITH YOUR CALIBRATION YOU DO NOT NEED THE FOLLOWING COMMENTED LINES

    //I'm expecting to receive a data stream from an uint32_t contained from calibration "CalibrationExample"
    ChipContainerStream<Occupancy,Occupancy,uint16_t,uint16_t>  theOccupancyStreamer("CBCPulseShape");

    // Try to see if the char buffer matched what I'm expection (container of uint32_t from CalibrationExample procedure)
    if(theOccupancyStreamer.attachBuffer(&dataBuffer))
    {
        //It matched! Decoding chip data
        theOccupancyStreamer.decodeChipData(fDetectorData);
        //Filling the histograms
        
        fillCBCPulseShapePlots(theOccupancyStreamer.getHeaderElement<0>(), theOccupancyStreamer.getHeaderElement<1>(), fDetectorData);
        //Cleaning the data container to be ready for the next TCP string
        fDetectorData.cleanDataStored();
        return true;
    }
    // the stream does not match, the expected (DQM interface will try to check if other DQM istogrammers are looking for this stream)
    return false;
    // SoC utilities only - END
}
