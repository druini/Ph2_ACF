//Simple test script to demonstrate use of middleware for the purposes of usercode development

#include <cstring>
#include <iostream>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/SSA.h"
#include "../HWDescription/OuterTrackerModule.h"
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
	//ofstream scurvecsv;
	//scurvecsv.open ("scurvetemp.csv");


	SystemController mysyscontroller;
	std::cout << "\nInitHW";
	mysyscontroller.InitializeHw( cHWFile );
	std::cout << "\nMPAI";
        MPAInterface* fMPAInterface = mysyscontroller.fMPAInterface;
	std::cout << "\nBOARD"<<std::endl;
	MPA* mpa1 = new MPA(0, 0, 0, 0,"settings/MPAFiles/MPA_default.txt");

	mpa1->loadfRegMap("settings/MPAFiles/MPA_default.txt");
	BeBoard* pBoard = mysyscontroller.fBoardVector.at( 0 );

	OuterTrackerModule* MPAM = new OuterTrackerModule();
	MPAM->addMPA(mpa1);
	pBoard->addModule(MPAM);

	std::chrono::milliseconds LongPOWait( 500 );
	std::chrono::milliseconds ShortWait( 10 );

	fMPAInterface->PS_Clear_counters();
	fMPAInterface->PS_Clear_counters();
	//fMPAInterface->activate_I2C_chip();

	std::pair<uint32_t, uint32_t> rows = {0,17};
	std::pair<uint32_t, uint32_t> cols = {0,120};
	//std::pair<uint32_t, uint32_t> rows = {5,7};
	//std::pair<uint32_t, uint32_t> cols = {1,5};

 	std::vector<TH1F*> scurves;
	std::string title;
	std::cout <<"Setup"<< std::endl;

	fMPAInterface->Set_threshold(mpa1,100);
	fMPAInterface->Activate_sync(mpa1);
	fMPAInterface->Activate_pp(mpa1);
	fMPAInterface->Set_calibration(mpa1,100);
	Stubs curstub;
	uint32_t npixtot = 0;
    //mysyscontroller.fMPAInterface->Start ( pBoard );
	for(size_t row=rows.first; row<rows.second; row++)
		{
		for(size_t col=cols.first; col<cols.second; col++)
			{
				std::cout <<row<<","<<col<<std::endl;
				fMPAInterface->Disable_pixel(mpa1,0,0);
				std::this_thread::sleep_for( ShortWait );
				fMPAInterface->Enable_pix_BRcal(mpa1,row, col, "rise", "edge");
				std::this_thread::sleep_for( ShortWait );
				fMPAInterface->Send_pulses(1,8);
				std::this_thread::sleep_for( ShortWait );
                //fMPAInterface->ReadData ( pBoard );
                const std::vector<Event*>& events = mysyscontroller.GetEvents ( pBoard );

                for ( __attribute__((unused)) auto& ev : events )
                {
				std::cout<<"tst"<<std::endl;
				}

				npixtot+=1;
			}
		}
    //mysyscontroller.fMPAInterface->Stop ( pBoard );

	std::cout <<"Numpix -- "<< npixtot <<std::endl;


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


	std::this_thread::sleep_for( LongPOWait );

}//int main
