#pragma once

#include "defines.h"

void log(int32 color, const char* prefix, const char* format, ...);

#define LOG_ERROR(format, ...) log(196, "(!)   [ERROR]", format __VA_OPT__(,) __VA_ARGS__)

#ifdef DEBUG


  #define LOG_WARNING(format, ...) log(220, "(?) [WARNING]", format __VA_OPT__(,) __VA_ARGS__)
  #define LOG_INFO(format, ...) log(32, "(*)    [INFO]", format __VA_OPT__(,) __VA_ARGS__)
  #define LOG_VERBOSE(format, ...) log(126, "(#) [VERBOSE]", format __VA_OPT__(,) __VA_ARGS__)

#else

  #define LOG_WARNING(format, ...)
  #define LOG_INFO(format, ...)
  #define LOG_VERBOSE(format, ...)

#endif
