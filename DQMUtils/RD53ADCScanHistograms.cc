/*!
  \file                  RD53ADCScanHistograms.cc
  \brief                 Implementation of ADCScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  16/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53ADCScanHistograms.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void ADCScanHistograms::fillADC(const double (&fitStart)[9], const double (&fitEnd)[9], const double (&VMUXvolt)[9][5000], const double (&ADCcode)[9][5000], const std::string* writeVar)
{
    auto canvas = new TCanvas();
    remove("Results/ADC_linearity.root"); // Remove old file
    TFile* file = new TFile("Results/ADC_linearity.root", "new");
    canvas->Print("Results/ADCplots.pdf[");
                    for(int variable = 0; variable < 1; variable++)
                    {
                        TGraph* linear = new TGraph(390, ADCcode[variable], VMUXvolt[variable]);
                        linear->SetTitle("Linear Graph;ADC Code;Voltage");
                        linear->SetName(writeVar[variable].c_str());
                        linear->Fit("pol1", "", "", fitStart[variable], fitEnd[variable]);
						TFitResultPtr r = linear->Fit("pol1", "S", "", fitStart[variable], fitEnd[variable]);
                        gStyle->SetOptFit(0);
                        gStyle->SetFitFormat(".2g");
                        linear->Draw("APL");
						canvas->Print("Results/ADCplots.pdf");
						remove( "Results/calibration.txt" ); //Remove old file
						std::ofstream outfile;
						outfile.open("Results/calibration.txt",std::ios_base::app);
						outfile << "Intercept: " << r->Value(0) << "\nSlope: " << r->Value(1) << "\nMax: " << (r->Value(0))+(r->Value(1))*(fitEnd[variable]);
						linear->Write();
                    }
    file->Write();
    canvas->Print("Results/ADCplots.pdf]");
}
