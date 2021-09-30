#include "memory_manager.h"

#include "sampler.h"

struct Sampler
{
  SamplerInterface interface;

  void* internalData;
};

bool8 allocateSampler(const SamplerInterface& interface, Sampler** outSampler)
{
  *outSampler = engineAllocObject<Sampler>(MEMORY_TYPE_GENERAL);
  Sampler* sampler = *outSampler;
  sampler->interface = interface;
  sampler->internalData = nullptr;
  
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

void samplerSetInternalData(Sampler* sampler, void* internalData)
{
  sampler->internalData = internalData;
}

void* samplerGetInternalData(Sampler* sampler)
{
  return sampler->internalData;
}
