/*!
  \file                  RD53PixelAlive.cc
  \brief                 Implementaion of PixelAlive scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53PixelAlive.h"

PixelAlive::PixelAlive (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nPixels2Inj, size_t nEvents, size_t nEvtsBurst, bool inject) :
  fileRes     (fileRes),
  rowStart    (rowStart),
  rowStop     (rowStop),
  colStart    (colStart),
  colStop     (colStop),
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

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);

  theChnGroupHandler = std::shared_ptr<RD53ChannelGroupHandler>(new RD53ChannelGroupHandler());
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);
  theChnGroupHandler->setChannelGroupParameters(nPixels2Inj, 1, 1);
}

PixelAlive::~PixelAlive ()
{
  delete theFile;
  theFile = nullptr;

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
}

void PixelAlive::Run ()
{
  ContainerFactory theDetectorFactory;

  theOccContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  this->fDetectorDataContainer = theOccContainer.get();
  theDetectorFactory.copyAndInitStructure<OccupancyAndPh,GenericDataVector>(*fDetectorContainer, *fDetectorDataContainer);

  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(inject);
  this->fMaskChannelsFromOtherGroups = true;
  this->measureData(nEvents, nEvtsBurst);


  // ################
  // # Error report #
  // ################
  this->ChipErrorReport();
}

void PixelAlive::Draw (bool display, bool save)
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

std::shared_ptr<DetectorDataContainer> PixelAlive::Analyze ()
{
  for (const auto cBoard : *theOccContainer.get())
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	LOG (INFO) << BOLDGREEN << "\t--> Average occupancy for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "] is " << BOLDYELLOW
		   << cChip->getSummary<GenericDataVector,OccupancyAndPh>().fOccupancy << RESET;
  
  return theOccContainer;
}

void PixelAlive::InitHisto ()
{
  std::stringstream myString;
  size_t ToTsize   = RD53::SetBits(RD53EvtEncoder::NBIT_TOT/NPIX_REGION)+1;
  size_t BCIDsize  = RD53::SetBits(RD53EvtEncoder::NBIT_BCID)+1;
  size_t TrgIDsize = RD53::SetBits(RD53EvtEncoder::NBIT_TRIGID)+1;


  // #######################
  // # Allocate histograms #
  // #######################
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
        {
	  myString.clear();
	  myString.str("");
          myString << "PixelAlive_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOcc2D.push_back(new TH2F(myString.str().c_str(),myString.str().c_str(),RD53::nCols,0,RD53::nCols,RD53::nRows,0,RD53::nRows));
	  theOcc2D.back()->SetXTitle("Columns");
	  theOcc2D.back()->SetYTitle("Rows");

	  myString.clear();
	  myString.str("");
          myString << "CanvasPixelAlive_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"                   << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"                  << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc2D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "ToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"      << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"     << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theToT.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),ToTsize,0,ToTsize));
	  theToT.back()->SetXTitle("ToT");
	  theToT.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasToT_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"            << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"           << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasToT.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "Occ1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"        << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"       << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theOcc1D.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),nEvents+1,0,nEvents+1));
	  theOcc1D.back()->SetXTitle("Occupancy");
	  theOcc1D.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasOcc1D_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"              << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"             << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasOcc1D.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));

	  
	  myString.clear();
	  myString.str("");
          myString << "BCID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"       << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"      << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theBCID.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),BCIDsize,0,BCIDsize));
	  theBCID.back()->SetXTitle("#DeltaBCID");
	  theBCID.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasBCID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"             << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"            << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasBCID.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));


	  myString.clear();
	  myString.str("");
          myString << "TrgID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"        << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"       << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theTrgID.push_back(new TH1F(myString.str().c_str(),myString.str().c_str(),TrgIDsize,0,TrgIDsize));
	  theTrgID.back()->SetXTitle("#DeltaTrigger-ID");
	  theTrgID.back()->SetYTitle("Entries");

	  myString.clear();
	  myString.str("");
          myString << "CanvasTrgID_Board" << std::setfill ('0') << std::setw (2) << +cBoard->getIndex()
		   << "_Mod"              << std::setfill ('0') << std::setw (2) << +cModule->getIndex()
		   << "_Chip"             << std::setfill ('0') << std::setw (2) << +cChip->getIndex();
	  theCanvasTrgID.push_back(new TCanvas(myString.str().c_str(),myString.str().c_str(),0,0,700,500));
	}
  
  theFile = new TFile(fileRes, "UPDATE");
}

void PixelAlive::FillHisto ()
{
  size_t index = 0;
  for (const auto cBoard : *theOccContainer.get())
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  for (auto row = 0; row < RD53::nRows; row++)
	    for (auto col = 0; col < RD53::nCols; col++)
	      if (cChip->getChannel<OccupancyAndPh>(row,col).fOccupancy != 0)
		{
		  theOcc2D[index]->SetBinContent(col+1,row+1,cChip->getChannel<OccupancyAndPh>(row,col).fOccupancy);
		  theOcc1D[index]->Fill(cChip->getChannel<OccupancyAndPh>(row,col).fOccupancy * nEvents);
		  theToT[index]->Fill(cChip->getChannel<OccupancyAndPh>(row,col).fPh);
		}

	  for (auto i = 1; i < cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1.size(); i++)
	    {
	      int deltaBCID = cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1[i] - cChip->getSummary<GenericDataVector,OccupancyAndPh>().data1[i-1];	      
	      theBCID[index]->Fill((deltaBCID > 0 ? 0 : RD53::SetBits(RD53EvtEncoder::NBIT_BCID)+1) + deltaBCID);
	    }

	  for (auto i = 1; i < cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2.size(); i++)
	    {
	      int deltaTrgID = cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2[i] -	cChip->getSummary<GenericDataVector,OccupancyAndPh>().data2[i-1];	      
	      theTrgID[index]->Fill((deltaTrgID > 0 ? 0 : RD53::SetBits(RD53EvtEncoder::NBIT_TRIGID)+1) + deltaTrgID);
	    }
	  
	  index++;
	}
}

void PixelAlive::Display ()
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
}

void PixelAlive::Save ()
{
  for (auto i = 0; i < theCanvasOcc2D.size(); i++) theCanvasOcc2D[i]->Write();
  for (auto i = 0; i < theCanvasToT.size();   i++) theCanvasToT[i]->Write();
  for (auto i = 0; i < theCanvasOcc1D.size(); i++) theCanvasOcc1D[i]->Write();
  for (auto i = 0; i < theCanvasBCID.size();  i++) theCanvasBCID[i]->Write();
  for (auto i = 0; i < theCanvasTrgID.size(); i++) theCanvasTrgID[i]->Write();
}

void PixelAlive::ChipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  LOG (INFO) << BOLDGREEN << "\t--> Readout chip error repor for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
	  LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
	}
}
