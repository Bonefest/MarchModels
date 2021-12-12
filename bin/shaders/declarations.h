#define MAX_STACK_SIZE 8

struct DistancesStack
{
  uint32  size;
  float32 distances[MAX_STACK_SIZE];
  uint32  geometry[MAX_STACK_SIZE]; // stores index of geometry corresponding to the distances
};

struct GlobalParameters
{
  float32  time;
  float32  tone;
  uint32   pixelGapX;
  uint32   pixelGapY;
  
  uint32   resX;
  uint32   resY;
  uint2    _pad0;

  float4   camPosition;
  quat     camOrientation;
  float4x4 camNDCCameraMat;
  float4x4 camCameraNDCMat;  
  float4x4 camNDCWorldMat;
  float4x4 camWorldNDCMat;
  float4x4 camCameraWorldMat;
  float4x4 camWorldCameraMat;
};
