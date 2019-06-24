/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion pf PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nTrig) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nTriggers(nTrig),
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
  
  theCanvas    = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theOccupancy = new TH2F("theOccupancy","PixelAlive",RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows);
}

PixelAlive::~PixelAlive()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;
  delete theCanvas;
  delete theOccupancy;
}

void PixelAlive::Run()
{
  DetectorContainer         theOccupancyContainer;
  fDetectorDataContainer = &theOccupancyContainer;
  ContainerFactory          theDetectorFactory;
  theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *fDetectorDataContainer);
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nTriggers);
  
  // #########################
  // # Filling the histogram #
  // #########################
  for (auto cBoard : fBoardVector)
    for (auto cFe : cBoard->fModuleVector)
      for (auto cChip : cFe->fReadoutChipVector)
	for (auto row = 0; row < RD53::nRows; row++)
	  for (auto col = 0; col < RD53::nCols; col++)
	    theOccupancy->SetBinContent(col+1,row+1,theOccupancyContainer.at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy);
}

void PixelAlive::Display()
{
  theFile = new TFile(fileName, "RECREATE");
  
  theOccupancy->Draw("gcolz");
  theCanvas->Modified();
  theCanvas->Update();
}

void PixelAlive::Save()
{
  theCanvas->Write();
  theFile->Write();
}
