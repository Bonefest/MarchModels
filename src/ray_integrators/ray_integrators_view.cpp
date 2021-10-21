#include "logging.h"
#include "ray_integrator.h"
#include "debug_ray_integrator.h"

void rayIntegratorDrawInputView(RayIntegrator* rayIntegrator)
{
  RayIntegratorType type = rayIntegratorGetType(rayIntegrator);
  switch(type)
  {
    case RAY_INTEGRATOR_TYPE_DEBUG: debugRayIntegratorDrawInputView(rayIntegrator); break;
    default: LOG_ERROR("Sampler's type '%d'('%s') doesn't have a view?",
                       type,
                       rayIntegratorTypeToString(type).c_str());
  }

}
