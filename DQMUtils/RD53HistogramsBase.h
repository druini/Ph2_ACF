/*!
  \file                  RD53HistogramsBase.h
  \brief                 Base class for Inner Tracker histograms
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef _RD53HistogramsBase_h_
#define _RD53HistogramsBase_h_

#include "../HWDescription/RD53.h"

#include <TCanvas.h>
#include <TGaxis.h>
#include <TPad.h>
#include <TFile.h>


class RD53HistogramsBase
{
 private:
  int canvasId = 0;
  std::vector<std::unique_ptr<TCanvas>> canvases;
  std::vector<std::unique_ptr<TGaxis>>  axes;

 protected:
  template <typename Hist>
    void bookImplementer (TFile* theOutputFile,
			  const DetectorContainer& theDetectorStructure,
			  const HistContainer<Hist>& histContainer,
			  DetectorDataContainer& dataContainer,
			  const char* XTitle = nullptr,
			  const char* YTitle = nullptr)
    {
      if (XTitle != nullptr) histContainer.fTheHistogram->SetXTitle(XTitle);
      if (YTitle != nullptr) histContainer.fTheHistogram->SetYTitle(YTitle);
      
      RootContainerFactory theRootFactory;
      theRootFactory.bookChipHistrograms(theOutputFile, theDetectorStructure, dataContainer, histContainer);
    }
  
  template <typename Hist> 
    void draw (DetectorDataContainer& HistDataContainer,
	       const char* opt               = "",
	       bool electronAxis             = false,
	       const char* electronAxisTitle = "")
    {
      for (auto cBoard : HistDataContainer)
	for (auto cModule : *cBoard)
	  for (auto cChip : *cModule)
	    {
	      canvases.emplace_back(new TCanvas(("Canvas_" + std::to_string(canvasId++)).c_str(), "IT Canvas"));
	      canvases.back()->cd();
	      Hist* hist = cChip->getSummary<HistContainer<Hist>>().fTheHistogram;
	      hist->Draw(opt);
	      canvases.back()->Modified();
	      canvases.back()->Update();

	      if (electronAxis == true)
		{
		  TPad* myPad = static_cast<TPad*>(canvases.back()->GetPad(0));
		  myPad->SetTopMargin(0.16);
		  
		  axes.emplace_back(new TGaxis(myPad->GetUxmin(), myPad->GetUymax(), myPad->GetUxmax(), myPad->GetUymax(),
					       RD53chargeConverter::VCAl2Charge(hist->GetXaxis()->GetBinLowEdge(1), true),
					       RD53chargeConverter::VCAl2Charge(hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetNbins()), true), 510, "-"));
		  axes.back()->SetTitle(electronAxisTitle);
		  axes.back()->SetTitleOffset(1.2);
		  axes.back()->SetTitleSize(0.035);
		  axes.back()->SetTitleFont(40);
		  axes.back()->SetLabelOffset(0.001);
		  axes.back()->SetLabelSize(0.035);
		  axes.back()->SetLabelFont(42);
		  axes.back()->SetLabelColor(kRed);
		  axes.back()->SetLineColor(kRed);
		  axes.back()->Draw();

		  canvases.back()->Modified();
		  canvases.back()->Update();
		}
	    }
    }
};

#endif
