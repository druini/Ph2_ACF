/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(const char* fileName, size_t rowStart, size_t rowEnd, size_t colStart, size_t colEnd, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject) :
  fileName    (fileName),
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
  
  if (fChannelGroupHandler != nullptr) delete fChannelGroupHandler;
  if (theFile              != nullptr) delete theFile;

  for (auto i = 0; i < theCanvasOcc2D.size(); i++)
    {
      if (theOcc2D[i]       != nullptr) delete theOcc2D[i];
      if (theCanvasOcc2D[i] != nullptr) delete theCanvasOcc2D[i];
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      if (theToT[i]       != nullptr) delete theToT[i];
      if (theCanvasToT[i] != nullptr) delete theCanvasToT[i];
    }
  
  for (auto i = 0; i < theCanvasOcc1D.size(); i++)
    {
      if (theOcc1D[i]       != nullptr) delete theOcc1D[i];
      if (theCanvasOcc1D[i] != nullptr) delete theCanvasOcc1D[i];
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      if (theErr[i]       != nullptr) delete theErr[i];
      if (theCanvasErr[i] != nullptr) delete theCanvasErr[i];
    }
}

void PixelAlive::Run()
{
  ContainerFactory theDetectorFactory;

  fDetectorDataContainer = &theOccupancyContainer;
  theDetectorFactory.copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, *fDetectorDataContainer);

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
  
}

void PixelAlive::InitHisto()
{
  std::string tmp;
  std::stringstream myString;
  size_t ToTsize = RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong();

  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cFe : *cBoard)
      for (const auto cChip : *cFe)
        {
	  tmp = fileName;
	  tmp = tmp.erase(tmp.find(".root"),5);

	  myString.clear();
	  myString.str("");
          myString << tmp << "_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theOcc2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theOcc2D.back()->SetXTitle("Columns");
	  theOcc2D.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasOcc2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  myString.clear();
	  myString.str("");
          myString << "theToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theToT.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),ToTsize,0,ToTsize));
	  theToT.back()->SetXTitle("ToT");
	  theToT.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasToT.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  myString.clear();
	  myString.str("");
          myString << "theOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"           << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"          << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theOcc1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),nEvents+1,0,nEvents+1));
	  theOcc1D.back()->SetXTitle("Occupancy");
	  theOcc1D.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasOcc1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	  
	  myString.clear();
	  myString.str("");
          myString << "theErr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theErr.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theErr.back()->SetXTitle("Columns");
	  theErr.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasErr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getId()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getId()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getId();
	  theCanvasErr.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}
  
  theFile = new TFile(fileName, "RECREATE");
}

void PixelAlive::FillHisto()
{
  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cFe : *cBoard)
      for (const auto cChip : *cFe)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		if (theOccupancyContainer.at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fOccupancy != 0)
		  {
		    theOcc2D[index]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fOccupancy);
		    theToT[index]->Fill(theOccupancyContainer.at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fPh);
		    theOcc1D[index]->Fill(theOccupancyContainer.at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fOccupancy * nEvents);
		  }

		if (theOccupancyContainer.at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fErrors != 0)
		  theErr[index]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getId())->at(cFe->getId())->at(cChip->getId())->getChannel<OccupancyAndPh>(row,col).fErrors);
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
  std::string tmp;
  std::stringstream myString;

  for (auto i = 0; i < theCanvasOcc2D.size(); i++)
    {
      theOcc2D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOcc2D[i]->GetName() << ".svg";
      theCanvasOcc2D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasToT.size(); i++)
    {
      theToT[i]->Write();
      myString.clear();
      myString.str("");
      myString << theToT[i]->GetName() << ".svg";
      theCanvasToT[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasOcc1D.size(); i++)
    {
      theOcc1D[i]->Write();
      myString.clear();
      myString.str("");
      myString << theOcc1D[i]->GetName() << ".svg";
      theCanvasOcc1D[i]->Print(myString.str().c_str());
    }

  for (auto i = 0; i < theCanvasErr.size(); i++)
    {
      theErr[i]->Write();
      myString.clear();
      myString.str("");
      myString << theErr[i]->GetName() << ".svg";
      theCanvasErr[i]->Print(myString.str().c_str());
    }
}
