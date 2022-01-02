#ifndef STACK_GLSL_INCLUDED
#define STACK_GLSL_INCLUDED

#include common.glsl

layout(std140, binding = STACKS_SSBO_BINDING) buffer StacksSSBO
{
    GeometriesStack stacks[];
};

uint32 getStackID(uint2 pixelCoord)
{
  return pixelCoord.y * params.gapResolution.x + pixelCoord.x;
}

GeometriesStack getStack(uint2 pixelCoord)
{
  return stacks[getStackID(pixelCoord)];
}

uint32 getStackSize(uint2 pixelCoord)
{
  return stacks[getStackID(pixelCoord)].size;
}

void stackPushGeometry(int2 pixelCoord, GeometryData geometry)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackSize = stacks[stackID].size;

  stacks[stackID].geometries[stackSize] = geometry;
  stacks[stackID].size = stackSize + 1;
}

GeometryData stackPopGeometry(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackSize = stacks[stackID].size;
  stacks[stackID].size = stackSize - 1;

  return stacks[stackID].geometries[stackSize - 1];
}

void stackClear(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  stacks[stackID].size = 0;
}

#endif
