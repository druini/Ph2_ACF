/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nTrig, bool inject) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nTriggers(nTrig),
  inject(inject),
  Tool()
{
  std::stringstream myString;
  myString.clear();
  myString.str("");


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
  

  // #######################
  // # Allocate histograms #
  // #######################
  for (auto i = 0; i < NHISTO; i++)
    {
      myString << "theOccupancy_" << i;
      theOccupancy.push_back(new TH2F(myString.str().c_str(),"PixelAlive",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
      theOccupancy.back()->SetXTitle("Columns");
      theOccupancy.back()->SetYTitle("Rows");
    }
    
  theFile   = new TFile(fileName, "RECREATE");
  theCanvas = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theCanvas->Divide(sqrt(theOccupancy.size()),sqrt(theOccupancy.size()));
}

PixelAlive::~PixelAlive()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;
  delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    delete theOccupancy[i];
}

void PixelAlive::Run()
{
  DetectorContainer         theOccupancyContainer;
  fDetectorDataContainer = &theOccupancyContainer;
  ContainerFactory          theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
  
  this->SetTestPulse(inject);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nTriggers);


  // #########################
  // # Filling the histogram #
  // #########################
  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      {
	size_t indx = 0;
	for (auto cChip : cFe->fChipVector)
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      theOccupancy[indx]->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy);
	indx++;
      }
}

void PixelAlive::Display()
{
  theFile->cd();

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
}
