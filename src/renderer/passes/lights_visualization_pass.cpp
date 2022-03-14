#include "image_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"
#include "renderer/billboard_system.h"

#include "passes_common.h"
#include "lights_visualization_pass.h"

static void destroyLightsVisualizationPass(RenderPass* pass) { }


static bool8 lightsVisualizationPassExecute(RenderPass* pass)
{
  std::vector<AssetPtr>& lightSources = sceneGetLightSources(rendererGetPassedScene());
  ImagePtr lightsImagesAtlas = imageManagerLoadImage("assets/lights_sprites.png");

  for(AssetPtr light: lightSources)
  {
    LightSourceType type = lightSourceGetType(light);
    float4 intensity = lightSourceGetIntensity(light);
    float3 position = lightSourceGetPosition(light);

    float2 uvSize = float2(0.25f, 1.0f);
    float2 uvOffset = float2((int32)type * uvSize.x, 0.0f);

    billboardSystemDrawImage(lightsImagesAtlas,
                             position,
                             float2(1.0f, 1.0f),
                             intensity,
                             uvOffset,
                             uvOffset + uvSize);
  }

  return TRUE;
}

static const char* lightsVisualizationPassGetName(RenderPass* pass)
{
  return "LightsVisualizationPass";
}

bool8 createLightsVisualizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyLightsVisualizationPass;
  interface.execute = lightsVisualizationPassExecute;
  interface.getName = lightsVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_LIGHTS_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }
  
  return TRUE;
}
