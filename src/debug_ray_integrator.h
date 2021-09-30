#pragma once

#include "ray_integrator.h"

enum DebugRayIntegratorMode
{
  DEBUG_RAY_INTEGRATOR_MODE_ONE_COLOR,
  DEBUG_RAY_INTEGRATOR_MODE_NORMAL_COLOR,
  DEBUG_RAY_INTEGRATOR_MODE_FAST_LAMBERT
};

ENGINE_API bool8 createDebugRayIntegrator(DebugRayIntegratorMode mode, RayIntegrator** outIntegrator);
ENGINE_API void debugRayIntegratorSetMode(RayIntegrator* integrator, DebugRayIntegratorMode mode);
ENGINE_API DebugRayIntegratorMode debugRayIntegratorGetMode(RayIntegrator* integrator);
