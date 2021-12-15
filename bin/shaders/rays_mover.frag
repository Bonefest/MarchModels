#version 450 core

#include stack.glsl

out float4 outCameraRay;

void main()
{
    int2 ifragCoord = int2(gl_FragCoord.x, gl_FragCoord.y);

    float32 distance = stackPopDistance(ifragCoord);
    stackClear(ifragCoord);

    outCameraRay = float4(0, 0, 0, distance);
}

