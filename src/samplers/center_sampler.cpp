#include "memory_manager.h"

#include "center_sampler.h"

struct CenterSamplerData
{
  int2 location;
  bool8 sampleGenerated;
};

static void destroyCenterSampler(Sampler* sampler)
{
  engineFreeObject((CenterSamplerData*)samplerGetInternalData(sampler), MEMORY_TYPE_GENERAL);
}

static void startSamplingPixel(Sampler* sampler, int2 location)
{
  CenterSamplerData* data = (CenterSamplerData*)samplerGetInternalData(sampler);
  data->location = location;
  data->sampleGenerated = FALSE;
}

static bool8 generateSample(Sampler* sampler, Sample& outSample)
{
  CenterSamplerData* data = (CenterSamplerData*)samplerGetInternalData(sampler);
  if(data->sampleGenerated == TRUE)
  {
    return FALSE;
  }

  float2 samplePosition = float2(data->location.x, data->location.y) + float2(0.5f, 0.5f);
  outSample.ndc = samplerConvertLocationToNDC(sampler, samplePosition);
  outSample.weight = 1.0f;
  
  data->sampleGenerated = TRUE;
  
  return TRUE;
}

bool8 createCenterSampler(uint2 sampleAreaSize, Sampler** outSampler)
{
  SamplerInterface interface = {};
  interface.destroy = destroyCenterSampler;
  interface.startSamplingPixel = startSamplingPixel;
  interface.generateSample = generateSample;
  
  allocateSampler(interface, sampleAreaSize, outSampler);

  CenterSamplerData* data = engineAllocObject<CenterSamplerData>(MEMORY_TYPE_GENERAL);
  data->sampleGenerated = TRUE;

  samplerSetInternalData(*outSampler, data);

  return TRUE;
}
