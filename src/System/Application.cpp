#include "General.h"
#include "Application.h"
#include "Config.h"
#include "Network.h"
#include "Storage.h"
#include "Log.h"

#include <thread>

Application::Application()
{
    //
}

Application::~Application()
{
    //
}

bool Application::Init(int argc, char** argv)
{
    sLog->Info("Agar.io game remake server emulator - KIV/UPS - semestral work project");
    sLog->Info("Made by Kennny (c) 2015\n");

    // Initialize all modules needed for application runtime

    // TODO: use cli args to override several config options
    if (!sConfig->Load())
        return false;

    if (!sStorage->Init())
        return false;

    if (!sNetwork->Startup())
        return false;

    sLog->Info("Initialization sequence complete!\n");

    return true;
}

int Application::Run()
{
    // Main application loop
    while (true)
    {
        sNetwork->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}
