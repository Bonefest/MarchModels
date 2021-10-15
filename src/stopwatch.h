#pragma once

#include "mtime.h"

namespace march
{
  class Stopwatch
  {
  public:

    Time restart();
    Time getElapsedTime() const;

    void setTimepoint(Time time);

  private:
    Time m_timepoint;

  };
}
