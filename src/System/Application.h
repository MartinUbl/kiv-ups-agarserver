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

        /* Prints server statistics */
        void PrintStats();

        /* Prints available commands */
        void PrintAvailableCommands();

    protected:
        /* Hidden singleton constructor */
        Application();

    private:
        //
};

#define sApplication Singleton<Application>::getInstance()

#endif
