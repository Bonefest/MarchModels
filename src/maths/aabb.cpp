#include "aabb.h"

const static float32 negInf = -std::numeric_limits<float32>::infinity();
const static float32 posInf = std::numeric_limits<float32>::infinity();

AABB::AABB() {}

AABB::AABB(float32 minX, float32 minY, float32 minZ,
           float32 maxX, float32 maxY, float32 maxZ):
  min(minX, minY, minZ),
  max(maxX, maxY, maxZ) { }

AABB::AABB(float3 inMin, float3 inMax):
  min(inMin), max(inMax) { }

AABB AABB::createUnbounded()
{
  return AABB(negInf, negInf, negInf,
              posInf, posInf, posInf);
}

AABB AABB::createWithCenter(float32 centerX, float32 centerY, float32 centerZ,
                            float32 width, float32 height, float32 depth)
{
  return AABB(centerX - width * 0.5f, centerY - height * 0.5f, centerZ - depth * 0.5f,
              centerX + width * 0.5f, centerY + height * 0.5f, centerZ + depth * 0.5f);
}

AABB AABB::createWithCenter(float3 center, float3 dimensions)
{
  return AABB::createWithCenter(center.x, center.y, center.z,
                                dimensions.x, dimensions.y, dimensions.z);
}

void AABB::translate(float3 translation)
{
  min += translation;
  max += translation;
}

AABB AABB::genTranslated(float3 translation)
{
  return AABB(min + translation, max + translation);
}

AABB AABB::genScaled(float3 scale)
{
  float3 center = getCenter();
  float3 dims = getDimensions();

  return createWithCenter(center, dims * scale);
}

void AABB::scale(float3 scale)
{
  *this = genScaled(scale);
}

void AABB::rotate(quat rotation)
{
  *this = genTransformed(rotation_matrix(rotation));
}

AABB AABB::genRotated(quat rotation)
{
  return genTransformed(rotation_matrix(rotation));
}

AABB AABB::genTransformed(float4x4 transformation)
{
  // NOTE: Move AABB to the origin
  float3 center = getCenter();
  AABB centered = genTranslated(-center);

  // NOTE: Transform each vertex, find the new most-bottom-near-left and most-top-right points
  float3 cMin = float3(), cMax = float3();
  for(uint32 i = 0; i < 8; i++)
  {
    float3 vertex = mul(transformation, getVertex(i));
    cMin.x = std::min(cMin.x, vertex.x);
    cMin.y = std::min(cMin.y, vertex.y);
    cMin.z = std::min(cMin.z, vertex.z);
    
    cMax.x = std::min(cMax.x, vertex.x);
    cMax.y = std::min(cMax.y, vertex.y);
    cMax.z = std::min(cMax.z, vertex.z);    
  }

  // NOTE: Generate new AABB, translate it to its initial position
  AABB transformedAABB = AABB(cMin, cMax);
  transformedAABB.translate(center);

  return transformedAABB;
}

void AABB::transform(float4x4 transformation)
{
  *this = genTransformed(transformation);
}

AABB AABB::operator|=(const AABB& aabb) { /** TODO */ }
AABB AABB::operator|(const AABB& aabb) const { /** TODO */ }

AABB AABB::operator&=(const AABB& aabb) { /** TODO */ }
AABB AABB::operator&(const AABB& aabb) const { /** TODO */ }

float3 AABB::getCenter() const
{
  return (min + max) * 0.5f;
}

float3 AABB::getVertex(uint32 i) const
{
  float3 dims = getDimensions();
  
  switch(i)
  {
  case 0: return min;
  case 1: return min + float3(0.0, 0.0, dims.z);
  case 2: return max - float3(0.0, dims.y, 0.0);
  case 3: return min + float3(dims.x, 0.0, 0.0);
  case 4: return min + float3(0.0, dims.y, 0.0);
  case 5: return max - float3(dims.x, 0.0, 0.0);
  case 6: return max;
  case 7: return max - float3(0.0, 0.0, dims.z);
  }

  assert(false);
  return float3();
}

float3 AABB::operator[](uint32 i) const
{
  return getVertex(i);
}

float32 AABB::getWidth() const
{
  return max.x - min.x;
}

float32 AABB::getHeight() const
{
  return max.y - min.y;
}
  
float32 AABB::getDepth() const
{
  return max.z - min.z;
}

float3 AABB::getDimensions() const
{
  return max - min;
}
  
