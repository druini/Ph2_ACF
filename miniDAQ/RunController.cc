#include "MiddlewareController.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 5000 // The server listening port

int main (int argc, char **argv)
{
  std::string loggerConfigFile = std::getenv("BASE_DIR");
  loggerConfigFile += "/settings/logger.conf";
  el::Configurations conf (loggerConfigFile);
  el::Loggers::reconfigureAllLoggers (conf);

  MiddlewareController theMiddlewareController(PORT);

  while(1){}

  return EXIT_SUCCESS;
}
