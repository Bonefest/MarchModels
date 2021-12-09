struct Stack
{
    uint32 length; // current length of the stack
    float32 distances[MAX_STACK_LENGTH]; // stores distances
    uint32 geometry[MAX_STACK_LENGTH]; // stores index of geometry corresponding to the distancesb
};

layout(std140, binding = 0) uniform DFParameters
{
    // Built-in parameters
    float32 time;
    float32 tone;
    uint32  pixelGapX;
    uint32  pixelGapY;
};

layout(std140, binding = 1) buffer Stacks
{
    Stack stacks[];
};

uniform sampler2D raysMap;

uniform uint2 resolution;
uniform float4x4 modelTransform;

uint32 getStackID(uint2 pixelCoord)
{
  return pixelCoord.y * resolution.x + pixelCoord.x;
}

Stack getStack(uint2 pixelCoord)
{
  return stacks[getStackID(pixelCoord)];
}

uint32 getStackLength(uint2 pixelCoord)
{
  stacks[getStackID(pixelCoord)].length;
}

void pushDistance(int2 pixelCoord, float32 distance)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackLen = stacks[stackID].length;
  stacks[stackID].distances[stackLen] = distance;
  stacks[stackID].geometry[stackLen] = GEOMETRY_ID;
  stacks[stackID].length = stackLen + 1;
}

float popDistance(int2 pixelCoord)
{
  uint32 stackID = getStackID(pixelCoord);
  uint32 stackLen = stacks[stackID].length;

  return stacks[stackID].distances[stackLen - 1];
}
