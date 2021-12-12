#version 330 core

void main()
{
  gl_Position = vec4((gl_VertexID % 2) * 4 - 1, (gl_VertexID & 2) * 2 - 1, 0.5, 1.0);
}
