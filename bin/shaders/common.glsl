#ifndef COMMON_GLSL_INCLUDED
#define COMMON_GLSL_INCLUDED

  #include declarations.h

  layout(std140, binding = GLOBAL_PARAMS_UBO_BINDING) uniform GlobalParametersUBO
  {
    GlobalParameters params;
    LightSourceParameters lightParams[MAX_LIGHT_SOURCES_COUNT];
    MaterialParameters materials[MAX_MATERIALS];    
  };

  layout(std140, binding = GEOMETRY_TRANSFORMS_UBO_BINDING) uniform GeometryParametersUBO
  {
    GeometryParameters geo[MAX_GEOMETRIES];
  };

  float32 normalizeAndGetLength(inout float3 vector)
  {
    float32 len = length(vector);
    vector /= len;

    return len;
  }

  float32 calculateAttenuationRadius(float2 k, float32 minAttenuation)
  {
    float32 D = k.x * k.x * minAttenuation * minAttenuation - 4 * minAttenuation * k.y * (minAttenuation - 1);
    float32 r = (-minAttenuation * k.x + sqrt(D)) / (2 * minAttenuation * k.y);

    return r;
  }

  float2 fragCoordToUV(float2 fragCoord)
  {
    return float2(fragCoord.x * params.invGapResolution.x, fragCoord.y * params.invGapResolution.y);
  }

  float32 distance2(float3 from, float3 to)
  {
    float3 vector = to - from;
    return dot(vector, vector);
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
  float32 worldZToNDC(float32 z)
  {
    float32 w = params.camWorldNDCMat[2][3] * z + params.camWorldNDCMat[3][3];
    return (z * params.camWorldNDCMat[2][2] + params.camWorldNDCMat[3][2]) / w;
  }

  float32 cameraZToNDC(float32 z)
  {
    float32 w = params.camCameraNDCMat[2][3] * z + params.camCameraNDCMat[3][3];
    return (z * params.camCameraNDCMat[2][2] + params.camCameraNDCMat[3][2]) / w;
  }

  float32 NDCZToWorld(float32 z)
  {
    float32 w = params.camNDCWorldMat[2][3] * z + params.camNDCWorldMat[3][3];
    return (z * params.camNDCWorldMat[2][2] + params.camNDCWorldMat[3][2]) / w;
  }

  float32 NDCZToCamera(float32 z)
  {
    
    float32 w = params.camNDCCameraMat[2][3] * z + params.camNDCCameraMat[3][3];
    return (z * params.camNDCCameraMat[2][2] + params.camNDCCameraMat[3][2]) / w;
  }

  float3 getWorldPos(float2 uv, int2 ifragCoord, sampler2D depthMap)
  {
    float3 ndcPos = float3(uv * float2(-2.0, 2.0) + float2(1.0, -1.0), texelFetch(depthMap, ifragCoord, 0) * 2.0 - 1.0);
    float4 worldPos = params.camNDCWorldMat * float4(ndcPos, 1.0);

    return worldPos.xyz / worldPos.w;
  }



#endif
