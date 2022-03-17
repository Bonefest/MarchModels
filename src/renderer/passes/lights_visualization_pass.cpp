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

    uint2 size = uint2(256, 256);
    uint2 offset = uint2((int32)type * size.x, 0);
    
    billboardSystemDrawImagePix(lightsImagesAtlas,
                                position,
                                size,
                                offset,
                                intensity,
                                float2(10.0f, 10.0f),
                                float3(),
                                TRUE);

    if(lightSourceIsSelected(light) == TRUE)
    {
      billboardSystemDrawImagePix(lightsImagesAtlas,
                                  position,
                                  uint2(64, 64),
                                  uint2(768, 128),
                                  float4(0, 1, 0, 1.0),
                                  float2(12.5f, 12.5f),
                                  float3(0.0f, 0.0f, 0.0f),
                                  TRUE);
    }    
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
