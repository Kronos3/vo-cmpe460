//#include <cfg/vo_car.h>

#include <Logger.hpp>
#include <csignal>
#include "Components.hpp"

volatile sig_atomic_t terminate = 0;

static void sighandler(int signum)
{
    (void) signum;

    Fw::Logger::logMsg("Exiting tasks\n");
    fsw_exit();

    terminate = 1;
}

I32 main()
{
    // register signal handlers to exit program
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);

    Fw::Logger::logMsg("Booting up\n");

    fsw_start();
    fsw_run();

    Fw::Logger::logMsg("Shutting down\n");
    return 0;
}
