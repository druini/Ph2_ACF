
#include <cstring>
#include <fstream>
#include "../Utils/Utilities.h"
#include "../HWDescription/Cbc.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/MPAInterface.h"
#include "../HWInterface/MPAGlibFWInterface.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../HWDescription/FrontEndDescription.h"
//#include "../tools/Calibration.h"
#include "../Utils/Timer.h"
//#include <TApplication.h>
#include <inttypes.h>
#include "../Utils/argvparser.h"
#include "../Utils/ConsoleColor.h"
#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Tracker/TrackerEvent.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

using namespace std;
INITIALIZE_EASYLOGGINGPP

int main( int argc, char* argv[] )
{

	ArgvParser cmd;

	cmd.defineOption( "file", "Hw Description File . Default value: settings/HWDescription_MAPSA.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "file", "f" );

	cmd.defineOption( "output", "Output Directory . Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative( "output", "o" );

	int result = cmd.parse( argc, argv );
	if ( result != ArgvParser::NoParserError )
	{
		std::cout << cmd.parseErrorDescription( result );
		exit( 1 );
	}

	std::string cHWFile = ( cmd.foundOption( "file" ) ) ? cmd.optionValue( "file" ) : "settings/HWDescription_MAPSA.xml";
	std::string cDirectory = ( cmd.foundOption( "output" ) ) ? cmd.optionValue( "output" ) : "Results/";

	SystemController mysyscontroller;
	std::cout << "\nInitHW";
	mysyscontroller.InitializeHw( cHWFile );
	std::cout << "\nMPAI";
        MPAInterface* fMPAInterface = mysyscontroller.fMPAInterface; 
	std::cout << "\nBOARD";
	BeBoard* pBoard = mysyscontroller.fBoardVector.at( 0 );
	
        std::vector < MPA* > fMPAVector;

 


	uint8_t pBeId = 0;
	uint8_t pFMCId = 0;
	uint8_t pFeId = 0;
	uint8_t pMPAId = 1;
	Module* MAPSA = new Module( pBeId, pFMCId, pFeId, 0 ); 

 	//MPA *mpa1 = new MPA(pBeId, pFMCId, pFeId, pMPAId) ;
	//MAPSA->addMPA(mpa1);

	for (int i=0;i<12;i++)
		MAPSA->addMPA(new MPA(pBeId, pFMCId, pFeId, i));

	//std::cout << "\n"<<MAPSA->getNMPA();
	pBoard->addModule(MAPSA);

	



	std::cout << "\nExecuting POWER ON...";
	mysyscontroller.fBeBoardInterface->PowerOn(pBoard);
	std::cout << "\nFirmware version: "; 
	mysyscontroller.fBeBoardInterface->ReadVer(pBoard);
	std::chrono::milliseconds cWait( 10 );
	std::vector<std::vector< uint32_t >> confsL;
	std::vector<std::vector< uint32_t >> confsR;

	for(int i=0;i<=5;i++)
	{

		std::this_thread::sleep_for( cWait );

		confsR.push_back(fMPAInterface->ReadConfig("calibratedRight", i+1, 1));
		confsL.push_back(fMPAInterface->ReadConfig("calibratedLeft", i+1, 1));
		std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod1({"OM","THDAC"},{3,50});

		fMPAInterface->ModifyPerif(mod1,&confsR[i]);
		fMPAInterface->ModifyPerif(mod1,&confsL[i]);
		/*
		for (int j=1;j<=24;j++)
			{
			std::pair < std::vector< std::string > ,std::vector< uint32_t >> mod2({"PML"},{1});
			fMPAInterface->ModifyPix(mod2,&confsR[i],j);
			fMPAInterface->ModifyPix(mod2,&confsL[i],j);
			}
		*/
		fMPAInterface->ConfigureMPA(&confsR[i], 1 , i+1, 0);
		fMPAInterface->ConfigureMPA(&confsL[i], 1 , i+1, 1);



	}

	fMPAInterface->SendConfig(6,6);
	std::chrono::milliseconds cWait1( 100 );//

	int ibuffer = 1;
	bool Kill=false;



	fMPAInterface->SequencerInit(1,500000,1,0);
	//fMPAInterface->TestbeamInit(500000,0, 0);

	std::cout<<"Clearing buffers"<<std::endl;
	for(int i=0;i<=1;i++)
		{
		for(int k=1;k<=4;k++)
			{
			for(int j=1;j<=6;j++)
				{
					fMPAInterface->HeaderInitMPA(j,i);
					std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(k,j,i);
				}
			fMPAInterface->ReadTrig(k);
			}
	}	fMPAInterface->Cleardata();

	int spill = 0;
	int tempspill = 0;
	int nev = 0;
	while (not Kill)
	{

        //string s;
        //getline(cin, s) ;
	//std::cout<<s<<std:::endl;
        //if ( s.length() == 0 )
          //  break ;
	tempspill=spill;
	spill+=fMPAInterface->WaitTestbeam();
	if (tempspill!=spill) std::cout<<"Starting Spill "<<spill<<std::endl;

	fMPAInterface->Cleardata();
	fMPAInterface->ReadTrig(ibuffer);



	for(int i=0;i<=5;i++)
	{

		//fMPAInterface->ModifyPerif(mod1,&confsR[i]);
		//fMPAInterface->ModifyPerif(mod1,&confsL[i]);

		for(int j=0;j<=1;j++)
		{
			std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(ibuffer,i+1,j);
			/*std::pair<std::vector<uint32_t>, std::vector<std::string>> formdata = fMPAInterface->FormatData(returndata);

			std::vector<uint32_t> counts = formdata.first;
			
			int totalcount = 0;
			for (int j=0;j<counts.size();j++)	totalcount+=counts[j];
			for (int j=0;j<counts.size();j++) std::cout<<"counts: "<<counts[j]<<std::endl;;
			

			
			std::pair<std::vector<uint32_t>, std::vector<uint64_t>> readmem = fMPAInterface->ReadMemory(formdata.second, 3);
			
			std::vector<uint32_t> bx = readmem.first;
			std::vector<uint64_t> pixmem = readmem.second;
			
			for (int j=0;j<pixmem.size();j++)
			{
				if (j>2 or ibuffer!=1 or i!=0) break;
				std::cout<<"BX: "<<bx[j]<<std::endl;
		    		std::bitset<48> p(pixmem[j]);
				std::cout<<"PIX: "<<p.to_string()<<std::endl;
			}*/
			
			if (j==0) fMPAInterface->ConfigureMPA(&confsR[i], 1 , i+1, j);
			if (j==1) fMPAInterface->ConfigureMPA(&confsL[i], 1 , i+1, j);
		}
	}

	fMPAInterface->SendConfig(6,6);
	//std::chrono::milliseconds cWait1( 100 );//
	ibuffer+=1;
	if (ibuffer >4) ibuffer=1 ;




        uint32_t PacketSize = mysyscontroller.fBeBoardInterface->ReadData ( pBoard, false );
	

	nev+=1;
	if (nev%100==0)	std::cout<<nev<<" Events"<<std::endl;
	/*
        const std::vector<Event*>* pEvents ;
        pEvents = &mysyscontroller.GetEvents ( pBoard );


	
        uint32_t total_trigs = (pEvents->at(0))->Gettotal_trigs();
        uint32_t trigger_total_counter = (pEvents->at(0))->Gettrigger_total_counter();
        uint32_t trigger_counter = (pEvents->at(0))->Gettrigger_counter() ;
	std::vector<uint32_t> trigger_offset_BEAM = (pEvents->at(0))->Gettrigger_offset_BEAM() ;
	std::vector<uint32_t> trigger_offset_MPA =(pEvents->at(0))->Gettrigger_offset_MPA() ;
	
	std::cout<<"total triggers"<<std::endl;	
 	std::cout<<total_trigs<<std::endl;	
	std::cout<<"trigger counter"<<std::endl;
	std::cout<<trigger_counter<<std::endl;
	std::cout<<"trigger total counter"<<std::endl;
        std::cout<<trigger_total_counter<<std::endl;
	std::cout<<"trigger_offset_BEAM"<<std::endl;
	int iic1 = 0;
        for( auto &vv : trigger_offset_BEAM) 
		{
		if (iic1>50) break;
		std::bitset<32> p(vv);
		std::cout<<iic1++<<"  "<<p.to_string()<<std::endl;
                }

	std::cout<<"trigger_offset_MPA"<<std::endl;
	
	int iic2 = 0;
        for( auto &vv : trigger_offset_MPA) 
		{

		std::bitset<32> p(vv);
		std::cout<<iic2++<<"  "<<p.to_string()<<std::endl;
                }


	if (ibuffer!=1) continue;
	const EventDataMap &EDM =(pEvents->at(0))->GetEventDataMap();
	for (auto &ev: EDM)
		{
			if (ev.first!= 1) continue;
			std::cout<<ev.first<<std::endl;

			int iic = 0;
                        for( auto &vv : ev.second) {
			      if (iic >75) continue;
		    	      std::bitset<32> p(vv);

                              std::cout<<iic++<<"  "<<p.to_string()<<std::endl;
                        }
		}
	
	std::cout<<PacketSize<<std::endl;
	*/

	
	}

}

