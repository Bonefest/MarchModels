#pragma once

#include "film.h"
#include "scene.h"
#include "camera.h"
#include "ray_integrator.h"
#include "samplers/sampler.h"

struct ImageIntegrator;

struct ImageIntegratorInterface
{
  void (*destroy)(ImageIntegrator*);  
  bool8 (*shouldIntegratePixelLocation)(int2 location);
};

ENGINE_API bool8 allocateImageIntegrator(ImageIntegratorInterface interface,
                                         Scene* scene,
                                         Sampler* sampler,
                                         RayIntegrator* rayIntegrator,
                                         Film* film,
                                         Camera* camera,
                                         ImageIntegrator** outIntegrator);

ENGINE_API void destroyImageIntegrator(ImageIntegrator* integrator);

ENGINE_API void imageIntegratorExecute(ImageIntegrator* integrator, float32 time = 0.0f);

ENGINE_API void imageIntegratorSetScene(ImageIntegrator* integrator, Scene* scene);
ENGINE_API void imageIntegratorSetSampler(ImageIntegrator* integrator, Sampler* sampler);
ENGINE_API void imageIntegratorSetRayIntegrator(ImageIntegrator* integrator, RayIntegrator* rayIntegrator);
ENGINE_API void imageIntegratorSetFilm(ImageIntegrator* integrator, Film* film);
ENGINE_API void imageIntegratorSetCamera(ImageIntegrator* integrator, Camera* camera);
