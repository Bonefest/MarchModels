#include <stdarg.h>

#include <cstdio>
#include <cstring>

#include "logging.h"

struct LogSystem
{
  FILE* logFile = NULL;
  std::queue<std::string> messages;
  uint32 maxMessages = 512;
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

  systemData.maxMessages = maxMessages;  

  return TRUE;
}

void shutdownLoggingSystem()
{
  if(systemData.logFile != NULL)
  {
    fclose(systemData.logFile);
  }
}

void log(LogMessageType type, const char* format, ...)
{
  char buf[4096] = {};

  // Appending prefix
  uint32 realSize = sprintf(buf, "%s", logMessageTypeToPrefix[type]);

  // Formatting message
  va_list args;
  va_start(args, format);
  realSize += vsprintf(buf + realSize, format, args);
  va_end(args);

  if(systemData.messages.size() > systemData.maxMessages)
  {
    systemData.messages.pop();
  }

  // Save uncolored message to the queue and file
  systemData.messages.push(buf);
  if(systemData.logFile != NULL)
  {
    fprintf(systemData.logFile, "%s\n", buf);
  }

  // Output colored message to the standard output
  fprintf(stdout, "\e[38;5;%dm%s\e[0m\n", logMessageTypeToColor[type], buf);
}

const std::queue<std::string>& logGetMessages()
{
  return systemData.messages;
}
