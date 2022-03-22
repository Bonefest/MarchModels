#version 450 core
#extension GL_ARB_shader_stencil_export : require

#include stack.glsl

out float4 outCameraRay;

layout(location = 0) uniform sampler2D depthMap;
layout(location = 1) uniform sampler2D normalsMap;

layout(location = 2) uniform uint32 lightIndex;

void markFragmentAsCulled(int2 ifragCoord)
{
  gl_FragStencilRefARB = 0;
  outCameraRay = float4(0.0f);
  stackAddTotalDistance(ifragCoord, INF_DISTANCE);
}

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    stackClear(ifragCoord);

    float2 uv = fragCoordToUV(gl_FragCoord.xy);
    float3 worldPos = getWorldPos(uv, ifragCoord, depthMap);
    float3 normal = texture(normalsMap, uv).xyz;

    const LightSourceParameters light = lightParams[lightIndex];

    // Based on light type, precalculate light direction. Additionally perform early-quit tests:
    //   1) Check if light source is too far (based on its attenuation) to be kicked
    //   2) Check if point is out of light source cone (based on angle between light and light forward axis)
    //   3) Check if source is below horizon (i.e lambert factor is negative)
    float3 l;
    switch(light.type)
    {
      case LIGHT_SOURCE_TYPE_DIRECTIONAL:
      {
        l = light.forward.xyz;

        if(dot(l, normal) < 0.0)
        {
          markFragmentAsCulled(ifragCoord);
          return;
        }
      } break;
      
      case LIGHT_SOURCE_TYPE_SPOT:
      {
        l = light.position.xyz - worldPos;
        float32 lDistance = normalizeAndGetLength(l);
        float32 attRadius = calculateAttenuationRadius(light.attenuationDistanceFactors, LIGHT_MIN_ATTENUATION);
        
        if(lDistance > attRadius || dot(l, light.forward.xyz) < light.attenuationAngleFactors.y || dot(l, normal) < 0)
        {
          markFragmentAsCulled(ifragCoord);
          return;
        }
      } break;
      
      case LIGHT_SOURCE_TYPE_POINT:
      {
        l = light.position.xyz - worldPos;
        float32 lDistance = normalizeAndGetLength(l);
        float32 attRadius = calculateAttenuationRadius(light.attenuationDistanceFactors, LIGHT_MIN_ATTENUATION);        
        if(lDistance > attRadius || dot(l, normal) < 0.0)
        {
          markFragmentAsCulled(ifragCoord);
          return;
        }
        
      } break;
    }

    gl_FragStencilRefARB = 1;        
    outCameraRay = float4(l, 0.1);

}
