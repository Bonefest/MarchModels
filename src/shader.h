#pragma once

#include "defines.h"

struct Shader;

ENGINE_API bool8 createEmptyShader(GLuint shaderType, Shader** outShader);
ENGINE_API bool8 createShaderFromFile(GLuint shaderType, const char* filename, Shader** outShader);
ENGINE_API bool8 createShaderFromMemory(GLuint shaderType, const char* source, Shader** outShader);
ENGINE_API void destroyShader(Shader* shader);

ENGINE_API bool8 compileShader(Shader* shader);
ENGINE_API bool8 shaderIsCompiled(Shader* shader);

ENGINE_API void shaderAttachSource(Shader* shader, const char* source);

ENGINE_API GLuint shaderGetType(Shader* shader);


