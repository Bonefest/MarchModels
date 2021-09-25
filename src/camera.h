#pragma once

#include "maths/common.h"

#include "defines.h"

struct Camera;

ENGINE_API bool8 createPerspectiveCamera(float32 aspectRatio,
                                         float32 fovY,
                                         float32 near,
                                         float32 far,
                                         Camera** outCamera);

ENGINE_API bool8 createPerspectiveCameraExt(float32 aspectRatio,
                                            float32 fovY,
                                            float32 near,
                                            float32 far,
                                            float3 position,
                                            quat orientation,
                                            Camera** outCamera);
ENGINE_API void destroyCamera(Camera* camera);

ENGINE_API void cameraSetPosition(Camera* camera, float3 position);
ENGINE_API float3 cameraGetPosition(Camera* camera);
ENGINE_API void cameraSetOrientation(Camera* camera, quat orientation);
ENGINE_API quat cameraGetOrientation(Camera* camera);

ENGINE_API void cameraPose(Camera* camera, quat orientation, float3 position);
ENGINE_API void cameraLookAt(Camera* camera, float3 position, float3 target, float3 up = float3(0.0f, 1.0f, 0.0f));
ENGINE_API float3 cameraProject(Camera* camera, float3 worldPosition);
ENGINE_API float3 cameraUnproject(Camera* camera, float3 ndc);
ENGINE_API float3 cameraToLocal(Camera* camera, float3 worldPosition);
ENGINE_API float3 cameraToWorld(Camera* camera, float3 localPosition);

ENGINE_API Ray cameraGenerateCameraRay(Camera* camera, float2 ndc);
ENGINE_API Ray cameraGenerateWorldRay(Camera* camera, float2 ndc);

ENGINE_API void cameraSetAspectRatio(Camera* camera, float32 aspectRatio);
ENGINE_API float32 cameraGetAspectRatio(Camera* camera);
ENGINE_API void cameraSetFovY(Camera* camera, float32 fovY);
ENGINE_API float32 cameraGetFovY(Camera* camera);
ENGINE_API void cameraSetFovX(Camera* camera, float32 fovX);
ENGINE_API float32 cameraGetFovX(Camera* camera);
