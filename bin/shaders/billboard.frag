#version 450 core

#include defines.glsl

// Input attributes
layout(location = 0) in float4 billboardColor;
layout(location = 1) in float2 uv;

// Output attributes
layout(location = 0) out float4 outColor;

// Uniforms
layout(location = 2) uniform sampler2D billboard;

void main()
{
  outColor = texture(billboard, uv) * billboardColor;
}
