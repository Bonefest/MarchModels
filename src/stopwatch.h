#pragma once

#include "mtime.h"

namespace march
{
  class Stopwatch
  {
  public:

    Time restart();

    void setTimepoint(Time time);
    void pause();
    Time unpause();

    Time getTimepoint() const;
    Time getPauseTimepoint() const;
    
    Time getElapsedTime() const;
    Time getPausedTime() const;
    
    bool8 isPaused() const;
    
  private:
    Time m_timepoint;
    Time m_pauseTimepoint;
    Time m_accumPauseTime;
    bool8 m_paused = FALSE;
  };
}
