#pragma once

#include "defines.h"

struct Sampler
{
  void(*startSamplingPixel)(Sampler* sampler, int2 location);
  bool8(*generateSample)(Sampler* sampler, float2& outNDC);

  void* internalData;
};

EDITOR_API createSimpleSampler(Sampler** outSampler);
EDITOR_API destroySimpleSampler(Sampler* sampler);

EDITOR_API createRandomSampler(uint32 seed, uint32 samplesPerPixel, Sampler** outSampler);
EDITOR_API destroyRandomSampler(Sampler* sampler);
