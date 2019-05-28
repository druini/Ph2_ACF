/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nTrig, float startValue, float stopValue, size_t nSteps) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nTriggers(nTrig),
  startValue(startValue),
  stopValue(stopValue),
  nSteps(nSteps),
  Tool()
{
  std::stringstream myString;
  myString.clear();
  myString.str("");


  // ########################
  // # Custom channel group #
  // ########################
  customBitset.reset();
  for (auto row = rowStart; row <= rowEnd; row++)
    for (auto col = colStart; col <= colEnd; col++)
      customBitset.set(RD53::nRows*col + row);
  
  customChannelGroup = new ChannelGroup<RD53::nRows,RD53::nCols>();
  customChannelGroup->setCustomPattern(customBitset);
  
  fChannelGroupHandler = new RD53ChannelGroupHandler();
  fChannelGroupHandler->setCustomChannelGroup(customChannelGroup);
  fChannelGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  float step = (stopValue - startValue) / nSteps;
  for (int i = 0; i < nSteps; i++) dacList.push_back(startValue + step * i);


  // #######################
  // # Allocate histograms #
  // #######################
  for (auto i = 0; i < NHISTO; i++)
    {
      myString << "theOccupancy_" << i;
      theOccupancy.push_back(new TH2F(myString.str().c_str(),"SCurve",nSteps,startValue,stopValue,nTriggers,0,1));
      theOccupancy.back()->SetXTitle("VCal");
      theOccupancy.back()->SetYTitle("Efficiency");
    }

  theNoise = new TH1F("theNoise","Noise",100,0,100);
  theNoise->SetXTitle("VCal");
  theNoise->SetYTitle("Entries");

  theThreshold = new TH1F("theThreshold","Threshold",1000,0,1000);
  theThreshold->SetXTitle("VCal");
  theThreshold->SetYTitle("Entries");

  theFile   = new TFile(fileName, "RECREATE");
  theCanvas = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theCanvas->Divide(sqrt(theOccupancy.size()),sqrt(theOccupancy.size()));
}

SCurve::~SCurve()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;
  delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    delete theOccupancy[i];

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    delete detectorContainerVector[i];
}

void SCurve::Run()
{
  ContainerFactory theDetectorFactory;

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorContainer());
      theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nTriggers, detectorContainerVector);


  // #########################
  // # Filling the histogram #
  // #########################
  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      {
	size_t indx = 0;
	for (auto cChip : cFe->fChipVector)
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      for (int i = 0; i < dacList.size(); i++)
		if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy != 0)
		  theOccupancy[indx]->Fill(dacList[i],detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy);
	indx++;
      }
}

void SCurve::Display()
{
  for (size_t i = 0; i < theOccupancy.size(); i++)
    {
      theCanvas->cd(i+1);
      theOccupancy[i]->Draw("gcolz");
    }
  
  theCanvas->Modified();
  theCanvas->Update();
}

void SCurve::Save()
{
  theCanvas->Write();
  theFile->Write();

  theCanvas->Print("SCurve.png");
}

void SCurve::Analyze()
{
  double mean, rms;
  std::vector<double> measurements;

  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fChipVector)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    {
	      for (int i = 0; i < dacList.size()-1; i++)
		if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy != 0)
		  measurements.push_back(detectorContainerVector[i+1]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy - 
					 detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy);
	      this->ComputeStats(measurements,mean,rms);
	      theThreshold->Fill(mean);
	      theNoise->Fill(rms);
	    }
  // LOG(INFO) << BOLDRED << "Need to implement SCurve analysis" << RESET;
  // @TMP@ Fill theNoise and theThreshold
}

void SCurve::ComputeStats(std::vector<double>& measurements, double& mean, double& rms)
{
  double mean2 = 0;
  mean = 0;

  for (auto m : measurements)
    {
      mean  += m;
      mean2 += m*m;
    }

  mean /= measurements.size();
  rms = (mean2/measurements.size() - mean*mean) * measurements.size() / (measurements.size()-1);
}
