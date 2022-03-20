#pragma once

static const RenderPassType RENDER_PASS_TYPE_SIMPLE_SHADING_PASS = 0xaff2a9a1;

ENGINE_API bool8 createSimpleShadingPass(RenderPass** outPass);

ENGINE_API void simpleShadingPassSetAmbientColor(RenderPass* pass, float3 ambientColor);
ENGINE_API float3 simpleShadingPassGetAmbinetColor(RenderPass* pass);
