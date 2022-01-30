#include "model3d.h"

const static uint32 MODEL3D_MAX_BUFFERS = 8;

struct Model3D
{
  GLuint vaoHandle = 0;
  GLuint eboHandle = 0;
  GLuint vboHandles[MODEL3D_MAX_BUFFERS];

};

bool8 createModel3DEmpty(Model3D** outModel)
{
  *outModel = engineAllocObject<Model3D>(MEMORY_TYPE_GENERAL);
  Model3D* model = *outModel;
  glGenVertexArrays(1, &model->vaoHandle);
  
  return TRUE;
}

void destroyModel3D(Model3D* model)
{
  if(model->vaoHandle != 0)
  {
    glDeleteBuffers(1, &model->vaoHandle);
  }

  if(model->eboHandle != 0)
  {
    glDeleteBuffers(1, &model->eboHandle);
  }

  for(uint32 i = 0; i < MODEL3D_MAX_BUFFERS; i++)
  {
    if(model->vboHandles[i] != 0)
    {
      glDeleteBuffers(1, &model->vboHandles[i]);
    }
  }
}

GLuint model3DGetVAOHandle(Model3D* model)
{
  return model->vaoHandle;
}

GLuint model3DGetEBOHandle(Model3D* model)
{
  return model->eboHandle;
}

bool8 model3DUsesIndices(Model3D* model)
{
  return model->eboHandle != 0 ? TRUE : FALSE;
}

void model3DAttachBuffer(Model3D* model,
                         uint32 bufferSlot,
                         const char* data,
                         uint32 dataSize,
                         GLenum usage)
{
  assert(bufferSlot < MODEL3D_MAX_BUFFERS);

  GLuint vboHandles = 0;
  glGenBuffers(1, &vboHandles);
  glBindBuffer(GL_ARRAY_BUFFER, vboHandles);
  glBufferData(GL_ARRAY_BUFFER, dataSize, data, usage);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if(model->vboHandles[bufferSlot] != 0)
  {
    glDeleteBuffers(1, &model->vboHandles[bufferSlot]);
  }

  model->vboHandles[bufferSlot] = vboHandles;
}

bool8 model3DUpdateBuffer(Model3D* model,
                          uint32 bufferSlot,
                          uint32 offset, const char* data, uint32 dataSize)
{
  assert(bufferSlot < MODEL3D_MAX_BUFFERS);

  glBindBuffer(GL_ARRAY_BUFFER, model->vboHandles[bufferSlot]);
  glBufferSubData(GL_ARRAY_BUFFER, offset, dataSize, data);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return TRUE;
}

bool8 model3DRemoveBuffer(Model3D* model, uint32 bufferSlot)
{
  assert(bufferSlot < MODEL3D_MAX_BUFFERS);

  if(model->vboHandles[bufferSlot] == 0)
  {
    return FALSE;
  }

  glDeleteBuffers(1, &model->vboHandles[bufferSlot]);
  model->vboHandles[bufferSlot] = 0;

  return TRUE;
}

GLuint model3DGetBufferHandle(Model3D* model, uint32 bufferSlot)
{
  assert(bufferSlot < MODEL3D_MAX_BUFFERS);

  return model->vboHandles[bufferSlot];
}

bool8 model3DHasBuffer(Model3D* model, uint32 bufferSlot)
{
  assert(bufferSlot < MODEL3D_MAX_BUFFERS);

  return model->vboHandles[bufferSlot] != 0 ? TRUE : FALSE;
}

bool8 model3DDescribeInput(Model3D* model,
                           uint32 vertexSlot,
                           uint32 bufferSlotToReadFrom,
                           uint32 elementsCount,
                           GLenum type,
                           uint32 stride,
                           uint32 byteOffset,
                           bool8 perVertex)
{
  if(model3DHasBuffer(model, bufferSlotToReadFrom) == FALSE)
  {
    return FALSE;
  }

  glBindVertexArray(model->vaoHandle);
  glBindBuffer(GL_ARRAY_BUFFER, model->vboHandles[bufferSlotToReadFrom]);

  glEnableVertexAttribArray(vertexSlot);
  glVertexAttribPointer(vertexSlot, elementsCount, type, GL_FALSE, stride, (void*)(byteOffset));
  glVertexAttribDivisor(vertexSlot, perVertex == TRUE ? 0 : 1);
  
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return TRUE;
}

void model3DEnableVertexSlot(Model3D* model, uint32 vertexSlot)
{
  glBindVertexArray(model->vaoHandle);
  glEnableVertexAttribArray(vertexSlot);
  glBindVertexArray(0);
}

void model3DDisableVertexSlot(Model3D* model, uint32 vertexSlot)
{
  glBindVertexArray(model->vaoHandle);  
  glDisableVertexAttribArray(vertexSlot);
  glBindVertexArray(0);
}

