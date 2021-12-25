#version 450 core

#include stack.glsl

out float4 outCameraRay;

uniform uint32 curIterIdx;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    float32 distance = getStack(ifragCoord).distances[0];

    bool notLastIteration = (curIterIdx + 1 < params.rasterItersMaxCount);
    if(notLastIteration)
    {
      stackClear(ifragCoord);
    }

    outCameraRay = float4(0, 0, 0, distance);
}

