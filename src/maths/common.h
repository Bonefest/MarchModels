#pragma once

#include "defines.h"
#include "linalg/linalg.h"

using namespace linalg;
using namespace linalg::aliases;

struct Ray
{
  Ray() = default;
  Ray(float3 inOrigin, float3 inDirection): origin(inOrigin), direction(inDirection) {}

  inline float3 get(float32 t) { return origin + direction * t; }
  
  float3 origin;
  float3 direction;
};
