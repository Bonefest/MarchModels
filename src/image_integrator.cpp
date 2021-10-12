#include "lua/lua_system.h"
#include "memory_manager.h"

#include "image_integrator.h"

struct ImageIntegrator
{
  Scene* scene;
  Sampler* sampler;
  RayIntegrator* rayIntegrator;
  Film* film;
  Camera* camera;

  uint2 pixelGap;
  uint2 initialOffset;
  
  void* internalData;
};

static void imageIntegratorSetupCommonScriptData(ImageIntegrator* integrator, float32 time)
{
  sol::state& lua = luaGetMainState();
  lua["args"]["time"] = time;
  // lua["args"]["camera"] = cameraGetLuaTable(integrator->camera);
}

static bool8 imageIntegratorShouldIntegratePixelLocation(ImageIntegrator* integrator, int2 loc)
{
  uint2 gap = integrator->pixelGap;
  uint2 offset = integrator->initialOffset;
  
  return (loc.x + offset.x) % gap.x == 0 && (loc.y + offset.y) % gap.y == 0;  
}

bool8 createImageIntegrator(Scene* scene,
                            Sampler* sampler,
                            RayIntegrator* rayIntegrator,
                            Film* film,
                            Camera* camera,
                            ImageIntegrator** outIntegrator)
{

  *outIntegrator = engineAllocObject<ImageIntegrator>(MEMORY_TYPE_GENERAL);
  ImageIntegrator* integrator = *outIntegrator;
  integrator->scene = scene;
  integrator->sampler = sampler;
  integrator->rayIntegrator = rayIntegrator;
  integrator->film = film;
  integrator->camera = camera;
  integrator->pixelGap = uint2(1, 1);
  integrator->initialOffset = uint2(0, 0);
  integrator->internalData = nullptr;
  
  return TRUE;
}

void destroyImageIntegrator(ImageIntegrator* integrator)
{
  engineFreeObject(integrator, MEMORY_TYPE_GENERAL);
}

void imageIntegratorExecute(ImageIntegrator* integrator, float32 time)
{
  imageIntegratorSetupCommonScriptData(integrator, time);
  
  uint2 filmSize = filmGetSize(integrator->film);
  for(int32 y = 0; y < filmSize.y; y++)
  {
    for(int32 x = 0; x < filmSize.x; x++)
    {
      int2 pixelLocation(x, y);

      float3 radiance(0.0f, 0.0f, 0.0f);
      if(imageIntegratorShouldIntegratePixelLocation(integrator, pixelLocation))
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

void imageIntegratorSetSize(ImageIntegrator* integrator, uint2 size)
{
  filmResize(integrator->film, size);
  cameraSetAspectRatio(integrator->camera, float32(size.x) / float32(size.y));
  samplerSetSampleAreaSize(integrator->sampler, size);
}

void imageIntegratorSetScene(ImageIntegrator* integrator, Scene* scene)
{
  integrator->scene = scene;
}

Scene* imageIntegratorGetScene(ImageIntegrator* integrator)
{
  return integrator->scene;
}
 
void imageIntegratorSetSampler(ImageIntegrator* integrator, Sampler* sampler)
{
  integrator->sampler = sampler;
}

Sampler* imageIntegratorGetSampler(ImageIntegrator* integrator)
{
  return integrator->sampler;
}

void imageIntegratorSetRayIntegrator(ImageIntegrator* integrator, RayIntegrator* rayIntegrator)
{
  integrator->rayIntegrator = rayIntegrator;
}

RayIntegrator* imageIntegratorGetRayIntegrator(ImageIntegrator* integrator)
{
  return integrator->rayIntegrator;
}

void imageIntegratorSetFilm(ImageIntegrator* integrator, Film* film)
{
  integrator->film = film;
}

Film* imageIntegratorGetFilm(ImageIntegrator* integrator)
{
  return integrator->film;
}
 
void imageIntegratorSetCamera(ImageIntegrator* integrator, Camera* camera)
{
  integrator->camera = camera;
}

void imageIntegratorSetPixelGap(ImageIntegrator* integrator, uint2 gap)
{
  integrator->pixelGap = gap + uint2(1, 1);
}

uint2 imageIntegratorGetPixelGap(ImageIntegrator* integrator)
{
  return integrator->pixelGap - uint2(1, 1);  
}

void imageIntegratorSetInitialOffset(ImageIntegrator* integrator, uint2 offset)
{
  integrator->initialOffset = offset;
}

uint2 imageIntegratorGetInitialOffset(ImageIntegrator* integrator)
{
  return integrator->initialOffset;
}

void imageIntegratorSetInternalData(ImageIntegrator* integrator, void* internalData)
{
  integrator->internalData = internalData;
}

void* imageIntegratorGetInternalData(ImageIntegrator* integrator)
{
  return integrator->internalData;
}
