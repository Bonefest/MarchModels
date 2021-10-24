#include "logging.h"
#include "sampler.h"
#include "center_sampler.h"

void samplerDrawInputView(Sampler* sampler)
{
  SamplerType type = samplerGetType(sampler);
  switch(type)
  {
    case SAMPLER_TYPE_CENTER_SAMPLER: centerSamplerDrawInputView(sampler); break;
    default: LOG_ERROR("Sampler's type '%d'('%s') doesn't have a view?", type, samplerTypeToString(type).c_str());
  }
}
