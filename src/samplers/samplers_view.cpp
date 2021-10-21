#include "logging.h"
#include "sampler.h"
#include "center_sampler.h"

void generateSampleInputView(Sampler* sampler)
{
  SamplerType type = samplerGetType(sampler);
  switch(type)
  {
    case SAMPLER_TYPE_CENTER_SAMPLER: centerSamplerGenerateInputView(sampler);
  }

  LOG_ERROR("Sampler's type '%d'('%s)' doesn't have a view?", type, samplerTypeToString(type).c_str());
}
