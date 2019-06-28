/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion of SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve (const char* fileRes, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps) :
  fileRes     (fileRes),
  rowStart    (rowStart),
  rowEnd      (rowEnd),
  colStart    (colStart),
  colEnd      (colEnd),
  nPixels2Inj (nPixels2Inj),
  nEvents     (nEvents),
  startValue  (startValue),
  stopValue   (stopValue),
  nSteps      (nSteps),
  Tool        ()
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

SCurve::~SCurve ()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      delete theOccupancy[i];
      delete theCanvasOcc[i];
    }
 
  for (auto i = 0; i < theCanvasTh1D.size(); i++)
    {
      delete theThreshold1D[i];
      delete theCanvasTh1D[i];
    }

  for (auto i = 0; i < theCanvasNo1D.size(); i++)
    {
      delete theNoise1D[i];
      delete theCanvasNo1D[i];
    }

  for (auto i = 0; i < theCanvasTh2D.size(); i++)
    {
      delete theThreshold2D[i];
      delete theCanvasTh2D[i];
    }

  for (auto i = 0; i < theCanvasNo2D.size(); i++)
    {
      delete theNoise2D[i];
      delete theCanvasNo2D[i];
    }

 for (auto i = 0; i < theAxis.size(); i++) delete theAxis[i];

  for (auto i = 0; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
}

void SCurve::Run ()
{
  ContainerFactory theDetectorFactory;

  for (auto i = 0; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);
}

void SCurve::Draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();

  theFile->Close();
}

std::shared_ptr<DetectorDataContainer> SCurve::Analyze ()
{
  float nHits, mean, rms;
  std::vector<float> measurements(dacList.size(),0);

  ContainerFactory theDetectorFactory;
  delete theThresholdAndNoiseContainer;
  theThresholdAndNoiseContainer = new DetectorDataContainer();
  theDetectorFactory.copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, *theThresholdAndNoiseContainer);

  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  int VCalOffset = static_cast<RD53*>(cChip)->getReg("VCAL_MED");

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		for (auto i = 0; i < dacList.size()-1; i++)
		  measurements[i+1] = (detectorContainerVector[i+1]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy - 
				       detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy);
	      
		this->ComputeStats(measurements, VCalOffset, nHits, mean, rms);

		if ((rms > 0) && (nHits > 0) && (isnan(rms) == false))
		  {
		    theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThreshold      = mean;
		    theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThresholdError = rms / sqrt(nHits);
		    theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fNoise          = rms;
		  }
	      }
	  
	  index++;

	  theThresholdAndNoiseContainer->normalizeAndAverageContainers(fDetectorContainer, fChannelGroupHandler->allChannelGroup(), 1);
	  LOG (INFO) << BOLDGREEN << "\t--> Average threshold for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		     << std::fixed << std::setprecision(1) << theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<ThresholdAndNoise,ThresholdAndNoise>().theSummary_.fThreshold
		     << BOLDGREEN << " (Delta_VCal)" << RESET;
	}

  return std::shared_ptr<DetectorDataContainer>(theThresholdAndNoiseContainer);
}

void SCurve::InitHisto ()
{
  std::stringstream myString;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  int VCalOffset = static_cast<RD53*>(cChip)->getReg("VCAL_MED");


	  myString.clear();
	  myString.str("");
          myString << "SCurve_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),nSteps,startValue-VCalOffset,stopValue-VCalOffset,nEvents/2 + 1,0,1 + 2./nEvents));
	  theOccupancy.back()->SetXTitle("#DeltaVCal");
	  theOccupancy.back()->SetYTitle("Efficiency");

	  myString.clear();
	  myString.str("");
          myString << "CanvaScurve_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"              << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"             << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "Noise1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theNoise1D.push_back( new TH1F(myString.str().c_str(),myString.str().c_str(),100,0,20));
	  theNoise1D.back()->SetXTitle("Noise (#DeltaVCal)");
	  theNoise1D.back()->SetYTitle("Entries");
	  
	  myString.clear();
          myString.str("");
          myString << "CanvasNo1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasNo1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "Threshold1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"              << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"             << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theThreshold1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),1000,0,1000));
	  theThreshold1D.back()->SetXTitle("Threshold (#DeltaVCal)");
	  theThreshold1D.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
	  myString << "CanvasTh1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasTh1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "Noise2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theNoise2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theNoise2D.back()->SetXTitle("Columns");
	  theNoise2D.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
	  myString << "CanvasNo2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasNo2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  
	  myString.clear();
          myString.str("");
          myString << "Threshold_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theThreshold2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theThreshold2D.back()->SetXTitle("Columns");
	  theThreshold2D.back()->SetYTitle("Rows");
	  
	  myString.clear();
	  myString.str("");
	  myString << "CanvasTh2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasTh2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileRes, "UPDATE");
}

void SCurve::FillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  int VCalOffset = static_cast<RD53*>(cChip)->getReg("VCAL_MED");
	  
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		for (auto i = 0; i < dacList.size(); i++)
		  if (fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col) == true)
		    theOccupancy[index]->Fill(dacList[i]-VCalOffset,detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<Occupancy>(row,col).fOccupancy);

		if (theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fNoise != 0)
		  {
		    theThreshold1D[index]->Fill(theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThreshold);
		    theNoise1D[index]->Fill(theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fNoise);
		    theThreshold2D[index]->SetBinContent(col+1,row+1,theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fThreshold);
		    theNoise2D[index]->SetBinContent(col+1,row+1,theThresholdAndNoiseContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<ThresholdAndNoise>(row,col).fNoise);
		  }
	      }

	  index++;
	}
}

void SCurve::Display ()
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

      TPad* myPad = (TPad*)theCanvasTh1D[i]->GetPad(0);
      myPad->SetTopMargin(0.16);
      theAxis.push_back(new TGaxis(myPad->GetUxmin(), myPad->GetUymax(), myPad->GetUxmax(), myPad->GetUymax(),
				   RD53VCal2Charge::Convert(theThreshold1D[i]->GetBinLowEdge(1)),
				   RD53VCal2Charge::Convert(theThreshold1D[i]->GetBinLowEdge(theThreshold1D[i]->GetNbinsX())),
				   510,"-"));
      theAxis.back()->SetTitle("Threshold (electrons)"); 
      theAxis.back()->SetTitleOffset(1.2);
      theAxis.back()->SetTitleSize(0.035);
      theAxis.back()->SetTitleFont(40);
      theAxis.back()->SetLabelOffset(0.001);
      theAxis.back()->SetLabelSize(0.035);
      theAxis.back()->SetLabelFont(42);
      theAxis.back()->SetLabelColor(kRed);
      theAxis.back()->SetLineColor(kRed);
      theAxis.back()->Draw();

      theCanvasTh1D[i]->Modified();
      theCanvasTh1D[i]->Update();
    }

  for (auto i = 0; i < theCanvasNo1D.size(); i++)
    {
      theCanvasNo1D[i]->cd();
      theNoise1D[i]->Draw();
      theCanvasNo1D[i]->Modified();
      theCanvasNo1D[i]->Update();

      TPad* myPad = (TPad*)theCanvasNo1D[i]->GetPad(0);
      myPad->SetTopMargin(0.16);
      theAxis.push_back(new TGaxis(myPad->GetUxmin(), myPad->GetUymax(), myPad->GetUxmax(), myPad->GetUymax(),
				   RD53VCal2Charge::Convert(theNoise1D[i]->GetBinLowEdge(1),true),
				   RD53VCal2Charge::Convert(theNoise1D[i]->GetBinLowEdge(theNoise1D[i]->GetNbinsX()),true),
				   510,"-"));
      theAxis.back()->SetTitle("Noise (electrons)"); 
      theAxis.back()->SetTitleOffset(1.2);
      theAxis.back()->SetTitleSize(0.035);
      theAxis.back()->SetTitleFont(40);
      theAxis.back()->SetLabelOffset(0.001);
      theAxis.back()->SetLabelSize(0.035);
      theAxis.back()->SetLabelFont(42);
      theAxis.back()->SetLabelColor(kRed);
      theAxis.back()->SetLineColor(kRed);
      theAxis.back()->Draw();

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

void SCurve::Save ()
{
  std::stringstream myString;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOccupancy[i]->GetName() << ".svg";
      theCanvasOcc[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasTh1D.size(); i++)
    {
      theCanvasTh1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theThreshold1D[i]->GetName() << ".svg";
      theCanvasTh1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasNo1D.size(); i++)
    {
      theCanvasNo1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theNoise1D[i]->GetName() << ".svg";
      theCanvasNo1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasTh2D.size(); i++)
    {
      theCanvasTh2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theThreshold2D[i]->GetName() << ".svg";
      theCanvasTh2D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasNo2D.size(); i++)
    {
      theCanvasNo2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theNoise2D[i]->GetName() << ".svg";
      theCanvasNo2D[i]->Print(myString.str().c_str());
    }

  theFile->Write();
}

void SCurve::ComputeStats (std::vector<float>& measurements, int offset, float& nHits, float& mean, float& rms)
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
