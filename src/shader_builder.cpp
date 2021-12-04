#include <sstream>

#include "memory_manager.h"
#include "shader_builder.h"

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

void shaderBuildAddMacro(ShaderBuild* build, const char* macro)
{
  build->code << "#define " << macro << endl;
}

void shaderBuildAddCode(ShaderBuild* build, const char* code, bool8 addNewLine)
{
  build->code << code;
  if(addNewLine == TRUE)
  {
    build->code << endl;
  }
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

bool8 shaderBuildGenerateShader(ShaderBuild* build, GLenum shaderType, GLuint* result)
{
  *result = glCreateShader(shaderType);
  if(*result == 0 || *result == GL_INVALID_ENUM)
  {
    return FALSE;
  }

  string code = build->code.str();
  const char* ccode = code.c_str();
  GLint length = 1;
  glShaderSource(*result, 1, &ccode, &length);
  glCompileShader(shaderType);

  GLint compileStatus;
  glGetShaderiv(*result, GL_COMPILE_STATUS, &compileStatus);

  if(compileStatus == GL_FALSE)
  {
    glDeleteShader(*result);
    return FALSE;
  }
  
  return TRUE;
}

