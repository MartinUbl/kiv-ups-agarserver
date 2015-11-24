#ifndef AGAR_HELPERS_H
#define AGAR_HELPERS_H

bool IsValidUsername(const char* username);

// method for retrieving time in milliseconds to determine difference
#ifdef _WIN32
#include <Windows.h>
inline uint32_t getMSTime() { return GetTickCount(); }
#else
#include <sys/time.h>
inline uint32_t getMSTime()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

// retrieves time difference in milliseconds
inline uint32_t getMSTimeDiff(uint32_t oldMSTime, uint32_t newMSTime)
{
    // uint32_t have limited data range and this is case when it overflow in this tick
    if (oldMSTime > newMSTime)
        return (0xFFFFFFFF - oldMSTime) + newMSTime;
    else
        return newMSTime - oldMSTime;
}

#endif
