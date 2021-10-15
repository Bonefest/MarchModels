#include "stopwatch.h"

namespace march
{
  Time Stopwatch::restart()
  {
    Time elapsedTime = getElapsedTime();
    m_timepoint = Time::current();

    return elapsedTime;
  }

  Time Stopwatch::getElapsedTime() const
  {
    return Time::current() - m_timepoint;
  }

  void Stopwatch::setTimepoint(Time time)
  {
    m_timepoint = time;
  }
}
