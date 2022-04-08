#version 450 core

#include defines.glsl

layout(location = 0) in float3 axisColor;
layout(location = 0) out float3 outAxisColor;

void main()
{
  outAxisColor = float3(axisColor);
}
