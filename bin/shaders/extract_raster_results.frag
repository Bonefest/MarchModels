#version 450 core

#include geometry_common.glsl

layout(location = 0) out uint32 outGeometryID;

void main()
{
  int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

  GeometryData front = stackFront(ifragCoord);
    
  if(front.distance < params.intersectionThreshold)
  {
    gl_FragDepth = cameraDistanceToNDC(texelFetch(raysMap, ifragCoord, 0).w);
    outGeometryID = front.id;
  }
  else
  {
    gl_FragDepth = 1.0f;
    outGeometryID = UKNOWN_GEOMETRY_ID;
  }
}
