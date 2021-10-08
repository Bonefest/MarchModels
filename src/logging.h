#pragma once

#include <queue>
#include <string>

#include "defines.h"

enum LogMessageType
{
  LOG_MESSAGE_TYPE_ERROR,
  LOG_MESSAGE_TYPE_WARNING,
  LOG_MESSAGE_TYPE_VERBOSE,
  LOG_MESSAGE_TYPE_INFO,
  
  LOG_MESSAGE_TYPE_COUNT
};

bool8 initLoggingSystem(uint32 maxMessages, const char* logFileName="");
void shutdownLoggingSystem();

void log(LogMessageType type, const char* format, ...);
const std::queue<std::string>& logGetMessages();

#define LOG_ERROR(format, ...) log(LOG_MESSAGE_TYPE_ERROR, format __VA_OPT__(,) __VA_ARGS__)

#ifdef DEBUG

  #define LOG_WARNING(format, ...) log(LOG_MESSAGE_TYPE_WARNING, format __VA_OPT__(,) __VA_ARGS__)
  #define LOG_INFO(format, ...) log(LOG_MESSAGE_TYPE_INFO, format __VA_OPT__(,) __VA_ARGS__)
  #define LOG_VERBOSE(format, ...) log(LOG_MESSAGE_TYPE_VERBOSE, format __VA_OPT__(,) __VA_ARGS__)

#else

  #define LOG_WARNING(format, ...)
  #define LOG_INFO(format, ...)
  #define LOG_VERBOSE(format, ...)

#endif
