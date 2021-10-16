#include "stopwatch.h"

namespace march
{
  Time Stopwatch::restart()
  {
    Time elapsedTime = getElapsedTime();
    m_timepoint = Time::current();
    m_pauseTimepoint = m_timepoint;
    m_accumPauseTime = Time();

    return elapsedTime;
  }

  void Stopwatch::setTimepoint(Time time)
  {
    m_timepoint = time;
  }  
  
  void Stopwatch::pause()
  {
    m_pauseTimepoint = Time::current();
    m_paused = TRUE;
  }

  Time Stopwatch::unpause()
  {
    Time pausedTime = getPausedTime();
    m_accumPauseTime += pausedTime;
    m_paused = FALSE;

    return pausedTime;
  }

  Time Stopwatch::getTimepoint() const
  {
    return m_timepoint;
  }

  Time Stopwatch::getPauseTimepoint() const
  {
    return m_pauseTimepoint;
  }
  
  Time Stopwatch::getElapsedTime() const
  {
    return Time::current() - m_timepoint - m_accumPauseTime - getPausedTime();
  }

  Time Stopwatch::getPausedTime() const
  {
    return isPaused() == TRUE ? Time::current() - m_pauseTimepoint : Time();
  }

  bool8 Stopwatch::isPaused() const
  {
    return m_paused;
  }

}
