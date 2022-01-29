#pragma once

#include "common.h"

// ----------------------------------------------------------------------------
// Ray
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Plane
// ----------------------------------------------------------------------------

/** 
 * Represents a plane nx * x + ny * y + nz * z + d = 0 in form:
 *     data.xyz - normal direction
 *     data.w   - distance across normal
 *
 * x + 1 * y + 0 * z + (-2) = 0, represented as:
 *     data.xyz = float3(0, 1, 0)
 *     data.w = -2
 */
struct Plane
{
  Plane() = default;
  Plane(const float4& normalAndDistance): data(normalAndDistance) {} 
  Plane(const float3& normal, float32 distance): data(normal, distance) {}
  
  inline float3 getNormal() const
  {
    return data.xyz();
  }

  inline float32 getDistance() const
  {
    return data.w;
  }

  inline float32 getDistanceToPoint(float3 point) const
  {
    return dot(point, data.xyz()) + data.w;
  }
    
  float4 data;
};

ENGINE_API Plane createPlane(const float4& normalAndDistance);
ENGINE_API Plane createPlane(const float3& normal, float32 distance);
ENGINE_API Plane createPlane(const float3& p1, const float3& p2, const float3& p3);

ENGINE_API float32 planeGetDistanceToPoint(Plane plane, float3 p);

// ----------------------------------------------------------------------------
// AABB
// ----------------------------------------------------------------------------
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

  float32 getVolume() const;

  bool8 intersects(const AABB&& aabb) const;
  bool8 isUnbounded() const;
  
  float3 min = float3();
  float3 max = float3();
};

ENGINE_API AABB AABBCentered(float32 centerX, float32 centerY, float32 centerZ,
                  float32 width, float32 height, float32 depth);

ENGINE_API AABB AABBCentered(float3 center, float3 dimensions);

ENGINE_API AABB AABBUnion(const AABB& lop, const AABB& rop);
ENGINE_API AABB AABBIntersection(const AABB& lop, const AABB& rop);
ENGINE_API bool8 AABBIntersect(const AABB& lop, const AABB& rop);

// ----------------------------------------------------------------------------
// Frustum
// ----------------------------------------------------------------------------

/**
 * Is defined as:
 *   1) a set of 6 planes: right, left, top, bottom, far, near;
 *      each plane is directed inside of the frustum.
 *   2) a set of 8 corners: nbl, nbr, fbl, fbr, ntl, ntr, ftl, ftr
 */
struct Frustum
{
  Frustum(const float3& nbl, const float3& nbr,
          const float3& fbl, const float3& fbr,
          
          const float3& ntl, const float3& ntr,
          const float3& ftl, const float3& ftr);
  
  Frustum(const Plane& right, const Plane& left,
          const Plane& top, const Plane& bottom,
          const Plane& far, const Plane& near);

  bool8 containsPoint(float3 p) const;
  bool8 intersects(const AABB& aabb) const;

  float3 corners[8];
  Plane planes[6];
};

/**
 * Creates a frustum from 8 given points, each representing a corresponding corner
 */
ENGINE_API Frustum createFrustum(const float3& nbl, const float3& nbr,
                                 const float3& fbl, const float3& fbr,
                                 
                                 const float3& ntl, const float3& ntr,
                                 const float3& ftl, const float3& ftr);

/**
 * @param convertFromNDCMat - represents a matrix, which transforms vectors from
 * a NDC space into some other space S. The resultant frustum will be defined in
 * that space S.
 */
ENGINE_API Frustum createFrustum(const float4x4& convertFromNDCMat);

ENGINE_API bool8 frustumIntersectsAABB(const Frustum& frustum, const AABB& aabb);
