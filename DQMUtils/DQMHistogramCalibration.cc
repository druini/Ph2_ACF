/*!
        \file                DQMHistogramCalibration.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
 */

#include "../DQMUtils/DQMHistogramCalibration.h"
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
DQMHistogramCalibration::DQMHistogramCalibration ()
{
}

//========================================================================================================================
DQMHistogramCalibration::~DQMHistogramCalibration ()
{

}


//========================================================================================================================
void DQMHistogramCalibration::book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure, std::map<std::string, double> pSettingsMap)
{

    ContainerFactory   theDetectorFactory;
    theDetectorFactory.copyStructure(theDetectorStructure, fDetectorData);
    
    RootContainerFactory theRootFactory;
    
}

//========================================================================================================================
bool DQMHistogramCalibration::fill(std::vector<char>& dataBuffer)
{
        return false;
}

//========================================================================================================================
void DQMHistogramCalibration::process()
{

}

//========================================================================================================================
void DQMHistogramCalibration::reset(void)
{

}

