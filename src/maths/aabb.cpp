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
  float3 cMin = float3(posInf, posInf, posInf);
  float3 cMax = float3(negInf, negInf, negInf);

  for(uint32 i = 0; i < 8; i++)
  {
    float3 vertex = mul(transformation, centered.getVertex(i));
    cMin.x = std::min(cMin.x, vertex.x);
    cMin.y = std::min(cMin.y, vertex.y);
    cMin.z = std::min(cMin.z, vertex.z);
    
    cMax.x = std::max(cMax.x, vertex.x);
    cMax.y = std::max(cMax.y, vertex.y);
    cMax.z = std::max(cMax.z, vertex.z);    
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

AABB AABB::operator|(const AABB& aabb) const
{
  return AABBUnion(*this, aabb);
}

AABB& AABB::operator|=(const AABB& aabb)
{
  *this = AABBUnion(*this, aabb);
  return *this;
}

AABB AABB::operator&(const AABB& aabb) const
{
  return AABBIntersection(*this, aabb);
}

AABB& AABB::operator&=(const AABB& aabb)
{
  *this = AABBIntersection(*this, aabb);
  return *this;
}

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

bool8 AABB::intersects(const AABB&& aabb) const
{
  return AABBIntersect(*this, aabb);
}

bool8 AABB::isUnbounded() const
{
  return min.x == negInf ||
         min.y == negInf ||
         min.z == negInf ||
         max.x == posInf ||
         max.y == posInf ||
         max.z == posInf;
}

AABB AABBCentered(float32 centerX, float32 centerY, float32 centerZ,
                  float32 width, float32 height, float32 depth)
{
  return AABB::createWithCenter(centerX, centerY, centerZ,
                                width, height, depth);
}

AABB AABBCentered(float3 center, float3 dimensions)
{
  return AABB::createWithCenter(center, dimensions);
}

AABB AABBUnion(const AABB& aabb1, const AABB& aabb2)
{
  float3 unionMin =
  {
    min(aabb1.min.x, aabb2.min.x),
    min(aabb1.min.y, aabb2.min.y),
    min(aabb1.min.z, aabb2.min.z),    
  };

  float3 unionMax =
  {
    max(aabb1.max.x, aabb2.max.x),
    max(aabb1.max.y, aabb2.max.y),
    max(aabb1.max.z, aabb2.max.z),    
  };

  return AABB(unionMin, unionMax);
}

AABB AABBIntersection(const AABB& aabb1, const AABB& aabb2)
{
  float3 interMin =
  {
    max(aabb1.min.x, aabb2.min.x),
    max(aabb1.min.y, aabb2.min.y),
    max(aabb1.min.z, aabb2.min.z),    
  };

  float3 interMax =
  {
    min(aabb1.max.x, aabb2.max.x),
    min(aabb1.max.y, aabb2.max.y),
    min(aabb1.max.z, aabb2.max.z),    
  };

  return AABB(interMin, interMax);
}

bool8 AABBIntersect(const AABB& aabb1, const AABB& aabb2)
{
  return aabb1.min.x < aabb2.max.x && // AABB1's Left-most   < AABB2's Right-most
         aabb1.min.y < aabb2.max.y && // AABB1's Bottom-most < AABB2's Top-most
         aabb1.min.z < aabb2.max.z && // AABB1's Near-most   < AABB2's Far-most
         aabb1.max.x > aabb2.min.x && // AABB1's Right-most  > AABB2's Left-most
         aabb1.max.y > aabb2.min.y && // AABB1's Top-most    > AABB2's Bottom-most
         aabb1.max.z > aabb2.min.z;   // AABB1's Far-most    > AABB2's Near-most    
}
