#include "MiddlewareController.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 5000 // The server listening port

//MAIN
int main(int argc, char **argv)
{
	MiddlewareController theMiddlewareController(PORT);

	while(1){}

	return EXIT_SUCCESS;
}
