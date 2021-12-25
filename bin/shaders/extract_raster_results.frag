#version 450 core

#include geometry_common.glsl

layout(location = 0) out float32 outDistance;
layout(location = 1) out uint32 outGeometryID;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    DistancesStack stack = getStack(ifragCoord);

    if(stack.distances[0] < INTERSECTION_THRESHOLD)
    {
        outDistance = texelFetch(raysMap, ifragCoord, 0).w;
        outGeometryID = stack.geometry[0];
    }
    else
    {
        outDistance = INF_DISTANCE;
        outGeometryID = UKNOWN_GEOMETRY_ID;
    }
}
