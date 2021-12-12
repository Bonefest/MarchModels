#include common.glsl

uniform sampler2D raysMap;
uniform float4x4 modelTransform;

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

  return stacks[stackID].distances[stackSize - 1];
}

void stackClear(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  stacks[stackID].size = 0;
}

float32 unionDistances(float32 d1, float32 d2)
{
  return min(d1, d2);
}

float32 intersectDistances(float32 d1, float32 d2)
{
  return max(d1, d2);
}

float32 subtractDistances(float32 d1, float32 d2)
{
  return max(d1, -d2);
}

