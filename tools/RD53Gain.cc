/*!OA
  \file                  RD53Gain.cc
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Gain.h"

Gain::Gain (const char* fileRes, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps) :
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

Gain::~Gain ()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;

  for (auto i = 0; i < theCanvasOcc.size(); i++)
    {
      delete theOccupancy[i];
      delete theCanvasOcc[i];
    }

  for (auto i = 0; i < theCanvasGa1D.size(); i++)
    {
      delete theGain1D[i];
      delete theCanvasGa1D[i];
    }
  
  for (auto i = 0; i < theCanvasIn1D.size(); i++)
    {
      delete theIntercept1D[i];
      delete theCanvasIn1D[i];
    }
  
  for (auto i = 0; i < theCanvasGa2D.size(); i++)
    {
      delete theGain2D[i];
      delete theCanvasGa2D[i];
    }
  
  for (auto i = 0; i < theCanvasIn2D.size(); i++)
    {
      delete theIntercept2D[i];
      delete theCanvasIn2D[i];
    }

  for (auto i = 0; i < theAxis.size(); i++) delete theAxis[i];

  for (auto i = 0; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
}

void Gain::Run ()
{
  ContainerFactory theDetectorFactory;

  for (auto i = 0; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
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

void Gain::Draw (bool display, bool save)
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

std::shared_ptr<DetectorDataContainer> Gain::Analyze ()
{
  double gain, gainErr, intercept, interceptErr;
  std::vector<float> x(dacList.size(),0);
  std::vector<float> y(dacList.size(),0);
  std::vector<float> e(dacList.size(),0);

  ContainerFactory theDetectorFactory;
  theGainAndInterceptContainer = new DetectorDataContainer();
  theDetectorFactory.copyAndInitStructure<GainAndIntercept>(*fDetectorContainer, *theGainAndInterceptContainer);

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
		  {
		    x[i] = dacList[i]-VCalOffset;
		    y[i] = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh;
		    e[i] = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPhError;
		  }
		
		this->ComputeStats(x,y,e,gain,gainErr,intercept,interceptErr);
		
		if (gain != 0)
		  {
		    theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain           = gain;
		    theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGainError      = gainErr;
		    theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fIntercept      = intercept;
		    theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fInterceptError = interceptErr;
		  }
	      }
	  
	  index++;
	}

  return std::shared_ptr<DetectorDataContainer>(theGainAndInterceptContainer);
}

void Gain::InitHisto ()
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
          myString << "Gain_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"       << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"      << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),
					  nSteps,startValue-VCalOffset,stopValue-VCalOffset,
					  nEvents/2,0,RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong()));
	  theOccupancy.back()->SetXTitle("#DeltaVCal");
	  theOccupancy.back()->SetYTitle("ToT");

	  myString.clear();
	  myString.str("");
          myString << "CanvasGain_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "Gain1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theGain1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),100,0,20e-3));
	  theGain1D.back()->SetXTitle("Gain (ToT/VCal)");
	  theGain1D.back()->SetYTitle("Entries");
  
	  myString.clear();
          myString.str("");
          myString << "CanvasGa1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasGa1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "Intercept1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"              << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"             << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theIntercept1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),100,-INTERCEPT_HALFRANGE,INTERCEPT_HALFRANGE));
	  theIntercept1D.back()->SetXTitle("Intercept (ToT)");
	  theIntercept1D.back()->SetYTitle("Entries");

	  myString.clear();
          myString.str("");
          myString << "CanvasIn1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasIn1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "Gain2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theGain2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theGain2D.back()->SetXTitle("Columns");
	  theGain2D.back()->SetYTitle("Rows");

	  myString.clear();
          myString.str("");
          myString << "CanvasGa2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasGa2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
          myString.str("");
          myString << "Intercept2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"              << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"             << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theIntercept2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theIntercept2D.back()->SetXTitle("Columns");
	  theIntercept2D.back()->SetYTitle("Rows");

	  myString.clear();
          myString.str("");
          myString << "CanvasIn2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
                   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
                   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasIn2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theFile = new TFile(fileRes, "UPDATE");
}

void Gain::FillHisto ()
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
		  if (detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh != 0)
		    theOccupancy[index]->Fill(dacList[i]-VCalOffset,detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh);
		
		if (theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain != 0)
		  {
		    theGain1D[index]->Fill(theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain);
		    theIntercept1D[index]->Fill(theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fIntercept);
		    theGain2D[index]->SetBinContent(col+1,row+1,theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain);
		    theIntercept2D[index]->SetBinContent(col+1,row+1,theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fIntercept);
		  }
	      }

	  index++;
	}
}

void Gain::Display ()
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

      TPad* myPad = (TPad*)theCanvasGa1D[i]->GetPad(0);
      myPad->SetTopMargin(0.16);
      theAxis.push_back(new TGaxis(myPad->GetUxmin(), myPad->GetUymax(), myPad->GetUxmax(), myPad->GetUymax(),
				   1./RD53VCal2Charge::Convert(1./theGain1D[i]->GetBinLowEdge(1),true),
				   1./RD53VCal2Charge::Convert(1./theGain1D[i]->GetBinLowEdge(theGain1D[i]->GetNbinsX()),true),
				   510,"-"));
      theAxis.back()->SetTitle("Gain (ToT/electrons)");
      theAxis.back()->SetTitleOffset(1.2);
      theAxis.back()->SetTitleSize(0.035);
      theAxis.back()->SetTitleFont(40);
      theAxis.back()->SetLabelOffset(0.001);
      theAxis.back()->SetLabelSize(0.035);
      theAxis.back()->SetLabelFont(42);
      theAxis.back()->SetLabelColor(kRed);
      theAxis.back()->SetLineColor(kRed);
      theAxis.back()->Draw();

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

void Gain::Save ()
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

  for (auto i = 0; i < theCanvasGa1D.size(); i++)
    {
      theCanvasGa1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theGain1D[i]->GetName() << ".svg";
      theCanvasGa1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasIn1D.size(); i++)
    {
      theCanvasIn1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theIntercept1D[i]->GetName() << ".svg";
      theCanvasIn1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasGa2D.size(); i++)
    {
      theCanvasGa2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theGain2D[i]->GetName() << ".svg";
      theCanvasGa2D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasIn2D.size(); i++)
    {
      theCanvasIn2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theIntercept2D[i]->GetName() << ".svg";
      theCanvasIn2D[i]->Print(myString.str().c_str());
    }

  theFile->Write();
}

void Gain::ComputeStats (std::vector<float>& x, std::vector<float>& y, std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr)
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
