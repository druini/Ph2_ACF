#include "MiddlewareController.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 5000 // The server listening port

int main(int argc, char** argv)
{
    std::string loggerConfigFile;
    if(std::getenv("PH2ACF_BASE_DIR") == nullptr && std::getenv("PH2ACF_ROOT") == nullptr)
        throw std::runtime_error("No PH2ACF_BASE_DIR or PH2ACF_ROOT environment variables have been set. You need to source some settings file!");
    else if(std::getenv("PH2ACF_ROOT") == nullptr)
        loggerConfigFile = std::getenv("PH2ACF_BASE_DIR");
    else
        loggerConfigFile = std::getenv("PH2ACF_ROOT");

    loggerConfigFile += "/settings/logger.conf";
    el::Configurations conf(loggerConfigFile);
    el::Loggers::reconfigureAllLoggers(conf);

    MiddlewareController theMiddlewareController(PORT);
    theMiddlewareController.startAccept();

    while(true) { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); }

    return EXIT_SUCCESS;
}
