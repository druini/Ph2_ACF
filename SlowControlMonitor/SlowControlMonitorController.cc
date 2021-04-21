#include "../Utils/ConsoleColor.h"
#include "SlowControlMonitorInterface.h"

#include <chrono>
#include <signal.h>
#include <thread>

volatile sig_atomic_t stop = 0;

void controlC_handler(int signum) { stop = 1; }

int main(int argc, char** argv)
{
    signal(SIGINT, controlC_handler);

    SlowControlMonitorInterface theSlowControlMonitorInterface("127.0.0.1", 7000);

    while(!stop)
    {
        theSlowControlMonitorInterface.readDeviceStatus();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return EXIT_SUCCESS;
}
