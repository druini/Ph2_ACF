/*!
  \file                  RD53VrefTrimmingHistograms.cc
  \brief                 Implementation of VrefTrimming histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  22/11/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53VrefTrimmingHistograms.h"

#include <boost/filesystem.hpp>
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void VrefTrimmingHistograms::fillVT(const double (&vdddVoltage)[16], const double (&vddaVoltage)[16])
{	
	TFile *file = new TFile("Results/vref_trimming.root","UPDATE");
	double trimBits[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	//VDDD
	TGraph* vddd_linear = new TGraph(16, trimBits, vdddVoltage);
	TFitResultPtr vddd_fit = vddd_linear->Fit("pol1", "S", "");
	double vddd_idealTrim = (1.2 - vddd_fit->Value(0))/vddd_fit->Value(1);
	
	TGraph* vddd_countPlot = NULL;
	if(file->GetListOfKeys()->Contains("VDDD_trim")){
		vddd_countPlot = (TGraph*)file->Get("VDDD_trim");
	}else{
		vddd_countPlot = new TGraph (1);
		vddd_countPlot->SetTitle("VDDD_trim");
		vddd_countPlot->SetName("VDDD_trim");	
	}
	vddd_countPlot->SetPoint(vddd_countPlot->GetN(),(vddd_countPlot->GetN()-1),vddd_idealTrim);
	vddd_countPlot->Write("",TObject::kOverwrite);
	
	//VDDA
	TGraph* vdda_linear = new TGraph(16, trimBits, vddaVoltage);
	TFitResultPtr vdda_fit = vdda_linear->Fit("pol1", "S", "");
	double vdda_idealTrim = (1.2 - vdda_fit->Value(0))/vdda_fit->Value(1);
	
	TGraph* vdda_countPlot = NULL;
	if(file->GetListOfKeys()->Contains("VDDA_trim")){
		vdda_countPlot = (TGraph*)file->Get("VDDA_trim");
	}else{
		vdda_countPlot = new TGraph (1);
		vdda_countPlot->SetTitle("VDDA_trim");
		vdda_countPlot->SetName("VDDA_trim");	
	}
	vdda_countPlot->SetPoint(vdda_countPlot->GetN(),(vdda_countPlot->GetN()-1),vdda_idealTrim);
	vdda_countPlot->Write("",TObject::kOverwrite);
	
	
    static const std::string fileName = "Results/vref_trimming.csv";
    std::ofstream outFile;
    if (boost::filesystem::exists(fileName))
        outFile.open(fileName, std::ios_base::app);
    else {
        outFile.open(fileName);
        outFile << "time, VDDD_trim, VDDA_trim\n";
    }
    auto now = time(0);
    outFile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S, ") << vddd_idealTrim << " ," << vdda_idealTrim <<"\n";
	
    file->Write();
    file->Close();
}
