#include <stdarg.h>

#include <sstream>

#include <glsl_include/shadinclude.hpp>

#include "fileio.h"
#include "memory_manager.h"

#include "shader_build.h"

using std::endl;
using std::string;
using std::stringstream;

struct ShaderBuild
{
  stringstream code;
};

bool8 createShaderBuild(ShaderBuild** outBuild)
{
  *outBuild = engineAllocObject<ShaderBuild>(MEMORY_TYPE_GENERAL);

  return TRUE;
}

void destroyShaderBuild(ShaderBuild* build)
{
  engineFreeObject(build, MEMORY_TYPE_GENERAL);
}

void shaderBuildAddVersion(ShaderBuild* build, int32 version, const char* api)
{
  build->code << "#version " << version << " " << api << endl;
}

void shaderBuildAddExtension(ShaderBuild* build, const char* extName)
{
  build->code << "#extension " << extName << " : require" << endl;
}

void shaderBuildAddMacro(ShaderBuild* build, const char* macroName, const char* macroValue)
{
  build->code << "#define " << macroName << " " << macroValue << endl;
}

void shaderBuildAddCode(ShaderBuild* build, const char* code, bool8 addNewLine)
{
  build->code << code;
  if(addNewLine == TRUE)
  {
    build->code << endl;
  }
}

void shaderBuildAddCodef(ShaderBuild* build, const char* format, ...)
{
  char buf[4096]{};

  va_list args;
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  shaderBuildAddCode(build, buf, FALSE);
}

void shaderBuildAddCodefln(ShaderBuild* build, const char* format, ...)
{
  char buf[4096]{};

  va_list args;
  va_start(args, format);
  vsprintf(buf, format, args);
  va_end(args);

  shaderBuildAddCode(build, buf, TRUE);
}

void shaderBuildAddFunction(ShaderBuild* build,
                                       const char* returnType,
                                       const char* functionName,
                                       const char* arguments,
                                       const char* functionBody)
{
  build->code << returnType << " " << functionName << "(" << arguments << ")" << endl;
  build->code << "{" << endl;
  build->code << functionBody << endl;
  build->code << "}" << endl;
}

void shaderBuildConcatenate(ShaderBuild* build, ShaderBuild* src)
{
  shaderBuildAddCode(build, shaderBuildGetCode(src).c_str());
}

void shaderBuildClear(ShaderBuild* build)
{
  stringstream().swap(build->code);
}

string shaderBuildGetCode(ShaderBuild* build)
{
  return build->code.str();
}

ShaderPtr shaderBuildGenerateShader(ShaderBuild* build, GLenum shaderType)
{
  Shader* shader = nullptr;
  if(createShaderFromMemory(shaderType, shaderBuildGetCode(build).c_str(), &shader) == FALSE)
  {
    return ShaderPtr(nullptr);
  }

  if(compileShader(shader) == FALSE)
  {
    return ShaderPtr(nullptr);
  }

  return ShaderPtr(shader);
}

bool8 shaderBuildIncludeFile(ShaderBuild* build, const char* filename)
{
  string fileContent = Shadinclude::load(filename);
  if(fileContent.empty())
  {
    return FALSE;
  }

  shaderBuildAddCode(build, fileContent.c_str());
  
  return TRUE;
}
