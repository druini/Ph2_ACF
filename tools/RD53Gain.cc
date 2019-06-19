/*!OA
  \file                  RD53Gain.cc
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Gain.h"

Gain::Gain(const char* fileName, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps) :
  fileName    (fileName),
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

Gain::~Gain()
{
  theFile->Close();
  
  if (fChannelGroupHandler != nullptr) delete fChannelGroupHandler;
  if (theFile              != nullptr) delete theFile;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      if (theOccupancy[i] != nullptr) delete theOccupancy[i];
      if (theCanvasOcc[i] != nullptr) delete theCanvasOcc[i];
    }

  for (auto i = 0; i < theCanvasGa1D.size(); i++)
    {
      if (theGain1D[i]     != nullptr) delete theGain1D[i];
      if (theCanvasGa1D[i] != nullptr) delete theCanvasGa1D[i];
    }
  
  for (auto i = 0; i < theCanvasIn1D.size(); i++)
    {
      if (theIntercept1D[i] != nullptr) delete theIntercept1D[i];
      if (theCanvasIn1D[i]  != nullptr) delete theCanvasIn1D[i];
    }
  
  for (auto i = 0; i < theCanvasGa2D.size(); i++)
    {
      if (theGain2D[i]     != nullptr) delete theGain2D[i];
      if (theCanvasGa2D[i] != nullptr) delete theCanvasGa2D[i];
    }
  
  for (auto i = 0; i < theCanvasIn2D.size(); i++)
    {
      if (theIntercept2D[i] != nullptr) delete theIntercept2D[i];
      if (theCanvasIn2D[i]  != nullptr) delete theCanvasIn2D[i];
    }

  for (auto i = 0; i < detectorContainerVector.size(); i++)
    if (detectorContainerVector[i] != nullptr) delete detectorContainerVector[i];
  
  if (theGainAndInterceptContainer != nullptr) delete theGainAndInterceptContainer;
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
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      theDetectorFactory.copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);
}

void Gain::Draw(bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();
}

void Gain::Analyze()
{
  double gain, gainErr, intercept, interceptErr;
  std::vector<float> x(dacList.size(),0);
  std::vector<float> y(dacList.size(),0);
  std::vector<float> e(dacList.size(),0);

  theGainAndInterceptContainer = new DetectorDataContainer();
  ContainerFactory theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<GainAndIntercept>(*fDetectorContainer, *theGainAndInterceptContainer);

  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cFe : *cBoard)
      for (const auto cChip : *cFe)
	{
	  size_t VCalOffset = static_cast<Chip* const>(cChip)->getReg("VCAL_MED");

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		for (auto i = 0; i < dacList.size()-1; i++)
		{
		  x[i] = dacList[i]-VCalOffset;
		  y[i] = detectorContainerVector[i]->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fPh;
		  e[i] = detectorContainerVector[i]->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fPhError;
		}
		
		this->ComputeStats(x,y,e,gain,gainErr,intercept,interceptErr);
		
		if (gain != 0)
		  {
		    theGainAndInterceptContainer->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<GainAndIntercept>(row,col).fGain           = gain;
		    theGainAndInterceptContainer->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<GainAndIntercept>(row,col).fGainError      = gainErr;
		    theGainAndInterceptContainer->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<GainAndIntercept>(row,col).fIntercept      = intercept;
		    theGainAndInterceptContainer->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<GainAndIntercept>(row,col).fInterceptError = interceptErr;
		    
		    theGain1D[index]->Fill(gain);
		    theIntercept1D[index]->Fill(intercept);
		    theGain2D[index]->SetBinContent(col+1,row+1,gain);
		    theIntercept2D[index]->SetBinContent(col+1,row+1,intercept);
		  }
	      }
	  
	  index++;
	}
}

void Gain::InitHisto()
{
  std::stringstream myString;

  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cFe : *cBoard)
      for (const auto cChip : *cFe)
	{
	  size_t VCalOffset = static_cast<Chip* const>(cChip)->getReg("VCAL_MED");


	  myString.clear();
	  myString.str("");
          myString << "Gain_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"       << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"      << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),nSteps,startValue-VCalOffset,stopValue-VCalOffset,nEvents/2,0,RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong()));
	  theOccupancy.back()->SetXTitle("#DeltaVCal");
	  theOccupancy.back()->SetYTitle("ToT");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasOcc.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "theGain1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theGain1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),100,0,15e-3));
	  theGain1D.back()->SetXTitle("Gain");
	  theGain1D.back()->SetYTitle("Entries");
  
	  myString.clear();
          myString.str("");
          myString << "theCanvasGa1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasGa1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "theIntercept1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theIntercept1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),100,-INTERCEPT_HALFRANGE,INTERCEPT_HALFRANGE));
	  theIntercept1D.back()->SetXTitle("ToT");
	  theIntercept1D.back()->SetYTitle("Entries");

	  myString.clear();
          myString.str("");
          myString << "theCanvasIn1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasIn1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "theGain2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theGain2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theGain2D.back()->SetXTitle("Columns");
	  theGain2D.back()->SetYTitle("Rows");

	  myString.clear();
          myString.str("");
          myString << "theCanvasGa2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasGa2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "theIntercept2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theIntercept2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theIntercept2D.back()->SetXTitle("Columns");
	  theIntercept2D.back()->SetYTitle("Rows");

	  myString.clear();
          myString.str("");
          myString << "theCanvasIn2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
                   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cFe->getId()
                   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasIn2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileName, "RECREATE");
}

void Gain::FillHisto()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cFe : *cBoard)
      for (const auto cChip : *cFe)
	{
	  size_t VCalOffset = static_cast<Chip* const>(cChip)->getReg("VCAL_MED");
	  
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      for (auto i = 0; i < dacList.size(); i++)
		if (detectorContainerVector[i]->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fPh != 0)
		  theOccupancy[index]->Fill(dacList[i]-VCalOffset,detectorContainerVector[i]->at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fPh);

	  index++;
	}
}

void Gain::Display()
{
  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      theCanvasOcc[i]->cd();
      theOccupancy[i]->Draw("gcolz");
      theCanvasOcc[i]->Modified();
      theCanvasOcc[i]->Update();
    }
  
  for (auto i = 0; i < theCanvasGa1D.size(); i++)
    {
      theCanvasGa1D[i]->cd();
      theGain1D[i]->Draw();
      theCanvasGa1D[i]->Modified();
      theCanvasGa1D[i]->Update();
    }
  
  for (auto i = 0; i < theCanvasIn1D.size(); i++)
    {
      theCanvasIn1D[i]->cd();
      theIntercept1D[i]->Draw();
      theCanvasIn1D[i]->Modified();
      theCanvasIn1D[i]->Update();
    }

  for (auto i = 0; i < theCanvasGa2D.size(); i++)
    {
      theCanvasGa2D[i]->cd();
      theGain2D[i]->Draw("gcolz");
      theCanvasGa2D[i]->Modified();
      theCanvasGa2D[i]->Update();
    }
  
  for (auto i = 0; i < theCanvasIn2D.size(); i++)
    {
      theCanvasIn2D[i]->cd();
      theIntercept2D[i]->Draw("gcolz");
      theCanvasIn2D[i]->Modified();
      theCanvasIn2D[i]->Update();
    }
}

void Gain::Save()
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

  for (auto i = 0; i < theCanvasGa1D.size(); i++)
    {
      theGain1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theGain1D[i]->GetName() << ".svg";
      theCanvasGa1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasIn1D.size(); i++)
    {
      theIntercept1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theIntercept1D[i]->GetName() << ".svg";
      theCanvasIn1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasGa2D.size(); i++)
    {
      theGain2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theGain2D[i]->GetName() << ".svg";
      theCanvasGa2D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasIn2D.size(); i++)
    {
      theIntercept2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theIntercept2D[i]->GetName() << ".svg";
      theCanvasIn2D[i]->Print(myString.str().c_str());
    }

  theFile->Write();
}

void Gain::ComputeStats(std::vector<float>& x, std::vector<float>& y, std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr)
// ##############################################
// # Linear regression with least-square method #
// # Model: y = f(x) = q + mx                   #
// # Measurements with uncertainty: Y = AX + E  #
// ##############################################
// # A = (XtX)^(-1)XtY                          #
// # X = | 1 x1 |                               #
// #     | 1 x2 |                               #
// #     ...                                    #
// # A = | q |                                  #
// #     | m |                                  #
// ##############################################
{
  float a  = 0, b  = 0, c  = 0, d  = 0;
  float ai = 0, bi = 0, ci = 0, di = 0;
  float it = 0;
  float det;

  intercept    = 0;
  gain         = 0;
  interceptErr = 0;
  gainErr      = 0;


  // #######
  // # XtX #
  // #######
  for (auto i = 0; i < x.size(); i++)
    if (e[i] != 0)
      {
	b += x[i];
	d += x[i] * x[i];
	it++;
      }
  a = it;
  c = b;


  // ##############
  // # (XtX)^(-1) #
  // ##############
  det = a*d - b*c;
  if (det != 0)
    {
      ai =  d/det;
      bi = -b/det;
      ci = -c/det;
      di =  a/det;
    }


  // #################
  // # (XtX)^(-1)XtY #
  // #################
  for (auto i = 0; i < x.size(); i++)
    if (e[i] != 0)
      {
	intercept    += (ai + bi*x[i]) * y[i];
	gain         += (ci + di*x[i]) * y[i];
	
	interceptErr += (ai + bi*x[i])*(ai + bi*x[i]) * e[i]*e[i];
	gainErr      += (ci + di*x[i])*(ci + di*x[i]) * e[i]*e[i];
      }
  
  interceptErr = sqrt(interceptErr);
  gainErr      = sqrt(gainErr);
}
