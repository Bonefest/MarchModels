#include "memory_manager.h"

#include "ray_integrator.h"

struct RayIntegrator
{
  RayIntegratorInterface interface;

  void* internalData;
};

bool8 allocateRayIntegrator(RayIntegratorInterface interface, RayIntegrator** outIntegrator)
{
  *outIntegrator = engineAllocObject<RayIntegrator>(MEMORY_TYPE_GENERAL);
  RayIntegrator* integrator = *outIntegrator;
  integrator->interface = interface;
  integrator->internalData = nullptr;

  return TRUE;
}

void destroyRayIntegrator(RayIntegrator* integrator)
{
  integrator->interface.shutdown(integrator);
  engineFreeObject(integrator, MEMORY_TYPE_GENERAL);
}

float3 rayIntegratorCalculateRadiance(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time)
{
  return integrator->interface.calculateRadiance(integrator, viewRay, scene, time);
}

void rayIntegratorSetInternalData(RayIntegrator* integrator, void* internalData)
{
  integrator->internalData = internalData;
}

void* rayIntegratorGetInternalData(RayIntegrator* integrator)
{
  return integrator->internalData;
}
