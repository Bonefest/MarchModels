#include <string>
#include <vector>
#include <cstring>
#include <unordered_map>

using std::string;
using std::vector;
using std::unordered_map;

#include "fileio.h"
#include "shader_manager.h"

struct ShaderReloadData
{
  ShaderPtr shader;
  string filename;
  // previous file's update time
};

struct ShaderManagerData
{
  vector<ShaderReloadData> autoreloadShaders;
  unordered_map<string, ShaderPtr> shaderAliases;
};

static ShaderManagerData data;

bool8 initShaderManager()
{
  return TRUE;
}

void shutdownShaderManager()
{
  data.autoreloadShaders.clear();
  data.shaderAliases.clear();
}

void shaderManagerUpdate()
{
  // TODO: reload shaders if needed
}

bool8 shaderManagerLoadShader(GLuint shaderType,
                              const char* filename,
                              bool8 autoreload,
                              const char* alias)
{
  // NOTE: Skip loading if already loaded
  if(shaderManagerHasShader(filename) == TRUE)
  {
    return TRUE;
  }
  
  Shader* shader = nullptr;
  if(createShaderFromFile(shaderType, filename, &shader) == FALSE)
  {
    return FALSE;
  }

  if(compileShader(shader) == FALSE)
  {
    LOG_ERROR("Cannot compile a shader '%s'!", filename);
    return FALSE;
  }
  
  ShaderPtr shaderPtr = ShaderPtr(shader);
  
  if(autoreload == TRUE)
  {
    data.autoreloadShaders.push_back(ShaderReloadData{shaderPtr, filename});
  }

  data.shaderAliases[filename] = shaderPtr;

  if(alias != nullptr && strlen(alias) > 0)
  {
    data.shaderAliases[alias] = shaderPtr;
  }

  return TRUE;
}


void shaderManagerAddShader(ShaderPtr shader, const char* alias)
{
  data.shaderAliases[alias] = shader;
}

ShaderPtr shaderManagerGetShader(const char* name)
{
  auto shaderIt = data.shaderAliases.find(name);
  if(shaderIt != data.shaderAliases.end())
  {
    return shaderIt->second;
  }

  return ShaderPtr(nullptr);
}

bool8 shaderManagerRemoveShader(ShaderPtr shader)
{
  if(shader == nullptr)
  {
    return FALSE;
  }

  for(auto it = data.autoreloadShaders.begin(); it != data.autoreloadShaders.end(); it++)
  {
    if(it->shader == shader)
    {
      data.autoreloadShaders.erase(it);
      break;
    }
  }

  for(auto it = data.shaderAliases.begin(); it != data.shaderAliases.end();)
  {
    if(it->second == shader)
    {
      it = data.shaderAliases.erase(it);
    }
    else
    {
      it++;
    }
  }

  return TRUE;
}

bool8 shaderManagerRemoveShader(const char* name)
{
  return shaderManagerRemoveShader(shaderManagerGetShader(name));
}

bool8 shaderManagerHasShader(const char* name)
{
  return shaderManagerGetShader(name) != nullptr;
}

