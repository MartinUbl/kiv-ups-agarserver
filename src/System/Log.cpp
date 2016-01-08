#include "General.h"
#include "Log.h"
#include "Config.h"

#include <iostream>
#include <cstdarg>

Log::Log()
{
    m_logFile = nullptr;

    std::string logFileName = sConfig->GetStringValue(CONF_LOG_FILE);
    if (logFileName.length() == 0)
    {
        std::cout << "No log file specified, server messages won't be logged into file" << std::endl;
    }
    else
    {
        m_logFile = fopen(logFileName.c_str(), "w");
        if (!m_logFile)
            std::cerr << "Could not open log file " << logFileName.c_str() << " for writing! Server log will not be put into file!" << std::endl;
    }
}

Log::~Log()
{
    if (m_logFile)
        fclose(m_logFile);
}

void Log::FileLog(const char* str)
{
    if (m_logFile)
    {
        fputs(str, m_logFile);
        fputc('\n', m_logFile);
    }
}

void Log::Info(const char *str, ...)
{
    va_list argList;
    va_start(argList, str);
    char buf[2048];
    vsnprintf(buf, 2048, str, argList);
    va_end(argList);
    std::cout << buf << std::endl;

    FileLog(buf);
}

void Log::Error(const char *str, ...)
{
    va_list argList;
    va_start(argList, str);
    char buf[2048];
    vsnprintf(buf, 2048, str, argList);
    va_end(argList);
    std::cerr << buf << std::endl;

    FileLog(buf);
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

    FileLog(buf);
}
