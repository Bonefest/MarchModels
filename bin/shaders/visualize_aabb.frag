#version 450 core

#include defines.glsl

layout(location = 0) in float3 aabbColor;
layout(location = 0) out float3 outAABBColor;

void main()
{
  outAABBColor = float3(aabbColor);
}
