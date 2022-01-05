#version 450 core

#include geometry_common.glsl

layout(location = 0) out float32 outDistance;
layout(location = 1) out uint32 outGeometryID;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    GeometryData front = stackFront(ifragCoord);

    if(front.distance < params.intersectionThreshold)
    {
        outDistance = texelFetch(raysMap, ifragCoord, 0).w;
        outGeometryID = front.id;
    }
    else
    {
        outDistance = INF_DISTANCE;
        outGeometryID = UKNOWN_GEOMETRY_ID;
    }
}
