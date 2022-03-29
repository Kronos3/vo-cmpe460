//#include <cfg/vo_car.h>

#include <Logger.hpp>
#include <csignal>
#include "Components.hpp"

Kernel* kernel_main = nullptr;

static void sighandler(int signum)
{
    (void) signum;
    FW_ASSERT(kernel_main);

    Fw::Logger::logMsg("Exiting tasks\n");
    kernel_main->exit();
}

I32 main()
{
    Kernel kernel;
    kernel_main = &kernel;

    // register signal handlers to exit program
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    Fw::Logger::logMsg("Booting up\n");

    kernel.start();

    // Run all functions until a shutdown is invoked
    kernel.run();

    Fw::Logger::logMsg("Shutting down\n");
    return 0;
}
