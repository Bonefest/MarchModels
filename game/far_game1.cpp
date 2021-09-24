#if defined(ENABLE_FAR_GAME1)

#include <vector>

#include <imgui/imgui.h>
#include <linalg/linalg.h>
#include <imgui/imgui_internal.h>

#include <logging.h>
#include <application.h>
#include <memory_manager.h>
#include <game_framework.h>

using namespace linalg;
using namespace linalg::aliases;

struct NewModelParams
{

  float32 r = 0.5f;
  float32 c = 1.0f;
  float32 d = 1.0f;
  float32 theta = 90.0;
  float32 startAlpha = HALF_PI;
  float32 endAlpha = TWO_PI;
  float32 alphaStepSize = 0.1f;
  float32 startT = 0.0f;
  float32 endT = 10.0f;
  float32 tStepSize = 0.1f;  
};

struct VertexData
{
  float3 position;
  float3 normal;
};

struct SceneModel
{
  float3 rotationAxis = float3(1.0, 0.0, 0.0);
  float rotationAngle = -90.0f;
  float4x4 transform = scaling_matrix(float3(0.5));
  
  uint32 meshVAO;
  uint32 meshVBO;  
  std::vector<VertexData> meshVertices;
};

struct SceneViewport
{
  uint32 colorTarget;
  uint32 depthTarget;
  uint32 framebufferHandle;
  ImVec2 size;
  bool8 focused;
};

struct Camera
{
  float3 position = float3(0.0f, 0.0f, -10.0f);
  float32 yaw = ONE_PI;
  float32 pitch = 0.0f;
  float32 movSpeed = 5.0f;
  float32 rotSpeed = TWO_PI / 3.0f;
  
  float32 nearPlane = 0.01f;
  float32 farPlane = 100.0f;
  float32 fovY = 45.0f;
  
  float4x4 transform;
  float4x4 projection;
};

struct GameInternalData
{
  const char* sideBarWindowName;
  const char* viewportWindowName;

  ImGuiID sideBarID;
  ImGuiID sceneViewportID;
  uint32 shaderProgramHandle;
  
  NewModelParams newModelParams;
  SceneModel sceneModel;
  SceneViewport sceneViewport;
  Camera mainCamera;
  
  bool wireframeMode = false;
  
};

static GameInternalData gameData;


// ----------------------------------------------------------------------------
// Helper functions
// ----------------------------------------------------------------------------
static bool8 loadShader(GLenum shaderType, const char* path, uint32* outShaderHandle)
{
  FILE* file = fopen(path, "r");
  assert(file != nullptr);

  fseek(file, 0, SEEK_END);
  uint32 fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* fileContent = (char*)editorAllocMem(fileSize);
  fread(fileContent, fileSize, 1, file);
  
  uint32 shaderHandle = glCreateShader(shaderType);
  glShaderSource(shaderHandle, 1, &fileContent, NULL);
  glCompileShader(shaderHandle);

  int succeeded;
  glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &succeeded);
  if(!succeeded)
  {
    char shaderCompileLog[1024];
    glGetShaderInfoLog(shaderHandle, 1024, NULL, shaderCompileLog);
    
    LOG_ERROR("Cannot load shader \"%s\"!", path);
    LOG_ERROR("Log: %s", shaderCompileLog);
    return FALSE;
  }

  editorFreeMem(fileContent, fileSize);
  *outShaderHandle = shaderHandle;

  return TRUE;
}

static bool8 generateShaderProgram(const char* vertexShaderPath,
                                   const char* fragmentShaderPath,
                                   uint32* outShaderProgram)
{
  uint32 vertexShader, fragmentShader;
  if(!loadShader(GL_VERTEX_SHADER, vertexShaderPath, &vertexShader) || 
     !loadShader(GL_FRAGMENT_SHADER, fragmentShaderPath, &fragmentShader))
  {
    return FALSE;
  }

  *outShaderProgram = glCreateProgram();
  glAttachShader(*outShaderProgram, vertexShader);
  glAttachShader(*outShaderProgram, fragmentShader);
  glLinkProgram(*outShaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return TRUE;
}


static float3 calculateSurfacePoint(float alpha, float t)
{
  float32 r = gameData.newModelParams.r;
  float32 c = gameData.newModelParams.c;
  float32 d = gameData.newModelParams.d;
  float32 O = toRad(gameData.newModelParams.theta);
  float32 alpha0 = 0.0f;
  
  float x = r * cos(alpha) - (r * (alpha0 - alpha) + t * cos(O) - c * sin(d * t)*sin(O)) * sin(alpha);
  float y = r * sin(alpha) - (r * (alpha0 - alpha) + t * cos(O) - c * sin(d * t)*sin(O)) * cos(alpha);
  float z = t * sin(O) + c * sin(d * t) * cos(O);

  return float3(x, y, z);
}

static float3 calculateSurfaceNormal(float alpha, float t)
{
  float3 point = calculateSurfacePoint(alpha, t);
  float3 shiftR = normalize(calculateSurfacePoint(alpha + 0.001, t) - point);
  float3 shiftU = normalize(calculateSurfacePoint(alpha, t + 0.001) - point);
  
  return normalize(cross(shiftR, shiftU));
}

static std::vector<VertexData> generateMeshVertices(const NewModelParams& newModelParams)
{
  std::vector<VertexData> newMesh;

  for(float32 alpha = newModelParams.startAlpha;
      alpha < newModelParams.endAlpha;
      alpha += newModelParams.alphaStepSize)
  {
    float32 nextAlpha = alpha + newModelParams.alphaStepSize;
    
    for(float32 t = newModelParams.startT;
        t < newModelParams.endT;
        t+= newModelParams.tStepSize)
    {

      float32 nextT = t + newModelParams.tStepSize;
      
      VertexData v1 = {calculateSurfacePoint(alpha, t), calculateSurfaceNormal(alpha, t)};
      VertexData v2 = {calculateSurfacePoint(nextAlpha, t), calculateSurfaceNormal(nextAlpha, t)};
      VertexData v3 = {calculateSurfacePoint(alpha, nextT), calculateSurfaceNormal(alpha, nextT)};

      VertexData v4 = {calculateSurfacePoint(nextAlpha, t), calculateSurfaceNormal(nextAlpha, t)};      
      VertexData v5 = {calculateSurfacePoint(nextAlpha, nextT), calculateSurfaceNormal(nextAlpha, nextT)};
      VertexData v6 = {calculateSurfacePoint(alpha, nextT), calculateSurfaceNormal(alpha, nextT)};      

      newMesh.push_back(v1);
      newMesh.push_back(v2);
      newMesh.push_back(v3);
      newMesh.push_back(v4);
      newMesh.push_back(v5);
      newMesh.push_back(v6);
      
    }
  }

  return newMesh;
}

// ----------------------------------------------------------------------------
// Initialization/Shutdown functions
// ----------------------------------------------------------------------------

static void updateModelTransforms(SceneModel& model);
static void generateMesh();

bool8 gameExtractSetupConfig(Application* app,
                             uint32* outScreenWidth,
                             uint32* outScreenHeight,
                             const char** outName)
{
  *outScreenWidth = 1280;
  *outScreenHeight = 720;
  *outName = "FAR Lab1";

  gameData.sideBarWindowName = "Sidebar";
  gameData.viewportWindowName = "Viewport";

  return TRUE;
}

static bool8 generateMeshGLData()
{
  glGenVertexArrays(1, &gameData.sceneModel.meshVAO);
  glGenBuffers(1, &gameData.sceneModel.meshVBO);

  glBindVertexArray(gameData.sceneModel.meshVAO);
  glBindBuffer(GL_ARRAY_BUFFER, gameData.sceneModel.meshVBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
  glEnableVertexAttribArray(1);  
  glBindVertexArray(0);
  
  return TRUE;
}

static bool8 generateFramebufferGLData()
{
  glGenFramebuffers(1, &gameData.sceneViewport.framebufferHandle);
  glGenTextures(1, &gameData.sceneViewport.colorTarget);
  glGenTextures(1, &gameData.sceneViewport.depthTarget);  

  glBindFramebuffer(GL_FRAMEBUFFER, gameData.sceneViewport.framebufferHandle);
  
  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D,
                         gameData.sceneViewport.colorTarget, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D,
                         gameData.sceneViewport.depthTarget, 0);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glBindTexture(GL_TEXTURE_2D, gameData.sceneViewport.colorTarget);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  return TRUE;
}

static bool8 initializeGLData()
{
  if(!generateMeshGLData()) return FALSE;
  
  if(!generateShaderProgram("../game/far1_shader.vert",
                            "../game/far1_shader.frag",
                            &gameData.shaderProgramHandle)) return FALSE;
  
  if(!generateFramebufferGLData()) return FALSE;

  updateModelTransforms(gameData.sceneModel);
  
  return TRUE;
}

bool8 gameInitialize(Application* app)
{
  initializeGLData();
  generateMesh();
  
  return TRUE;
}

void gameShutdown(Application* app)
{
  
}

// ----------------------------------------------------------------------------
// Main logic
// ----------------------------------------------------------------------------

void updateModelTransforms(SceneModel& model)
{
  model.rotationAxis = normalize(gameData.sceneModel.rotationAxis);    

  float angle = toRad(model.rotationAngle);
  quat rotQuat = rotation_quat(model.rotationAxis, angle);
  model.transform = rotation_matrix(rotQuat);  
}

static void updateCameraProjMatrix(ImVec2 size, Camera& camera)
{
  float aspect = size.x / size.y;
  camera.projection = perspective_matrix(toRad(camera.fovY),
                                         aspect,
                                         camera.nearPlane,
                                         camera.farPlane);
}

static void updateSceneViewportData(ImVec2 size)
{
  gameData.sceneViewport.size = size;  

  // Attachments update
  glBindTexture(GL_TEXTURE_2D, gameData.sceneViewport.colorTarget);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
               size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  glBindTexture(GL_TEXTURE_2D, gameData.sceneViewport.depthTarget);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
               size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Framebuffer update
  glBindFramebuffer(GL_FRAMEBUFFER, gameData.sceneViewport.framebufferHandle);
  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D,
                         gameData.sceneViewport.colorTarget, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D,
                         gameData.sceneViewport.depthTarget, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Matrix update
  updateCameraProjMatrix(size, gameData.mainCamera);
}

static void generateMesh()
{
  SceneModel& sceneModel = gameData.sceneModel;
  sceneModel.meshVertices.clear();
  sceneModel.meshVertices = generateMeshVertices(gameData.newModelParams);
  
  glBindBuffer(GL_ARRAY_BUFFER, sceneModel.meshVBO);
  glBufferData(GL_ARRAY_BUFFER,
               sceneModel.meshVertices.size() * sizeof(VertexData),
               &sceneModel.meshVertices[0],
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  LOG_INFO("New model mesh has been generated successfully!");
}

void gameUpdate(Application* app, float64 delta)
{
  Camera& mainCamera = gameData.mainCamera;
  
  quat yawQuat = rotation_quat(float3(0.0f, 1.0f, 0.0), mainCamera.yaw);
  quat pitchQuat = rotation_quat(float3(1.0f, 0.0f, 0.0), mainCamera.pitch);
  quat rotQuat = qmul(pitchQuat, yawQuat);
    
  if(gameData.sceneViewport.focused)
  {
    GLFWwindow* window = applicationGetWindow();
    
    float3 forward = qrot(rotQuat, float3(0.0f, 0.0f, 1.0f));
    float3 side = qrot(rotQuat, float3(1.0f, 0.0f, 0.0f));
    float3 up = float3(0.0f, 1.0f, 0.0f);

    float32 speed = mainCamera.movSpeed;
    speed *= (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 2.0f : 1.0f;
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
      mainCamera.position -= forward * speed * float32(delta);
    }
    else if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
      gameData.mainCamera.position += forward * speed * float32(delta);
    }    

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
      gameData.mainCamera.position += side * speed * float32(delta);
    }
    else if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
      gameData.mainCamera.position -= side * speed * float32(delta);

    }    

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
      gameData.mainCamera.position += up * speed * float32(delta);
    }
    else if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
      gameData.mainCamera.position -= up * speed * float32(delta);

    }    
    
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
      gameData.mainCamera.yaw += mainCamera.rotSpeed * float32(delta);
    }
    else if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
      gameData.mainCamera.yaw -= mainCamera.rotSpeed * float32(delta);

    }    
    
  }

  gameData.mainCamera.transform = inverse(mul(translation_matrix(gameData.mainCamera.position),
                                              rotation_matrix(rotQuat)));  
}

static void gameGenerateLayout(Application* app)
{
  uint32 screenWidth = applicationGetScreenWidth(), screenHeight = applicationGetScreenHeight();  
  ImGuiID mainNodeID = ImGui::GetID("Main");
  
  if(ImGui::DockBuilderGetNode(mainNodeID) == nullptr)
  {
    ImGui::DockBuilderAddNode(mainNodeID, ImGuiDockNodeFlags_DockSpace |
                              ImGuiDockNodeFlags_NoTabBar |
                              ImGuiDockNodeFlags_NoCloseButton);
    
    ImGui::DockBuilderSetNodeSize(mainNodeID, ImVec2(screenWidth, screenHeight));

    ImGuiID sideBarID, sceneViewportID;

    /** DockBuilderSpitNode() splits given node on two parts */
    ImGui::DockBuilderSplitNode(mainNodeID, ImGuiDir_Left, 0.25f, &sideBarID, &sceneViewportID);

    ImGui::DockBuilderDockWindow(gameData.sideBarWindowName, sideBarID);
    ImGui::DockBuilderDockWindow(gameData.viewportWindowName, sceneViewportID);    

    ImGui::DockBuilderFinish(mainNodeID);

    gameData.sideBarID = sideBarID;
    gameData.sceneViewportID = sceneViewportID;
  }

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(screenWidth, screenHeight));

  static const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("Fundamentals of Augmented Reality | Assignment #1", nullptr, windowFlags);
  ImGui::DockSpace(mainNodeID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
  ImGui::End();
}

static void generateCameraSidebarSection(Application* app)
{
  ImGui::Text("Camera parameters");
  ImGui::SliderAngle("Camera Yaw", &gameData.mainCamera.yaw);
  ImGui::SliderAngle("Camera Pitch", &gameData.mainCamera.pitch, -90.0f, 90.0f);
  ImGui::SliderFloat3("Camera position", (float32*)&gameData.mainCamera.position, -10.0f, 10.0f, "%.1f");

  gameData.mainCamera.yaw = fmod(gameData.mainCamera.yaw, float32(TWO_PI));
}

static void generateSceneModelSidebarSection(Application* app)
{
  bool sceneObjChanged = false;

  ImGui::Text("Scene object parameters");
  sceneObjChanged |= ImGui::SliderFloat("X axis", &gameData.sceneModel.rotationAxis.x, -1.0f, 1.0f, "%.2f");
  sceneObjChanged |= ImGui::SliderFloat("Y axis", &gameData.sceneModel.rotationAxis.y, -1.0f, 1.0f, "%.2f");
  sceneObjChanged |= ImGui::SliderFloat("Z axis", &gameData.sceneModel.rotationAxis.z, -1.0f, 1.0f, "%.2f");
  sceneObjChanged |= ImGui::SliderFloat("Angle", &gameData.sceneModel.rotationAngle, 0.0f, 360.0f, "%.2f");
  if(sceneObjChanged)
  {
    updateModelTransforms(gameData.sceneModel);
  }

  ImGui::Checkbox("Enable wireframe", &gameData.wireframeMode);  
}

static void generateNewModelSidebarSection(Application* app)
{
  ImGui::Text("New object parameters");
  ImGui::SliderFloat("r constant", &gameData.newModelParams.r, 0.1f, 5.0f, "%.2f");
  ImGui::SliderFloat("c constant", &gameData.newModelParams.c, 0.1f, 5.0f, "%.2f");
  ImGui::SliderFloat("d constant", &gameData.newModelParams.d, 0.1f, 5.0f, "%.2f");
  ImGui::SliderFloat("theta angle", &gameData.newModelParams.theta, 0.0f, 360.0f, "%.2f");  
  ImGui::Spacing();
  ImGui::SliderAngle("start alpha", &gameData.newModelParams.startAlpha);
  ImGui::SliderAngle("end alpha", &gameData.newModelParams.endAlpha);
  ImGui::SliderFloat("alpha step size", &gameData.newModelParams.alphaStepSize, 0.01f, 1.0f);
  ImGui::Spacing();
  ImGui::SliderFloat("start t", &gameData.newModelParams.startT, -10.0, 10.0, "%.2f");
  ImGui::SliderFloat("end t", &gameData.newModelParams.endT, gameData.newModelParams.startT, 10.0, "%.2f");
  ImGui::SliderFloat("t step size", &gameData.newModelParams.tStepSize, 0.05, 1.0, "%.2f");
  ImGui::Spacing();  
  if(ImGui::Button("Regenerate", ImVec2(0.0f, 0.0f)))
  {
    generateMesh();
  }
 
}

static void generateSidebarWindow(Application* app)
{
  ImGui::Begin(gameData.sideBarWindowName, nullptr);

  generateCameraSidebarSection(app);
  ImGui::Separator();
  generateSceneModelSidebarSection(app);
  ImGui::Separator();  
  generateNewModelSidebarSection(app);
  ImGui::Separator();  
  
  ImGui::End();
}

static void renderScene()
{
  GLint prevViewportData[4];
  glGetIntegerv(GL_VIEWPORT, prevViewportData);
  glViewport(0, 0, gameData.sceneViewport.size.x, gameData.sceneViewport.size.y);

  glBindFramebuffer(GL_FRAMEBUFFER, gameData.sceneViewport.framebufferHandle);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, gameData.wireframeMode ? GL_LINE : GL_FILL);
  
  glUseProgram(gameData.shaderProgramHandle);

  GLint modelUniformLocation = glGetUniformLocation(gameData.shaderProgramHandle, "model");
  GLint viewUniformLocation = glGetUniformLocation(gameData.shaderProgramHandle, "view");
  GLint projectionUniformLocation = glGetUniformLocation(gameData.shaderProgramHandle, "projection");
  
  glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, &gameData.sceneModel.transform[0][0]);
  glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, &gameData.mainCamera.transform[0][0]);
  glUniformMatrix4fv(projectionUniformLocation, 1, GL_FALSE, &gameData.mainCamera.projection[0][0]);  
  
  glBindVertexArray(gameData.sceneModel.meshVAO);
  glDrawArrays(GL_TRIANGLES, 0, gameData.sceneModel.meshVertices.size());

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(prevViewportData[0], prevViewportData[1], prevViewportData[2], prevViewportData[3]);
}

static void generateViewportWindow(Application* app)
{
  ImGui::Begin(gameData.viewportWindowName, nullptr);
  ImVec2 size = ImGui::GetWindowSize(); size.y -= 35;
  
  if(gameData.sceneViewport.size.x != size.x || gameData.sceneViewport.size.y != size.y)
  {
    updateSceneViewportData(size);
  }

  gameData.sceneViewport.focused = ImGui::IsWindowFocused();
  renderScene();
  
  ImGui::Image((void*)gameData.sceneViewport.colorTarget,
               gameData.sceneViewport.size,
               ImVec2(1.0, 0.0), ImVec2(0.0f, -1.0f));
  
  ImGui::End();  
}

void gameDraw(Application* app, float64 delta)
{
  gameGenerateLayout(app);
  generateSidebarWindow(app);
  generateViewportWindow(app);
}

void gameProcessInput(Application* app, const EventData& eventData, void* sender)
{
  if(eventData.type == EVENT_TYPE_KEY_PRESSED)
  {
    if(eventData.i32[0] == GLFW_KEY_ESCAPE)
    {
      glfwSetWindowShouldClose(applicationGetWindow(), TRUE);
      return;
    }
  }
}

bool8 initializeGameFramework(GameFramework* outFramework)
{
  outFramework->extractSetupConfig = gameExtractSetupConfig;
  outFramework->initialize = gameInitialize;
  outFramework->shutdown = gameShutdown;
  outFramework->update = gameUpdate;
  outFramework->draw = gameDraw;
  outFramework->processInput = gameProcessInput;
  
  return TRUE;
}

#endif
