#if defined(ENABLE_TESTBED_GAME)

#include <vector>

#include <imgui/imgui.h>
#include <linalg/linalg.h>
#include <imgui/imgui_internal.h>

#include <film.h>
#include <logging.h>
#include <application.h>
#include <memory_manager.h>
#include <game_framework.h>

struct TestbedData
{
  Film* film;
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
  assert(createFilm(uint2(640, 480), &data.film));
  return TRUE;
}

void shutdown(Application* app)
{
  destroyFilm(data.film);
}

void update(Application* app, float64 delta)
{

}

void draw(Application* app, float64 delta)
{
  static uint32 counter = 0;
  for(uint32 y = 0; y < 100; y++)
  {
    for(uint32 x = 0; x < 100; x++)
    {
      counter++;
      float value = float(counter % 255) / 255.0f;
      filmSetPixel(data.film, int2(x + 100, y + 100), float3(value));
    }
  }
  
  ImGui::SetNextWindowSize(ImVec2(640.0f, 480.0f));
  ImGui::Begin("Test window");
  ImGui::Image((void*)filmGetGLTexture(data.film), ImVec2(640.0f, 480.0f));
  ImGui::End();
}

void processInput(Application* app, const EventData& eventData, void* sender)
{

}

#endif
