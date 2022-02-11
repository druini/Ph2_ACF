/*!
  \file                  RD53ADCScanHistograms.cc
  \brief                 Implementation of ADCScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  16/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53ADCScanHistograms.h"

#include <boost/filesystem.hpp>
using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void ADCScanHistograms::fillADC(const double (&fitStart)[9], const double (&fitEnd)[9], const std::vector<std::vector<double>>& VMUXvolt, const std::vector<std::vector<double>>& ADCcode, const std::string* writeVar)
{
	static char auxvar[LOGNAME_SIZE];
	time_t now = time(0);
	strftime(auxvar, sizeof(auxvar), LOGNAME_FORMAT, localtime(&now));
	std::string outputname;
	outputname = auxvar;	
	
    auto canvas = new TCanvas();
    TFile* file = new TFile(("Results/ADC_linearity_" + outputname + ".root").c_str(), "new");
    canvas->Print(("Results/ADCplots_" + outputname + ".pdf[").c_str());
	for(int variable = 0; variable < 1; variable++)
	{
		TGraph* linear = new TGraph(ADCcode.size(), &(ADCcode[0][variable]), &(VMUXvolt[0][variable]));
		linear->SetTitle("Linear Graph;ADC Code;Voltage");
		linear->SetName(writeVar[variable].c_str());
		linear->Fit("pol1", "", "", fitStart[variable], fitEnd[variable]);
		TFitResultPtr r = linear->Fit("pol1", "S", "", fitStart[variable], fitEnd[variable]);
		gStyle->SetOptFit(0);
		gStyle->SetFitFormat(".2g");
		linear->Draw("APL");
		canvas->Print(("Results/ADCplots_" + outputname + ".pdf").c_str());
		static const std::string fileName = "Results/ADCCalibration.csv";
		std::ofstream outFile;
		if (boost::filesystem::exists(fileName))
			outFile.open(fileName, std::ios_base::app);
		else {
			outFile.open(fileName);
			outFile << "time, Intercept, Slope\n";
		}
		auto now = time(0);
		outFile << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S, ");
		outFile << r->Value(0) << ", " << r->Value(1) << "\n";
		linear->Write();
	}
    file->Write();
    canvas->Print(("Results/ADCplots_" + outputname + ".pdf]").c_str());
    file->Close();
}
