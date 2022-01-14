#pragma once

#include "common.h"

struct AABB
{
  AABB();
  
  AABB(float32 minX, float32 minY, float32 minZ,
       float32 maxX, float32 maxY, float32 maxZ);

  AABB(float3 min, float3 max);

  static AABB createUnbounded();
  static AABB createWithCenter(float32 centerX, float32 centerY, float32 centerZ,
                               float32 width, float32 height, float32 depth);

  static AABB createWithCenter(float3 center, float3 dimensions);
  
  void translate(float3 translation);
  AABB genTranslated(float3 translation);

  // NOTE: Scaling is applied relatively to its local center
  void scale(float3 scale);
  AABB genScaled(float3 scale);

  // NOTE: Rotation is applied relatively to its local center
  void rotate(quat rotation);
  AABB genRotated(quat rotation);

  // NOTE: Transformation is applied relatively to its local center
  void transform(float4x4 transformation);
  AABB genTransformed(float4x4 transformation);

  AABB& operator|=(const AABB& aabb);
  AABB operator|(const AABB& aabb) const;

  AABB& operator&=(const AABB& aabb);
  AABB operator&(const AABB& aabb) const;

  //   (max)         
  //       6--------------5
  //      /|             /|
  //     / |            / |
  //    /  |           /  |
  //   /   |          /   |
  //  /    |         /    |
  // 7--------------4     |                y    
  // |     2--------|-----1                ^     z
  // |    /         |    /                 |    /\
  // |   /          |   /                  |   /
  // |  /           |  /                   |  /
  // | /            | /                    | /
  // |/             |/         x           |/
  // 3--------------0          <-----------+
  //                 (min)
  
  float3 getCenter() const;
  
  float3 getVertex(uint32 i) const;
  float3 operator[](uint32 i) const;
  
  float32 getWidth() const;
  float32 getHeight() const;
  float32 getDepth() const;

  float3 getDimensions() const;

  bool8 intersects(const AABB&& aabb) const;
  bool8 isUnbounded() const;
  
  float3 min = float3();
  float3 max = float3();
};

AABB AABBCentered(float32 centerX, float32 centerY, float32 centerZ,
                  float32 width, float32 height, float32 depth);

AABB AABBCentered(float3 center, float3 dimensions);

AABB AABBUnion(const AABB& lop, const AABB& rop);
AABB AABBIntersection(const AABB& lop, const AABB& rop);
bool8 AABBIntersect(const AABB& lop, const AABB& rop);


