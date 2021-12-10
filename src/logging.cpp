#include <stdarg.h>

#include <cstdio>
#include <cstring>

#include "logging.h"
#include "event_system.h"

struct LogSystem
{
  FILE* logFile = NULL;
};

static LogSystem systemData;

static const int32 logMessageTypeToColor[LOG_MESSAGE_TYPE_COUNT] =
{
  196,
  220,
  126,
  32,  
};

static const char* logMessageTypeToPrefix[LOG_MESSAGE_TYPE_COUNT] =
{
  "(!)   [ERROR]",
  "(?) [WARNING]",
  "(#) [VERBOSE]",
  "(*)    [INFO]",  
};

bool8 initLoggingSystem(uint32 maxMessages, const char* outputFileName)
{
  if(strlen(outputFileName) > 0)
  {
    systemData.logFile = fopen(outputFileName, "w");
    if(systemData.logFile == NULL)
    {
      return FALSE;
    }
  }

  return TRUE;
}

void shutdownLoggingSystem()
{
  if(systemData.logFile != NULL)
  {
    fclose(systemData.logFile);
  }
}

void _log(LogMessageType type, const char* format, ...)
{
  char buf[4096] = {};

  // Appending prefix
  uint32 prefixSize = sprintf(buf, "%s", logMessageTypeToPrefix[type]);
  
  // Formatting message
  va_list args;
  va_start(args, format);
  uint32 msgSize = vsprintf(buf + prefixSize, format, args);
  va_end(args);

  // Sending uncolored formatted message without prefix to the listeners
  EventData logData = {};
  logData.type = EVENT_TYPE_LOG_MESSAGE;
  logData.u32[0] = type;
  logData.u32[1] = msgSize;
  logData.ptr[0] = buf + prefixSize;

  triggerEvent(logData);
  
  // Save uncolored message to the file
  if(systemData.logFile != NULL)
  {
    fprintf(systemData.logFile, "%s\n", buf);
  }

  // Output colored message to the standard output
  fprintf(stdout, "\e[38;5;%dm%s\e[0m\n", logMessageTypeToColor[type], buf);
}
