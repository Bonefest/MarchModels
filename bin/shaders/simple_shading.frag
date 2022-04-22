#version 450 core

#include pbr_common.glsl

layout(location = 0) out float4 outColor;

layout(location = 0) uniform uint32 lightsCount;
layout(location = 1) uniform sampler2D atlasTexture;
layout(location = 2) uniform usampler2D idTexture;
layout(location = 3) uniform sampler2D depthTexture;
layout(location = 4) uniform sampler2D normalsTexture;
layout(location = 5) uniform sampler2D shadowsTexture;

void main()
{
  int2 ifragCoord = int2(gl_FragCoord.xy);  
  float2 uv = fragCoordToUV(gl_FragCoord.xy);
  
  float3 worldPos = getWorldPos(uv, ifragCoord, depthTexture);
  float3 normal = texelFetch(normalsTexture, ifragCoord, 0).xyz;
  float4 shadows = texelFetch(shadowsTexture, ifragCoord, 0);

  bool isSurface = dot(normal, normal) > 0.1;
  
  float3 radiance = 0.0f.xxx;
  if(isSurface)
  {
    uint32 id = texelFetch(idTexture, ifragCoord, 0).r;
    MaterialParameters material = materials[geo[id].materialID];

    float3 objectPos = (geo[id].worldGeoMat * float4(worldPos, 1.0)).xyz;
    float3 view = normalize(params.camPosition.xyz - worldPos);    

    float3 diffuseColor = material.diffuseTextureEnabled == 1 ? psample(atlasTexture,
                                                                        objectPos,
                                                                        normal,
                                                                        material.diffuseTextureUVRect,
                                                                        material.projectionMode).rgb : material.diffuseColor.rgb;

    float4 mriao = material.mriaoTextureEnabled == 1 ? psample(atlasTexture,
                                                               objectPos,
                                                               normal,
                                                               material.mriaoTextureUVRect,
                                                               material.projectionMode) : material.mriao;
    

    radiance = simplifiedRenderingEquation(worldPos,
                                           normal,
                                           view,
                                           material.ambientColor.rgb,
                                           diffuseColor,
                                           mriao.y,
                                           mriao.w,
                                           shadows,
                                           lightsCount);
  }
  
  outColor = float4(radiance, isSurface);
}
