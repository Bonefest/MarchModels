#pragma once

#include "ptr.h"
#include "defines.h"

struct Model3D;

ENGINE_API bool8 createModel3DFromFile(const char* filename, Model3D** outModel);
ENGINE_API bool8 createModel3DEmpty(Model3D** outModel);
ENGINE_API void destroyModel3D(Model3D* model);

ENGINE_API void model3DAttachBuffer(Model3D* model,
                                    uint32 bufferSlot,
                                    const char* data,
                                    uint32 dataSize,
                                    GLenum usage);

ENGINE_API bool8 model3DUpdateBuffer(Model3D* model,
                                     uint32 bufferSlot,
                                     uint32 offset, const char* data, uint32 dataSize);

ENGINE_API bool8 model3DRemoveBuffer(Model3D* model, uint32 bufferSlot);

ENGINE_API GLuint model3DGetBufferHandle(Model3D* model, uint32 bufferSlot);
ENGINE_API bool8 model3DHasBuffer(Model3D* model, uint32 bufferSlot);

ENGINE_API bool8 model3DDescribeInput(Model3D* model,
                                      uint32 vertexSlot,
                                      uint32 bufferSlotToReadFrom,
                                      uint32 elementsCount,
                                      GLenum type,
                                      uint32 stride,
                                      uint32 byteOffset,
                                      bool8 perVertex = TRUE);

ENGINE_API void model3DEnableVertexSlot(Model3D* model, uint32 vertexSlot);
ENGINE_API void model3DDisableVertexSlot(Model3D* model, uint32 vertexSlot);

ENGINE_API GLuint model3DGetVAOHandle(Model3D* model);
ENGINE_API GLuint model3DGetEBOHandle(Model3D* model);
ENGINE_API bool8 model3DUsesIndices(Model3D* model);

using Model3DPtr = SharedPtr<Model3D, &destroyModel3D>;
