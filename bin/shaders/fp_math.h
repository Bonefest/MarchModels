#ifndef FP_MATH_H_INCLUDED
#define FP_MATH_H_INCLUDED

#define FP_FRACTIONAL 1e4
#define FP_RANGE 2e9

// Five integral digits, four fractional digits
// Allows to represent values in range [-199999.9999, 199999.9999]
uint32 floatToFixedPoint(float32 value)
{
  return uint32(value * FP_FRACTIONAL + FP_RANGE);
}

float32 fixedPointToFloat(uint32 value)
{
  return (value - FP_RANGE) / FP_FRACTIONAL;
}

#endif
