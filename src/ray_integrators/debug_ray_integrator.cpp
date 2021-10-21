#include <imgui/imgui.h>

#include "memory_manager.h"

#include "debug_ray_integrator.h"

struct DebugRayIntegratorData
{
  DebugRayIntegratorMode mode;
};

const char* modeLabels[] =
{
  "One color",
  "Normal color",
  "Fast lambert"
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
  interface.type = RAY_INTEGRATOR_TYPE_DEBUG;
  
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

void debugRayIntegratorDrawInputView(RayIntegrator* integrator)
{
  DebugRayIntegratorData* data = (DebugRayIntegratorData*)rayIntegratorGetInternalData(integrator);
  int mode = (int)data->mode;

  if(ImGui::Combo("Integration mode", &mode, modeLabels, DEBUG_RAY_INTEGRATOR_MODE_COUNT))
  {
    data->mode = (DebugRayIntegratorMode)mode;
  }
}

