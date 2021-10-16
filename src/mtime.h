#pragma once

#include "defines.h"

namespace march
{
  
  class Time
  {
  public:
    Time() = default;

    void setSecs(float64 secs);
    void setMsecs(float64 msecs);

    float64 asSecs() const;
    float64 asMsec() const;

    Time operator+(const Time& rop) const;
    Time& operator+=(const Time& rop);

    Time operator-(const Time& rop) const;
    Time& operator-=(const Time& rop);
    
    Time operator*(float64 rop) const;
    friend Time operator*(float64 lop, const Time& rop);

    bool operator<(const Time& rop) const;
    bool operator<=(const Time& rop) const;
    bool operator>(const Time& rop) const;
    bool operator>=(const Time& rop) const;
    bool operator==(const Time& rop) const;
    bool operator!=(const Time& rop) const;
    
    static Time current();
    static Time secs(float64 seconds);
    static Time msecs(float64 milliseconds);

  private:
    Time(float64 time);

  private:
    float64 m_internalTime = 0.0f;
  };

}
