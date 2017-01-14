
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
		fMPAInterface->ConfigureMPA(&confsR[i], 1 , i+1, 0);
		fMPAInterface->ConfigureMPA(&confsL[i], 1 , i+1, 1);



	}

	fMPAInterface->SendConfig(6,6);
	std::chrono::milliseconds cWait1( 100 );//

	int ibuffer = 1;
	bool Kill=false;



	//fMPAInterface->SequencerInit(1,500000,1,0);
	fMPAInterface->TestbeamInit(500000,0, 0);

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

	tempspill=spill;
	spill+=fMPAInterface->WaitTestbeam();
	if (tempspill!=spill) std::cout<<"Starting Spill "<<spill<<std::endl;

	fMPAInterface->Cleardata();
	fMPAInterface->ReadTrig(ibuffer);



	for(int i=0;i<=5;i++)
	{

		for(int j=0;j<=1;j++)
		{
			std::pair<std::vector<uint32_t>, std::vector<uint32_t>>  returndata = fMPAInterface->ReadMPAData(ibuffer,i+1,j);
		}
	}

	ibuffer+=1;
	if (ibuffer >4) ibuffer=1 ;

        //uint32_t PacketSize = mysyscontroller.fBeBoardInterface->ReadData ( pBoard, false );
	

	nev+=1;
	if (nev%100==0)	std::cout<<nev<<" Events"<<std::endl;
	

        std::vector<uint32_t>*  curdata = fMPAInterface->GetcurData();
	int iic1 = 0;
        for( int iic1=0;iic1<curdata->size();iic1++) 
		{
		std::cout<<iic1<<" "<< curdata->at(iic1) <<std::endl;
                }

	
	}

}

