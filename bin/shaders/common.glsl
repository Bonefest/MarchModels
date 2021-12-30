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
    // uv [(0..1), (0..1)] -> ndc [(1..-1), (-1..1)] - note, we inverse x axis, because
    // in our coordinate system x is going left (so column 0 should have ndc.x = 1)
    float2 ndc = float2(1.0 - 2.0 * uv.x, uv.y * 2.0 - 1);

    // NDC point on near plane  
    float4 fullNNDC = float4(ndc.x, ndc.y, 0.0f, 1.0f);

    // NDC point on far plane
    float4 fullFNDC = float4(ndc.x, ndc.y, 1.0f, 1.0f);

    // Transform near ndc point back to world space
    float4 frustumNPoint = params.camNDCWorldMat * fullNNDC;
    frustumNPoint /= frustumNPoint.w;

    // Transform far ndc point back to world space
    float4 frustumFPoint = params.camNDCWorldMat * fullFNDC;
    frustumFPoint /= frustumFPoint.w;

    return normalize(frustumFPoint.xyz - frustumNPoint.xyz);
  }

  // NOTE: Convert given value to nonlinear space
  float3 gammaEncode(float3 color)
  {
    return pow(color, params.invGamma.rrr);
  }

  // NOTE: Convert given value to linear space
  float3 gammaDecode(float3 color)
  {
    return pow(color, params.gamma.rrr);
  }



#endif
