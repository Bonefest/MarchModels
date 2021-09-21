#include <stdarg.h>

#include <cstdio>

#include "logging.h"

void log(int32 color, const char* prefix, const char* format, ...)
{
  char buf[4096] = {};
  uint32 n = sprintf(buf, "\e[38;5;%dm%s ", color, prefix);

  va_list args;
  va_start(args, format);
  n += vsprintf(buf + n, format, args);
  va_end(args);
  sprintf(buf + n, "%s", "\e[0m\n");

  fprintf(stdout, "%s", buf);
}
