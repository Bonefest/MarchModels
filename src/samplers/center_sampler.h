#pragma once

#include "sampler.h"

ENGINE_API bool8 createCenterSampler(uint2 sampleAreaSize, Sampler** outSampler);

ENGINE_API void centerSamplerSetAreaSize(Sampler* sampler, uint2 newAreaSize);
ENGINE_API uint2 centerSamplerGetAreaSize(Sampler* sampler);
