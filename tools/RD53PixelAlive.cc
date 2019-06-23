/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(const char* fileRes, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject) :
  fileRes     (fileRes),
  rowStart    (rowStart),
  rowEnd      (rowEnd),
  colStart    (colStart),
  colEnd      (colEnd),
  nPixels2Inj (nPixels2Inj),
  nEvents     (nEvents),
  nEvtsBurst  (nEvtsBurst),
  inject      (inject),
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
}

PixelAlive::~PixelAlive()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;

  for (auto i = 0; i < theCanvasOcc2D.size(); i++)
    {
      delete theOcc2D[i];
      delete theCanvasOcc2D[i];
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      delete theToT[i];
      delete theCanvasToT[i];
    }
  
  for (auto i = 0; i < theCanvasOcc1D.size(); i++)
    {
      delete theOcc1D[i];
      delete theCanvasOcc1D[i];
    }

  for (auto i = 0; i < theCanvasBCID.size(); i++)
    {
      delete theBCID[i];
      delete theCanvasBCID[i];
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      delete theErr[i];
      delete theCanvasErr[i];
    }
}

void PixelAlive::Run()
{
  ContainerFactory theDetectorFactory;

  fDetectorDataContainer = &theContainer;
  theDetectorFactory.copyAndInitStructure<OccupancyAndPh,GenericDataVector>(*fDetectorContainer, *fDetectorDataContainer);

  this->SetTestPulse(inject);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents, nEvtsBurst);
}

void PixelAlive::Draw(bool display, bool save)
{
  TApplication* myApp;
  
  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);

  this->InitHisto();
  this->FillHisto();
  this->Display();

  if (save    == true) this->Save();
  if (display == true) myApp->Run();
}

void PixelAlive::Analyze()
{
  for (const auto cBoard : theContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	LOG (INFO) << BOLDGREEN << "\t--> Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		   << cChip->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.fOccupancy << RESET;
}

void PixelAlive::InitHisto()
{
  std::string tmp;
  std::stringstream myString;
  size_t ToTsize   = RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong()+1;
  size_t BCIDsize  = RD53::SetBits<RD53EvtEncoder::NBIT_BCID>(RD53EvtEncoder::NBIT_BCID).to_ulong()+1;
  size_t TrgIDsize = RD53::SetBits<RD53EvtEncoder::NBIT_TRIGID>(RD53EvtEncoder::NBIT_TRIGID).to_ulong()+1;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
	  tmp = fileRes;
	  tmp = tmp.erase(tmp.find(".root"),5);

	  myString.clear();
	  myString.str("");
          myString << tmp << "_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOcc2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theOcc2D.back()->SetXTitle("Columns");
	  theOcc2D.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "theToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theToT.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),ToTsize,0,ToTsize));
	  theToT.back()->SetXTitle("ToT");
	  theToT.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasToT.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "theOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"           << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"          << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOcc1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),nEvents+1,0,nEvents+1));
	  theOcc1D.back()->SetXTitle("Occupancy");
	  theOcc1D.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  
	  myString.clear();
	  myString.str("");
          myString << "theBCID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theBCID.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),BCIDsize,0,BCIDsize));
	  theBCID.back()->SetXTitle("#DeltaBCID");
	  theBCID.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasBCID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"               << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasBCID.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "theTrgID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"           << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"          << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theTrgID.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),TrgIDsize,0,TrgIDsize));
	  theTrgID.back()->SetXTitle("#DeltaTrigger-ID");
	  theTrgID.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasTrgID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasTrgID.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "theErr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theErr.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theErr.back()->SetXTitle("Columns");
	  theErr.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasErr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasErr.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}
  
  theFile = new TFile(fileRes, "RECREATE");
}

void PixelAlive::FillHisto()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		if (theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy != 0)
		  {
		    theOcc2D[index]->SetBinContent(col+1,row+1,theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy);
		    theToT[index]->Fill(theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh);
		    theOcc1D[index]->Fill(theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fOccupancy * nEvents);
		  }
		
		if (theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fErrors != 0)
		  theErr[index]->SetBinContent(col+1,row+1,theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fErrors);
	      }
	  
	  for (auto i = 1; i < theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.data1.size(); i++)
	    {
	      int deltaBCID = theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.data1[i] -
		theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.data1[i-1];
	      
	      theBCID[index]->Fill((deltaBCID > 0 ? 0 : RD53::SetBits<RD53EvtEncoder::NBIT_BCID>(RD53EvtEncoder::NBIT_BCID).to_ulong()+1) + deltaBCID);
	    }
	  
	  for (auto i = 1; i < theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.data2.size(); i++)
	    {
	      int deltaTrgID = theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.data2[i] -
		theContainer.at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getSummary<GenericDataVector,OccupancyAndPh>().theSummary_.data2[i-1];
	      
	      theTrgID[index]->Fill((deltaTrgID > 0 ? 0 : RD53::SetBits<RD53EvtEncoder::NBIT_TRIGID>(RD53EvtEncoder::NBIT_TRIGID).to_ulong()+1) + deltaTrgID);
	    }
	  
	  index++;
	}
}

void PixelAlive::Display()
{
  for (auto i = 0; i < theCanvasOcc2D.size(); i++)
    {
      theCanvasOcc2D[i]->cd();
      theOcc2D[i]->Draw("gcolz");
      theCanvasOcc2D[i]->Modified();
      theCanvasOcc2D[i]->Update();
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      theCanvasToT[i]->cd();
      theToT[i]->Draw();
      theCanvasToT[i]->Modified();
      theCanvasToT[i]->Update();
    }

  for (auto i = 0; i < theCanvasOcc1D.size(); i++)
    {
      theCanvasOcc1D[i]->cd();
      theOcc1D[i]->Draw();
      theCanvasOcc1D[i]->Modified();
      theCanvasOcc1D[i]->Update();
    }

  for (auto i = 0; i < theCanvasBCID.size(); i++)
    {
      theCanvasBCID[i]->cd();
      theBCID[i]->Draw();
      theCanvasBCID[i]->Modified();
      theCanvasBCID[i]->Update();
    }

  for (auto i = 0; i < theCanvasTrgID.size(); i++)
    {
      theCanvasTrgID[i]->cd();
      theTrgID[i]->Draw();
      theCanvasTrgID[i]->Modified();
      theCanvasTrgID[i]->Update();
    }

  for (auto i = 0; i < theCanvasErr.size(); i++)
    {
      theCanvasErr[i]->cd();
      theErr[i]->Draw();
      theCanvasErr[i]->Modified();
      theCanvasErr[i]->Update();
    }
}

void PixelAlive::Save()
{
  std::stringstream myString;

  for (auto i = 0; i < theCanvasOcc2D.size(); i++)
    {
      theCanvasOcc2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOcc2D[i]->GetName() << ".svg";
      theCanvasOcc2D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      theCanvasToT[i]->Write();
      myString.clear();
      myString.str("");
      myString << theToT[i]->GetName() << ".svg";
      theCanvasToT[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasOcc1D.size(); i++)
    {
      theCanvasOcc1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOcc1D[i]->GetName() << ".svg";
      theCanvasOcc1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasBCID.size(); i++)
    {
      theCanvasBCID[i]->Write();
      myString.clear();
      myString.str("");
      myString << theBCID[i]->GetName() << ".svg";
      theCanvasBCID[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasTrgID.size(); i++)
    {
      theCanvasTrgID[i]->Write();
      myString.clear();
      myString.str("");
      myString << theTrgID[i]->GetName() << ".svg";
      theCanvasTrgID[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasErr.size(); i++)
    {
      theCanvasErr[i]->Write();
      myString.clear();
      myString.str("");
      myString << theErr[i]->GetName() << ".svg";
      theCanvasErr[i]->Print(myString.str().c_str());
    }
}
