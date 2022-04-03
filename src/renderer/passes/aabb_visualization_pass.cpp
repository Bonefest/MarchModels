#include <imgui/imgui.h>

#include "utils.h"
#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/model3d.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "aabb_visualization_pass.h"

using std::vector;

const static uint32 MAX_INSTANCES = 1024;
const static uint32 CUBE_VRT_BUFFER_SLOT = 0;
const static uint32 CUBE_INST_BUFFER_SLOT = 1;
const static uint32 FRUSTUM_VRT_BUFFER_SLOT = 0;

const static float32 cubeVertexData[] =
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

const static uint32 cubeIndexData[] =
{
  0, 1, 4, 1, 5, 4, // front face
  3, 2, 7, 2, 6, 7, // back face
  2, 0, 4, 2, 4, 6, // left face
  1, 3, 7, 1, 7, 5, // right face
  2, 3, 1, 2, 1, 0, // bottom face
  4, 5, 7, 4, 7, 6, // top face
};

struct AABBVisualizationPassInstanceData
{
  float3 position;
  float3 size; 
  float3 color;
};

struct AABBVisualizationPassData
{
  Model3DPtr cubesModel;
  Model3DPtr frustumModel;
  
  GLuint ldrFBO;
  
  ShaderProgramPtr aabbVisualizationProgram;
  ShaderProgramPtr frustumVisualizationProgram;

  AABBVisualizationMode visualizationMode;
  bool8 showParents = TRUE;
  bool8 showFrustum = FALSE;
};

static void destroyAABBVisualizationPass(RenderPass* pass)
{
  AABBVisualizationPassData* data = (AABBVisualizationPassData*)renderPassGetInternalData(pass);
  data->cubesModel = Model3DPtr(nullptr);
  data->frustumModel = Model3DPtr(nullptr);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->aabbVisualizationProgram = ShaderProgramPtr(nullptr);
  data->frustumVisualizationProgram = ShaderProgramPtr(nullptr);
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static void gatherAABBs(AABBVisualizationPassData* data,
                        Asset* geometry,
                        vector<AABBVisualizationPassInstanceData>& unselectedInstances,
                        vector<AABBVisualizationPassInstanceData>& selectedInstances)
{
  vector<AssetPtr>& children = geometryGetChildren(geometry);
  for(auto child: children)
  {
    gatherAABBs(data, child, unselectedInstances, selectedInstances);
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

    AABBVisualizationPassInstanceData instanceData = {};
    instanceData.position = aabb.getCenter();
    instanceData.size = aabb.getDimensions();
    
    if(geometryIsEnabled(geometry))
    {
      instanceData.color = float3(0.0, 1.0, 0.0);
    }
    else
    {
      instanceData.color = float3(1.0, 0.0, 0.0);
    }
    
    if(geometryIsSelected(geometry) == TRUE)
    {

      unselectedInstances.push_back(instanceData);
    }
    else
    {
      instanceData.color *= 0.6f;
      selectedInstances.push_back(instanceData);
    }
  }
}

static void drawAABBs(AABBVisualizationPassData* data)
{
  vector<AABBVisualizationPassInstanceData> selectedInstances;
  vector<AABBVisualizationPassInstanceData> unselectedInstances;  
  gatherAABBs(data, sceneGetGeometryRoot(rendererGetPassedScene()),
              selectedInstances, unselectedInstances);


  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->aabbVisualizationProgram);

  float4x4 viewProj = cameraGetWorldNDCMat(rendererGetPassedCamera());
  glUniformMatrix4fv(0, 1, GL_FALSE, &viewProj[0][0]);
  
  glBindVertexArray(model3DGetVAOHandle(data->cubesModel));

  glLineWidth(2.0);
  model3DUpdateBuffer(data->cubesModel, CUBE_INST_BUFFER_SLOT,
                      0, &selectedInstances[0], sizeof(AABBVisualizationPassInstanceData) * selectedInstances.size());
  
  glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, selectedInstances.size());
  
  glLineWidth(1.0);

  model3DUpdateBuffer(data->cubesModel, CUBE_INST_BUFFER_SLOT,
                      0, &unselectedInstances[0], sizeof(AABBVisualizationPassInstanceData) * unselectedInstances.size());
  
  glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, unselectedInstances.size());
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
  glDisable(GL_DEPTH_TEST);
}

static void drawFrustum(AABBVisualizationPassData* data)
{
  Camera* camera = rendererGetPassedCamera();
  const Frustum& cameraFrustum = cameraGetFrustum(camera);

  model3DUpdateBuffer(data->frustumModel, FRUSTUM_VRT_BUFFER_SLOT,
                      0, &cameraFrustum.corners[0], sizeof(float3) * 8);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->frustumVisualizationProgram);

  float4x4 viewProj = cameraGetWorldNDCMat(camera);
  glUniformMatrix4fv(0, 1, GL_FALSE, &viewProj[0][0]);
  
  glBindVertexArray(model3DGetVAOHandle(data->frustumModel));
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
  glDisable(GL_DEPTH_TEST);  
}

static bool8 aabbVisualizationPassExecute(RenderPass* pass)
{
  AABBVisualizationPassData* data = (AABBVisualizationPassData*)renderPassGetInternalData(pass);

  drawAABBs(data);
  if(data->showFrustum == TRUE)
  {
    drawFrustum(data);
  }
  
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
  ImGui::Checkbox("Show frustum", (bool*)&data->showFrustum);
  ImGui::SameLine();
  ImGui::PushItemWidth(200.0);
  ImGui::Combo("AABB Visualization mode",
               (int32*)&data->visualizationMode,
               visualizationModesLabels,
               ARRAY_SIZE(visualizationModesLabels));
  ImGui::PopItemWidth();
}

static const char* aabbVisualizationPassGetName(RenderPass* pass)
{
  return "AABBVisualizationPass";
}

static ShaderProgram* createAABBVisualizationProgram()
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

static ShaderProgram* createFrustumVisualizationProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_VERTEX_SHADER, "shaders/visualize_frustum.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/visualize_frustum.frag"));

  if(linkShaderProgram(program) == FALSE)
  {
    destroyShaderProgram(program);
    return nullptr;
  }
  
  return program;
}

static Model3DPtr createCubesModel()
{
  Model3D* model = nullptr;
  assert(createModel3DEmpty(&model));

  model3DAttachIndexBuffer(model, cubeIndexData, sizeof(cubeIndexData), GL_STATIC_DRAW);  
  model3DAttachBuffer(model, CUBE_VRT_BUFFER_SLOT, cubeVertexData, sizeof(cubeVertexData), GL_STATIC_DRAW);
  model3DAttachBuffer(model, CUBE_INST_BUFFER_SLOT, NULL, sizeof(float32) * 6 * MAX_INSTANCES, GL_DYNAMIC_DRAW);

  model3DDescribeInput(model, 0, CUBE_VRT_BUFFER_SLOT, 3, GL_FLOAT, 3 * sizeof(float32), 0);
  model3DDescribeInput(model, 1, CUBE_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(AABBVisualizationPassInstanceData), offsetof(AABBVisualizationPassInstanceData, position), FALSE);
  model3DDescribeInput(model, 2, CUBE_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(AABBVisualizationPassInstanceData), offsetof(AABBVisualizationPassInstanceData, size), FALSE);
  model3DDescribeInput(model, 3, CUBE_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(AABBVisualizationPassInstanceData), offsetof(AABBVisualizationPassInstanceData, color), FALSE);  

  return Model3DPtr(model);
}

static Model3DPtr createFrustumModel()
{
  Model3D* model = nullptr;
  assert(createModel3DEmpty(&model));

  // NOTE: We model frustum as a skewed cube
  model3DAttachIndexBuffer(model, cubeIndexData, sizeof(cubeIndexData), GL_STATIC_DRAW);
  // NOTE: Vertices of the frustum will be updated each frame (extracted from the camera)  
  model3DAttachBuffer(model, FRUSTUM_VRT_BUFFER_SLOT, cubeVertexData, sizeof(cubeVertexData), GL_STREAM_DRAW);

  model3DDescribeInput(model, 0, FRUSTUM_VRT_BUFFER_SLOT, 3, GL_FLOAT, 3 * sizeof(float32), 0);

  return Model3DPtr(model);
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
  
  data->ldrFBO = createFramebufferD(rendererGetResourceHandle(RR_LDR_MAP_TEXTURE),
                                    rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->cubesModel = createCubesModel();
  data->frustumModel = createFrustumModel();
  
  data->aabbVisualizationProgram = ShaderProgramPtr(createAABBVisualizationProgram());
  assert(data->aabbVisualizationProgram != nullptr);

  data->frustumVisualizationProgram = ShaderProgramPtr(createFrustumVisualizationProgram());
  assert(data->frustumVisualizationProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}

