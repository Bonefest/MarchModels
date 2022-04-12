#version 450 core

#include common.glsl

layout(location = 0) out float3 outGridColor;

void main()
{
  outGridColor = float3(0.5f, 0.5f, 0.5f);
}
