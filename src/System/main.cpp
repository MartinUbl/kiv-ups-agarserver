#include "General.h"
#include "Application.h"

/* Application entry point */
int main(int argc, char** argv)
{
    // initialize
    if (!sApplication->Init(argc, argv))
        return 1;

    // and run!
    return sApplication->Run();
}
