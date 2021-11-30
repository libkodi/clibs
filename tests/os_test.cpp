#include <iostream>
#include "clibs/os.h"

#if defined(OS_WIN64)
#define OS "win64"
#elif defined(OS_WIN32)
#define OS "win32"
#elif defined(OS_LINUX)
#define OS "linux"
#elif defined(OS_UNIX)
#define OS "unix"
#elif defined(OS_APPLE)
#define OS "apple"
#elif defined(OS_ANDROID)
#define OS "android"
#else
#define OS "unknown"
#endif

int main(int argc, char const *argv[])
{
    std::cout << "Current OS: " << OS << std::endl;
    return 0;
}