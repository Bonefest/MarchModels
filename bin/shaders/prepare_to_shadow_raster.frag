#version 450 core
#extension GL_ARB_shader_stencil_export : require

#include geometry_common.glsl

out float4 outCameraRay;

layout(location = 0) uniform texture2D depthMap;
layout(location = 1) uniform texture2D normalsMap;

layout(location = 2) uniform uint32 lightIndex;

float3 getWorldPos(float2 uv)
{
  float3 ndcPos = float3(uv * float2(-2.0, 2.0) + float2(1.0, -1.0), texture(depth, uv) * 2.0 - 1.0);
  float4 worldPos = params.camNDCWorldMat * float4(ndcPos, 1.0);

  return worldPos.xyz / worldPos.w;
}

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    stackClear(ifragCoord);

    float2 uv = fragCoordToUV(gl_FragCoord);
    float32 worldPos = getWorldPos(uv);
    float3 normal = texture(normalsMap, uv);
    
    // TODO: Based on light type generate light direction
    // TODO: Based on light type generate initial coverage mask
    
    outCameraRay = float4(generateRayDir(uv), 0.0);
}
