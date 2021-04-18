#include "PowerSupplyInterface.h"
#include "Utils/easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#define PORT 7000 // The server listening port

int main(int argc, char** argv)
{
    std::string loggerConfigFile;
    if(std::getenv("PH2ACF_BASE_DIR") == nullptr) throw std::runtime_error("No PH2ACF_BASE_DIR environment variables have been set. You need to source some settings file!");

    loggerConfigFile = std::string(std::getenv("PH2ACF_BASE_DIR")) + "/settings/logger.conf";
    el::Configurations conf(loggerConfigFile);
    el::Loggers::reconfigureAllLoggers(conf);

    PowerSupplyInterface thePowerSupplyInterface(PORT);
    thePowerSupplyInterface.startAccept();

    while(true) { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); }

    return EXIT_SUCCESS;
}
