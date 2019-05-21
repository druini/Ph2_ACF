/*!
  \file                  RD53SCurve.cc
  \brief                 Implementaion pf SCurve scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53SCurve.h"

SCurve::SCurve(const char* fName, size_t rStart, size_t rEnd, size_t cStart, size_t cEnd, size_t nPix, size_t nTrig, float startValue, float stopValue, int nSteps) :
  fileName(fName),
  rowStart(rStart),
  rowEnd(rEnd),
  colStart(cStart),
  colEnd(cEnd),
  nPixels2Inj(nPix),
  nTriggers(nTrig),
  startValue(startValue),
  stopValue(stopValue),
  nSteps(nSteps),
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
  size_t indx = 0;
//   for (auto cBoard : fBoardVector)
//     for (auto cFe : cBoard->fModuleVector)
//       for (auto cChip : cFe->fChipVector)
// 	{
	  myString << "theOccupancy_" << indx;
	  theOccupancy.push_back(new TH2F(myString.str().c_str(),"SCurve",nSteps,startValue,stopValue,nTriggers,0,1));
	//   indx++;
	// }
    
  theFile = new TFile(fileName, "RECREATE");
  theCanvas = new TCanvas("theCanvas","RD53Canvas",0,0,700,500);
  theCanvas->Divide(sqrt(theOccupancy.size()),sqrt(theOccupancy.size()));
}

SCurve::~SCurve()
{
  theFile->Close();
  
  delete fChannelGroupHandler;
  delete theFile;
  delete theCanvas;
  for (size_t i = 0; i < theOccupancy.size(); i++)
    delete theOccupancy[i];
}

void SCurve::Run()
{
//   DetectorContainer         theOccupancyContainer;
//   fDetectorDataContainer = &theOccupancyContainer;
  ContainerFactory          theDetectorFactory;

  float step = (stopValue - startValue) / nSteps;

  std::vector<uint16_t> dacList;
  for (int i = 0; i < nSteps; i++) {
      dacList.push_back(startValue + step * i);
  }

  std::vector<DetectorContainer*> detectorContainerVector(dacList.size());
  for (auto& p : detectorContainerVector)
    theDetectorFactory.copyAndInitStructure<Occupancy>(*fDetectorContainer, *p);
  
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nTriggers, detectorContainerVector);
//   this->measureData(nTriggers);

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
          for (int i = 0; i < dacList.size(); i++)
	          theOccupancy[indx]->Fill(dacList[i],detectorContainerVector[i]->at(cBoard->getBeId())->at(cFe->getFeId())->at(cChip->getChipId())->getChannel<Occupancy>(row,col).fOccupancy/nTriggers);
	    indx++;
      }
}

void SCurve::Display()
{
//   theFile = new TFile(fileName, "RECREATE");
  theFile->cd();

  for (size_t i = 0; i < theOccupancy.size(); i++)
    {
      theCanvas->cd(i+1);
      theOccupancy[i]->Draw("gcolz");
    }
  
  theCanvas->Modified();
  theCanvas->Update();
}

void SCurve::Save()
{
  theCanvas->Write();
  theFile->Write();
}
