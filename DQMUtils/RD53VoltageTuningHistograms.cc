/*!
  \file                  RD53VoltageTuningHistograms.cc
  \brief                 Implementation of Threshold calibration histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53VoltageTuningHistograms.h"

using namespace Ph2_HwDescription;

void VoltageTuningHistograms::book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure, const Ph2_System::SettingsMap& settingsMap)
{
    ContainerFactory::copyStructure(theDetectorStructure, DetectorData);

    //    const uint16_t rangeVoltage_dig = RD53Shared::setBits(static_cast<RD53*>(theDetectorStructure.at(0)->at(0)->at(0)->at(0))->getNumberOfBits("VOUT_dig_ShuLDO")) + 1;

    //    const uint16_t rangeVoltage_ana = RD53Shared::setBits(static_cast<RD53*>(theDetectorStructure.at(0)->at(0)->at(0)->at(0))->getNumberOfBits("VOUT_ana_ShuLDO")) + 1;

    //    std::cout << "Nbins = " << rangeVoltage_dig << " " << rangeVoltage_ana << " " << rangeVoltage_test << " " << rangeVoltage_test2 << std::endl;
    
    auto hVoltage_dig = CanvasContainer<TH1F>("Voltage_dig", "Voltage_dig", 32, 0, 32);

    auto hVoltage_ana = CanvasContainer<TH1F>("Voltage_ana", "Voltage_ana", 32, 0, 32);
    
    bookImplementer(theOutputFile, theDetectorStructure, Voltage_dig, hVoltage_dig, "Voltage_dig", "Entries");
    bookImplementer(theOutputFile, theDetectorStructure, Voltage_ana, hVoltage_ana, "Voltage_ana", "Entries");
}

bool VoltageTuningHistograms::fill(std::vector<char>& dataBuffer)
{
    ChipContainerStream<EmptyContainer, uint16_t> thedigiStreamer("Voltage_dig");
    ChipContainerStream<EmptyContainer, uint16_t> theanaStreamer("Voltage_ana");

    if(thedigiStreamer.attachBuffer(&dataBuffer))
    {
        thedigiStreamer.decodeChipData(DetectorData);
        VoltageTuningHistograms::fill_dig(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }
    else if(theanaStreamer.attachBuffer(&dataBuffer))
    {
        theanaStreamer.decodeChipData(DetectorData);
        VoltageTuningHistograms::fill_dig(DetectorData);
        DetectorData.cleanDataStored();
        return true;
    }

    return false;
}

void VoltageTuningHistograms::fill_dig(const DetectorDataContainer& DataContainer)
{
    for(const auto cBoard: DataContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

		    auto* hVoltage_dig = Voltage_dig.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
		    
                    hVoltage_dig->Fill(cChip->getSummary<uint16_t>());
		}
}



void VoltageTuningHistograms::fill_ana(const DetectorDataContainer& DataContainer)
{
    for(const auto cBoard: DataContainer)
        for(const auto cOpticalGroup: *cBoard)
            for(const auto cHybrid: *cOpticalGroup)
                for(const auto cChip: *cHybrid)
                {
                    if(cChip->getSummaryContainer<uint16_t>() == nullptr) continue;

                    auto* hVoltage_ana = Voltage_ana.at(cBoard->getIndex())->at(cOpticalGroup->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex())->getSummary<CanvasContainer<TH1F>>().fTheHistogram;
		    
                    hVoltage_ana->Fill(cChip->getSummary<uint16_t>());

		}
}


void VoltageTuningHistograms::process() {
  draw<TH1F>(Voltage_ana);
  draw<TH1F>(Voltage_dig);
}
