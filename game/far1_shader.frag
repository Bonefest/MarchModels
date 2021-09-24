#version 330 core

in vec3 vertexNormal;
layout(location = 0) out vec3 outColor;

void main()
{
  outColor = vertexNormal * 0.5 + 0.5;
}
