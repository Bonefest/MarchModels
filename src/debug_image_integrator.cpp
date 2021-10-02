#include "memory_manager.h"

#include "debug_image_integrator.h"

struct DebugImageIntegratorData
{
  uint2 step;
  uint2 offset;
};

static void destroyDebugImageIntegrator(ImageIntegrator* integrator)
{
  engineFreeMem((DebugImageIntegratorData*)imageIntegratorGetInternalData(integrator), MEMORY_TYPE_GENERAL);
}

static bool8 debugImageIntegratorShouldIntegratePixelLocation(ImageIntegrator* integrator, int2 location)
{
  DebugImageIntegratorData* data = (DebugImageIntegratorData*)imageIntegratorGetInternalData(integrator);
  return (location.x + data->offset.x) % data->step.x == 0 && (location.y + data->offset.y) % data->step.y == 0;
}

bool8 createDebugImageIntegrator(uint2 pixelStep, uint2 initialOffset,
                                 Scene* scene,
                                 Sampler* sampler,
                                 RayIntegrator* rayIntegrator,
                                 Film* film,
                                 Camera* camera,
                                 ImageIntegrator** outImageIntegrator)
{
  ImageIntegratorInterface interface = {};
  interface.destroy = destroyDebugImageIntegrator;
  interface.shouldIntegratePixelLocation = debugImageIntegratorShouldIntegratePixelLocation;

  if(allocateImageIntegrator(interface, scene, sampler, rayIntegrator, film, camera, outImageIntegrator) == FALSE)
  {
    return FALSE;
  }

  DebugImageIntegratorData* data = engineAllocObject<DebugImageIntegratorData>(MEMORY_TYPE_GENERAL);
  data->step = pixelStep + uint2(1, 1);
  data->offset = initialOffset;
  
  ImageIntegrator* integrator = *outImageIntegrator;
  imageIntegratorSetInternalData(integrator, data);
  
  return TRUE;
}

void debugImageIntegratorSetPixelStep(ImageIntegrator* integrator, uint2 step)
{
  DebugImageIntegratorData* data = (DebugImageIntegratorData*)imageIntegratorGetInternalData(integrator);
  data->step = step + uint2(1, 1);
}

uint2 debugImageIntegratorGetPixelStep(ImageIntegrator* integrator)
{
  DebugImageIntegratorData* data = (DebugImageIntegratorData*)imageIntegratorGetInternalData(integrator);
  return data->step - uint2(1, 1);
}

void debugImageIntegratorSetInitialOffset(ImageIntegrator* integrator, uint2 offset)
{
  DebugImageIntegratorData* data = (DebugImageIntegratorData*)imageIntegratorGetInternalData(integrator);
  data->offset = offset;
}

uint2 debugImageIntegratorGetInitialOffset(ImageIntegrator* integrator)
{
  DebugImageIntegratorData* data = (DebugImageIntegratorData*)imageIntegratorGetInternalData(integrator);
  return data->offset;
}
