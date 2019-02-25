#include "DQMHistogramController.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 40000        // The destination port of the datastream

//MAIN
int main(int argc, char **argv)
{
	DQMHistogramController theDQMHistogramController(PORT);

	while(1) //listen to port BEAGLEBONE_PORT
	{
		theDQMHistogramController.accept(1, 0);
	}

	return 0;
}
