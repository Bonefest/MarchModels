#include "memory_manager.h"

#include "image_integrator.h"

struct ImageIntegrator
{
  ImageIntegratorInterface interface;
  
  Scene* scene;
  Sampler* sampler;
  RayIntegrator* rayIntegrator;
  Film* film;
  Camera* camera;

  void* internalData;
};

bool8 allocateImageIntegrator(ImageIntegratorInterface interface,
                              Scene* scene,
                              Sampler* sampler,
                              RayIntegrator* rayIntegrator,
                              Film* film,
                              Camera* camera,
                              ImageIntegrator** outIntegrator)
{

  *outIntegrator = engineAllocObject<ImageIntegrator>(MEMORY_TYPE_GENERAL);
  ImageIntegrator* integrator = *outIntegrator;
  integrator->interface = interface;
  integrator->scene = scene;
  integrator->sampler = sampler;
  integrator->rayIntegrator = rayIntegrator;
  integrator->film = film;
  integrator->camera = camera;
  integrator->internalData = nullptr;
  
  return TRUE;
}

void destroyImageIntegrator(ImageIntegrator* integrator)
{
  integrator->interface.destroy(integrator);

  engineFreeObject(integrator, MEMORY_TYPE_GENERAL);
}

void imageIntegratorExecute(ImageIntegrator* integrator, float32 time)
{
  uint2 filmSize = filmGetSize(integrator->film);
  for(int32 y = 0; y < filmSize.y; y++)
  {
    for(int32 x = 0; x < filmSize.x; x++)
    {
      int2 pixelLocation(x, y);

      float3 radiance(0.0f, 0.0f, 0.0f);
      if(integrator->interface.shouldIntegratePixelLocation(pixelLocation))
      {
        samplerStartSamplingPixel(integrator->sampler, pixelLocation);

        Sample sample;
        while(samplerGenerateSample(integrator->sampler, sample))
        {
          Ray viewRay = cameraGenerateWorldRay(integrator->camera, sample.ndc);
          
          float3 sampleRadiance = rayIntegratorCalculateRadiance(integrator->rayIntegrator,
                                                                 viewRay,
                                                                 integrator->scene,
                                                                 time);

          radiance += sampleRadiance * sample.weight;
        }
      }

      filmSetPixel(integrator->film, pixelLocation, radiance);
    }
  }
}

void imageIntegratorSetScene(ImageIntegrator* integrator, Scene* scene)
{
  integrator->scene = scene;
}
 
void imageIntegratorSetSampler(ImageIntegrator* integrator, Sampler* sampler)
{
  integrator->sampler = sampler;
}
 
void imageIntegratorSetRayIntegrator(ImageIntegrator* integrator, RayIntegrator* rayIntegrator)
{
  integrator->rayIntegrator = rayIntegrator;
}
 
void imageIntegratorSetFilm(ImageIntegrator* integrator, Film* film)
{
  integrator->film = film;
}
 
void imageIntegratorSetCamera(ImageIntegrator* integrator, Camera* camera)
{
  integrator->camera = camera;
}
