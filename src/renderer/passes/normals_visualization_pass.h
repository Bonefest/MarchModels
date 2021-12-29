#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_NORMALS_VISUALIZATION = 0x367f593e;

ENGINE_API bool8 createNormalsVisualizationPass(RenderPass** outPass);
