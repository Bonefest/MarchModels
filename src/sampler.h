#pragma once

#include "maths/common.h"

#include "defines.h"

using namespace linalg::aliases;

struct Sampler
{
  void(*startSamplingPixel)(Sampler* sampler, int2 location);
  bool8(*generateSample)(Sampler* sampler, float2& outNDC);

  void* internalData;
};

ENGINE_API bool8 createCenterSampler(int2 sampleAreaSize, Sampler** outSampler);
ENGINE_API void destroyCenterSampler(Sampler* sampler);

ENGINE_API bool8 createRandomSampler(int2 sampleAreaSize,
                                     uint32 seed,
                                     uint32 samplesPerPixel,
                                     Sampler** outSampler);
ENGINE_API void destroyRandomSampler(Sampler* sampler);
