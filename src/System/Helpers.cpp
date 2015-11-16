#include "General.h"
#include "Helpers.h"

bool IsValidUsername(const char* username)
{
    char ch;
    int i;

    for (i = 0; username[i] != '\0'; i++)
    {
        ch = username[i];
        if (   (ch >= '0' && ch <= '9')
            || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
            || ch == '.' || ch == '-' || ch == '_' || ch == '+')
            continue;

        return false;
    }

    return true;
}
