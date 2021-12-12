#version 450 core

#include geometry_common.glsl

out vec4 outCameraRay;

float3 generateRayDir(float2 uv)
{
  float2 ndc = uv * 2.0f - 1.0f;
  
  // NDC point on near plane  
  float4 fullNNDC = float4(ndc.x, ndc.y, 0.0f, 1.0f);

  // NDC point on far plane
  float4 fullFNDC = float4(ndc.x, ndc.y, 1.0f, 1.0f);

  // Transform near ndc point back to camera frustum
  float4 frustumNPoint = params.camNDCCameraMat * fullNNDC;
  frustumNPoint /= frustumNPoint.w;

  // Transform far ndc point back to camera frustum
  float4 frustumFPoint = params.camNDCCameraMat * fullFNDC;
  frustumFPoint /= frustumFPoint.w;
  
  return normalize(frustumFPoint.xyz - frustumNPoint.xyz);
}

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    stackClear(ifragCoord);

    // TODO: Now we generate a ray going through a center of a pixel. In future
    // we may want to use some sort of src/samplers/sampler.h
    float2 uv = float2(gl_FragCoord.x * params.invResolution.x, gl_FragCoord.y * params.invResolution.y);

    // TODO: Somehow generate stencil mask here (if possible with GL_ARB_shader_stencil_export)
    
    outCameraRay = float4(generateRayDir(uv), 0.0);
}
