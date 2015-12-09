#ifndef AGAR_APPLICATION_H
#define AGAR_APPLICATION_H

#include "Singleton.h"

/* Main application singleton class */
class Application
{
    friend class Singleton<Application>;
    public:
        ~Application();

        /* Initializes application using commandline arguments */
        bool Init(int argc, char** argv);
        /* Main run method called from main() entry point */
        int Run();

        /* Updates all available entities */
        int Update();

    protected:
        /* Hidden singleton constructor */
        Application();

    private:
        /* timestamp of last update */
        uint32_t m_lastUpdate;

        /* world update thread */
        std::thread *m_updateThread;
};

#define sApplication Singleton<Application>::getInstance()

#endif
