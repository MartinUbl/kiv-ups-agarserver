#ifndef AGAR_LOG_H
#define AGAR_LOG_H

#include "Singleton.h"

/* Logging class singleton */
class Log
{
    friend class Singleton<Log>;
    public:
        ~Log();

        /* Logs string with INFO severity */
        void Info(const char *str, ...);
        /* Logs string with ERROR severity */
        void Error(const char *str, ...);
        /* Logs string with DEBUG severity */
        void Debug(const char *str, ...);

    protected:
        /* Hidden singleton constructor */
        Log();

    private:
        //
};

#define sLog Singleton<Log>::getInstance()

#endif
