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
};

ENGINE_API bool8 allocateSampler(const SamplerInterface& interface, uint2 sampleAreaSize, Sampler** outSampler);
ENGINE_API void destroySampler(Sampler* sampler);

ENGINE_API void samplerStartSamplingPixel(Sampler* sampler, int2 location);
ENGINE_API bool8 samplerGenerateSample(Sampler* sampler, Sample& outSampler);

ENGINE_API void samplerSetSampleAreaSize(Sampler* sampler, uint2 sampleAreaSize);
ENGINE_API uint2 samplerGetSampleAreaSize(Sampler* sampler);

float2 samplerConvertLocationToNDC(Sampler* sampler, int2 location);
float2 samplerConvertLocationToNDC(Sampler* sampler, float2 location);

void samplerSetInternalData(Sampler* sampler, void* internalData);
void* samplerGetInternalData(Sampler* sampler);

