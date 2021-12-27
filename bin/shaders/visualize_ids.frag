#version 450 core

#include defines.glsl

layout(location = 0) out float3 outDistanceColor;

layout(location = 0) uniform usampler2D idsMap;

float32 goldNoise(float2 xy, float32 seed){
  return fract(tan(distance(xy*1.61803398874989484820459, xy)*seed)*xy.x);
}

float3 generateColor(uint32 id)
{
  float2 xy = float2(id, id);
  return float3(goldNoise(xy, 5), goldNoise(xy, 6), goldNoise(xy, 7));
}

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    
    uint32 id = texelFetch(idsMap, ifragCoord, 0).r;
    
    outDistanceColor = generateColor(id);
}
