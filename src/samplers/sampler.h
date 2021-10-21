#pragma once

#include "maths/common.h"

#include "defines.h"

using SamplerType = uint32;

static const SamplerType SAMPLER_TYPE_UNKNOWN = (SamplerType)-1;

struct Sample
{
  float2 ndc;
  float32 weight;
};

struct Sampler;

struct SamplerInterface
{
  void(*destroy)(Sampler* sampler);
  void(*startSamplingPixel)(Sampler* sampler, int2 location);
  bool8(*generateSample)(Sampler* sampler, Sample& outSample);

  SamplerType type;
};

// ----------------------------------------------------------------------------
// Main API
// ----------------------------------------------------------------------------

ENGINE_API bool8 allocateSampler(const SamplerInterface& interface, uint2 sampleAreaSize, Sampler** outSampler);
ENGINE_API void destroySampler(Sampler* sampler);

ENGINE_API void samplerStartSamplingPixel(Sampler* sampler, int2 location);
ENGINE_API bool8 samplerGenerateSample(Sampler* sampler, Sample& outSampler);

ENGINE_API void samplerSetSampleAreaSize(Sampler* sampler, uint2 sampleAreaSize);
ENGINE_API uint2 samplerGetSampleAreaSize(Sampler* sampler);

ENGINE_API SamplerType samplerGetType(Sampler* sampler);

float2 samplerConvertLocationToNDC(Sampler* sampler, int2 location);
float2 samplerConvertLocationToNDC(Sampler* sampler, float2 location);

void samplerSetInternalData(Sampler* sampler, void* internalData);
void* samplerGetInternalData(Sampler* sampler);

// ----------------------------------------------------------------------------
// Factory API
// ----------------------------------------------------------------------------

ENGINE_API bool8 samplerCreate(SamplerType type, uint2 sampleAreaSize, Sampler** outSampler);
ENGINE_API std::string samplerTypeToString(SamplerType type);
ENGINE_API SamplerType samplerStringToType(const std::string& type);

// ----------------------------------------------------------------------------
// View API
// ----------------------------------------------------------------------------

/**
 * Each sampler has its data. View generates an ImGui UI which allows to manipulate
 * that data. Logically, view is a secondary interface of the sampler, i.e it's not
 * a part of the main interface. Views are needed more for editor-related stuff.
 */
// TODO: We may want to pass some additional information to the view, e.g orientation
// of the screen, position of the window, window's size or even window itself.
// This will allow the view to adapt correspondingly.
ENGINE_API void drawSampleInputView(Sampler* sampler);
