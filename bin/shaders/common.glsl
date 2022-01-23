#ifndef COMMON_GLSL_INCLUDED
#define COMMON_GLSL_INCLUDED

  #include declarations.h

  layout(std140, binding = GLOBAL_PARAMS_UBO_BINDING) uniform GlobalParametersUBO
  {
    GlobalParameters params;
  };

  float2 fragCoordToUV(float2 fragCoord)
  {
    return float2(fragCoord.x * params.invGapResolution.x, fragCoord.y * params.invGapResolution.y);
  }

  float3 generateRayDir(float2 uv)
  {
    // uv [(0..1), (0..1)] -> ndc [(1..-1), (-1..1)] - note, we inverse x axis, because
    // in our coordinate system x is going left (so column 0 should have ndc.x = 1)
    float2 ndc = float2(1.0 - 2.0 * uv.x, uv.y * 2.0 - 1);

    // NDC point on near plane  
    float4 fullNNDC = float4(ndc.x, ndc.y, -1.0f, 1.0f);

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

  // NOTE: The next four functions perform multiplication by <un>projection matrix explicitly
  // (only z, w components are needed) and perform in place perspective division.
  float32 worldDistanceToNDC(float32 distance)
  {
    float32 w = params.camWorldNDCMat[2][3] * distance + params.camWorldNDCMat[3][3];
    return (distance * params.camWorldNDCMat[2][2] + params.camWorldNDCMat[3][2]) / w;
  }

  float32 cameraDistanceToNDC(float32 distance)
  {
    float32 w = params.camCameraNDCMat[2][3] * distance + params.camCameraNDCMat[3][3];
    return (distance * params.camCameraNDCMat[2][2] + params.camCameraNDCMat[3][2]) / w;
  }

  float32 NDCDistanceToWorld(float32 distance)
  {
    float32 w = params.camNDCWorldMat[2][3] * distance + params.camNDCWorldMat[3][3];
    return (distance * params.camNDCWorldMat[2][2] + params.camNDCWorldMat[3][2]) / w;
  }

  float32 NDCDistanceToCamera(float32 distance)
  {
    
    float32 w = params.camNDCCameraMat[2][3] * distance + params.camNDCCameraMat[3][3];
    return (distance * params.camNDCCameraMat[2][2] + params.camNDCCameraMat[3][2]) / w;
  }


#endif
