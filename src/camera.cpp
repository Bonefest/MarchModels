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

  return swizzle<0, 1, 2>(position / position.w);
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

Ray cameraGenerateCameraRay(Camera* camera, float2 ndc)
{
  cameraRecalculateTransforms(camera);

  float4 fullNDC = float4(ndc.x, ndc.y, 0.0f, 1.0f);
  float4 localDir = mul(camera->transformNDCToCamera, fullNDC);
  localDir /= localDir.w;

  return Ray(float3(), swizzle<0, 1, 2>(localDir));
}

Ray cameraGenerateWorldRay(Camera* camera, float2 ndc)
{
  cameraRecalculateTransforms(camera);

  float4 fullNDC = float4(ndc.x, ndc.y, 0.0f, 1.0f);
  float4 globalDir = mul(camera->transformNDCToWorld, fullNDC);
  globalDir /= globalDir.w;

  return Ray(camera->position, swizzle<0, 1, 2>(globalDir)); 
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
