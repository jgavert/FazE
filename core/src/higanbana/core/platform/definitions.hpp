#pragma once

#ifndef HIGANBANA_DEFINED
  #define HIGANBANA_DEFINED
  #if defined(WIN32) || defined(_WIN32)
    #define HIGANBANA_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define _UNICODE
    #define UNICODE
    #undef WINVER
    #undef _WIN32_WINNT
    #define WINVER 0x0A00
    #define _WIN32_WINNT 0x0A00
    #ifndef NOMINMAX
      #define NOMINMAX
    #endif
    #define _CONSOLE
    //#define _CRT_SECURE_NO_WARNINGS
    //#define _WINSOCK_DEPRECATED_NO_WARNINGS
  #else
    #define HIGANBANA_PLATFORM_LINUX
  #endif
#endif