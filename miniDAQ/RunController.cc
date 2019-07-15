#include "MiddlewareController.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 5000 // The server listening port

//MAIN
int main(int argc, char **argv)
{
    //configure the logger
    el::Configurations conf ("settings/logger.conf");
    el::Loggers::reconfigureAllLoggers (conf);

	MiddlewareController theMiddlewareController(PORT);

	while(1){}

	return EXIT_SUCCESS;
}
