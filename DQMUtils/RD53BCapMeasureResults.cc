/*!
  \file                  RD53BCapMeasureResults.cc
  \brief                 Save capacitance measurements to csv file
*/

#include "RD53BCapMeasureResults.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

double RD53BCapMeasureResults::calcCapacitance(double vmeas, double vmeaspar, double vddameas){
  double meas_cap = 0.01*1e15*(((vmeas/4990)/(10e6*(vddameas-vmeas))) - ((vmeaspar/4990)/(10e6*(vddameas-vmeaspar))));
  return meas_cap;
}

void RD53BCapMeasureResults::fillCapacitance( const double (&CapVoltages)[4])
{

        bool fileExists = false;
        std::ifstream checkInput;
        checkInput.open("Results/capMeasure.csv");
        if (checkInput.is_open()) fileExists=true;
        checkInput.close();


	std::ofstream outputFile;
	outputFile.open("Results/capMeasure.csv",std::ios_base::app);
        double capacitance = calcCapacitance(CapVoltages[0],CapVoltages[2],2*CapVoltages[1]);

        
        if(!fileExists){
            outputFile << "Vmain, VDDAmain, Vpara, VDDApara, capacitance [fF]" << "\n";
         }
        for(int i=0;i<4;i++){
                outputFile << CapVoltages[i] << ",";
            }  
        outputFile <<capacitance << "\n"; //Each line contains Vmain, VDDAmain, Vpara, VDDApara, capacitance [fF]
        outputFile.close();
}
