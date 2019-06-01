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

Gain::~Gain()
{
  theFile->Close();
  
  if (fChannelGroupHandler != nullptr) delete fChannelGroupHandler;
  if (theFile != nullptr)              delete theFile;
  if (theCanvas != nullptr)            delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    if (theOccupancy[i] != nullptr) delete theOccupancy[i];

  if (theCanvasGa1D != nullptr)  delete theCanvasGa1D;
  if (theGain1D != nullptr)      delete theGain1D;

  if (theCanvasIn1D != nullptr)  delete theCanvasIn1D;
  if (theIntercept1D != nullptr) delete theIntercept1D;

  if (theCanvasGa2D != nullptr)  delete theCanvasGa2D;
  if (theGain2D != nullptr)      delete theGain2D;

  if (theCanvasIn2D != nullptr)  delete theCanvasIn2D;
  if (theIntercept2D != nullptr) delete theIntercept2D;

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    if (detectorContainerVector[i] != nullptr) delete detectorContainerVector[i];
  
  if (theGainAndInterceptContainer != nullptr) delete theGainAndInterceptContainer;
}

void Gain::InitHisto()
{
  std::stringstream myString;

  // #######################
  // # Allocate histograms #
  // #######################
  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fChipVector)
	{
	  myString.clear();
	  myString.str("");
          myString << "Gain_Board" << std::setfill ('0') << std::setw (3) << +cBoard->getBeId()
		   << "_Mod"       << std::setfill ('0') << std::setw (3) << +cFe->getFeId()
		   << "_Chip"      << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),nSteps,startValue,stopValue,nEvents,0,RD53::SetBits<NBIT_TOT/NPIX_REGION>(NBIT_TOT/NPIX_REGION).to_ulong() - 1));
	  theOccupancy.back()->SetXTitle("VCal");
	  theOccupancy.back()->SetYTitle("ToT");
	}
  
  theGain1D = new TH1F("theGain1D","Gain-1D",100,0,1);
  theGain1D->SetXTitle("Gain");
  theGain1D->SetYTitle("Entries");

  theIntercept1D = new TH1F("theIntercept1D","Intercept-1D",100,-10,10);
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
  size_t index = 0;
  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fChipVector)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      for (auto i = 0; i < dacList.size(); i++)
		if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT != 0)
		  theOccupancy[index]->Fill(dacList[i],detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT);
	  index++;
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

void Gain::Analyze()
{
  double gain, intercept;

  theGainAndInterceptContainer = new DetectorContainer();
  ContainerFactory  theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<ThresholdAndNoise>(*fDetectorContainer, *theGainAndInterceptContainer);

  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fChipVector)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    {
	      this->ComputeStats(gain,intercept);
	      
	      if (gain != 0)
		{
		  theGainAndInterceptContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fThreshold = gain;
		  theGainAndInterceptContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<ThresholdAndNoise>(row,col).fNoise     = intercept;

		  theGain1D->Fill(gain);
		  theIntercept1D->Fill(intercept);
		  theGain2D->SetBinContent(col+1,row+1,gain);
		  theIntercept2D->SetBinContent(col+1,row+1,intercept);
		}
	    }
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

void Gain::ComputeStats(double& gain, double& intercept)
{
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}
