#include <string>
#include <cstring>

using std::string;

#include <glsl_include/shadinclude.hpp>

#include "fileio.h"
#include "logging.h"
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

  string fileContent = Shadinclude::load(filename);
  if(fileContent.empty())
  {
    return FALSE;
  }
  
  shaderAttachSource(*outShader, fileContent.c_str());
  
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
    shader->shader = 0;
  }

  shaderFreeSource(shader);

  engineFreeObject(shader, MEMORY_TYPE_GENERAL);
}

bool8 compileShader(Shader* shader)
{
  if(shader->source == nullptr || shader->sourceSize == 0)
  {
    LOG_ERROR("Attempt to compile a shader without an attached source!");
    return FALSE;
  }

  glShaderSource(shader->shader, 1, &shader->source, NULL);
  glCompileShader(shader->shader);

  GLint compileStatus;
  glGetShaderiv(shader->shader, GL_COMPILE_STATUS, &compileStatus);  

  if(compileStatus == GL_FALSE)
  {
    char log[255];
    glGetShaderInfoLog(shader->shader, 255, NULL, log);

    LOG_ERROR("Shader compilation has failed with message: '%s'", log);
    
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

GLuint shaderGetGLShader(Shader* shader)
{
  return shader->shader;
}
