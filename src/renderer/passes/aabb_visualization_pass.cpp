#include <imgui/imgui.h>

#include "utils.h"
#include "program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "aabb_visualization_pass.h"

struct AABBVisualizationPassData
{
  GLuint cubeVBO;
  GLuint cubeEBO;
  GLuint instancesVBO;
  GLuint visualizationVAO;
  
  GLuint ldrFBO;
  
  ShaderProgram* visualizationProgram;

  AABBVisualizationMode visualizationMode;
  bool8 showParents = TRUE;
};

static void destroyAABBVisualizationPass(RenderPass* pass)
{
  AABBVisualizationPassData* data = (AABBVisualizationPassData*)renderPassGetInternalData(pass);
  glDeleteBuffers(1, &data->cubeVBO);
  glDeleteBuffers(1, &data->cubeEBO);
  glDeleteBuffers(1, &data->instancesVBO);
  glDeleteBuffers(1, &data->visualizationVAO);
  glDeleteFramebuffers(1, &data->ldrFBO);
  
  destroyShaderProgram(data->visualizationProgram);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static void gatherAABBs(AABBVisualizationPassData* data, Asset* geometry, std::vector<float3>& outAABBData)
{
  std::vector<AssetPtr>& children = geometryGetChildren(geometry);
  for(auto child: children)
  {
    gatherAABBs(data, child, outAABBData);
  }

  if(data->showParents == TRUE || geometryIsLeaf(geometry) == TRUE)
  {
    AABB aabb;
    if(data->visualizationMode == AABB_VISUALIZATION_MODE_DYNAMIC)
    {
      aabb = geometryGetDynamicAABB(geometry);
    }
    else if(data->visualizationMode == AABB_VISUALIZATION_MODE_FINAL)
    {
      aabb = geometryGetFinalAABB(geometry);
    }

    outAABBData.push_back(aabb.getCenter());
    outAABBData.push_back(aabb.getDimensions());    
  }
}

static bool8 aabbVisualizationPassExecute(RenderPass* pass)
{
  AABBVisualizationPassData* data = (AABBVisualizationPassData*)renderPassGetInternalData(pass);

  std::vector<float3> aabbData;
  gatherAABBs(data, sceneGetGeometryRoot(rendererGetPassedScene()), aabbData);
  
  glBindBuffer(GL_ARRAY_BUFFER, data->instancesVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float32) * 3 * aabbData.size(), &aabbData[0]);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->visualizationProgram);

  float4x4 viewProj = cameraGetWorldNDCMat(rendererGetPassedCamera());
  
  glUniformMatrix4fv(0, 1, GL_FALSE, &viewProj[0][0]);
  
  glBindVertexArray(data->visualizationVAO);
  glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, aabbData.size() >> 1);
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
  glDisable(GL_DEPTH_TEST);
  
  return TRUE;
}

static void aabbVisualizationPassDrawInputView(RenderPass* pass)
{
  AABBVisualizationPassData* data = (AABBVisualizationPassData*)renderPassGetInternalData(pass);

  const static char* visualizationModesLabels[] =
  {
    "Show dynamic AABB",
    "Show final AABB"
  };

  ImGui::Checkbox("Show parents", (bool*)&data->showParents);
  ImGui::SameLine();
  ImGui::PushItemWidth(200.0);
  ImGui::Combo("AABB Visualization mode",
               (int32*)&data->visualizationMode,
               visualizationModesLabels,
               ARRAY_SIZE(visualizationModesLabels));
}

static const char* aabbVisualizationPassGetName(RenderPass* pass)
{
  return "AABBVisualizationPass";
}

static ShaderProgram* createVisualizationProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_VERTEX_SHADER, "shaders/visualize_aabb.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/visualize_aabb.frag"));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

static GLuint createLDRFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_LDR_MAP_TEXTURE), 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE), 0); 
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return framebuffer;
}

static void createCubeBuffers(GLuint* outVBO, GLuint* outEBO)
{
  const static float32 vertexData[] =
  {
     // Coordinates are provided in our RHS system (x - left, y - up, z - forward)
     0.5f, -0.5f, -0.5f, // Left, bottom, near
    -0.5f, -0.5f, -0.5f, // Right, bottom, near
     0.5f, -0.5f,  0.5f, // Left, bottom, far
    -0.5f, -0.5f,  0.5f, // Right, bottom, far

     0.5f,  0.5f, -0.5f, // Left, top, near
    -0.5f,  0.5f, -0.5f, // Right, top, near
     0.5f,  0.5f,  0.5f, // Left, top, far
    -0.5f,  0.5f,  0.5f, // Right, top, far
  };

  const static uint32 indexData[] =
  {
    0, 1, 4, 1, 5, 4, // front face
    3, 2, 7, 2, 6, 7, // back face
    2, 0, 4, 2, 4, 6, // left face
    1, 3, 7, 1, 7, 5, // right face
    2, 3, 1, 2, 1, 0, // bottom face
    4, 5, 7, 4, 7, 6, // top face
    
  };
  
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLuint ebo = 0;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  *outVBO = vbo;
  *outEBO = ebo;
}

GLuint createInstancesBuffer()
{
  const static uint32 maxInstances = 1024;
  
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * 2 * maxInstances, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return vbo;
}

GLuint createVAO(GLuint cubeVBO, GLuint cubeEBO, GLuint instancesVBO)
{
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
  
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float32), NULL);

  glBindBuffer(GL_ARRAY_BUFFER, instancesVBO);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), NULL);
  glVertexAttribDivisor(1, 1);

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float32), (void*)(sizeof(float32) * 3));
  glVertexAttribDivisor(2, 1);  

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return vao;
}

bool8 createAABBVisualizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyAABBVisualizationPass;
  interface.execute = aabbVisualizationPassExecute;
  interface.drawInputView = aabbVisualizationPassDrawInputView;
  interface.getName = aabbVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_AABB_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  AABBVisualizationPassData* data = engineAllocObject<AABBVisualizationPassData>(MEMORY_TYPE_GENERAL);
  
  data->ldrFBO = createLDRFramebuffer();
  assert(data->ldrFBO != 0);

  createCubeBuffers(&data->cubeVBO, &data->cubeEBO);
  
  data->instancesVBO = createInstancesBuffer();
  assert(data->instancesVBO != 0);
  
  data->visualizationVAO = createVAO(data->cubeVBO, data->cubeEBO, data->instancesVBO);
  assert(data->visualizationVAO != 0);
  
  data->visualizationProgram = createVisualizationProgram();
  assert(data->visualizationProgram != nullptr);

  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}

