#include "MiddlewareController.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 5000        // The destination port of the datastream

//MAIN
int main(int argc, char **argv)
{
	MiddlewareController theMiddlewareController(PORT);

	while(1) //listen to port BEAGLEBONE_PORT
	{
		theMiddlewareController.accept(1, 0);
	}

	return 0;
}