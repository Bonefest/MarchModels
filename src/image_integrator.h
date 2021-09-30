#pragma once

#include "film.h"
#include "scene.h"
#include "camera.h"
#include "sampler.h"
#include "ray_integrator.h"

struct ImageIntegrator;

struct ImageIntegratorInterface
{
  bool8 (*shouldIntegratePixelLocation)(int2 location);
  void (*shutdown)(ImageIntegrator*);
};

bool8 allocateImageIntegrator(ImageIntegratorInterface interface,
                              Scene* scene,
                              Sampler* sampler,
                              RayIntegrator* rayIntegrator,
                              Film* film,
                              Camera* camera,
                              ImageIntegrator** outIntegrator);

void destroyImageIntegrator(ImageIntegrator* integrator);

void imageIntegratorExecute(ImageIntegrator* integrator, float time = 0.0f);

void imageIntegratorSetScene(ImageIntegrator* integrator, Scene* scene);
void imageIntegratorSetSampler(ImageIntegrator* integrator, Sampler* sampler);
void imageIntegratorSetRayIntegrator(ImageIntegrator* integrator, RayIntegrator* rayIntegrator);
void imageIntegratorSetFilm(ImageIntegrator* integrator, Film* film);
void imageIntegratorSetCamera(ImageIntegrator* integrator, Camera* camera);

