#include "memory_manager.h"

#include "sampler.h"

struct Sampler
{
  SamplerInterface interface;
  uint2 areaSize;
  float2 inversedAreaSize;
  
  void* internalData;
};

bool8 allocateSampler(const SamplerInterface& interface, uint2 sampleAreaSize, Sampler** outSampler)
{
  *outSampler = engineAllocObject<Sampler>(MEMORY_TYPE_GENERAL);
  Sampler* sampler = *outSampler;
  sampler->interface = interface;
  sampler->internalData = nullptr;

  samplerSetSampleAreaSize(*outSampler, sampleAreaSize);
  
  return TRUE;
}

void destroySampler(Sampler* sampler)
{
  sampler->interface.destroy(sampler);

  engineFreeObject(sampler, MEMORY_TYPE_GENERAL);
}

void samplerStartSamplingPixel(Sampler* sampler, int2 location)
{
  sampler->interface.startSamplingPixel(sampler, location);
}

bool8 samplerGenerateSample(Sampler* sampler, Sample& outSample)
{
  return sampler->interface.generateSample(sampler, outSample);
}

void samplerSetSampleAreaSize(Sampler* sampler, uint2 sampleAreaSize)
{
  sampler->areaSize = sampleAreaSize;
  sampler->inversedAreaSize = float2(1.0f / float32(sampleAreaSize.x), 1.0f / float32(sampleAreaSize.y));
}

uint2 samplerGetSampleAreaSize(Sampler* sampler)
{
  return sampler->areaSize;
}

SamplerType samplerGetType(Sampler* sampler)
{
  return sampler->interface.type;
}

float2 samplerConvertLocationToNDC(Sampler* sampler, int2 location)
{
  return samplerConvertLocationToNDC(sampler, float2(location.x, location.y));
}

float2 samplerConvertLocationToNDC(Sampler* sampler, float2 location)
{
  return location * sampler->inversedAreaSize * 2.0f - 1.0f;
}


void samplerSetInternalData(Sampler* sampler, void* internalData)
{
  sampler->internalData = internalData;
}

void* samplerGetInternalData(Sampler* sampler)
{
  return sampler->internalData;
}
