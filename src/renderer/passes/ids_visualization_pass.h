#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_IDS_VISUALIZATION = 0x9c849394;

ENGINE_API bool8 createIDsVisualizationPass(RenderPass** outPass);

