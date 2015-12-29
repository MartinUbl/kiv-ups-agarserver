#include "General.h"
#include "Application.h"
#include "Config.h"
#include "Network.h"
#include "Storage.h"
#include "Log.h"
#include "Gameplay.h"
#include "Helpers.h"

#include <thread>

void applicationUpdateWorker();

Application::Application()
{
    m_lastUpdate = getMSTime();
}

Application::~Application()
{
    //
}

bool Application::Init(int argc, char** argv)
{
    sLog->Info("Agar.io game remake server emulator - KIV/UPS - semestral work project");
    sLog->Info("Made by Kennny (c) 2015\n");

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

    m_updateThread = new std::thread(applicationUpdateWorker);

    sLog->Info("Initialization sequence complete!\n");

    return true;
}

int Application::Run()
{
    // Main application loop
    while (true)
    {
        sNetwork->Update();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

int Application::Update()
{
    uint32_t lastUpdate = m_lastUpdate;

    sGameplay->Update(getMSTimeDiff(m_lastUpdate, getMSTime()));

    m_lastUpdate = getMSTime();

    return getMSTimeDiff(lastUpdate, m_lastUpdate);
}

void applicationUpdateWorker()
{
    int delay;

    while (true)
    {
        delay = sApplication->Update();

        if (delay > 100)
            delay = 100;

        std::this_thread::sleep_for(std::chrono::milliseconds(100 - delay + 1));
    }
}
