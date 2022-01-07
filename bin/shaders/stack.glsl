#ifndef STACK_GLSL_INCLUDED
#define STACK_GLSL_INCLUDED

#include common.glsl

layout(std430, binding = STACKS_SSBO_BINDING) buffer StacksSSBO
{
  float32 _stacks[];
};

uint32 getStackID(uint2 pixelCoord)
{
  return pixelCoord.y * params.gapResolution.x + pixelCoord.x;
}

uint32 getStackIndex(uint2 pixelCoord)
{
  return getStackID(pixelCoord) * GEOMETRY_STACK_MEMBERS_COUNT;
}

GeometryData stackFront(uint2 pixelCoord)
{
  uint32 byteOffset = getStackIndex(pixelCoord);

  GeometryData data;
  
  data.distance = _stacks[byteOffset + 1];
  data.id = uint32(_stacks[byteOffset + 2]);

  return data;
}

GeometryData stackBack(uint2 pixelCoord)
{
  uint32 byteOffset = getStackIndex(pixelCoord);
  uint32 stackSize = uint32(_stacks[byteOffset]);
  
  GeometryData data;
  
  data.distance = _stacks[byteOffset + (stackSize - 1) * GEOMETRY_MEMBERS_COUNT + 1];
  data.id = uint32(_stacks[byteOffset + (stackSize - 1) * GEOMETRY_MEMBERS_COUNT + 2]);

  return data;
}

uint32 getStackSize(uint2 pixelCoord)
{
  uint32 byteOffset = getStackIndex(pixelCoord);
  return uint32(_stacks[byteOffset]);
}

void stackPushGeometry(uint2 pixelCoord, GeometryData geometry)
{
  uint32 byteOffset = getStackIndex(pixelCoord);
  uint32 stackSize = uint32(_stacks[byteOffset]);

  _stacks[byteOffset + stackSize * GEOMETRY_MEMBERS_COUNT + 1]  = geometry.distance;
  _stacks[byteOffset + stackSize * GEOMETRY_MEMBERS_COUNT + 2]  = geometry.id;

  _stacks[byteOffset] = stackSize + 1;
}

GeometryData stackPopGeometry(uint2 pixelCoord)
{
  GeometryData data = stackBack(pixelCoord);
  uint32 byteOffset = getStackIndex(pixelCoord);
  _stacks[byteOffset] -= 1.0f;
  
  return data;
}

void stackClear(uint2 pixelCoord)
{
  uint32 byteOffset = getStackIndex(pixelCoord);
  _stacks[byteOffset] = 0;
}

#endif
