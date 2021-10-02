#pragma once

#include "image_integrator.h"

/**
 *
 * @param pixelStep Indicates how much empty pixels there are between two filled.
 * Example: pixelStep = (0, 0) means each pixel is is filled.
 * Example: pixelStep = (0, 1) means that each horizontal pixel is filled and there are
 * one empty row between each pixel.
 *
 * @param initialOffset Shifts the pattern (x, y) pixels to the right.
 */
ENGINE_API bool8 createDebugImageIntegrator(uint2 pixelStep, uint2 initialOffset,
                                            Scene* scene,
                                            Sampler* sampler,
                                            RayIntegrator* rayIntegrator,
                                            Film* film,
                                            Camera* camera,
                                            ImageIntegrator** outImageIntegrator);

ENGINE_API void debugImageIntegratorSetPixelStep(ImageIntegrator* integrator, uint2 step);
ENGINE_API uint2 debugImageIntegratorGetPixelStep(ImageIntegrator* integrator);

ENGINE_API void debugImageIntegratorSetInitialOffset(ImageIntegrator* integrator, uint2 offset);
ENGINE_API uint2 debugImageIntegratorGetInitialOffset(ImageIntegrator* integrator);
