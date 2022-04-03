#pragma once

#include "ptr.h"
#include "shader.h"

struct ShaderProgram;

ENGINE_API bool8 createShaderProgram(ShaderProgram** outProgram);
ENGINE_API void destroyShaderProgram(ShaderProgram* program);

ENGINE_API bool8 shaderProgramAttachShader(ShaderProgram* program, ShaderPtr shader);
ENGINE_API ShaderPtr shaderProgramGetShader(ShaderProgram* program, GLuint type);
ENGINE_API bool8 shaderProgramHasShader(ShaderProgram* program, GLuint type);

ENGINE_API bool8 linkShaderProgram(ShaderProgram* program);
ENGINE_API bool8 shaderProgramIsLinked(ShaderProgram* program);

ENGINE_API void shaderProgramUse(ShaderProgram* program);
ENGINE_API GLuint shaderProgramGetGLHandle(ShaderProgram* program);

using ShaderProgramPtr = SharedPtr<ShaderProgram, destroyShaderProgram>;
