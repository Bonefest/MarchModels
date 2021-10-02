#include "memory_manager.h"

#include "center_sampler.h"

struct CenterSamplerData
{
  uint2 areaSize;
  float2 inversedAreaSize;
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
  outSample.ndc = (samplePosition * data->inversedAreaSize) * 2.0f - 1.0f;
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
  
  allocateSampler(interface, outSampler);

  CenterSamplerData* data = engineAllocObject<CenterSamplerData>(MEMORY_TYPE_GENERAL);
  data->sampleGenerated = TRUE;

  samplerSetInternalData(*outSampler, data);
  centerSamplerSetAreaSize(*outSampler, sampleAreaSize);

  return TRUE;
}

void centerSamplerSetAreaSize(Sampler* sampler, uint2 newAreaSize)
{
  CenterSamplerData* data = (CenterSamplerData*)samplerGetInternalData(sampler);
  data->inversedAreaSize = float2(1.0f / float32(newAreaSize.x), 1.0f / float32(newAreaSize.y));  
  data->areaSize = newAreaSize;
}

uint2 centerSamplerGetAreaSize(Sampler* sampler)
{
  CenterSamplerData* data = (CenterSamplerData*)samplerGetInternalData(sampler);  
  return data->areaSize;
}
