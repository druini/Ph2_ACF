/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, size_t nEvtsBurst, bool inject) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nEvents(nEvts),
  nEvtsBurst(nEvtsBurst),
  inject(inject),
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


void PixelAlive::InitHisto()
{
  std::string tmp;
  std::stringstream myString;

  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
        {
	  tmp = fileName;
	  tmp = tmp.erase(tmp.find(".root"),5);

	  myString.clear();
	  myString.str("");
          myString << tmp << "_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"          << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"         << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOcc2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theOcc2D.back()->SetXTitle("Columns");
	  theOcc2D.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc2D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasOcc2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  myString.clear();
	  myString.str("");
          myString << "theToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theToT.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong(),0,RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong()));
	  theToT.back()->SetXTitle("ToT");
	  theToT.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasToT.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  myString.clear();
	  myString.str("");
          myString << "theOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"           << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"          << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOcc1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),nEvents+1,0,nEvents+1));
	  theOcc1D.back()->SetXTitle("Occupancy");
	  theOcc1D.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"                 << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"                << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasOcc1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	  
	  myString.clear();
	  myString.str("");
          myString << "theErr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"         << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"        << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theErr.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theErr.back()->SetXTitle("Columns");
	  theErr.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvasErr_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"               << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"              << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvasErr.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}
  
  theFile = new TFile(fileName, "RECREATE");
}


void PixelAlive::Run()
{
  DetectorDataContainer     theOccupancyContainer;
  fDetectorDataContainer = &theOccupancyContainer;
  ContainerFactory          theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<OccupancyAndToT>(*fDetectorContainer, *fDetectorDataContainer);

  this->SetTestPulse(inject);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents, nEvtsBurst);


  // #########################
  // # Filling the histogram #
  // #########################
  size_t index = 0;
  for (const auto& cBoard : fBoardVector)
    for (const auto& cFe : cBoard->fModuleVector)
      for (const auto& cChip : cFe->fChipVector)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      {
		theOcc2D[index]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);

		if (theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy != 0)
		  {
		    theToT[index]->Fill(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT);
		    theOcc1D[index]->Fill(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy * nEvents);
		  }

		if (theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fErrors != 0)
		  theErr[index]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fErrors);
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
