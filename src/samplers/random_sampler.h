#pragma once

#include "sampler.h"

ENGINE_API bool8 createRandomSampler(uint2 sampleAreaSize,
                                     uint32 seed,
                                     uint32 samplesPerPixel,
                                     Sampler** outSampler);
