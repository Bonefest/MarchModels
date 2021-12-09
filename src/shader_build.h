#pragma once

#include <string>

#include "defines.h"

struct ShaderBuild;

ENGINE_API bool8 createShaderBuild(ShaderBuild** outBuild);
ENGINE_API void destroyShaderBuild(ShaderBuild* build);

ENGINE_API void shaderBuildAddVersion(ShaderBuild* build, int32 version, const char* api = "core");
ENGINE_API void shaderBuildAddExtension(ShaderBuild* build, const char* extName);
ENGINE_API void shaderBuildAddMacro(ShaderBuild* build, const char* macroName, const char* macroValue);
ENGINE_API void shaderBuildAddCode(ShaderBuild* build, const char* code, bool8 addNewLine = TRUE);
ENGINE_API void shaderBuildAddFunction(ShaderBuild* build,
                                       const char* returnType,
                                       const char* functionName,
                                       const char* arguments,
                                       const char* functionBody);
ENGINE_API void shaderBuildConcatenate(ShaderBuild* build, ShaderBuild* src);
ENGINE_API bool8 shaderBuildIncludeFile(ShaderBuild* build, const char* filename);
ENGINE_API void shaderBuildClear(ShaderBuild* build);

ENGINE_API std::string shaderBuildGetCode(ShaderBuild* build);
ENGINE_API bool8 shaderBuildGenerateShader(ShaderBuild* build, GLenum shaderType, GLuint* result);
