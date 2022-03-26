#version 450 core

#include common.glsl

layout(location = 0) out float3 outColor;

layout(location = 0) uniform uint32 lightsCount;
layout(location = 1) uniform float3 ambientColor;
layout(location = 2) uniform sampler2D depthTexture;
layout(location = 3) uniform sampler2D normalsTexture;
layout(location = 4) uniform sampler2D shadowsTexture;


float3 getLightDirection(uint32 lightIndex, float3 p, inout float32 lpDistance)
{
  if(lightParams[lightIndex].type == LIGHT_SOURCE_TYPE_DIRECTIONAL)
  {
    return -lightParams[lightIndex].forward;
  }

  lpDistance = distance(p, lightParams[lightIndex].position.xyz);
  return (lightParams[lightIndex].position.xyz - p) / lpDistance;
}

void main()
{
  int2 ifragCoord = int2(gl_FragCoord.xy);  
  float2 uv = fragCoordToUV(gl_FragCoord.xy);
  
  float3 worldPos = getWorldPos(uv, ifragCoord, depthTexture);
  float3 normal = texelFetch(normalsTexture, ifragCoord, 0).xyz;
  float4 shadows = texelFetch(shadowsTexture, ifragCoord, 0);

  float3 radiance = dot(normal, normal) > 0.1 ? ambientColor : 0.0f.xxx;
  for(uint32 i = 0; i < lightsCount; i++)
  {
    float32 lpDistance = 1.0f;
    float3 l = getLightDirection(i, worldPos, lpDistance);
    
    float2 attFactors = lightParams[i].attenuationDistanceFactors;
    float32 attenuation = 1.0f / (attFactors.x * lpDistance + attFactors.y * lpDistance * lpDistance);

    if(lightParams[i].type == LIGHT_SOURCE_TYPE_SPOT)
    {
      float2 angFactors = lightParams[i].attenuationAngleFactors;
      float32 t = max(dot(-l, lightParams[i].forward.xyz), 0.0);
      
      attenuation *= mix(0.0f, 1.0f, clamp((t - angFactors.y) / (angFactors.x - angFactors.y), 0, 1));
    }
    
    radiance += lightParams[i].intensity.rgb * max(dot(normal, l), 0.0) * attenuation * shadows[i % 4];
  }
  
  outColor = radiance;
}
