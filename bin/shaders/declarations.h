#ifndef DECLARATIONS_H_INCLUDED
#define DECLARATIONS_H_INCLUDED

  #if !defined(__cplusplus)
    #include defines.glsl
  #else
    #include <maths/common.h>
  #endif

  #define GLOBAL_PARAMS_UBO_BINDING       0
  #define GEOMETRY_TRANSFORMS_UBO_BINDING 1
  #define STACKS_SSBO_BINDING             2
  #define AABB_CALCULATION_SSBO_BINDING   3

  #define MAX_GEOMETRIES                  64
  #define MAX_MATERIALS                   32

  #define MAX_LIGHT_SOURCES_COUNT         4
  #define MAX_STACK_SIZE                  8
  #define INF_DISTANCE                    77777.0
  #define INT_DISTANCE                    0.0001
  #define UNKNOWN_GEOMETRY_ID             65535
  #define UNKNOWN_MATERIAL_ID             65535
  // One geometry has two members (see GeometryData declaration)
  #define GEOMETRY_MEMBERS_COUNT 2
  // 4 bytes per member, 1 member for size, 2 members per array element,
  // 2 * MAX_STACK_SIZE members of array

  struct GeometryData
  {
    float32 distance;
    uint32 id;
  };

  // One geometry stack consists of
  //   1. one member for size
  //   2. one member for total traversed distance
  //   3. array of geometry (see GeometriesStack declaration)
  #define GEOMETRY_STACK_MEMBERS_COUNT (2 + GEOMETRY_MEMBERS_COUNT * MAX_STACK_SIZE)

  struct GeometriesStack
  {
    uint32  size;
    GeometryData geometries[MAX_STACK_SIZE];
  };

  #if !defined(__cplusplus)
    GeometryData createGeometryData(float32 distance, uint32 id)
    {
      GeometryData data;
      data.distance = distance;
      data.id = id;

      return data;
    }
  #endif

  struct GlobalParameters
  {
    float32  time;
    float32  tone;
    float32  gamma;
    float32  invGamma;

    float32  intersectionThreshold;
    float32  worldSize;
    uint32   lightsCount;
    uint32   shadowLightsCount;
    
    uint32   pixelGapX;
    uint32   pixelGapY;
    uint32   rasterItersMaxCount;
    uint32   _gap1;
    
    uint2    resolution;
    float2   invResolution;

    uint2    gapResolution;
    float2   invGapResolution;
    
    float4   camPosition;
    quat     camOrientation;
    float4   camFwdAxis;
    float4   camSideAxis;
    
    float4   camUpAxis;
    float4   camMisc; // x=Near, y=Far, z=FovX, w=FovY
    float4   _gap3;
    float4   _gap4;
    
    float4x4 camNDCCameraMat;
    float4x4 camCameraNDCMat;  
    float4x4 camNDCWorldMat;
    float4x4 camWorldNDCMat;
    float4x4 camCameraWorldMat;
    float4x4 camWorldCameraMat;
  };

  struct GeometryTransformParameters
  {
    float4   position;
    float4   scale;
    float4x4 geoWorldMat;
    float4x4 worldGeoMat;
    float4x4 geoParentMat;
    float4x4 parentGeoMat;
  };

  struct GeometryParameters
  {
    uint32 materialID;
    uint32 _gap1;
    uint32 _gap2;
    uint32 _gap3;    
    
    float4   position;
    float4   scale;
    float4x4 geoWorldMat;
    float4x4 worldGeoMat;
    float4x4 geoParentMat;
    float4x4 parentGeoMat;
  };

  #define MATERIAL_TEXTURE_PROJECTION_MODE_TRIPLANAR   0
  #define MATERIAL_TEXTURE_PROJECTION_MODE_SPHERICAL   1
  #define MATERIAL_TEXTURE_PROJECTION_MODE_CYLINDRICAL 2

  #define MATERIAL_TEXTURE_TYPE_DIFFUSE  0
  #define MATERIAL_TEXTURE_TYPE_SPECULAR 1
  #define MATERIAL_TEXTURE_TYPE_BUMP     2
  #define MATERIAL_TEXTURE_TYPE_MRIAO    3
  #define MATERIAL_TEXTURE_TYPE_COUNT    4

  #if defined(__cplusplus)
    using MaterialTextureProjectionMode = uint32;
    using MaterialTextureType = uint32;
  #endif

  struct MaterialTextureParameters
  {
    uint32 enabled;
    float32 blendingFactor;
    float32 _gap1;
    float32 _gap2;    
    
    float4 defaultValue;
    float4 uvRect;
  };

  struct MaterialParameters
  {
    uint32 projectionMode;
    uint32 _gap1;
    uint32 _gap2;
    uint32 _gap3;

    float4 ambientColor;
    float4 emissionColor;
    float4 _gap4;
    float4 _gap5;    
    
    MaterialTextureParameters textures[MATERIAL_TEXTURE_TYPE_COUNT];
  };

  struct AABBCalculationBufferParameters
  {
    uint4 min;
    uint4 max;
  };

  #define LIGHT_SOURCE_TYPE_DIRECTIONAL 0
  #define LIGHT_SOURCE_TYPE_SPOT        1
  #define LIGHT_SOURCE_TYPE_POINT       2
  #define LIGHT_MIN_ATTENUATION     0.001

  #if defined(__cplusplus)
    using LightSourceType = uint32;
  #endif

  struct LightSourceParameters
  {
    uint32  type;
    uint32  enabled;
    uint32  shadowEnabled;
    float32 shadowFactor;    

    // x = linear attenuation, y = quadratic attenuation
    float2  attenuationDistanceFactors;

    // x = cos(inner angle), y = cos(outer angle)
    float2  attenuationAngleFactors;

    float4  _gap1;
    float4  _gap2;
    
    float4  position;
    float4  forward;
    float4  intensity;
    float4  attenuation;
    
  };

  #define FOG_TYPE_LINEAR       0
  #define FOG_TYPE_EXPONENTIAL  1
  #define FOG_TYPE_EXPONENTIAL2 2

  #if defined(__cplusplus)
    using FogType = uint32;
  #endif

#endif
