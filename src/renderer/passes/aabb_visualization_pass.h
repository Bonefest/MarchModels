#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_AABB_VISUALIZATION = 0xde16d448;

ENGINE_API bool8 createAABBVisualizationPass(RenderPass** outPass);
