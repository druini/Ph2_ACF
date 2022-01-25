/*!
  \file                  RD53BCapMeasureHistograms.cc
  \brief                 Implementation of CapMeasure histograms
*/

#include "RD53BCapMeasureHistograms.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

double RD53BCapMeasureHistograms::calcCapacitance(double vmeas, double vmeaspar, double vddameas){
  double meas_cap = 0.01*1e15*(((vmeas/4990)/(10e6*(vddameas-vmeas))) - ((vmeaspar/4990)/(10e6*(vddameas-vmeaspar))));
  return meas_cap;
}

void RD53BCapMeasureHistograms::fillCap( const double (&VMain)[16], const double (&VDDAMain)[16], const double (&VPara)[16], const double (&VDDAPara)[16], const double (&VTrim)[16] )
{
	TFile *file = new TFile("Results/capMeasure.root","UPDATE");
	std::ofstream myfile;
	myfile.open("Results/capMeasure.log",std::ios_base::app);
        myfile << "================Results to follow==================="<< "\n";
        for(int trimSetting=0; trimSetting<16; trimSetting++){ 
            myfile << " === trim setting ==== "<< trimSetting << " Corresponding to  "<<VTrim[trimSetting] <<"\n";
            TGraph * thisGraph = NULL;
            if(file->GetListOfKeys()->Contains("VMain")){
                thisGraph = (TGraph*) file->Get("VMain");
            } else {
                thisGraph = new TGraph(1);
                thisGraph->SetTitle("VMain");
                thisGraph->SetName("VMain");
            }
            thisGraph->SetPoint(thisGraph->GetN(),(thisGraph->GetN()-1), VMain[trimSetting]);
            thisGraph->Write("",TObject::kOverwrite);
            myfile << "Main voltage = "<< VMain[trimSetting]<< "\n";

            thisGraph = NULL;
            if(file->GetListOfKeys()->Contains("VPara")){
                thisGraph = (TGraph*) file->Get("VPara");
            } else {
                thisGraph = new TGraph(1);
                thisGraph->SetTitle("VPara");
                thisGraph->SetName("VPara");
            }
            thisGraph->SetPoint(thisGraph->GetN(),(thisGraph->GetN()-1), VPara[trimSetting]);
            thisGraph->Write("",TObject::kOverwrite);
            myfile << "Parasitic voltage = "<< VPara[trimSetting]<< "\n";

            thisGraph = NULL;
            if(file->GetListOfKeys()->Contains("VDDAMain")){
                thisGraph = (TGraph*) file->Get("VDDAMain");
            } else {
                thisGraph = new TGraph(1);
                thisGraph->SetTitle("VDDAMain");
                thisGraph->SetName("VDDAMain");
            }
            thisGraph->SetPoint(thisGraph->GetN(),(thisGraph->GetN()-1), 2*VDDAMain[trimSetting]);
            thisGraph->Write("",TObject::kOverwrite);
            myfile << "Main VDDA  = "<< 2*VDDAMain[trimSetting]<< "\n";

             thisGraph = NULL;
            if(file->GetListOfKeys()->Contains("VDDAPara")){
                thisGraph = (TGraph*) file->Get("VDDAPara");
            } else {
                thisGraph = new TGraph(1);
                thisGraph->SetTitle("VDDAPara");
                thisGraph->SetName("VDDAPara");
            }
            thisGraph->SetPoint(thisGraph->GetN(),(thisGraph->GetN()-1), 2*VDDAPara[trimSetting]);
            thisGraph->Write("",TObject::kOverwrite);
            myfile << "Parasitic VDDA  = "<< 2*VDDAPara[trimSetting]<< "\n";

            double capacitance = calcCapacitance(VMain[trimSetting],VPara[trimSetting],2*VDDAMain[trimSetting]);

             thisGraph = NULL;
            if(file->GetListOfKeys()->Contains("capacitance")){
                    thisGraph = (TGraph*)file->Get("capacitance");
            }else{
                    thisGraph = new TGraph (1);
                    thisGraph->SetTitle("capacitance");
                    thisGraph->SetName("capacitance");
            }
            thisGraph->SetPoint(thisGraph->GetN(),2*VDDAMain[trimSetting],capacitance);
            thisGraph->Write("",TObject::kOverwrite);

            myfile << "capacitance  = "<< capacitance <<"\n";
       }


        myfile << "================END OF LOOP==================="<< "\n";
	file->Write();
	myfile.close();
}
