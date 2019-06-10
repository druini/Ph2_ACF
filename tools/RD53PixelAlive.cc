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
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      if (theOccupancy[i] != nullptr) delete theOccupancy[i];
      if (theCanvas[i]    != nullptr) delete theCanvas[i];
    }

  if (theToT       != nullptr) delete theToT;
  if (theCanvasToT != nullptr) delete theCanvasToT;

  if (theBCID       != nullptr) delete theBCID;
  if (theCanvasBCID != nullptr) delete theCanvasBCID;

  if (theOcc       != nullptr) delete theOcc;
  if (theCanvasOcc != nullptr) delete theCanvasOcc;

  if (theErr       != nullptr) delete theErr;
  if (theCanvasErr != nullptr) delete theCanvasErr;
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
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theOccupancy.back()->SetXTitle("Columns");
	  theOccupancy.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "theCanvas_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getBeId()
		   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cFe->getFeId()
		   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theCanvas.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}

  theToT = new TH1F("theToT","ToT",RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong(),0,RD53::SetBits<RD53EvtEncoder::NBIT_TOT/NPIX_REGION>(RD53EvtEncoder::NBIT_TOT/NPIX_REGION).to_ulong());
  theToT->SetXTitle("ToT");
  theToT->SetYTitle("Entries");

  theOcc = new TH1F("theOcc","Occupancy",100,0,nEvents);
  theOcc->SetXTitle("Occupancy");
  theOcc->SetYTitle("Entries");

  theErr = new TH2F("theErr","Errors",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
  theErr->SetXTitle("Columns");
  theErr->SetYTitle("Rows");

  theFile       = new TFile(fileName, "RECREATE");
  theCanvasToT  = new TCanvas("theCanvasToT","RD53Canvas",0,0,700,500);
  theCanvasBCID = new TCanvas("theCanvasBCID","RD53Canvas",0,0,700,500);
  theCanvasOcc  = new TCanvas("theCanvasOcc","RD53Canvas",0,0,700,500);
  theCanvasErr  = new TCanvas("theCanvasErr","RD53Canvas",0,0,700,500);
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
		theOccupancy[index]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);

		if (theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy != 0)
		  {
		    theToT->Fill(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fToT);
		    theOcc->Fill(theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy * nEvents);
		  }

		theErr->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fErrors);
	      }

	  index++;
	}
}


void PixelAlive::Display()
{
  for (auto i = 0; i < theCanvas.size(); i++)
    {
      theCanvas[i]->cd();
      theOccupancy[i]->Draw("gcolz");
      theCanvas[i]->Modified();
      theCanvas[i]->Update();
    }

  theCanvasToT->cd();
  theToT->Draw();
  theCanvasToT->Modified();
  theCanvasToT->Update();

  theCanvasBCID->cd();
  theBCID->Draw();
  theCanvasBCID->Modified();
  theCanvasBCID->Update();

  theCanvasOcc->cd();
  theOcc->Draw();
  theCanvasOcc->Modified();
  theCanvasOcc->Update();

  theCanvasErr->cd();
  theErr->Draw();
  theCanvasErr->Modified();
  theCanvasErr->Update();
}

 
void PixelAlive::Save()
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
  
  theToT->Write();
  theBCID->Write();
  theOcc->Write();
  theErr->Write();
  theFile->Write();

  tmp = fileName;
  tmp = tmp.erase(tmp.find(".root"),5) + "_ToT.svg";
  theCanvasToT->Print(tmp.c_str());

  tmp = fileName;
  tmp = tmp.erase(tmp.find(".root"),5) + "_BCID.svg";
  theCanvasBCID->Print(tmp.c_str());

  tmp = fileName;
  tmp = tmp.erase(tmp.find(".root"),5) + "_Occ.svg";
  theCanvasOcc->Print(tmp.c_str());

  tmp = fileName;
  tmp = tmp.erase(tmp.find(".root"),5) + "_Err.svg";
  theCanvasErr->Print(tmp.c_str());
}
