#include "memory_manager.h"

#include "camera.h"

struct Camera
{
  float32 aspectRatio;
  float32 fovY;
  float32 near;
  float32 far;
  float3 position;
  quat orientation;

  Frustum frustum;
  bool8 updateFrustum = TRUE;

  float4x4 transformCameraToNDC;
  float4x4 transformNDCToCamera;
  
  float4x4 transformWorldToCamera;
  float4x4 transformCameraToWorld;  

  float4x4 transformNDCToWorld;
  float4x4 transformWorldToNDC;  

  bool8 dirty;
};

static void cameraRecalculateTransforms(Camera* camera)
{
  if(camera->dirty == FALSE) return;
   
  float4x4 projTransform = perspective_matrix(camera->fovY,
                                              camera->aspectRatio,
                                              camera->near,
                                              camera->far,
                                              pos_z,
                                              neg_one_to_one);

  float4x4 invProjTransform = inverse(projTransform);
  
  camera->transformCameraToWorld = pose_matrix(camera->orientation, camera->position);
  camera->transformWorldToCamera = inverse(camera->transformCameraToWorld);
  camera->transformCameraToNDC = projTransform;
  camera->transformNDCToCamera = invProjTransform;
  camera->transformNDCToWorld = mul(camera->transformCameraToWorld, invProjTransform);
  camera->transformWorldToNDC = inverse(camera->transformNDCToWorld);

  if(camera->updateFrustum == TRUE)
  {
    camera->frustum = createFrustum(camera->transformNDCToWorld);
  }
  
  camera->dirty = FALSE;
}

bool8 createPerspectiveCamera(float32 aspectRatio,
                              float32 fovY,
                              float32 near,
                              float32 far,
                              Camera** outCamera)
{
  return createPerspectiveCameraExt(aspectRatio,
                                    fovY,
                                    near, far,
                                    float3(),
                                    rotation_quat(float3(0.0f, 1.0f, 0.0f), 0.0f),
                                    outCamera);
}

bool8 createPerspectiveCameraExt(float32 aspectRatio,
                                 float32 fovY,
                                 float32 near,
                                 float32 far,
                                 float3 position,
                                 quat orientation,
                                 Camera** outCamera)
{
  *outCamera = engineAllocObject<Camera>(MEMORY_TYPE_GENERAL);
  
  Camera* camera = *outCamera;
  camera->aspectRatio = aspectRatio;
  camera->fovY = fovY;
  camera->near = near;
  camera->far = far;
  camera->position = position;
  camera->orientation = orientation;
  camera->dirty = TRUE;

  return TRUE;
}

void destroyCamera(Camera* camera)
{
  engineFreeObject(camera, MEMORY_TYPE_GENERAL);
}

void cameraSetPosition(Camera* camera, float3 position)
{
  camera->position = position;
}

float3 cameraGetPosition(Camera* camera)
{
  camera->dirty = TRUE;
  return camera->position;
}

void cameraSetOrientation(Camera* camera, quat orientation)
{
  camera->dirty = TRUE;
  camera->orientation = orientation;
}

quat cameraGetOrientation(Camera* camera)
{
  return camera->orientation;
}

void cameraSetOrientation(Camera* camera, float3 eulerAngles)
{
  cameraSetOrientation(camera, eulerAngles.x, eulerAngles.y);
}

void cameraSetOrientation(Camera* camera, float32 yaw, float32 pitch)
{
  // NOTE: We need to clamp pitch in range [-89, 89] degrees because
  // +- 90 degrees pitch is ambigious: later, when we need to deduce
  // eueler angles from quaternion, we cannot correctly decide what
  // yaw the camera has (it could have both 0 and 180 degrees).
  //
  // Solutions:
  // 1) Simply clamp, as we do here
  // 2) Store euler angles directly and use them instead. (more flexible)
  
  pitch = clamp(pitch, -toRad(89.0f), toRad(89.0f));
  
  camera->dirty = TRUE;
  camera->orientation = qmul(rotation_quat(float3(0.0f, 1.0f, 0.0f), yaw),
                             rotation_quat(float3(1.0f, 0.0f, 0.0f), pitch)); 
                             
}

float3 cameraGetEulerAngles(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  float3 axis = qrot(camera->orientation, float3(0.0f, 0.0f, 1.0f));

  float32 pitch = asin(axis.y);
  float32 yaw = atan2(axis.x, axis.z);

  // NOTE: We need to negate pitch, because positive rotation around x axis is going in opposite order
  // (we imagine that's going upward, while in reality it's going downward)
  return float3(yaw, -pitch, 0.0f);
}

void cameraPose(Camera* camera, quat orientation, float3 position)
{
  camera->dirty = TRUE;
  camera->position = position;
  camera->orientation = orientation;
}

void cameraLookAt(Camera* camera, float3 position, float3 target, float3 up)
{
  camera->dirty = TRUE;
  camera->position = position;

  float3 dir = normalize(target - position);

  // NOTE: Axis of rotation from z axis to the required direction.
  float3 axis = cross(float3(0.0f, 0.0f, 1.0f), dir);
  float angle = acos(dot(axis, dir));

  camera->orientation = rotation_quat(axis, angle);
}

float3 cameraProject(Camera* camera, float3 worldPosition)
{
  cameraRecalculateTransforms(camera);

  float4 position = float4(worldPosition.x,
                           worldPosition.y,
                           worldPosition.z,
                           1.0f);

  position = mul(camera->transformWorldToNDC, position);

  return (position / position.w).xyz();
}

float3 cameraToLocal(Camera* camera, float3 worldPosition)
{
  cameraRecalculateTransforms(camera);

  return mul(camera->transformWorldToCamera, worldPosition);
}

float3 cameraToWorld(Camera* camera, float3 localPosition)
{
  cameraRecalculateTransforms(camera);

  return mul(camera->transformCameraToWorld, localPosition);
}

float4x4 cameraGetNDCCameraMat(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->transformNDCToCamera;
}

float4x4 cameraGetCameraNDCMat(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->transformCameraToNDC;
}

float4x4 cameraGetCameraWorldMat(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->transformCameraToWorld;
}

float4x4 cameraGetWorldCameraMat(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->transformWorldToCamera;
}

float4x4 cameraGetNDCWorldMat(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->transformNDCToWorld;
}
float4x4 cameraGetWorldNDCMat(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->transformWorldToNDC;
}

Ray cameraGenerateCameraRay(Camera* camera, float2 ndc)
{
  cameraRecalculateTransforms(camera);

  // NDC point on near plane  
  float4 fullNNDC = float4(ndc.x, ndc.y, 0.0f, 1.0f);

  // NDC point on far plane
  float4 fullFNDC = float4(ndc.x, ndc.y, 1.0f, 1.0f);

  // Transform near ndc point back to camera frustum
  float4 frustumNPoint = mul(camera->transformNDCToCamera, fullNNDC);
  frustumNPoint /= frustumNPoint.w;

  // Transform far ndc point back to camera frustum
  float4 frustumFPoint = mul(camera->transformNDCToCamera, fullFNDC);
  frustumFPoint /= frustumFPoint.w;
  
  float3 normalizedDir = normalize((frustumFPoint).xyz() - (frustumNPoint).xyz());

  return Ray(float3(), normalizedDir);
}

Ray cameraGenerateWorldRay(Camera* camera, float2 ndc)
{
  cameraRecalculateTransforms(camera);

  float4 fullNNDC = float4(ndc.x, ndc.y, 0.0f, 1.0f);
  float4 fullFNDC = float4(ndc.x, ndc.y, 1.0f, 1.0f);
  
  float4 frustumNPoint = mul(camera->transformNDCToWorld, fullNNDC);
  frustumNPoint /= frustumNPoint.w;

  float4 frustumFPoint = mul(camera->transformNDCToWorld, fullFNDC);
  frustumFPoint /= frustumFPoint.w;
  
  float3 normalizedDir = normalize((frustumFPoint).xyz() - (frustumNPoint).xyz());

  return Ray(camera->position, normalizedDir);
}

const Frustum& cameraGetFrustum(Camera* camera)
{
  cameraRecalculateTransforms(camera);
  
  return camera->frustum;
}

void cameraSetUpdateFrustum(Camera* camera, bool8 update)
{
  camera->updateFrustum = update;
  camera->dirty |= (update == TRUE ? TRUE : FALSE);
}

bool8 cameraUpdatesFrustum(Camera* camera)
{
  return camera->updateFrustum;
}

void cameraSetAspectRatio(Camera* camera, float32 aspectRatio)
{
  camera->dirty = TRUE;
  camera->aspectRatio = aspectRatio;
}

float32 cameraGetAspectRatio(Camera* camera)
{
  return camera->aspectRatio;
}

void cameraSetFovY(Camera* camera, float32 fovY)
{
  camera->dirty = TRUE;
  camera->fovY = fovY;
}

float32 cameraGetFovY(Camera* camera)
{
  return camera->fovY;
}

void cameraSetFovX(Camera* camera, float32 fovX)
{
  camera->dirty = TRUE;
  camera->fovY = fovX / camera->aspectRatio;
}

float32 cameraGetFovX(Camera* camera)
{
  return camera->fovY * camera->aspectRatio;
}

float32 cameraGetNear(Camera* camera)
{
  return camera->near;
}

float32 cameraGetFar(Camera* camera)
{
  return camera->far;
}
