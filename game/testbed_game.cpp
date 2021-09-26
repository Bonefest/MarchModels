#if defined(ENABLE_TESTBED_GAME)

#include <vector>

#include <imgui/imgui.h>
#include <linalg/linalg.h>
#include <imgui/imgui_internal.h>

#include <film.h>
#include <camera.h>
#include <sampler.h>
#include <logging.h>
#include <application.h>
#include <memory_manager.h>
#include <game_framework.h>

struct TestbedData
{
  Film* film;
  Sampler* centerSampler;
  Camera* camera;
  uint2 filmSize;
};

static TestbedData data;

static bool8 extractSetupConfig(Application* app,
                                uint32* outScreenWidth,
                                uint32* outScreenHeight,
                                const char** outName);

static bool8 initialize(Application* app);

static void shutdown(Application* app);
static void update(Application* app, float64 delta);
static void draw(Application* app, float64 delta);
static void processInput(Application* app, const EventData& eventData, void* sender);

bool8 initializeGameFramework(GameFramework* outFramework)
{
  outFramework->extractSetupConfig = extractSetupConfig;
  outFramework->initialize = initialize;
  outFramework->shutdown = shutdown;
  outFramework->update = update;
  outFramework->draw = draw;
  outFramework->processInput = processInput;

  return TRUE;
}

bool8 extractSetupConfig(Application* app,
                         uint32* outScreenWidth,
                         uint32* outScreenHeight,
                         const char** outName)
{
  *outScreenWidth = 1280;
  *outScreenHeight = 720;
  *outName = "Testbed";

  return TRUE;
}

bool8 initialize(Application* app)
{
  data.filmSize = uint2(640, 480);
  assert(createFilm(data.filmSize, &data.film));
  assert(createCenterSampler(data.filmSize, &data.centerSampler));
  assert(createPerspectiveCamera(float32(data.filmSize.x) / float32(data.filmSize.y),
                                 toRad(45.0f),
                                 1.0f, 100.0f,
                                 &data.camera));
  return TRUE;
}

void shutdown(Application* app)
{
  destroyFilm(data.film);
}

void update(Application* app, float64 delta)
{

}

static float sphereSDF(float4 sphere, float3 p)
{
  return length(p - swizzle<0, 1, 2>(sphere)) - sphere.w;
}

static float3 integrate(Ray ray)
{
  const float4 sphere = float4(0.0f, 0.0f, 5.0f, 1.0f);
  const float32 minLimit = 0.1f;
  const uint32 maxIters = 3;

  float3 p = ray.origin;
  float3 radiance;

  for(uint32 n = 0; n < maxIters; n++)
  {
    float minDistance = 64.0f;    
    minDistance = min(minDistance, sphereSDF(sphere, p));
    if(minDistance < minLimit)
    {
      radiance = float3(1.0f);
      break;
    }
    
    p += ray.direction * minDistance;
  }

  return radiance;
}

static void drawImage()
{
  for(uint32 y = 0; y < data.filmSize.y; y++)
  {
    for(uint32 x = 0; x < data.filmSize.x; x++)
    {
      int2 location(x, y);
      data.centerSampler->startSamplingPixel(data.centerSampler, location);

      float2 sampleNDCPos;
      float3 radiance;
      while(data.centerSampler->generateSample(data.centerSampler, sampleNDCPos))
      {
        // (SamplerIntegrator calls camera internally)
        // radiance += SamplerIntegrator::Render(sampleNDCPos);

        // TODO: Weight it somehow
        Ray ray = cameraGenerateWorldRay(data.camera, sampleNDCPos);
        radiance += integrate(ray);
      }

      filmSetPixel(data.film, location, radiance);
    }
  }
}

void draw(Application* app, float64 delta)
{
  drawImage();
  
  ImGui::SetNextWindowSize(ImVec2(640.0f, 480.0f));
  ImGui::Begin("Test window");
  ImGui::Image((void*)filmGetGLTexture(data.film), ImVec2(data.filmSize.x, data.filmSize.y));
  ImGui::End();
}

void processInput(Application* app, const EventData& eventData, void* sender)
{

}

#endif
