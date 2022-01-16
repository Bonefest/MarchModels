// ----------------------------------------------------------------------------
// Task: Based on given transform function (IDFs + SDF + ODFs), calculate size
// of an approximated conservative AABB volume.
//
// Key idea:
//   1. Send Resolution x Resolution rays in direction Z;
//      Send Resolution x Resolution rays in direction X;

//   2. If ray hits the surface and it's has the most-left/most-right/most-top/
//      most-bottom/most-near/most-far position - write the coordinate in the
//      buffer.
//
// Additional improvements:
//   1. [Implemented] Poisson sampling - Each draw call will use a bit different
//      start position for each ray, which will decrease the chance of missing
//      some high-frequent details.
//
//   2. [Not implemented] Narrowing range - Each draw call will narrow range
//      of start positions: e.g it's meaningless to send rays at positions lower
//      than current known maximal coordinate
//
//   3. [Implemented] Adaptive intersection threshold - The more poisson samples
//      are used, the more chance of intersection, the less intersection threshold
//      is applied.
//
// Problems:
//   1. [Potentially solved] Some extremely high-frequent details may be
//       missed --> The volume won't be fully conservative.
// 
//   2. [Potentially solved] Dimensions depend on each other: Infinitely thin,
//      inifinitely wide plane may have final AABB = (0, 0, 0), because all rays
//      will miss the plane.
// ----------------------------------------------------------------------------

#include fp_math.h

#define POISSON_DISK_12S_1R
#include poisson_samples.inc

layout(std140, binding = AABB_CALCULATION_SSBO_BINDING) buffer GeometryTransformParametersUBO
{
  AABBCalculationBufferParameters aabbParams;
};

layout(location = 0) uniform uint32 calculationIteration;

void main()
{
  const uint32 iterationsCount = 100;
  
  int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

  float2 offset = PoissonSamples[calculationIteration] / POISSON_DISK_RADIUS / params.worldSize;
  
  // [0, resolution.x) -> [-WORLD_SIZE * 0.5, WORLD_SIZE * 0.5]
  float2 rayStartPos = (ifragCoord.xy * params.invAABBCalculationResolution - 0.5) * params.worldSize + offset;

  // Point moving along z axis, XY coordinates are fixed
  float3 pXYSide = float3(rayStartPos, -WORLD_SIZE * 0.5);

  // Point moving along x axis, YZ coordinates are fixed
  float3 pYZSide = float3(-WORLD_SIZE * 0.5, rayStartPos.yx);

  bool pXYSideHasIntersection = false;
  bool pYZSideHasIntersection = false;  

  // Adaptive factor: samples in range [12, 52] -> threshold factor [1.2, 2] 
  float32 factor = (32 - 0.02 * POISSON_SAMPLES_COUNT) + 1.2;
  float32 threshold = params.intersectionThreshold * factor;
  
  for(uint32 i = 0; i < iterationsCount; i++)
  {
    if(!pXYSideHasIntersection)
    {
      float32 xySideDistance = transform(pXYSide);
      pXYSide.z += xySideDistance;
      
      pXYSideHasIntersection = xySideDistance < threshold;
    }

    if(!pYZSideHasIntersection)
    {
      float32 yzSideDistance = transform(pYZSide);
      pYZSide.x += yzSideDistance;

      pYZSideHasIntersection = yzSideDistance < threshold;
    }
  }

  if(pXYSideHasIntersection)
  {
    uint32 fpX = floatToFixedPoint(pXYSide.x);
    uint32 fpY = floatToFixedPoint(pXYSide.y);    
    
    atomicMin(aabbParams.min.x, fpX);
    atomicMax(aabbParams.max.x, fpX);

    atomicMin(aabbParams.min.y, fpY);
    atomicMax(aabbParams.max.y, fpY);    
  }

  if(pYZSideHasIntersection)
  {
    uint32 fpX = floatToFixedPoint(pYZSide.y);
    uint32 fpY = floatToFixedPoint(pYZSide.x);    
    
    atomicMin(aabbParams.min.z, fpZ);
    atomicMax(aabbParams.max.z, fpZ);

    atomicMin(aabbParams.min.y, fpY);
    atomicMax(aabbParams.max.y, fpY);    
  }  
}