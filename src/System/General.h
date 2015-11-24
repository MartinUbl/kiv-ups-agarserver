#ifndef AGAR_GENERAL_H
#define AGAR_GENERAL_H

#include <iostream>
#include <cstdint>
#include <cstring>
#include <cstdio>
#ifdef _WIN32
/* it is critical to include winsock2.h before Windows.h due to Windows.h attempt to include old winsock.h header */
#include <winsock2.h>
#include <Windows.h>
#endif

/* several paths differs when in debug mode */
#ifdef _DEBUG
#define BASE_PATH "../"
#define CONFIG_PATH BASE_PATH "config/"
#else
#define BASE_PATH "./"
#define CONFIG_PATH BASE_PATH
#endif

#endif
