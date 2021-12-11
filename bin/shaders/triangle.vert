#version 330 core

void main()
{
  gl_Position = vec3((gl_vertexID % 2) * 4 - 2, (gl_VertexID & 2) * 2 - 2, 0.5);
}
