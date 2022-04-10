#include <imgui/imgui.h>

#include "utils.h"
#include "shader_program.h"
#include "memory_manager.h"
#include "shader_manager.h"
#include "renderer/model3d.h"
#include "renderer/renderer.h"
#include "renderer/renderer_utils.h"

#include "ui_widgets_visualization_pass.h"

using std::vector;

const static uint32 MAX_INSTANCES = 1024;

const static uint32 CUBE_VRT_BUFFER_SLOT = 0;
const static uint32 CUBE_INST_BUFFER_SLOT = 1;

const static uint32 FRUSTUM_VRT_BUFFER_SLOT = 0;

const static uint32 AXES_INST_BUFFER_SLOT = 1;

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

struct AxesInstanceData
{
  float4 color;
  float4x4 modelWorldMat;
};

struct UIWidgetsVisualizationPassData
{
  Model3DPtr cubesModel;
  Model3DPtr frustumModel;
  Model3DPtr axisModel;
  
  GLuint ldrFBO;
  
  ShaderProgramPtr aabbVisualizationProgram;
  ShaderProgramPtr frustumVisualizationProgram;
  ShaderProgramPtr axisVisualizationProgram;

  AABBVisualizationMode visualizationMode;
  bool8 showParents = TRUE;
  bool8 showAABB    = TRUE;
  bool8 showFrustum = FALSE;
  bool8 showAxes    = TRUE;
};

static void destroyUIWidgetsVisualizationPass(RenderPass* pass)
{
  UIWidgetsVisualizationPassData* data = (UIWidgetsVisualizationPassData*)renderPassGetInternalData(pass);
  data->cubesModel = Model3DPtr(nullptr);
  data->frustumModel = Model3DPtr(nullptr);
  data->axisModel = Model3DPtr(nullptr);
  glDeleteFramebuffers(1, &data->ldrFBO);

  data->aabbVisualizationProgram = ShaderProgramPtr(nullptr);
  data->frustumVisualizationProgram = ShaderProgramPtr(nullptr);
  data->axisVisualizationProgram = ShaderProgramPtr(nullptr);  
  
  engineFreeObject(data, MEMORY_TYPE_GENERAL);
}

static void gatherAABBs(UIWidgetsVisualizationPassData* data,
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

static void pushAxes(float3 position, float4x4 commonMat, vector<AxesInstanceData>& instances)
{
  // x axis
  instances.push_back(AxesInstanceData{
      float4(1.0f, 0.0f, 0.0f, 1.0f),
      mul(commonMat, rotation_matrix(rotation_quat(float3(0.0f, 1.0f, 0.0f), float32(HALF_PI))))
  });

  // y axis
  instances.push_back(AxesInstanceData{
      float4(0.0f, 1.0f, 0.0f, 1.0f),
      mul(commonMat, rotation_matrix(rotation_quat(float3(-1.0f, 0.0f, 0.0f), float32(HALF_PI))))
  });    

  // z axis
  instances.push_back(AxesInstanceData{
      float4(0.0f, 0.0f, 1.0f, 1.0f),
      commonMat
  });    
}

static void gatherGeometryAxes(Asset* geometry,
                               vector<AxesInstanceData>& instances)
{
  vector<AssetPtr>& children = geometryGetChildren(geometry);
  for(auto child: children)
  {
    gatherGeometryAxes(child, instances);
  }

  if(geometryIsSelected(geometry) == TRUE)
  {
    const AABB& finalAABB = geometryGetFinalAABB(geometry);
    float3 axisPosition = finalAABB.getCenter() + float3(0.0f, 0.5f + finalAABB.getHeight() * 0.5f, 0.0f);
    quat axisOrientation = geometryGetOrientation(geometry);
    float4x4 axisCommonMat = mul(translation_matrix(axisPosition), rotation_matrix(axisOrientation));
    
    pushAxes(axisPosition, axisCommonMat, instances);
  }
}

static void gatherLightSourcesAxes(const std::vector<AssetPtr>& lightSources,
                                   vector<AxesInstanceData>& instances)
{
  for(AssetPtr lsource: lightSources)
  {
    float3 axisPosition = lightSourceGetPosition(lsource);
    float4x4 axisOrientation = eulerToMat(lightSourceGetOrientation(lsource));
    float4x4 axisCommonMat = mul(translation_matrix(axisPosition), axisOrientation);
    
    pushAxes(axisPosition, axisCommonMat, instances);
  }
}

static void drawAABBs(UIWidgetsVisualizationPassData* data)
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

static void drawFrustum(UIWidgetsVisualizationPassData* data)
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

static void drawAxes(UIWidgetsVisualizationPassData* data)
{
  vector<AxesInstanceData> axes;  
  gatherGeometryAxes(sceneGetGeometryRoot(rendererGetPassedScene()), axes);
  gatherLightSourcesAxes(sceneGetEnabledLightSources(rendererGetPassedScene()), axes);

  glLineWidth(2.0);
  model3DUpdateBuffer(data->axisModel, AXES_INST_BUFFER_SLOT,
                      0, &axes[0], sizeof(AxesInstanceData) * axes.size());

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  
  glBindFramebuffer(GL_FRAMEBUFFER, data->ldrFBO);
  shaderProgramUse(data->axisVisualizationProgram);
  
  glBindVertexArray(model3DGetVAOHandle(data->axisModel));
  glDrawArraysInstanced(GL_LINES, 0, 2, axes.size());
  
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  
  glDisable(GL_DEPTH_TEST);  
}

static bool8 uiWidgetsVisualizationPassExecute(RenderPass* pass)
{
  UIWidgetsVisualizationPassData* data = (UIWidgetsVisualizationPassData*)renderPassGetInternalData(pass);

  if(data->showAABB == TRUE)
  {
    drawAABBs(data);
  }
  
  if(data->showFrustum == TRUE)
  {
    drawFrustum(data);
  }

  if(data->showAxes == TRUE)
  {
    drawAxes(data);
  }
  
  return TRUE;
}

static void uiWidgetsVisualizationPassDrawInputView(RenderPass* pass)
{
  UIWidgetsVisualizationPassData* data = (UIWidgetsVisualizationPassData*)renderPassGetInternalData(pass);

  const static char* visualizationModesLabels[] =
  {
    "Show dynamic AABB",
    "Show final AABB"
  };


  ImGui::Checkbox("Show frustum", (bool*)&data->showFrustum);
  ImGui::SameLine();  

  ImGui::Checkbox("Show axes", (bool*)&data->showAxes);
  
  ImGui::Checkbox("Show AABB", (bool*)&data->showAABB);
  ImGui::SameLine();
  
  ImGui::Checkbox("Show AABB parents", (bool*)&data->showParents);
  ImGui::SameLine();

  ImGui::PushItemWidth(200.0);
  ImGui::Combo("AABB Visualization mode",
               (int32*)&data->visualizationMode,
               visualizationModesLabels,
               ARRAY_SIZE(visualizationModesLabels));
  ImGui::PopItemWidth();
}

static const char* uiWidgetsVisualizationPassGetName(RenderPass* pass)
{
  return "UIWidgetsVisualizationPass";
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

static ShaderProgram* createAxesVisualizationProgram()
{
  ShaderProgram* program = nullptr;
  
  createShaderProgram(&program);
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_VERTEX_SHADER, "shaders/visualize_axis.vert"));
  shaderProgramAttachShader(program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/visualize_axis.frag"));

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

static Model3DPtr createAxisModel()
{
  Model3D* model = nullptr;
  assert(createModel3DEmpty(&model));
  
  model3DAttachBuffer(model, AXES_INST_BUFFER_SLOT, NULL, sizeof(AxesInstanceData) * MAX_INSTANCES, GL_DYNAMIC_DRAW);

  model3DDescribeInput(model, 0, AXES_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(AxesInstanceData), 0, FALSE);  // color
  model3DDescribeInput(model, 1, AXES_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(AxesInstanceData), 4 * sizeof(float32), FALSE);  // matrix col 1
  model3DDescribeInput(model, 2, AXES_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(AxesInstanceData), 8 * sizeof(float32), FALSE);  // matrix col 2
  model3DDescribeInput(model, 3, AXES_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(AxesInstanceData), 12 * sizeof(float32), FALSE); // matrix col 3
  model3DDescribeInput(model, 4, AXES_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(AxesInstanceData), 16 * sizeof(float32), FALSE); // matrix col 4  

  return Model3DPtr(model);
}

bool8 createUIWidgetsVisualizationPass(RenderPass** outPass)
{
  RenderPassInterface interface = {};
  interface.destroy = destroyUIWidgetsVisualizationPass;
  interface.execute = uiWidgetsVisualizationPassExecute;
  interface.drawInputView = uiWidgetsVisualizationPassDrawInputView;
  interface.getName = uiWidgetsVisualizationPassGetName;
  interface.type = RENDER_PASS_TYPE_UI_WIDGETS_VISUALIZATION;

  if(allocateRenderPass(interface, outPass) == FALSE)
  {
    return FALSE;
  }

  UIWidgetsVisualizationPassData* data = engineAllocObject<UIWidgetsVisualizationPassData>(MEMORY_TYPE_GENERAL);
  
  data->ldrFBO = createFramebufferD(rendererGetResourceHandle(RR_LDR1_MAP_TEXTURE),
                                    rendererGetResourceHandle(RR_DEPTH1_MAP_TEXTURE));
  assert(data->ldrFBO != 0);

  data->cubesModel = createCubesModel();
  data->frustumModel = createFrustumModel();
  data->axisModel = createAxisModel();
  
  data->aabbVisualizationProgram = ShaderProgramPtr(createAABBVisualizationProgram());
  assert(data->aabbVisualizationProgram != nullptr);

  data->frustumVisualizationProgram = ShaderProgramPtr(createFrustumVisualizationProgram());
  assert(data->frustumVisualizationProgram != nullptr);
  
  data->axisVisualizationProgram = ShaderProgramPtr(createAxesVisualizationProgram());
  assert(data->frustumVisualizationProgram != nullptr);
  
  renderPassSetInternalData(*outPass, data);
  
  return TRUE;
}

