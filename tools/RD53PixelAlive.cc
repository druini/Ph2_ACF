/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nEvts, bool inject) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nEvents(nEvts),
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
  if (theFile != nullptr)              delete theFile;
  if (theCanvas != nullptr)            delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    if (theOccupancy[i] != nullptr) delete theOccupancy[i];
}

void PixelAlive::InitHisto()
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
          myString << "PixelAlive_Board" << std::setfill ('0') << std::setw (3) << +cBoard->getBeId()
		   << "_Mod"             << std::setfill ('0') << std::setw (3) << +cFe->getFeId()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getChipId();
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theOccupancy.back()->SetXTitle("Columns");
	  theOccupancy.back()->SetYTitle("Rows");
	}

  theFile   = new TFile(fileName, "RECREATE");
  theCanvas = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theCanvas->Divide(sqrt(theOccupancy.size()),sqrt(theOccupancy.size()));
}

void PixelAlive::Run()
{
  DetectorContainer         theOccupancyContainer;
  fDetectorDataContainer = &theOccupancyContainer;
  ContainerFactory          theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<OccupancyAndToT>(*fDetectorContainer, *fDetectorDataContainer);
  
  this->SetTestPulse(inject);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents);


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
	      theOccupancy[index]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<OccupancyAndToT>(row,col).fOccupancy);
	  index++;
	}
}

void PixelAlive::Display()
{
  for (size_t i = 0; i < theOccupancy.size(); i++)
    {
      theCanvas->cd(i+1);
      theOccupancy[i]->Draw("gcolz");
    }
  
  theCanvas->Modified();
  theCanvas->Update();
}

void PixelAlive::Save()
{
  theCanvas->Write();
  theFile->Write();

  theCanvas->Print("PixelAlive.png");
}
