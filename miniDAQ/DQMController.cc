#include "DQMHistogramController.h"
#include "../Utils/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

//MAIN
int main(int argc, char **argv)
{
	DQMHistogramController theDQMHistogramController("192.168.1.100",6000);
	theDQMHistogramController.readMessage();

	return 0;
}
