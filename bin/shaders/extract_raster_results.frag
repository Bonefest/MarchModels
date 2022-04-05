#version 450 core

#include geometry_common.glsl

layout(location = 0) out uint32 outGeometryID;

void main()
{
  int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);
    
  if(!stackEmpty(ifragCoord))
  {
    GeometryData front = stackFront(ifragCoord);
    if(front.distance < params.intersectionThreshold)
    {

      float4 ray = texelFetch(raysMap, ifragCoord, 0);
      float3 worldPos = ray.xyz * ray.w + params.camPosition.xyz;
      float4 ndcPos = params.camWorldNDCMat * float4(worldPos, 1.0);
      ndcPos /= ndcPos.w;

      // NOTE: We need explicitly convert value in range [0, 1], in case
      // we are writing directly to the gl_FragDepth
      gl_FragDepth = ndcPos.z * 0.5 + 0.5;
      outGeometryID = front.id;

      return;
    }
  }
  
  gl_FragDepth = 1.0f;
  outGeometryID = UNKNOWN_GEOMETRY_ID;
}
