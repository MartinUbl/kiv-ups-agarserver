#include "General.h"
#include "Application.h"
#include "Config.h"
#include "Network.h"
#include "Storage.h"
#include "Log.h"
#include "Gameplay.h"
#include "Helpers.h"

#include <signal.h>
#include <thread>

Application::Application()
{
    //
}

Application::~Application()
{
    //
}

void sigIntHandler(int s)
{
    sNetwork->Shutdown();
    sGameplay->Shutdown();

    sApplication->PrintStats();

    exit(1);
}

bool Application::Init(int argc, char** argv)
{
    sLog->Info("Agar.io game remake server emulator - KIV/UPS - semestral work project");
    sLog->Info("Made by Kennny (c) 2015\n");

    // catch sigint
    signal(SIGINT, sigIntHandler);
#ifndef _WIN32
    // ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

    // init PRNG
    srand((unsigned int)time(nullptr));

    // Initialize all modules needed for application runtime

    // TODO: use cli args to override several config options
    if (!sConfig->Load())
        return false;

    if (!sStorage->Init())
        return false;

    if (!sNetwork->Startup())
        return false;

    sGameplay->Init();

    sLog->Info("Initialization sequence complete!\n");

    return true;
}

void Application::PrintStats()
{
    sLog->Info("Server received packets: %llu", sNetwork->GetRecvPacketsCount());
    sLog->Info("Server sent packets: %llu", sNetwork->GetSentPacketsCount());
    sLog->Info("Server received bytes: %llu B", sNetwork->GetRecvBytesCount());
    sLog->Info("Server sent bytes: %llu B", sNetwork->GetSentBytesCount());
}

int Application::Run()
{
    std::string input;

    while (true)
    {
        std::cout << "> ";
        std::cin >> input;

        // shut server down
        if (input == "exit")
        {
            sNetwork->Shutdown();
            sGameplay->Shutdown();

            PrintStats();

            break;
        }
        else if (input == "stats")
        {
            PrintStats();
        }

        std::cout << std::endl;
    }

    return 0;
}
