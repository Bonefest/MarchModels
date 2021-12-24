#pragma once

#include "film.h"
#include "scene.h"
#include "camera.h"
#include "samplers/sampler.h"
#include "renderer/renderer.h"
#include "ray_integrators/ray_integrator.h"

struct ImageIntegrator;

ENGINE_API bool8 createImageIntegrator(Scene* scene,
                                       Sampler* sampler,
                                       RayIntegrator* rayIntegrator,
                                       Film* film,
                                       Camera* camera,
                                       ImageIntegrator** outIntegrator);

ENGINE_API void destroyImageIntegrator(ImageIntegrator* integrator);

ENGINE_API void imageIntegratorExecute(ImageIntegrator* integrator, const RenderingParameters& parameters);
ENGINE_API void imageIntegratorSetSize(ImageIntegrator* integrator, uint2 size);

ENGINE_API void imageIntegratorSetScene(ImageIntegrator* integrator, Scene* scene);
ENGINE_API Scene* imageIntegratorGetScene(ImageIntegrator* integrator);
ENGINE_API void imageIntegratorSetSampler(ImageIntegrator* integrator, Sampler* sampler);
ENGINE_API Sampler* imageIntegratorGetSampler(ImageIntegrator* integrator);
ENGINE_API void imageIntegratorSetRayIntegrator(ImageIntegrator* integrator, RayIntegrator* rayIntegrator);
ENGINE_API RayIntegrator* imageIntegratorGetRayIntegrator(ImageIntegrator* integrator);
ENGINE_API void imageIntegratorSetFilm(ImageIntegrator* integrator, Film* film);
ENGINE_API Film* imageIntegratorGetFilm(ImageIntegrator* integrator);
ENGINE_API void imageIntegratorSetCamera(ImageIntegrator* integrator, Camera* camera);
ENGINE_API Camera* imageIntegratorGetCamera(ImageIntegrator* integrator);

/**
 * @param gap Indicates how much empty pixels there are between two filled.
 * Example: pixelGap = (0, 0) means each pixel is filled.
 * Example: pixelGap = (0, 1) means that each horizontal pixel is filled and there is
 * one empty row between each pixel.
 */
ENGINE_API void imageIntegratorSetPixelGap(ImageIntegrator* integrator, uint2 gap);
ENGINE_API uint2 imageIntegratorGetPixelGap(ImageIntegrator* integrator);

/**
 * @param offset Shifts the gap pattern (x, y) pixels to the right-down.
 */
ENGINE_API void imageIntegratorSetInitialOffset(ImageIntegrator* integrator, uint2 offset);
ENGINE_API uint2 imageIntegratorGetInitialOffset(ImageIntegrator* integrator);
// ENGINE_API void imageIntegratorSetIntegrateDeciderFunc(bool8(*drawPixel)(ImageIntegrator* integrator, int2 location));

void imageIntegratorSetInternalData(ImageIntegrator* integrator, void* internalData);
void* imageIntegratorGetInternalData(ImageIntegrator* integrator);
