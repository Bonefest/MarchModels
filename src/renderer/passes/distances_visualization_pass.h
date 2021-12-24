#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_DISTANCES_VISUALIZATION = 0x9c849394;

ENGINE_API bool8 createDistancesVisualizationPass(float2 distancesRange,
                                                  float3 closestColor, float3 farthestColor,
                                                  RenderPass** outPass);

ENGINE_API float2 distancesVisualizationPassGetDistancesRange(RenderPass* pass);
ENGINE_API float3 distancesVisualizationPassGetClosestColor(RenderPass* pass);
ENGINE_API float3 distancesVisualizationPassGetFarthestColor(RenderPass* pass);
