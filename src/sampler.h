#pragma once

#include "maths/common.h"

#include "defines.h"

struct Sample
{
  float2 ndc;
  float32 weight;
};

struct Sampler;

struct SamplerInterface
{
  void(*destroy)(Sampler* sampler);
  void(*startSamplingPixel)(Sampler* sampler, int2 location);
  bool8(*generateSample)(Sampler* sampler, Sample& outSample);

  void* internalData;
};

ENGINE_API bool8 allocateSampler(const SamplerInterface& interface, Sampler** outSampler);
ENGINE_API void destroySampler(Sampler* sampler);

ENGINE_API void samplerStartSamplingPixel(Sampler* sampler, int2 location);
ENGINE_API bool8 samplerGenerateSample(Sampler* sampler, Sample& outSampler);

ENGINE_API void samplerSetInternalData(Sampler* sampler, void* internalData);
ENGINE_API void* samplerGetInternalData(Sampler* sampler);

ENGINE_API bool8 createCenterSampler(uint2 sampleAreaSize, Sampler** outSampler);
ENGINE_API bool8 createRandomSampler(uint2 sampleAreaSize,
                                     uint32 seed,
                                     uint32 samplesPerPixel,
                                     Sampler** outSampler);

