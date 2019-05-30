/*!
  \file                  RD53Gain.cc
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Gain.h"

Gain::Gain(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, float startValue, float stopValue, size_t nSteps) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nEvents(nEvts),
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
  for (auto i = 0; i < nSteps; i++) dacList.push_back(startValue + step * i);


  // #######################
  // # Allocate histograms #
  // #######################
  for (auto i = 0; i < NHISTO; i++)
    {
      myString << "theOccupancy_" << i;
      theOccupancy.push_back(new TH2F(myString.str().c_str(),"Gain",nSteps,startValue,stopValue,nEvents*161./160,0,16.1));
      theOccupancy.back()->SetXTitle("VCal");
      theOccupancy.back()->SetYTitle("ToT");
    }

  theGain1D = new TH1F("theGain1D","Gain-1D",100,0,100);
  theGain1D->SetXTitle("Gain");
  theGain1D->SetYTitle("Entries");

  theIntercept1D = new TH1F("theIntercept1D","Intercept-1D",1000,0,1000);
  theIntercept1D->SetXTitle("ToT");
  theIntercept1D->SetYTitle("Entries");

  theGain2D = new TH2F("theGain2D","Gain-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theGain2D->SetXTitle("Columns");
  theGain2D->SetYTitle("Rows");

  theIntercept2D = new TH2F("theThreshold","Intercept-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theIntercept2D->SetXTitle("Columns");
  theIntercept2D->SetYTitle("Rows");

  theFile     = new TFile(fileName, "RECREATE");
  theCanvas   = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theCanvas->Divide(sqrt(theOccupancy.size()),sqrt(theOccupancy.size()));
  theCanvasGa1D = new TCanvas("theCanvasGa1D","RD53Canvas",0,0,700,500);
  theCanvasIn1D = new TCanvas("theCanvasIn1D","RD53Canvas",0,0,700,500);
  theCanvasGa2D = new TCanvas("theCanvasGa2D","RD53Canvas",0,0,700,500);
  theCanvasIn2D = new TCanvas("theCanvasIn2D","RD53Canvas",0,0,700,500);
}

Gain::~Gain()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;
  delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    delete theOccupancy[i];

  delete theCanvasGa1D;
  delete theGain1D;

  delete theCanvasIn1D;
  delete theIntercept1D;

  delete theCanvasGa2D;
  delete theGain2D;

  delete theCanvasIn2D;
  delete theIntercept2D;

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    delete detectorContainerVector[i];
}

void Gain::Run()
{
  ContainerFactory theDetectorFactory;

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorContainer());
      theDetectorFactory.copyAndInitStructure<OccupancyAndToT>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);


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
	      for (auto i = 0; i < dacList.size(); i++)
		if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy != 0)
		  theOccupancy[indx]->Fill(dacList[i],detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);
	indx++;
      }
}

void Gain::Display()
{
  for (size_t i = 0; i < theOccupancy.size(); i++)
    {
      theCanvas->cd(i+1);
      theOccupancy[i]->Draw("gcolz");
    }
  
  theCanvas->Modified();
  theCanvas->Update();

  theCanvasIn1D->cd();
  theIntercept1D->Draw();
  theCanvasIn1D->Modified();
  theCanvasIn1D->Update();

  theCanvasGa1D->cd();
  theGain1D->Draw();
  theCanvasGa1D->Modified();
  theCanvasGa1D->Update();

  theCanvasIn2D->cd();
  theIntercept2D->Draw("gcolz");
  theCanvasIn2D->Modified();
  theCanvasIn2D->Update();

  theCanvasGa2D->cd();
  theGain2D->Draw("gcolz");
  theCanvasGa2D->Modified();
  theCanvasGa2D->Update();
}

void Gain::Save()
{
  theCanvas->Write();
  theCanvasGa1D->Write();
  theCanvasIn1D->Write();
  theCanvasGa2D->Write();
  theCanvasIn2D->Write();
  theFile->Write();

  theCanvas->Print("Gain.png");
  theCanvasGa1D->Print("Gain1D.png");
  theCanvasIn1D->Print("InterceptD.png");
  theCanvasGa2D->Print("Gain2D.png");
  theCanvasIn2D->Print("Intercept2D.png");
}

void Gain::Analyze()
{
  double mean, rms;
  std::vector<double> measurements;
  measurements.push_back(0);

  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fChipVector)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    {
	      measurements.clear();

	      for (auto i = 0; i < dacList.size()-1; i++)
		measurements.push_back(detectorContainerVector[i+1]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy - 
				       detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);

	      this->ComputeStats(measurements,mean,rms);

	      theIntercept1D->Fill(mean);
	      theGain1D->Fill(rms);
	      theIntercept2D->SetBinContent(col+1,row+1,mean);
	      theGain2D->SetBinContent(col+1,row+1,rms);
	    }
}

void Gain::ComputeStats(std::vector<double>& measurements, double& mean, double& rms)
{
  double mean2  = 0;
  double weight = 0;
  mean = 0;

  for (auto i = 0; i < dacList.size(); i++)
    {
      mean   += dacList[i]*measurements[i];
      weight += measurements[i];

      mean2 += dacList[i]*dacList[i]*measurements[i];
    }

  mean /= weight;
  rms   = sqrt((mean2/weight - mean*mean) * weight / (weight - 1./nEvents));
}
