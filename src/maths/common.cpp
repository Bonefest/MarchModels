#include "common.h"

float4x4 eulerToMat(float2 euler)
{
  return rotation_matrix(qmul(rotation_quat(float3(1.0f, 0.0f, 0.0f), euler.y),
                              rotation_quat(float3(0.0f, 1.0f, 0.0f), euler.x)));
}
