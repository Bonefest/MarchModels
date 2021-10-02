#include "memory_manager.h"

#include "debug_ray_integrator.h"

struct DebugRayIntegratorData
{
  DebugRayIntegratorMode mode;
};

static void destroyDebugRayIntegrator(RayIntegrator* integrator)
{
  engineFreeObject((DebugRayIntegratorData*)rayIntegratorGetInternalData(integrator), MEMORY_TYPE_GENERAL);
}

static float3 debugRayCalculateRadiance(RayIntegrator* integrator, Ray viewRay, Scene* scene, float32 time)
{
  IntersectionDetails details = sceneFindIntersection(scene, viewRay, FALSE);
  if(details.intersected == TRUE)
  {
    return float3(1.0f, 0.0f, 0.0f);
  }

  return float3(0.0f, 0.2f, 0.8f);
}

bool8 createDebugRayIntegrator(DebugRayIntegratorMode mode, RayIntegrator** outIntegrator)
{
  RayIntegratorInterface interface = {};
  interface.destroy = destroyDebugRayIntegrator;
  interface.calculateRadiance = debugRayCalculateRadiance;
  
  if(!allocateRayIntegrator(interface, outIntegrator))
  {
    return FALSE;
  }

  DebugRayIntegratorData* data = engineAllocObject<DebugRayIntegratorData>(MEMORY_TYPE_GENERAL);
  data->mode = mode;

  rayIntegratorSetInternalData(*outIntegrator, data);

  return TRUE;
}

void debugRayIntegratorSetMode(RayIntegrator* integrator, DebugRayIntegratorMode mode)
{
  DebugRayIntegratorData* data = (DebugRayIntegratorData*)rayIntegratorGetInternalData(integrator);
  data->mode = mode;
}

DebugRayIntegratorMode debugRayIntegratorGetMode(RayIntegrator* integrator)
{
  DebugRayIntegratorData* data = (DebugRayIntegratorData*)rayIntegratorGetInternalData(integrator);
  return data->mode;
}
