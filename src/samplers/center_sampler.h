#pragma once

#include "sampler.h"

static const SamplerType SAMPLER_TYPE_CENTER_SAMPLER = 0xbbe82af1;

ENGINE_API bool8 createCenterSampler(uint2 sampleAreaSize, Sampler** outSampler);

void centerSamplerDrawInputView(Sampler* sampler);
