layout(std140, binding = AABB_CALCULATION_SSBO_BINDING) buffer GeometryTransformParametersUBO
{
  AABBCalculationBufferParameters aabbParams;
};

void main()
{
  const uint32 iterationsCount = 100;
  
  int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

  // [0, resolution.x) -> [-WORLD_SIZE * 0.5, WORLD_SIZE * 0.5]
  float2 rayStartPos = (ifragCoord.xy * params.invAABBCalculationResolution - 0.5) * params.worldSize;

  // Point moving along z axis, XY coordinates are fixed
  float3 pXYSide = float3(rayStartPos, -WORLD_SIZE * 0.5);

  // Point moving along x axis, YZ coordinates are fixed
  float3 pYZSide = float3(-WORLD_SIZE * 0.5, rayStartPos.yx);

  bool pXYSideHasIntersection = false;
  bool pYZSideHasIntersection = false;  
  
  for(uint32 i = 0; i < iterationsCount; i++)
  {
    if(!pXYSideHasIntersection)
    {
      float32 xySideDistance = transform(pXYSide);
      pXYSide.z += xySideDistance;
      
      pXYSideHasIntersection = xySideDistance < params.intersectionThreshold;
    }

    if(!pYZSideHasIntersection)
    {
      float32 yzSideDistance = transform(pYZSide);
      pYZSide.x += yzSideDistance;

      pYZSideHasIntersection = yzSideDistance < params.intersectionThreshold;
    }
  }

  if(pXYSideHasIntersection)
  {
    atomicMin(aabbParams.minPixelCoords.x, ifragCoord.x);
    atomicMax(aabbParams.maxPixelCoords.x, ifragCoord.x);

    atomicMin(aabbParams.minPixelCoords.y, ifragCoord.y);
    atomicMax(aabbParams.maxPixelCoords.y, ifragCoord.y);    
  }

  if(pYZSideHasIntersection)
  {
    atomicMin(aabbParams.minPixelCoords.z, ifragCoord.x);
    atomicMax(aabbParams.maxPixelCoords.z, ifragCoord.x);

    atomicMin(aabbParams.minPixelCoords.y, ifragCoord.y);
    atomicMax(aabbParams.maxPixelCoords.y, ifragCoord.y);    
  }  
}
