/*!
        \file                DQMHistogramBase.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch

*/

#ifndef __DQMHISTOGRAMBASE_H__
#define __DQMHISTOGRAMBASE_H__

#include <string>
#include <vector>
#include <memory>

#include "../HWDescription/RD53.h"
#include "../RootUtils/RootContainerFactory.h"
#include "../RootUtils/HistContainer.h"
#include "../Utils/Container.h"

#include <TCanvas.h>
#include <TGaxis.h>
#include <TPad.h>
#include <TFile.h>

class DetectorDataContainer;
class DetectorContainer;

/*!
 * \class DQMHistogramBase
 * \brief Base class for monitoring histograms
 */
class DQMHistogramBase
{

  public:
    /*!
     * constructor
     */
    DQMHistogramBase (){;}

    /*!
     * destructor
     */
    virtual ~DQMHistogramBase(){;}

    /*!
     * \brief Book histograms
     * \param theDetectorStructure : Container of the Detector structure
     */
    virtual void book(TFile *outputFile, const DetectorContainer &theDetectorStructure, std::map<std::string, double> pSettingsMap) = 0;

    /*!
     * \brief Book histograms
     * \param configurationFileName : xml configuration file
     */
    virtual bool fill (std::vector<char>& dataBuffer) = 0;
    
    /*!
     * \brief SAve histograms
     * \param outFile : ouput file name
     */
    virtual void process () = 0;
    
    /*!
     * \brief Book histograms
     * \param configurationFileName : xml configuration file
     */
    virtual void reset(void) = 0;
    //virtual void summarizeHistos();

 private:
    static int& canvasId() {
      static int value = 0;
      return value;
    };
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
	      canvases.emplace_back(new TCanvas(("Canvas_" + std::to_string(canvasId()++)).c_str(), "IT Canvas"));
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

  auto findValue (const std::map<std::string, double>& pSettingsMap, const char* name)
  {
    auto setting = pSettingsMap.find(name);
    return ((setting != std::end(pSettingsMap)) ? setting->second : 0);
  }
};

#endif
