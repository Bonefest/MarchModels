#include "material.h"

uint32 makeTexHandle(uint32 index, uint32 x, uint32 y)
{
  return (index & 0xFF) & ((x & 0xFF) << 8) & ((y & 0xFF) << 8);
}

uint3 extractTexHandle(uint32 handle)
{
  return uint3(handle & 0xFF00, handle & 0xFF000, handle & 0xFF);
}

struct Material
{
  uint32 diffuseTexHandle;
  uint32 specularTexHandle;
  uint32 bumpTexHandle;
  uint32 mriaoTexHandle;  

  float32 ior;
  float32 ao;
  float32 metallic;
  float32 roughness;
  
  float4 ambientColor;
  float4 diffuseColor;
  float4 specularColor;
  float4 emissionColor;
};
