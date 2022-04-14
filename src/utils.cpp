#include "utils.h"

float4 calculateUVRect(uint2 imageSize, uint2 offset, uint2 size)
{
  float2 invSize = float2(1.0f / float32(imageSize.x), 1.0f / float32(imageSize.y));
  
  float2 uvMin = float2(float32(offset.x) * invSize.x, float32(offset.y) * invSize.y);
  float2 uvMax = float2(float32(size.x) * invSize.x, float32(size.y) * invSize.y) + uvMin;
  
  return float4(uvMin.x, uvMin.y, uvMax.x, uvMax.y);
}
