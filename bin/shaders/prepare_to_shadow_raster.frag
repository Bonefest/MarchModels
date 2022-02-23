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

    const LightSourceParamters light = lightParams[lightIndex];

    // Based on light type, precalculate light direction. Additionally perform early-quit tests:
    //   1) Check if light source is too far (based on its attenuation) to be kicked
    //   2) Check if light point is out of light source cone (based on angle between normal and light direction)
    float3 l;
    switch(light.type)
    {
      case LIGHT_SOURCE_TYPE_DIRECTIONAL:
      {
        l = light.direction;

        if(dot(l, normal) < 0.0)
        {
          gl_FragStencilRefARB = 0;
          outCameraRay = float4(0.0f);
          return;
        }
      } break;
      
      case LIGHT_SOURCE_TYPE_SPOT:
      {
        l = light.position - worldPos;
        float32 lDistance = normalizeAndGetLength(l);
        float32 attRadius = calculateAttenuationRadius(light.attenuationDistanceFactors, LIGHT_MIN_ATTENUATION);
        
        if(lDistance > attRadius || dot(l, normal) < light.attenuationAngleFactors.y)
        {
          gl_FragStencilRefARB = 0;
          outCameraRay = float4(0.0f);
          return;
        }
      } break;
      
      case LIGHT_SOURCE_TYPE_POINT:
      {
        l = light.position - worldPos;
        float32 lDistance = normalizeAndGetLength(l);
        float32 attRadius = calculateAttenuationRadius(light.attenuationDistanceFactors, LIGHT_MIN_ATTENUATION);        
        if(lDistance > attRadius || dot(l, normal) < 0.0)
        {
          gl_FragStencilRefARB = 0;
          outCameraRay = float4(0.0f);
          return;
        }
        
      } break;
    }

    gl_FragStencilRefARB = 1;        
    outCameraRay = float4(l, 0.0);

}
