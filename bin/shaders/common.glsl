layout(std140, binding = 0) uniform GlobalParameters
{
    float32 time;
    float32 tone;
    uint32  pixelGapX;
    uint32  pixelGapY;
    uint32  resX;
    uint32  resY;

    float3  cameraPos;
    float3  cameraDir;
};
