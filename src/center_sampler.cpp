#include "memory_manager.h"

#include "sampler.h"

struct CenterSamplerData
{
  float2 inversedAreaSize;
  int2 location;
  bool8 sampleGenerated;
};

static void startSamplingPixel(Sampler* sampler, int2 location)
{
  CenterSamplerData* data = (CenterSamplerData*)sampler->internalData;
  data->location = location;
  data->sampleGenerated = FALSE;
}

static bool8 generateSample(Sampler* sampler, float2& outNDC)
{
  CenterSamplerData* data = (CenterSamplerData*)sampler->internalData;  
  if(data->sampleGenerated == TRUE)
  {
    return FALSE;
  }

  float2 samplePosition = float2(data->location.x, data->location.y) + float2(0.5f, 0.5f);
  outNDC = (samplePosition * data->inversedAreaSize) * 2.0f - 1.0f;
  data->sampleGenerated = TRUE;
  
  return TRUE;
}

bool8 createCenterSampler(uint2 sampleAreaSize, Sampler** outSampler)
{
  *outSampler = engineAllocObject<Sampler>(MEMORY_TYPE_GENERAL);
  Sampler* sampler = *outSampler;
  sampler->startSamplingPixel = startSamplingPixel;
  sampler->generateSample = generateSample;
  
  CenterSamplerData* data = engineAllocObject<CenterSamplerData>(MEMORY_TYPE_GENERAL);
  data->inversedAreaSize = float2(1.0f / float32(sampleAreaSize.x), 1.0f / float32(sampleAreaSize.y));
  data->sampleGenerated = TRUE;

  sampler->internalData = data;

  return TRUE;
}

void destroyCenterSampler(Sampler* sampler)
{
  engineFreeObject((CenterSamplerData*)sampler->internalData, MEMORY_TYPE_GENERAL);
  engineFreeObject(sampler, MEMORY_TYPE_GENERAL);
}

