#include <stdarg.h>

#include <cstdio>
#include <cstring>

#include "logging.h"
#include "event_system.h"
#include "memory_manager.h"

using std::deque;
using std::string;

struct LogSystem
{
  FILE* logFile = NULL;
};

struct Logger
{
  bool8 outputToStdout;
  bool8 generateEvents;
  bool8 prependTime;
  uint32 maxMessages;

  deque<LogMessage> messages;
  
  FILE* logFile = NULL;
};

static Logger* globalLogger;

static const int32 logMessageTypeToColor[LOG_MESSAGE_TYPE_COUNT] =
{
  196,
  220,
  126,
  32,
  16,
};

static const char* logMessageTypeToPrefix[LOG_MESSAGE_TYPE_COUNT] =
{
  "(!)   [ERROR]",
  "(?) [WARNING]",
  "(#) [VERBOSE]",
  "(*)    [INFO]",
  "(+) [SUCCESS]",
};

bool8 createLogger(uint32 maxMessages,
                   bool8 outputToStdout,
                   bool8 generateEvents,
                   bool8 prependTime,
                   const char* outputFileName,
                   Logger** outLogger)
{
  *outLogger = engineAllocObject<Logger>(MEMORY_TYPE_GENERAL);
  Logger* logger = *outLogger;
  
  if(outputFileName != nullptr && strlen(outputFileName) > 0)
  {
    logger->logFile = fopen(outputFileName, "w");
    if(logger->logFile == NULL)
    {
      engineFreeObject(logger, MEMORY_TYPE_GENERAL);
      return FALSE;
    }
  }

  logger->maxMessages = maxMessages;
  logger->outputToStdout = outputToStdout;
  logger->generateEvents = generateEvents;
  
  return TRUE;
}
                   
void destroyLogger(Logger* logger)
{
  if(logger->logFile != NULL)
  {
    fclose(logger->logFile);
  }  

  engineFreeObject(logger, MEMORY_TYPE_GENERAL);
}

bool8 initGlobalLogger(uint32 maxMessages, const char* outputFileName)
{
  return createLogger(maxMessages, TRUE, TRUE, FALSE, outputFileName, &globalLogger);
}

void shutdownGlobalLogger()
{
  if(globalLogger != nullptr)
  {
    destroyLogger(globalLogger);
  }
}

void logMsg(Logger* logger, LogMessageType type, const char* format, ...)
{
  if(logger == nullptr)
  {
    logger = globalLogger;

    if(logger == nullptr)
    {
      return;
    }
  }
  
  char buf[16384] = {};

  // Appending prefix
  uint32 prefixSize = sprintf(buf, "%s ", logMessageTypeToPrefix[type]);
  
  // Formatting message
  va_list args;
  va_start(args, format);
  uint32 msgSize = vsprintf(buf + prefixSize, format, args);
  va_end(args);

  if(logger->generateEvents == TRUE)
  {
    // Sending uncolored formatted message without prefix to the listeners
    EventData logData = {};
    logData.type = EVENT_TYPE_LOG_MESSAGE;
    logData.u32[0] = type;
    logData.u32[1] = msgSize;
    logData.ptr[0] = buf + prefixSize;

    triggerEvent(logData);
  }

  // Save uncolored message to the file
  if(logger->logFile != NULL)
  {
    fprintf(logger->logFile, "%s\n", buf);
  }

  if(logger->outputToStdout == TRUE)
  {
    // Output colored message to the standard output
    fprintf(stdout, "\e[38;5;%dm%s\e[0m\n", logMessageTypeToColor[type], buf);
  }

  if(logger->maxMessages > 0)
  {
    logger->messages.push_back(LogMessage{buf, type});
    if(logger->messages.size() > logger->maxMessages)
    {
      logger->messages.pop_front();
    }
  }
}

void logClear(Logger* logger)
{
  logger->messages.clear();
}

const std::deque<LogMessage>& logGetMessages(Logger* logger)
{
  return logger->messages;
}
