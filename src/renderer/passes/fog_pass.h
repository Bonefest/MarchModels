#pragma once

#include "render_pass.h"

static const RenderPassType RENDER_PASS_TYPE_FOG = 0x3ab32641;

ENGINE_API bool8 createFogPass(RenderPass** outPass);
