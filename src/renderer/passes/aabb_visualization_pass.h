#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_AABB_VISUALIZATION = 0xde16d448;

enum AABBVisualizationMode
{
  AABB_VISUALIZATION_MODE_DYNAMIC,
  AABB_VISUALIZATION_MODE_FINAL
};

ENGINE_API bool8 createAABBVisualizationPass(RenderPass** outPass);

ENGINE_API void aabbVisualizationPassSetVisualizationMode(RenderPass* pass, AABBVisualizationMode mode);
ENGINE_API AABBVisualizationMode aabbVisualizationPassGetVisualizationMode(RenderPass* pass);

ENGINE_API void aabbVisualizationPassSetShowParents(RenderPass* pass, bool8 showParents);
ENGINE_API bool8 aabbVisualizationPassShowParents(RenderPass* pass);
