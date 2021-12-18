#ifndef STACK_GLSL_INCLUDED
#define STACK_GLSL_INCLUDED

#include common.glsl

layout(std140, binding = STACKS_SSBO_BINDING) buffer StacksSSBO
{
    DistancesStack stacks[];
};

uint32 getStackID(uint2 pixelCoord)
{
  return pixelCoord.y * params.resolution.x + pixelCoord.x;
}

DistancesStack getStack(uint2 pixelCoord)
{
  return stacks[getStackID(pixelCoord)];
}

uint32 getStackSize(uint2 pixelCoord)
{
  return stacks[getStackID(pixelCoord)].size;
}

void stackPushDistance(int2 pixelCoord, float32 distance)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackSize = stacks[stackID].size;
  stacks[stackID].distances[stackSize] = distance;
  // TODO: stacks[stackID].geometry[stackSize] = GEOMETRY_ID;
  stacks[stackID].size = stackSize + 1;
}

float32 stackPopDistance(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackSize = stacks[stackID].size;
  stacks[stackID].size = stackSize - 1;

  return stacks[stackID].distances[stackSize - 1];
}

void stackClear(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  stacks[stackID].size = 0;
}

#endif