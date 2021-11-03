/*!
  \file                  RD53DACScanHistograms.cc
  \brief                 Implementation of DACScan histograms
  \author                Umberto MOLINATTI
  \version               1.0
  \date                  09/04/21
  Support:               email to umberto.molinatti@cern.ch
*/

#include "RD53DACScanHistograms.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

void DACScanHistograms::fillDAC(DetectorContainer& DataContainer, double* fitStart, double* fitEnd, double** VMUXvolt, double** DACcode, std::string* writeVar)
{
    auto canvas = new TCanvas();
    remove("Results/DAC_linearity.root"); // Remove old file
    TFile* file = new TFile("Results/DAC_linearity.root", "new");
    canvas->Print("Results/DAC_plots.pdf[");
                    for(int variable = 0; variable < 1; variable++)
                    {
                        TGraph* linear = new TGraph(390, DACcode[variable], VMUXvolt[variable]);
                        linear->SetTitle("CAL_MED;Injected DAC Code;Calibration DAC Output Voltage [V]");
                        linear->SetName("CAL_MED");
                        linear->Fit("pol1","","",20,4080);
						TFitResultPtr r = linear->Fit("pol1", "S", "", fitStart[variable], fitEnd[variable]);
                        gStyle->SetOptFit(0);
                        gStyle->SetFitFormat(".2g");
                        linear->Draw("APL");
                        canvas->Print("Results/DAC_plots.pdf");
						remove( "Results/DAC_calibration.txt" ); //Remove old file
						std::ofstream outfile;
						outfile.open("Results/DAC_calibration.txt",std::ios_base::app);
						outfile << "Intercept: " << r->Value(0) << "\nSlope: " << r->Value(1) << "\nMax: " << (r->Value(0))+(r->Value(1))*(fitEnd[variable]);
                        linear->Write();
                }
    file->Write();
    canvas->Print("Results/DAC_plots.pdf]");
}
