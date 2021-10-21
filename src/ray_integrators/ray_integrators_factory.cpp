#include "logging.h"
#include "ray_integrator.h"
#include "debug_ray_integrator.h"

bool8 rayIntegratorCreate(RayIntegratorType type, RayIntegrator** outRayIntegrator)
{
  switch(type)
  {
    case RAY_INTEGRATOR_TYPE_DEBUG: return createDebugRayIntegrator(DEBUG_RAY_INTEGRATOR_MODE_ONE_COLOR,
                                                                    outRayIntegrator);
  }

  LOG_ERROR("Ray integrators factory cannot create an integrator with type '%d'!'", type);
  return FALSE;
}

std::string rayIntegratorTypeToString(RayIntegratorType type)
{
  switch(type)
  {
    case RAY_INTEGRATOR_TYPE_DEBUG: return "DebugRayIntegrator";
  }

  LOG_ERROR("Ray integrators factory cannot convert type '%d' to a string!", type);
  return "Unknown";
}

RayIntegratorType rayIntegratorStringToType(const std::string& type)
{
  if(type == "DebugRayIntegrator")
  {
    return RAY_INTEGRATOR_TYPE_DEBUG;
  }

  LOG_ERROR("Samplers factory cannot convert string '%s' to a type!", type.c_str());
  return RAY_INTEGRATOR_TYPE_UKNOWN;
}
