#ifndef COMMON_GLSL_INCLUDED
#define COMMON_GLSL_INCLUDED

  #include declarations.h

  layout(std140, binding = GLOBAL_PARAMS_UBO_BINDING) uniform GlobalParametersUBO
  {
    GlobalParameters params;
  };

  float2 fragCoordToUV(float2 fragCoord)
  {
    return float2(fragCoord.x * params.invResolution.x, fragCoord.y * params.invResolution.y);
  }

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


#endif
