#ifndef CB_PLATFORM_H
#define CB_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
    #define CB_WINDOWS
#elif defined(__linux__)
    #define CB_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
    #define CB_MAC
#elif defined(__ANDROID__)
   #define CB_ANDROID
#elif defined(__EMSCRIPTEN__)
    #define CB_EMSCRIPTEN
#else
    #define CB_UNKNOWN
#endif

#endif /* CB_PLATFORM_H */