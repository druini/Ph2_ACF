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
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowEnd; row++)
    for (auto col = colStart; col <= colEnd; col++)
      customChannelGroup.enableChannel(row,col);

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

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      if (theOccupancy[i] != nullptr) delete theOccupancy[i];
      if (theCanvasOcc[i] != nullptr) delete theCanvasOcc[i];
    }
 
  for (auto i = 0; i < theCanvasTh1D.size(); i++)
    {
      if (theThreshold1D[i] != nullptr) delete theThreshold1D[i];
      if (theCanvasTh1D[i]  != nullptr) delete theCanvasTh1D[i];
    }

  for (auto i = 0; i < theCanvasNo1D.size(); i++)
    {
      if (theNoise1D[i]    != nullptr) delete theNoise1D[i];
      if (theCanvasNo1D[i] != nullptr) delete theCanvasNo1D[i];
    }

  for (auto i = 0; i < theCanvasTh2D.size(); i++)
    {
      if (theThreshold2D[i] != nullptr) delete theThreshold2D[i];
      if (theCanvasTh2D[i]  != nullptr) delete theCanvasTh2D[i];
    }

  for (auto i = 0; i < theCanvasNo2D.size(); i++)
    {
      if (theNoise2D[i]    != nullptr) delete theNoise2D[i];
      if (theCanvasNo2D[i] != nullptr) delete theCanvasNo2D[i];
    }

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
	  size_t VCalOffset = cChip->getReg("VCAL_MED");


	  myString.clear();
	  myString.str("");
          myString << "SCurve_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),nSteps,startValue-VCalOffset,stopValue-VCalOffset,nEvents/2 + 1,0,1 + 2./nEvents));
	  theOccupancy.back()->SetXTitle("#DeltaVCal");
	  theOccupancy.back()->SetYTitle("Efficiency");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasOcc.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "theNoise1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theNoise1D.push_back( new TH1F(myString.str().c_str(),myString.str().c_str(),100,0,100));
	  theNoise1D.back()->SetXTitle("#DeltaVCal");
	  theNoise1D.back()->SetYTitle("Entries");
	  
	  myString.clear();
          myString.str("");
          myString << "theCanvasNo1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
                   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
                   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasNo1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "theThreshold1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
                   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
                   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theThreshold1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),1000,0,1000));
	  theThreshold1D.back()->SetXTitle("#DeltaVCal");
	  theThreshold1D.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
	  myString << "theCanvasTh1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasTh1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "theNoise2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
                   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
                   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theNoise2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theNoise2D.back()->SetXTitle("Columns");
	  theNoise2D.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
	  myString << "theCanvasNo2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasNo2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  
	  myString.clear();
          myString.str("");
          myString << "theThreshold_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
                   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
                   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theThreshold2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theThreshold2D.back()->SetXTitle("Columns");
	  theThreshold2D.back()->SetYTitle("Rows");
	  
	  myString.clear();
	  myString.str("");
	  myString << "theCanvasTh2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasTh2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileName, "RECREATE");
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
      theDetectorFactory.copyAndInitStructure<OccupancyPhTrim>(*fDetectorContainer, *detectorContainerVector.back());
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
		if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyPhTrim>(row,col).fOccupancy != 0)
		  theOccupancy[index]->Fill(dacList[i]-VCalOffset,detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyPhTrim>(row,col).fOccupancy);

	  index++;
	}
}


void SCurve::Display()
{
  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->cd();
      theOccupancy[i]->Draw("gcolz");
      theCanvasOcc[i]->Modified();
      theCanvasOcc[i]->Update();
    }
  
  for (auto i = 0; i < theCanvasTh1D.size(); i++)
    {
      theCanvasTh1D[i]->cd();
      theThreshold1D[i]->Draw();
      theCanvasTh1D[i]->Modified();
      theCanvasTh1D[i]->Update();
    }

  for (auto i = 0; i < theCanvasNo1D.size(); i++)
    {
      theCanvasNo1D[i]->cd();
      theNoise1D[i]->Draw();
      theCanvasNo1D[i]->Modified();
      theCanvasNo1D[i]->Update();
    }
  
  for (auto i = 0; i < theCanvasTh2D.size(); i++)
    {
      theCanvasTh2D[i]->cd();
      theThreshold2D[i]->Draw("gcolz");
      theCanvasTh2D[i]->Modified();
      theCanvasTh2D[i]->Update();
    }
 
  for (auto i = 0; i < theCanvasNo2D.size(); i++)
    {
      theCanvasNo2D[i]->cd();
      theNoise2D[i]->Draw("gcolz");
      theCanvasNo2D[i]->Modified();
      theCanvasNo2D[i]->Update();
    }
}


void SCurve::Analyze()
{
  float nHits, mean, rms;
  std::vector<float> measurements;

  theThresholdAndNoiseContainer = new DetectorDataContainer();
  ContainerFactory  theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, *theThresholdAndNoiseContainer);

  size_t index = 0;
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
		  measurements.push_back(detectorContainerVector[i+1]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyPhTrim>(row,col).fOccupancy - 
					 detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyPhTrim>(row,col).fOccupancy);
	      
		this->ComputeStats(measurements, VCalOffset, nHits, mean, rms);

		if (rms != 0)
		  {
		    theThresholdAndNoiseContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fThreshold      = mean;
		    theThresholdAndNoiseContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fThresholdError = rms / sqrt(nHits);
		    theThresholdAndNoiseContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fNoise          = rms;

		    theThreshold1D[index]->Fill(mean);
		    theNoise1D[index]->Fill(rms);
		    theThreshold2D[index]->SetBinContent(col+1,row+1,mean);
		    theNoise2D[index]->SetBinContent(col+1,row+1,rms);
		  }
	      }

	  index++;
	}
}


void SCurve::Save()
{
  std::string tmp;
  std::stringstream myString;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theOccupancy[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOccupancy[i]->GetName() << ".svg";
      theCanvasOcc[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasTh1D.size(); i++)
    {
      theThreshold1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theThreshold1D[i]->GetName() << ".svg";
      theCanvasTh1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasNo1D.size(); i++)
    {
      theNoise1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theNoise1D[i]->GetName() << ".svg";
      theCanvasNo1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasTh2D.size(); i++)
    {
      theThreshold2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theThreshold2D[i]->GetName() << ".svg";
      theCanvasTh2D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasNo2D.size(); i++)
    {
      theNoise2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theNoise2D[i]->GetName() << ".svg";
      theCanvasNo2D[i]->Print(myString.str().c_str());
    }

  theFile->Write();
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
