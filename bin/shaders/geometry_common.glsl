uniform sampler2D raysMap;
uniform float4x4 modelTransform;

layout(std140, binding = 1) buffer Stacks
{
    DistancesStack stacks[];
};

uint32 getStackID(uint2 pixelCoord)
{
  return pixelCoord.y * resolution.x + pixelCoord.x;
}

Stack getStack(uint2 pixelCoord)
{
  return stacks[getStackID(pixelCoord)];
}

uint32 getStackSize(uint2 pixelCoord)
{
  stacks[getStackID(pixelCoord)].size;
}

void pushDistance(int2 pixelCoord, float32 distance)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackSize = stacks[stackID].size;
  stacks[stackID].distances[stackSize] = distance;
  stacks[stackID].geometry[stackSize] = GEOMETRY_ID;
  stacks[stackID].length = stackSize + 1;
}

float32 popDistance(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackSize = stacks[stackID].size;

  return stacks[stackID].distances[stackSize - 1];
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

