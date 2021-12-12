#include <vector>
using std::vector;

#include "logging.h"
#include "memory_manager.h"

#include "program.h"

struct ShaderProgram
{
  GLuint program;
  vector<ShaderPtr> attachedShaders;
  bool8 linked;
};

bool8 createShaderProgram(ShaderProgram** outProgram)
{
  *outProgram = engineAllocObject<ShaderProgram>(MEMORY_TYPE_GENERAL);
  ShaderProgram* program = *outProgram;
  program->program = glCreateProgram();
  program->linked = FALSE;
  
  return TRUE;
}

void destroyShaderProgram(ShaderProgram* program)
{
  if(program->program != 0)
  {
    glDeleteProgram(program->program);
    program->program = 0;
  }

  program->attachedShaders.clear();

  engineFreeObject(program, MEMORY_TYPE_GENERAL);
}

bool8 shaderProgramAttachShader(ShaderProgram* program, ShaderPtr shader)
{
  assert(shader != nullptr && program->program != 0);

  if(program->linked == TRUE)
  {
    LOG_ERROR("Attempt to attach a shader to an already linked program!");
    return FALSE;
  }
  
  // NOTE: In theory, OpenGL allows to attach shaders with same type.
  // For now, our API doesn't allow that, so we check whether a shader
  // with same type was already attached and deattach previous shader in
  // positive case.
  for(auto shaderIt = program->attachedShaders.begin();
      shaderIt != program->attachedShaders.end();
      shaderIt++)
  {
    if(shaderGetType(shader) == shaderGetType(*shaderIt))
    {
      glDetachShader(program->program, shaderGetGLShader(*shaderIt));
      program->attachedShaders.erase(shaderIt);
      break;
    }
  }

  glAttachShader(program->program, shaderGetGLShader(shader));
  program->attachedShaders.push_back(shader);

  return TRUE;
}

ShaderPtr shaderProgramGetShader(ShaderProgram* program, GLuint type)
{
  for(ShaderPtr shader: program->attachedShaders)
  {
    if(shaderGetType(shader) == type)
    {
      return shader;
    }
  }

  return ShaderPtr(nullptr);
}

bool8 shaderProgramHasShader(ShaderProgram* program, GLuint type)
{
  return shaderProgramGetShader(program, type) != nullptr;
}

bool8 linkShaderProgram(ShaderProgram* program)
{
  if(program->linked == TRUE)
  {
    LOG_ERROR("Attempt to link an already linked program!");
    return FALSE;
  }

  glLinkProgram(program->program);

  GLint linkStatus = 0;
  glGetProgramiv(program->program, GL_LINK_STATUS, &linkStatus);

  if(linkStatus == GL_FALSE)
  {
    char log[255]{};
    glGetProgramInfoLog(program->program, 255, NULL, log);
    LOG_ERROR("Link has failed with message: '%s'", log);
  }

  program->linked = TRUE;
  
  return TRUE;
}

bool8 shaderProgramIsLinked(ShaderProgram* program)
{
  return program->linked;
}

void shaderProgramUse(ShaderProgram* program)
{
  glUseProgram(program->program);
}

GLuint shaderProgramGetGLProgram(ShaderProgram* program)
{
  return program->program;
}
