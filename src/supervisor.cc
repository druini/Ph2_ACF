#include <cstring>
#include "../HWDescription/Chip.h"
#include "../HWDescription/Module.h"
#include "../HWDescription/BeBoard.h"
#include "../HWInterface/ChipInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/argvparser.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "../Utils/MiddlewareInterface.h"


using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;

INITIALIZE_EASYLOGGINGPP

static bool controlC = false;

void interruptHandler(int handler)
{
	std::cout << __PRETTY_FUNCTION__ << " Sig handler: " << handler << std::endl;
	controlC = true;
}

bool checkExitStatus(int status, std::string programName)
{
	if (WIFEXITED(status) && !WEXITSTATUS(status))
	{
		std::cout << programName << " executed successfull." << std::endl;
		return true;
	}
	else if (WIFEXITED(status) && WEXITSTATUS(status))
	{
		if (WEXITSTATUS(status) == 127)
		{
			// execv failed
			std::cout << programName << " execv failed." << std::endl;
			return false;
		}
		else
		{
			std::cout << programName << " terminated normally, but returned a non-zero status." << std::endl;
			return true;
		}
	}
	else
	{
		std::cout << programName << " didn't terminate normally." << std::endl;
		return false;
	}
}

int main ( int argc, char* argv[] )
{

	if(getenv("BASE_DIR") == nullptr)
	{
		std::cout << "You must source setup.sh or export the BASE_DIR environmental variable. Exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	std::string baseDir = std::string(getenv("BASE_DIR")) + "/";
	std::string binDir  = baseDir + "bin/";


	//configure the logger
	el::Configurations conf (baseDir + "settings/logger.conf");
	el::Loggers::reconfigureAllLoggers (conf);

	ArgvParser cmd;

	// init
	cmd.setIntroductoryDescription ( "CMS Ph2_ACF  calibration routine using K. Uchida's algorithm or a fast algorithm" );
	// error codes
	cmd.addErrorCode ( 0, "Success" );
	cmd.addErrorCode ( 1, "Error" );
	// options
	cmd.setHelpOption ( "h", "help", "Print this help page" );

	cmd.defineOption ( "file", "Hw Description File", ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired );
	cmd.defineOptionAlternative ( "file", "f" );

	cmd.defineOption ( "calibration", "Calibration to run", ArgvParser::OptionRequiresValue | ArgvParser::OptionRequired );
	cmd.defineOptionAlternative ( "calibration", "c" );

	cmd.defineOption ( "output", "Output Directory. Default value: Results", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/ );
	cmd.defineOptionAlternative ( "output", "o" );

	cmd.defineOption ( "allChan", "Do calibration using all channels? Default: false", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative ( "allChan", "a" );

	cmd.defineOption ( "batch", "Run the application in batch mode", ArgvParser::NoOptionAttribute );
	cmd.defineOptionAlternative ( "batch", "b" );


	int result = cmd.parse ( argc, argv );

	if ( result != ArgvParser::NoParserError )
	{
		LOG (INFO) << cmd.parseErrorDescription ( result );
		exit ( 1 );
	}

	// now query the parsing results
	std::string cHWFile    = cmd.optionValue ( "file" );
	std::string cDirectory = ( cmd.foundOption ( "output" ) ) ? cmd.optionValue ( "output" ) : "Results/";
	cDirectory += cmd.optionValue ( "calibration" );

	bool cAllChan   = (cmd.foundOption ("allChan")) ? true : false;
	bool batchMode  = (cmd.foundOption ("batch")  ) ? true : false;



	pid_t  runControllerPid = -1;
	pid_t  dqmControllerPid = 10;
	int ret = 1;
	int runControllerStatus;
	int dqmControllerStatus;

	std::cout << "forking run" << std::endl;
	runControllerPid   = fork();
	if (runControllerPid == -1)// pid == -1 means error occured
	{
		LOG (ERROR) << "Can't fork RunController, error occured";
		exit(EXIT_FAILURE);
	}
	else if (runControllerPid == 0)// pid == 0 means child process created
	{
		// getpid() returns process id of calling process
		//printf("Child runControllerPid, pid = %u\n",getpid());

		// the argv list first argument should point to
		// filename associated with file being executed
		// the array pointer must be terminated by NULL
		// pointer
		char * argv[] = {"RunController", NULL};

		// the execv() only return if error occured.
		// The return value is -1
		execv((binDir + "RunController").c_str(), argv);
		LOG (ERROR) << "Can't run RunController, error occured";
		exit(0);
	}

	std::cout << "forking dqm" << std::endl;
	dqmControllerPid = fork();
	if (dqmControllerPid == -1)// pid == -1 means error occured
	{
		LOG (ERROR) << "Can't fork DQMHistogrammer, error occured";
		exit(EXIT_FAILURE);
	}

	else if (dqmControllerPid == 0)// pid == 0 means child process created
	{
		char * argv[] = {"DQMController", NULL};
		execv((binDir + "DQMController").c_str(),NULL);
		LOG (ERROR) << "Can't run DQMController, error occured";
		exit(EXIT_FAILURE);
	}

	// a positive number is returned for the pid of
	// parent process
	// getppid() returns process id of parent of
	// calling process
	printf("Parent process, pid = %u\n",getppid());

	struct sigaction act;
    act.sa_handler = interruptHandler;
    sigaction(SIGINT, &act, NULL);

    int interfaceStatus = 0;
	MiddlewareInterface theInterface("127.0.0.1",5000);
	theInterface.initialize();
	interfaceStatus = 1;

    int runControllerPidStatus = 0;
	int dqmControllerPidStatus = 0;
	bool done = false;
	while(!done)
    {
		if(runControllerPidStatus == 0 && (runControllerPidStatus = waitpid(runControllerPid, &runControllerStatus, WNOHANG)) != 0)
		{
			if(!checkExitStatus(runControllerStatus,"RunController"))
			{
	    		kill(dqmControllerPid,SIGKILL);
	    		exit(EXIT_FAILURE);
			}

		}
		if(dqmControllerPidStatus == 0 && (dqmControllerPidStatus = waitpid(dqmControllerPid, &dqmControllerStatus, WNOHANG)) != 0)
		{
			if(!checkExitStatus(dqmControllerStatus,"DQMController"))
			{
	    		kill(runControllerPid,SIGKILL);
	    		exit(EXIT_FAILURE);
			}

		}
    	if(controlC)
    	{
    		kill(runControllerPid,SIGKILL);
    		kill(dqmControllerPid,SIGKILL);
    		exit(EXIT_FAILURE);
    	}
    	if(runControllerPidStatus != 0 && dqmControllerPidStatus != 0)
    		done = true;
    	else
    	{
    		std::cout << "Sending Configure!!!" << std::endl;
    		switch(interfaceStatus)
    		{
    		case 1:
				theInterface.configure();
				interfaceStatus = 2;
				break;
			case 2:
				theInterface.start("5");
				interfaceStatus = 3;
				break;
			case 3:
				theInterface.stop();
				interfaceStatus = 4;
				break;
			case 4:
				done = true;
				break;
    		}
			std::cout << "SLEEPING!!!" << std::endl;
    		if(!done) usleep(1000000);
    	}

	}
	checkExitStatus(runControllerStatus,"RunController");
	checkExitStatus(dqmControllerStatus,"DQMController");

	return EXIT_SUCCESS;
}
