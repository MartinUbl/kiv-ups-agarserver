#include "General.h"
#include "Log.h"
#include "Config.h"

#include <iostream>
#include <cstdarg>

Log::Log()
{
    //
}

Log::~Log()
{
    //
}

void Log::Info(const char *str, ...)
{
    va_list argList;
    va_start(argList, str);
    char buf[2048];
    vsnprintf(buf, 2048, str, argList);
    va_end(argList);
    std::cout << buf << std::endl;
}

void Log::Error(const char *str, ...)
{
    va_list argList;
    va_start(argList, str);
    char buf[2048];
    vsnprintf(buf, 2048, str, argList);
    va_end(argList);
    std::cerr << buf << std::endl;
}

void Log::Debug(const char *str, ...)
{
    if (!sConfig->GetIntValue(CONF_DEBUG_LOG))
        return;

    va_list argList;
    va_start(argList, str);
    char buf[2048];
    vsnprintf(buf, 2048, str, argList);
    va_end(argList);
    std::cout << buf << std::endl;
}
