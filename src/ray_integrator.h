#pragma once

#include "scene.h"
#include "maths/common.h"

struct RayIntegrator;

struct RayIntegratorInterface
{
  void(*destroy)(RayIntegrator*);
  float3(*calculateRadiance)(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time);
};

ENGINE_API bool8 allocateRayIntegrator(RayIntegratorInterface interface, RayIntegrator** outIntegrator);
ENGINE_API void destroyRayIntegrator(RayIntegrator* integrator);

ENGINE_API float3 rayIntegratorCalculateRadiance(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time);

void rayIntegratorSetInternalData(RayIntegrator* integrator, void* internalData);
void* rayIntegratorGetInternalData(RayIntegrator* integrator);
