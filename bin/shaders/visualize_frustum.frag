#version 450 core

#include defines.glsl

layout(location = 0) out float3 outFrustumColor;

void main()
{
    outFrustumColor = float3(1.0, 0.0, 0.0);
}