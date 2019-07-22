//Simple test script to demonstrate use of middleware for the purposes of usercode development

#include <cstring>
#include <iostream>
#include <fstream>
#include <numeric>  // for std::accumulate
#include "../Utils/Utilities.h"
#include "../HWDescription/SSA.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/D19cFWInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/FrontEndDescription.h"
#include "../Utils/Timer.h"
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "TH1.h"
#include "TCanvas.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main( int argc, char* argv[] )
{


	std::string cHWFile = "settings/HWDescription_MPA.xml";
	ofstream myfile;
	ofstream scurvecsv;
	scurvecsv.open ("scurvetemp.csv");


	SystemController mysyscontroller;
	std::cout << "\nInitHW";
	mysyscontroller.InitializeHw( cHWFile );
	std::cout << "\nMPAI";
        MPAInterface* fMPAInterface = mysyscontroller.fMPAInterface;
	std::cout << "\nBOARD"<<std::endl;

	MPA* mpa1 = new MPA(0, 0, 0, 0,"settings/MPAFiles/MPA_default.txt");
	mpa1->loadfRegMap("settings/MPAFiles/MPA_default.txt");

	std::chrono::milliseconds LongPOWait( 500 );
	std::chrono::milliseconds ShortWait( 10 );

        // should be done from configure hw
        //fMPAInterface->Align_out();

	fMPAInterface->PS_Clear_counters();
	fMPAInterface->PS_Clear_counters();
	//fMPAInterface->activate_I2C_chip();



        std::pair<uint32_t, uint32_t> rows = {0,17};
        std::pair<uint32_t, uint32_t> cols = {0,120};
        std::pair<uint32_t, uint32_t> th = {0,40};

 	std::vector<TH1F*> scurves;
	std::string title;

	fMPAInterface->Activate_async(mpa1);
	fMPAInterface->Set_calibration(mpa1,50);

	uint32_t npixtot = 0;
	for(uint16_t row=rows.first; row<rows.second; row++)
		{
		for(uint16_t col=cols.first; col<cols.second; col++)
			{
				fMPAInterface->Enable_pix_counter(mpa1,row, col);
				title = std::to_string(row)+","+std::to_string(col);
 				scurves.push_back(new TH1F(title.c_str(),title.c_str(),255,-0.5,254.5));
				npixtot+=1;
			}
		}
	std::cout <<"Numpix -- "<< npixtot <<std::endl;
	std::vector<uint16_t> countersfifo;
	// uint32_t curpnum = 0;
	uint32_t totalevents = 0;
	uint32_t totaleventsprev = 0;
	uint32_t nrep = 0;
	for(uint16_t ith=th.first;ith<th.second;ith++)
		{
		std::cout<<"ITH= "<<ith<<std::endl;
		fMPAInterface->Set_threshold(mpa1,ith);

		std::this_thread::sleep_for( ShortWait );
		fMPAInterface->Send_pulses(2000);
		std::this_thread::sleep_for( ShortWait );
		// curpnum = 0;
		scurvecsv << ith<<",";

		//FIFO readout
		countersfifo = fMPAInterface->ReadoutCounters_MPA(0);

		//Randomly the counters fail
		//this fixes the issue but this needs to be looked at further
		totalevents = std::accumulate(countersfifo.begin()+1, countersfifo.end(), 0);
		std::cout<<totalevents<<std::endl;
		if (totaleventsprev>50 and totalevents==0)
			{
				ith-=1;
				nrep+=1;
				std::cout<<"Repeat "<<nrep<<std::endl;
				if (nrep<5) continue;
				totaleventsprev = 0;
			}

		for (size_t icc=0; icc<2040u; icc++)
			{
 			scurves[icc]->SetBinContent(scurves[icc]->FindBin(ith), countersfifo[icc]);
			scurvecsv << countersfifo[icc]<<",";
			}
		nrep=0;

		//I2C readout
		/*for(int row=rows.first; row<rows.second; row++)
			{
			for(int col=cols.first; col<cols.second; col++)
				{
					counters[curpnum]=fMPAInterface->Read_pixel_counter(row, col);

 					scurves[curpnum]->SetBinContent(scurves[curpnum]->FindBin(ith), counters[curpnum]);
					scurvecsv << counters[curpnum]<<",";
					curpnum+=1;
				}
			}

		std::cout<<"Thresh "<<ith<<" - Counts[0] "<<counters[0]<<" - Counts[1] "<<counters[1]<<std::endl;
		*/
		scurvecsv <<"\n";
		fMPAInterface->PS_Clear_counters(8);
		fMPAInterface->PS_Clear_counters(8);
		totaleventsprev = totalevents;
		}


 	TCanvas * c1 = new TCanvas("c1", "c1", 1000, 500);
	int ihist = 0;
	for (auto& hist : scurves)
		{
		//std::cout<<"drawing "<<ihist<<hist->>Integral()<<std::endl;
		if (ihist==0)
			{
			hist->SetLineColor(1);
			hist->SetTitle(";Thresh DAC;Counts");
			hist->SetMaximum(40000);
			hist->SetStats(0);
			hist->Draw("L");
			}
		else
			{
			hist->SetLineColor(ihist%60+1);
			hist->Draw("sameL");
			}
		ihist += 1;
		}
	c1->Print("scurvetemp.root","root");
	scurvecsv.close();

}//int main
