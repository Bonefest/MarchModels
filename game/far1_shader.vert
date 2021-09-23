#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

// uniform mat4x4 MVP;

vec3 vertices[3] = vec3[3](
  vec3(-0.5f, -0.25f, 0.0f),
  vec3( 0.0f,  0.25f, 0.0f),
  vec3( 0.5f, -0.25f, 0.0f)
);

void main()
{
  // gl_Position = MVP * vec4(inPosition, 1.0);
  gl_Position = vec4(vertices[gl_VertexID], 1.0);
}
