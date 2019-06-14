/*!
  \file                  RD53Gain.cc
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Gain.h"

Gain::Gain(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, size_t startValue, size_t stopValue, size_t nSteps) :
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
  // // ########################
  // // # Custom channel group #
  // // ########################
  // customBitset.reset();
  // for (auto row = rowStart; row <= rowEnd; row++)
  //   for (auto col = colStart; col <= colEnd; col++)
  //     customBitset.set(RD53::nRows*col + row);
  
  // customChannelGroup = new ChannelGroup<RD53::nRows,RD53::nCols>();
  // customChannelGroup->setCustomPattern(customBitset);
  
  // fChannelGroupHandler = new RD53ChannelGroupHandler();
  // fChannelGroupHandler->setCustomChannelGroup(customChannelGroup);
  // fChannelGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);


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
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      if (theOccupancy[i] != nullptr) delete theOccupancy[i];
      if (theCanvas[i]    != nullptr) delete theCanvas[i];
    }

  if (theGain1D     != nullptr)  delete theGain1D;
  if (theCanvasGa1D != nullptr)  delete theCanvasGa1D;

  if (theIntercept1D != nullptr) delete theIntercept1D;
  if (theCanvasIn1D  != nullptr) delete theCanvasIn1D;

  if (theGain2D     != nullptr)  delete theGain2D;
  if (theCanvasGa2D != nullptr)  delete theCanvasGa2D;

  if (theIntercept2D != nullptr) delete theIntercept2D;
  if (theCanvasIn2D  != nullptr) delete theCanvasIn2D;

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
  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
	{
	  size_t VCalOffset = cChip->getReg("VCAL_MED");

	  myString.clear();
	  myString.str("");
          myString << "Gain_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"       << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"      << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),nSteps,startValue-VCalOffset,stopValue-VCalOffset,nEvents/2,0,RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong()));
	  theOccupancy.back()->SetXTitle("#DeltaVCal");
	  theOccupancy.back()->SetYTitle("ToT");

	  myString.clear();
	  myString.str("");
          myString << "theCanvas_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvas.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theGain1D = new TH1F("theGain1D","Gain-1D",100,0,1e-2);
  theGain1D->SetXTitle("Gain");
  theGain1D->SetYTitle("Entries");

  theIntercept1D = new TH1F("theIntercept1D","Intercept-1D",100,-3,3);
  theIntercept1D->SetXTitle("ToT");
  theIntercept1D->SetYTitle("Entries");

  theGain2D = new TH2F("theGain2D","Gain-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theGain2D->SetXTitle("Columns");
  theGain2D->SetYTitle("Rows");

  theIntercept2D = new TH2F("theIntercept2D","Intercept-2D",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theIntercept2D->SetXTitle("Columns");
  theIntercept2D->SetYTitle("Rows");

  theFile       = new TFile(fileName, "RECREATE");
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
		if (detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT != 0)
		  theOccupancy[index]->Fill(dacList[i]-VCalOffset,detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT);

	  index++;
	}
}


void Gain::Display()
{
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      theCanvas[i]->cd();
      theOccupancy[i]->Draw("gcolz");
      theCanvas[i]->Modified();
      theCanvas[i]->Update();
    }
  
  theCanvasGa1D->cd();
  theGain1D->Draw();
  theCanvasGa1D->Modified();
  theCanvasGa1D->Update();

  theCanvasIn1D->cd();
  theIntercept1D->Draw();
  theCanvasIn1D->Modified();
  theCanvasIn1D->Update();

  theCanvasGa2D->cd();
  theGain2D->Draw("gcolz");
  theCanvasGa2D->Modified();
  theCanvasGa2D->Update();

  theCanvasIn2D->cd();
  theIntercept2D->Draw("gcolz");
  theCanvasIn2D->Modified();
  theCanvasIn2D->Update();
}


void Gain::Analyze()
{
  double gain, gainErr, intercept, interceptErr;
  std::vector<float> x, y, e;

  theGainAndInterceptContainer = new DetectorDataContainer();
  ContainerFactory  theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<GainAndIntercept>(*fDetectorContainer, *theGainAndInterceptContainer);

  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
	{
	  size_t VCalOffset = cChip->getReg("VCAL_MED");

	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		x.clear();
		y.clear();
		e.clear();

		for (auto i = 0; i < dacList.size()-1; i++)
		  {
		    x.push_back(dacList[i]-VCalOffset);
		    y.push_back(detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT);
		    e.push_back(detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToTError);
		  }

		this->ComputeStats(x,y,e,gain,gainErr,intercept,interceptErr);

		if (gain != 0)
		  {
		    theGainAndInterceptContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<GainAndIntercept>(row,col).fGain           = gain;
		    theGainAndInterceptContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<GainAndIntercept>(row,col).fGainError      = gainErr;
		    theGainAndInterceptContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<GainAndIntercept>(row,col).fIntercept      = intercept;
		    theGainAndInterceptContainer->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<GainAndIntercept>(row,col).fInterceptError = interceptErr;

		    theGain1D->Fill(gain);
		    theIntercept1D->Fill(intercept);
		    theGain2D->SetBinContent(col+1,row+1,gain);
		    theIntercept2D->SetBinContent(col+1,row+1,intercept);
		  }
	      }
	}
}


void Gain::Save()
{ 
  std::string tmp;
  std::stringstream myString;
  
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      theOccupancy[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOccupancy[i]->GetName() << ".svg";
      theCanvas[i]->Print(myString.str().c_str());
    }

  theGain1D->Write();
  theIntercept1D->Write();
  theGain2D->Write();
  theIntercept2D->Write();
  theFile->Write();

  theCanvasGa1D->Print("Gain1D.svg");
  theCanvasIn1D->Print("InterceptD.svg");
  theCanvasGa2D->Print("Gain2D.svg");
  theCanvasIn2D->Print("Intercept2D.svg");
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
  float det;

  intercept    = 0;
  gain         = 0;
  interceptErr = 0;
  gainErr      = 0;


  // #######
  // # XtX #
  // #######
  a = x.size();
  for (auto i = 0; i < x.size(); i++)
    {
      b  += x[i];
      d  += x[i] * x[i];
    }
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
    {
      intercept    += (ai + bi*x[i]) * y[i];
      gain         += (ci + di*x[i]) * y[i];

      interceptErr += (ai + bi*x[i])*(ai + bi*x[i]) * e[i]*e[i];
      gainErr      += (ci + di*x[i])*(ci + di*x[i]) * e[i]*e[i];
    }

  interceptErr = sqrt(interceptErr);
  gainErr      = sqrt(gainErr);
}
