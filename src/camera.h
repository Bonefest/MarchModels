#pragma once

#include "defines.h"

struct Camera;

bool8 createPerspectiveCamera(float32 aspectRatio, float32 fovY, Camera** outCamera);
void destroyCamera(Camera* camera);

void cameraSetPosition(Camera* camera, float3 position);
float3 cameraGetPosition(Camera* camera);
void cameraSetOrientation(Camera* camera, quat orientation);
quat cameraGetOrientation(Camera* camera);

void cameraPose(Camera* camera, quat orientation, float3 position);
void cameraLookAt(Camera* camera, float3 target, float3 position);
float3 cameraProject(Camera* camera, float3 worldPosition);
float3 cameraToLocal(Camera* camera, float3 worldPosition);
float3 cameraToWorld(Camera* camera, float3 localPosition);

Ray cameraGenerateCameraRay(Camera* camera, float2 ndc);
Ray cameraGenerateWorldRay(Camera* camera, float2 ndc);

void cameraSetAspectRatio(Camera* camera, float32 aspectRatio);
float32 cameraGetAspectRatio(Camera* camera);
void cameraSetFovY(Camera* camera, float32 fovY);
float32 cameraGetFovY(Camera* camera);
void cameraSetFovX(Camera* camera, float32 fovX);
float32 cameraGetFovX(Camera* camera);
