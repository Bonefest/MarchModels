#pragma once

#include "shader.h"

ENGINE_API bool8 initShaderManager();
ENGINE_API void shutdownShaderManager();

ENGINE_API void shaderManagerUpdate();

ENGINE_API bool8 shaderManagerLoadShader(GLuint shaderType,
                                         const char* filename,
                                         bool8 autoreload = TRUE,
                                         const char* alias = nullptr);

ENGINE_API void shaderManagerAddShader(ShaderPtr shader, const char* alias);
ENGINE_API ShaderPtr shaderManagerGetShader(const char* name);
ENGINE_API bool8 shaderManagerRemoveShader(const char* name);
ENGINE_API bool8 shaderManagerRemoveShader(ShaderPtr shader);

ENGINE_API bool8 shaderManagerHasShader(const char* name);
  
