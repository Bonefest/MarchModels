#include "sampler.h"
#include "logging.h"
#include "center_sampler.h"

bool8 samplerCreate(SamplerType type, uint2 sampleAreaSize, Sampler** outSampler)
{
  switch(type)
  {
    case SAMPLER_TYPE_CENTER_SAMPLER: return createCenterSampler(sampleAreaSize, outSampler);
  }

  LOG_ERROR("Samplers factory cannot create a sampler with type '%d'!", type);
  return FALSE;
}

std::string samplerTypeToString(SamplerType type)
{
  switch(type)
  {
    case SAMPLER_TYPE_CENTER_SAMPLER: "CenterSampler";
  }

  LOG_ERROR("Samplers factory cannot convert type '%d' to a string!", type);
  return "Unknown";
}

SamplerType samplerStringToType(const std::string& type)
{
  if(type == "CenterSampler")
  {
    return SAMPLER_TYPE_CENTER_SAMPLER;
  }

  LOG_ERROR("Samplers factory cannot convert string '%s' to a type!", type.c_str());
  return SAMPLER_TYPE_UNKNOWN;
}
