#pragma once

#include "image.h"
#include "defines.h"

ENGINE_API bool8 initializeBillboardSystem();
ENGINE_API void shutdownBillboardSystem();

/**
 * @param offset = (NDC offset X, NDC offset Y, Camera Space offset Z)
 */
ENGINE_API void billboardSystemDrawImage(ImagePtr image,
                                         float3 worldPosition,
                                         float2 uvMin = float2(0, 0),
                                         float2 uvMax = float2(1, 1),
                                         float4 color = float4(1, 1, 1, 1),                                         
                                         float2 scale  = float2(1, 1),
                                         float3 offset = float3(0.0f, 0.0f, 0.0f),
                                         bool8 usePainterOrder = FALSE);

ENGINE_API void billboardSystemDrawImagePix(ImagePtr image,
                                            float3 worldPosition,
                                            uint2 pixelSize,
                                            uint2 pixelOffset = uint2(0, 0),
                                            float4 color = float4(1, 1, 1, 1),
                                            float2 scale = float2(1, 1),
                                            float3 offset = float3(0.0f, 0.0f, 0.0f),
                                            bool8 usePainterOrder = FALSE);

ENGINE_API void billboardSystemPresent();
