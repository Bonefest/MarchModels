#pragma once

#include "scene.h"
#include "maths/common.h"

using RayIntegratorType = uint32;

static const RayIntegratorType RAY_INTEGRATOR_TYPE_UKNOWN = (RayIntegratorType)-1;

struct RayIntegrator;

struct RayIntegratorInterface
{
  void(*destroy)(RayIntegrator*);
  float3(*calculateRadiance)(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time);

  RayIntegratorType type;
};

// ----------------------------------------------------------------------------
// Main API
// ----------------------------------------------------------------------------

ENGINE_API bool8 allocateRayIntegrator(RayIntegratorInterface interface, RayIntegrator** outIntegrator);
ENGINE_API void destroyRayIntegrator(RayIntegrator* integrator);

ENGINE_API float3 rayIntegratorCalculateRadiance(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time);

ENGINE_API RayIntegratorType rayIntegratorGetType(RayIntegrator* integrator);

void rayIntegratorSetInternalData(RayIntegrator* integrator, void* internalData);
void* rayIntegratorGetInternalData(RayIntegrator* integrator);

// ----------------------------------------------------------------------------
// Factory API
// ----------------------------------------------------------------------------

ENGINE_API bool8 rayIntegratorCreate(RayIntegratorType type, RayIntegrator** outRayIntegrator);
ENGINE_API std::string rayIntegratorTypeToString(RayIntegratorType type);
ENGINE_API RayIntegratorType rayIntegratorStringToType(const std::string& type);

// ----------------------------------------------------------------------------
// View API
// ----------------------------------------------------------------------------

ENGINE_API void rayIntegratorDrawInputView(RayIntegrator* rayIntegrator);
