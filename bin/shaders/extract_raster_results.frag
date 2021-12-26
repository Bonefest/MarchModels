#version 450 core

#include geometry_common.glsl

layout(location = 0) out float32 outDistance;
layout(location = 1) out uint32 outGeometryID;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    GeometriesStack stack = getStack(ifragCoord);

    if(stack.geometries[0].distance < params.intersectionThreshold)
    {
        outDistance = texelFetch(raysMap, ifragCoord, 0).w;
        outGeometryID = stack.geometries[0].id;
    }
    else
    {
        outDistance = INF_DISTANCE;
        outGeometryID = UKNOWN_GEOMETRY_ID;
    }
}
