#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_UI_WIDGETS_VISUALIZATION = 0xde16d448;

enum AABBVisualizationMode
{
  AABB_VISUALIZATION_MODE_DYNAMIC,
  AABB_VISUALIZATION_MODE_FINAL
};

ENGINE_API bool8 createUIWidgetsVisualizationPass(RenderPass** outPass);

ENGINE_API void uiWidgetsVisualizationPassSetAABBVisualizationMode(RenderPass* pass, AABBVisualizationMode mode);
ENGINE_API AABBVisualizationMode uiWidgetsVisualizationPassGetAABBVisualizationMode(RenderPass* pass);

ENGINE_API void uiWidgetsVisualizationPassSetShowAABBParents(RenderPass* pass, bool8 showParents);
ENGINE_API bool8 uiWidgetsVisualizationPassShowAABBParents(RenderPass* pass);
