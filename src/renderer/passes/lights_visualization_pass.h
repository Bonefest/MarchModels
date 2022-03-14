#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_LIGHTS_VISUALIZATION = 0x491abf31;

ENGINE_API bool8 createLightsVisualizationPass(RenderPass** outPass);
