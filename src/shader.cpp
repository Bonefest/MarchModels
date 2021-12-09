#include <cstring>

#include "fileio.h"
#include "memory_manager.h"

#include "shader.h"

struct Shader
{
  GLuint shader;
  GLuint type;
  char* source;
  uint32 sourceSize;
  bool8 compiled;
};

bool8 createEmptyShader(GLuint shaderType, Shader** outShader)
{
  *outShader = engineAllocObject<Shader>(MEMORY_TYPE_GENERAL);
  
  Shader* shader = *outShader;
  shader->shader = glCreateShader(shaderType);
  shader->type = shaderType;
  shader->source = nullptr;
  shader->sourceSize = 0;
  shader->compiled = FALSE;

  *outShader = shader;
  return TRUE;
}

bool8 createShaderFromFile(GLuint shaderType, const char* filename, Shader** outShader)
{

  assert(createEmptyShader(shaderType, outShader));

  uint32 fileSize;
  char* fileContent;

  assert(readWholeFile(filename, &fileSize, &fileContent));
  shaderAttachSource(*outShader, fileContent);
  freeFileContent(fileSize, fileContent);
  
  return TRUE;
}

bool8 createShaderFromMemory(GLuint shaderType, const char* source, Shader** outShader)
{
  assert(createEmptyShader(shaderType, outShader));
  shaderAttachSource(*outShader, source);

  return TRUE;
}

static void shaderFreeSource(Shader* shader)
{
  if(shader->source != nullptr)
  {
    engineFreeMem(shader->source, shader->sourceSize, MEMORY_TYPE_GENERAL);
    shader->source = nullptr;
    shader->sourceSize = 0;
  }
}

void destroyShader(Shader* shader)
{
  if(shader->shader != 0)
  {
    glDeleteShader(shader->shader);
  }

  shaderFreeSource(shader);
}

bool8 compileShader(Shader* shader)
{
  if(shader->source == nullptr || shader->sourceSize == 0)
  {
    return FALSE;
  }

  GLint length = 1;
  glShaderSource(shader->shader, 1, &shader->source, &length);
  glCompileShader(shader->shader);

  GLint compileStatus;
  glGetShaderiv(shader->shader, GL_COMPILE_STATUS, &compileStatus);  

  if(compileStatus == GL_FALSE)
  {
    return FALSE;
  }

  shaderFreeSource(shader);
  shader->compiled = TRUE;
  
  return TRUE;
}

bool8 shaderIsCompiled(Shader* shader)
{
  return shader->compiled;
}

void shaderAttachSource(Shader* shader, const char* source)
{
  shaderFreeSource(shader);

  shader->sourceSize = strlen(source);
  shader->source = (char*)engineAllocMem(shader->sourceSize, MEMORY_TYPE_GENERAL);
  engineCopyMem(shader->source, source, shader->sourceSize);
}

GLuint shaderGetType(Shader* shader)
{
  return shader->type;
}
