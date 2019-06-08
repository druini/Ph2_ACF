/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, size_t startValue, size_t stopValue, size_t nSteps) :
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
}


SCurve::~SCurve()
{
  theFile->Close();
  
  if (fChannelGroupHandler != nullptr) delete fChannelGroupHandler;
  if (theFile              != nullptr) delete theFile;
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      if (theOccupancy[i] != nullptr) delete theOccupancy[i];
      if (theCanvas[i]    != nullptr) delete theCanvas[i];
    }
 
  if (theThreshold1D != nullptr) delete theThreshold1D;
  if (theCanvasTh1D  != nullptr) delete theCanvasTh1D;

  if (theNoise1D    != nullptr)  delete theNoise1D;
  if (theCanvasNo1D != nullptr)  delete theCanvasNo1D;

  if (theThreshold2D != nullptr) delete theThreshold2D;
  if (theCanvasTh2D  != nullptr) delete theCanvasTh2D;

  if (theNoise2D    != nullptr)  delete theNoise2D;
  if (theCanvasNo2D != nullptr)  delete theCanvasNo2D;

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    if (detectorContainerVector[i] != nullptr) delete detectorContainerVector[i];
  
  if (theThresholdAndNoiseContainer != nullptr) delete theThresholdAndNoiseContainer;
}


void SCurve::InitHisto()
{
  std::stringstream myString;

  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
	{
	  myString.clear();
	  myString.str("");
          myString << "SCurve_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),nSteps,startValue,stopValue,nEvents/2 + 1,0,1 + 2./nEvents));
	  theOccupancy.back()->SetXTitle("#DeltaVCal");
	  theOccupancy.back()->SetYTitle("Efficiency");

	  myString.clear();
	  myString.str("");
          myString << "theCanvas_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvas.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theNoise1D = new TH1F("theNoise1D","Noise-1D",100,0,100);
  theNoise1D->SetXTitle("#DeltaVCal");
  theNoise1D->SetYTitle("Entries");

  theThreshold1D = new TH1F("theThreshold1D","Threshold-1D",1000,0,1000);
  theThreshold1D->SetXTitle("#DeltaVCal");
  theThreshold1D->SetYTitle("Entries");

  theNoise2D = new TH2F("theNoise2D","Noise-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theNoise2D->SetXTitle("Columns");
  theNoise2D->SetYTitle("Rows");

  theThreshold2D = new TH2F("theThreshold","Threshold-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theThreshold2D->SetXTitle("Columns");
  theThreshold2D->SetYTitle("Rows");

  theFile       = new TFile(fileName, "RECREATE");
  theCanvasTh1D = new TCanvas("theCanvasTh1D","RD53Canvas",0,0,700,500);
  theCanvasNo1D = new TCanvas("theCanvasNo1D","RD53Canvas",0,0,700,500);
  theCanvasTh2D = new TCanvas("theCanvasTh2D","RD53Canvas",0,0,700,500);
  theCanvasNo2D = new TCanvas("theCanvasNo2D","RD53Canvas",0,0,700,500);
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
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      theDetectorFactory.copyAndInitStructure<OccupancyAndToT>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);


  // #########################
  // # Filling the histogram #
  // #########################
  size_t index = 0;
  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
	{
	  size_t VCalOffset = cChip->getReg("VCAL_MED");

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      for (auto i = 0; i < dacList.size(); i++)
		{
		  if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy != 0)
		    theOccupancy[index]->Fill(dacList[i]-VCalOffset,detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);
		}

	  index++;
	}
}


void SCurve::Display()
{
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      theCanvas[i]->cd();
      theOccupancy[i]->Draw("gcolz");
      theCanvas[i]->Modified();
      theCanvas[i]->Update();
    }
  
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
  float nHits, mean, rms;
  std::vector<float> measurements;

  theThresholdAndNoiseContainer = new DetectorDataContainer();
  ContainerFactory  theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, *theThresholdAndNoiseContainer);

  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
	{
	  size_t VCalOffset = cChip->getReg("VCAL_MED");

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		measurements.clear();
		measurements.push_back(0);

		for (auto i = 0; i < dacList.size()-1; i++)
		  measurements.push_back(detectorContainerVector[i+1]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy - 
					 detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);
	      
		this->ComputeStats(measurements, VCalOffset, nHits, mean, rms);

		if (rms != 0)
		  {
		    theThresholdAndNoiseContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fThreshold      = mean;
		    theThresholdAndNoiseContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fThresholdError = rms / sqrt(nHits);
		    theThresholdAndNoiseContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fNoise          = rms;

		    theThreshold1D->Fill(mean);
		    theNoise1D->Fill(rms);
		    theThreshold2D->SetBinContent(col+1,row+1,mean);
		    theNoise2D->SetBinContent(col+1,row+1,rms);
		  }
	      }
	}
}


void SCurve::Save()
{
  std::stringstream myString;

  for (auto i = 0; i < theCanvas.size(); i++)
    {
      theOccupancy[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOccupancy[i]->GetName() << ".svg";
      theCanvas[i]->Print(myString.str().c_str());
    }

  theThreshold1D->Write();
  theNoise1D->Write();
  theThreshold2D->Write();
  theNoise2D->Write();
  theFile->Write();

  theCanvasTh1D->Print("SCurveTh1D.svg");
  theCanvasNo1D->Print("SCurveNo1D.svg");
  theCanvasTh2D->Print("SCurveTh2D.svg");
  theCanvasNo2D->Print("SCurveNo2D.svg");
}


void SCurve::ComputeStats(std::vector<float>& measurements, size_t offset, float& nHits, float& mean, float& rms)
{
  float mean2  = 0;
  float weight = 0;
  mean         = 0;

  for (auto i = 0; i < dacList.size(); i++)
    {
      mean   += measurements[i]*(dacList[i]-offset);
      weight += measurements[i];

      mean2  += measurements[i]*(dacList[i]-offset)*(dacList[i]-offset);
    }

  nHits = weight * nEvents;

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
