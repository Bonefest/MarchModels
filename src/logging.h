#pragma once

#include <deque>
#include <string>

#include "defines.h"

enum LogMessageType
{
  LOG_MESSAGE_TYPE_ERROR,
  LOG_MESSAGE_TYPE_WARNING,
  LOG_MESSAGE_TYPE_VERBOSE,
  LOG_MESSAGE_TYPE_INFO,
  LOG_MESSAGE_TYPE_SUCCESS,
  
  LOG_MESSAGE_TYPE_COUNT
};

struct Logger;

struct LogMessage
{
  std::string message;
  LogMessageType type;
};

bool8 createLogger(uint32 maxMessages,
                   bool8 outputToStdout,
                   bool8 generateEvents,
                   bool8 prependTime,
                   const char* outputFileName,
                   Logger** outLogger);

void destroyLogger(Logger* logger);

bool8 initGlobalLogger(uint32 maxMessages, const char* logFileName="");
void shutdownGlobalLogger();

ENGINE_API void logMsg(Logger* logger, LogMessageType type, const char* format, ...);
ENGINE_API void logClear(Logger* logger);
ENGINE_API const std::deque<LogMessage>& logGetMessages(Logger* logger);

#define LOG_ERROR(format, ...) logMsg(nullptr, LOG_MESSAGE_TYPE_ERROR, format __VA_OPT__(,) __VA_ARGS__)

#ifdef DEBUG

  #define LOG_WARNING(format, ...) logMsg(nullptr, LOG_MESSAGE_TYPE_WARNING, format __VA_OPT__(,) __VA_ARGS__)
  #define LOG_INFO(format, ...) logMsg(nullptr, LOG_MESSAGE_TYPE_INFO, format __VA_OPT__(,) __VA_ARGS__)
  #define LOG_VERBOSE(format, ...) logMsg(nullptr, LOG_MESSAGE_TYPE_VERBOSE, format __VA_OPT__(,) __VA_ARGS__)

#else

  #define LOG_WARNING(format, ...)
  #define LOG_INFO(format, ...)
  #define LOG_VERBOSE(format, ...)

#endif
