#include "mtime.h"

namespace march
{

  Time::Time(float64 time)
  {
    m_internalTime = time;
  }
  
  void Time::setSecs(float64 secs)
  {
    m_internalTime = secs;
  }

  void Time::setMsecs(float64 msecs)
  {
    m_internalTime = msecs / 1000.0f;
  }

  float64 Time::asSecs() const
  {
    return m_internalTime;
  }

  float64 Time::asMsec() const
  {
    return m_internalTime * 1000.0f;
  }

  Time Time::operator+(const Time& rop) const
  {
    return Time(m_internalTime + rop.m_internalTime);
  }

  Time& Time::operator+=(const Time& rop)
  {
    m_internalTime += rop.m_internalTime;
    return *this;
  }

  Time Time::operator-(const Time& rop) const
  {
    return Time(m_internalTime - rop.m_internalTime);
  }

  Time& Time::operator-=(const Time& rop)
  {
    m_internalTime -= rop.m_internalTime;
    return *this;
  }

  Time Time::operator*(float64 rop) const
  {
    return Time(m_internalTime * rop);
  }

  Time operator*(float64 lop, const Time& rop)
  {
    return rop * lop;
  }

  Time Time::current()
  {
    return secs(glfwGetTime());
  }

  Time Time::secs(float64 seconds)
  {
    Time time;
    time.setSecs(seconds);

    return time;
  }

  Time Time::msecs(float64 milliseconds)
  {
    Time time;
    time.setMsecs(milliseconds);

    return time;
  }
}
