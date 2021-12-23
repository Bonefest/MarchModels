#version 450 core

#include geometry_common.glsl

layout(location = 0) out float32 outDistance;
layout(location = 1) out uint32 outGeometryID;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    DistancesStack stack = getStack(ifragCoord);
    
    outDistance = stack.distances[0];
    outGeometryID = stack.geometry[0];
}
