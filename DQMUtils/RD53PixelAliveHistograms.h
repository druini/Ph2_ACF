#pragma once

#include "../RootUtils/HistContainer.h"
#include "../RootUtils/RootContainerFactory.h"
#include "../Utils/GenericDataVector.h"
#include "../Utils/OccupancyAndPh.h"
#include "RD53HistogramsBase.h"
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>

class RD53PixelAliveHistograms : RD53HistogramsBase {
    DetectorDataContainer Occupancy2D;
    DetectorDataContainer ToT;
    DetectorDataContainer Occupancy1D;
    DetectorDataContainer BCID;
    DetectorDataContainer TriggerID;
    DetectorDataContainer Error;

    int nEvents;

public:
    RD53PixelAliveHistograms(int nEvents) : nEvents(nEvents) {}

    void book(TFile* theOutputFile, const DetectorContainer& theDetectorStructure);

    void fill(const DetectorDataContainer& data);

    void process();
};