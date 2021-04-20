#include "../Utils/ConsoleColor.h"
#include "SlowControlMonitorInterface.h"
#include "Utils/easylogging++.h"

#include <chrono>
#include <signal.h>
#include <thread>

INITIALIZE_EASYLOGGINGPP

volatile sig_atomic_t stop = 0;

void controlC_handler(int signum) { stop = 1; }

int main(int argc, char** argv)
{
    signal(SIGINT, controlC_handler);

    std::string loggerConfigFile;
    if(std::getenv("PH2ACF_BASE_DIR") == nullptr) throw std::runtime_error("No PH2ACF_BASE_DIR environment variables have been set. You need to source some settings file!");

    loggerConfigFile = std::string(std::getenv("PH2ACF_BASE_DIR")) + "/settings/logger.conf";
    el::Configurations conf(loggerConfigFile);
    el::Loggers::reconfigureAllLoggers(conf);

    SlowControlMonitorInterface theSlowControlMonitorInterface("127.0.0.1", 7000);

    while(!stop)
    {
        theSlowControlMonitorInterface.readDeviceStatus();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
