#version 450 core

#include common.glsl

layout(location = 0) out float4 outFogColor;

layout(location = 0) uniform sampler2D depthMap;
layout(location = 1) uniform usampler2D idMap;
layout(location = 2) uniform uint32 fogType;
layout(location = 3) uniform float2 fogMinMax;
layout(location = 4) uniform float2 fogData;
layout(location = 5) uniform float3 fogColor;

float32 calculateLinearFog(float32 distance)
{
  const float32 near = fogData.x;
  const float32 far = fogData.y;
  
  return (distance - near) / (far - near);
}

float32 calculateExponentialFog(float32 distance)
{
  const float32 density = fogData.x;

  return 1.0f - exp(-density * distance);
}

float32 calculateExponential2Fog(float32 distance)
{
  const float32 density = fogData.x;

  return 1.0f - exp(-(density * density * distance * distance));
}

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    float32 fogIntensity = 0.0f;    
    bool isSurface = texelFetch(idMap, ifragCoord, 0).r != UNKNOWN_GEOMETRY_ID;
    
    if(isSurface)
    {
      const float32 fogMin = fogMinMax.x;
      const float32 fogMax = fogMinMax.y;
      
      float2 uv = fragCoordToUV(gl_FragCoord.xy);
      float3 worldPos = getWorldPos(uv, ifragCoord, depthMap);
      float3 cameraPos = params.camPosition.xyz;

      float32 distanceToCam = distance(worldPos, cameraPos);

      switch(fogType)
      {
        case FOG_TYPE_LINEAR: fogIntensity = calculateLinearFog(distanceToCam); break;
        case FOG_TYPE_EXPONENTIAL: fogIntensity = calculateExponentialFog(distanceToCam); break;
        case FOG_TYPE_EXPONENTIAL2: fogIntensity = calculateExponential2Fog(distanceToCam); break;                
      }

      fogIntensity = clamp(fogIntensity, fogMin, fogMax);
    }

    outFogColor = float4(fogColor, fogIntensity);
}
