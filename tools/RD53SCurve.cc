/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, float startValue, float stopValue, size_t nSteps) :
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
      theOccupancy.push_back(new TH2F(myString.str().c_str(),"SCurve",nSteps,startValue,stopValue,nEvents+1,0,1+1./nEvents));
      theOccupancy.back()->SetXTitle("VCal");
      theOccupancy.back()->SetYTitle("Efficiency");
    }

  theNoise1D = new TH1F("theNoise1D","Noise-1D",100,0,100);
  theNoise1D->SetXTitle("VCal");
  theNoise1D->SetYTitle("Entries");

  theThreshold1D = new TH1F("theThreshold1D","Threshold-1D",1000,0,1000);
  theThreshold1D->SetXTitle("VCal");
  theThreshold1D->SetYTitle("Entries");

  theNoise2D = new TH2F("theNoise2D","Noise-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theNoise2D->SetXTitle("Columns");
  theNoise2D->SetYTitle("Rows");

  theThreshold2D = new TH2F("theThreshold","Threshold-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theThreshold2D->SetXTitle("Columns");
  theThreshold2D->SetYTitle("Rows");

  theFile       = new TFile(fileName, "RECREATE");
  theCanvas     = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theCanvas->Divide(sqrt(theOccupancy.size()),sqrt(theOccupancy.size()));
  theCanvasTh1D = new TCanvas("theCanvasTh1D","RD53Canvas",0,0,700,500);
  theCanvasNo1D = new TCanvas("theCanvasNo1D","RD53Canvas",0,0,700,500);
  theCanvasTh2D = new TCanvas("theCanvasTh2D","RD53Canvas",0,0,700,500);
  theCanvasNo2D = new TCanvas("theCanvasNo2D","RD53Canvas",0,0,700,500);
}

SCurve::~SCurve()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;
  delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    delete theOccupancy[i];

  delete theCanvasNo1D;
  delete theNoise1D;

  delete theCanvasTh1D;
  delete theThreshold1D;

  delete theCanvasNo2D;
  delete theNoise2D;

  delete theCanvasTh2D;
  delete theThreshold2D;

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

void SCurve::Display()
{
  for (size_t i = 0; i < theOccupancy.size(); i++)
    {
      theCanvas->cd(i+1);
      theOccupancy[i]->Draw("gcolz");
    }
  
  theCanvas->Modified();
  theCanvas->Update();

  theCanvasTh1D->cd();
  theThreshold1D->Draw();
  theCanvasTh1D->Modified();
  theCanvasTh1D->Update();

  theCanvasNo1D->cd();
  theNoise1D->Draw();
  theCanvasNo1D->Modified();
  theCanvasNo1D->Update();

  theCanvasTh2D->cd();
  theThreshold2D->Draw("gcolz");
  theCanvasTh2D->Modified();
  theCanvasTh2D->Update();

  theCanvasNo2D->cd();
  theNoise2D->Draw("gcolz");
  theCanvasNo2D->Modified();
  theCanvasNo2D->Update();
}

void SCurve::Analyze()
{
  float mean, rms;
  std::vector<float> measurements;

  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fChipVector)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    {
	      measurements.clear();
	      measurements.push_back(0);

	      for (auto i = 0; i < dacList.size()-1; i++)
		measurements.push_back(detectorContainerVector[i+1]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy - 
				       detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);
	      
	      this->ComputeStats(measurements,mean,rms);

	      if (rms != 0)
		{
		  theThreshold1D->Fill(mean);
		  theNoise1D->Fill(rms);
		  theThreshold2D->SetBinContent(col+1,row+1,mean);
		  theNoise2D->SetBinContent(col+1,row+1,rms);
		}
	    }
}

void SCurve::Save()
{
  theCanvas->Write();
  theCanvasTh1D->Write();
  theCanvasNo1D->Write();
  theCanvasTh2D->Write();
  theCanvasNo2D->Write();
  theFile->Write();

  theCanvas->Print("SCurve.png");
  theCanvasTh1D->Print("SCurveTh1D.png");
  theCanvasNo1D->Print("SCurveNo1D.png");
  theCanvasTh2D->Print("SCurveTh2D.png");
  theCanvasNo2D->Print("SCurveNo2D.png");
}

void SCurve::ComputeStats(std::vector<float>& measurements, float& mean, float& rms)
{
  float mean2  = 0;
  float weight = 0;
  mean         = 0;

  for (auto i = 0; i < dacList.size(); i++)
    {
      mean   += measurements[i]*dacList[i];
      weight += measurements[i];

      mean2  += measurements[i]*dacList[i]*dacList[i];
    }

  if (weight != 0)
    {
      mean /= weight;
      rms   = sqrt((mean2/weight - mean*mean) * weight / (weight - 1./nEvents));
    }
  else
    {
      mean = 0;
      rms  = 0;
    }
}
