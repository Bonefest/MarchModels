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
  float r = 1.0f;
  float c = 1.0f;
  float d = 1.0f;
  float theta = 90.0;
};

struct VertexData
{
  float3 position;
  float3 normal;
};

struct SceneModel
{
  float3 rotationAxis = float3(1.0, 0.0, 0.0);
  float rotationAngle = 0.0f;

  uint32 meshVAO;
  uint32 meshVBO;  
  std::vector<VertexData> meshVertices;
};

struct GameInternalData
{
  const char* sideBarWindowName;
  const char* viewportWindowName;

  ImGuiID sideBarID;
  ImGuiID viewportID;

  NewModelParams newModelParams;
  SceneModel sceneModel;

  uint32 shaderProgramHandle;
  uint32 viewportTextureHandle;
  uint32 viewportFramebufferHandle;
  ImVec2 viewportSize;
  
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
  float r = gameData.newModelParams.r;
  float c = gameData.newModelParams.c;
  float d = gameData.newModelParams.d;
  float O = gameData.newModelParams.theta;
  float alpha0 = 0.0f;
  
  float x = r * cos(alpha) - (r * (alpha0 - alpha) + t * cos(O) - c * sin(d * t)*sin(O)) * sin(alpha);
  float y = r * sin(alpha) - (r * (alpha0 - alpha) + t * cos(O) - c * sin(d * t)*sin(O)) * cos(alpha);
  float z = t * sin(O) + c * sin(d * t) * cos(O);

  return float3(x, y, z);
}

static std::vector<VertexData> generateMeshVertices(const NewModelParams& newModelParams)
{
  std::vector<VertexData> newMesh;

  const uint32 alphaSteps = 300;
  const float32 tStep = 0.1;

  auto getAlpha = [alphaSteps](uint32 index){ return float32(index % alphaSteps) / float32(alphaSteps) * TWO_PI; };
  
  for(float32 t = -5.0; t < 5.0; t += tStep)
  {
    float32 nextT = t + tStep;
    for(uint32 i = 0; i < alphaSteps + 1; i++)
    {
      float32 alpha = getAlpha(i);
      float32 nextAlpha = getAlpha(i + 1);

      VertexData v1 = {calculateSurfacePoint(alpha, t)};
      VertexData v2 = {calculateSurfacePoint(nextAlpha, t)};
      VertexData v3 = {calculateSurfacePoint(alpha, nextT)};

      newMesh.push_back(v1);
      newMesh.push_back(v2);
      newMesh.push_back(v3);      
    }
  }

  return newMesh;
}


// ----------------------------------------------------------------------------
// Initialization/Shutdown functions
// ----------------------------------------------------------------------------

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

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)offsetof(VertexData, position));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)offsetof(VertexData, normal));
  glEnableVertexAttribArray(1);  
  glBindVertexArray(0);
  
  return TRUE;
}

static bool8 generateFramebufferGLData()
{
  glGenFramebuffers(1, &gameData.viewportFramebufferHandle);
  glGenTextures(1, &gameData.viewportTextureHandle);

  glBindFramebuffer(GL_FRAMEBUFFER, gameData.viewportFramebufferHandle);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gameData.viewportTextureHandle, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glBindTexture(GL_TEXTURE_2D, gameData.viewportTextureHandle);
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

static void updateViewportFramebuffer(ImVec2 size)
{
  glBindTexture(GL_TEXTURE_2D, gameData.viewportTextureHandle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
  gameData.viewportSize = size;

  glBindFramebuffer(GL_FRAMEBUFFER, gameData.viewportFramebufferHandle);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gameData.viewportTextureHandle, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
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

    ImGuiID sideBarID, viewportID;

    /** DockBuilderSpitNode() splits given node on two parts */
    ImGui::DockBuilderSplitNode(mainNodeID, ImGuiDir_Left, 0.25f, &sideBarID, &viewportID);

    ImGui::DockBuilderDockWindow(gameData.sideBarWindowName, sideBarID);
    ImGui::DockBuilderDockWindow(gameData.viewportWindowName, viewportID);    

    ImGui::DockBuilderFinish(mainNodeID);

    gameData.sideBarID = sideBarID;
    gameData.viewportID = viewportID;
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

static void generateSidebarWindow(Application* app)
{
  ImGui::Begin(gameData.sideBarWindowName, nullptr);

  ImGui::Text("Scene object parameters");
  ImGui::SliderFloat("X axis", &gameData.sceneModel.rotationAxis.x, -1.0f, 1.0f, "%.2f");
  ImGui::SliderFloat("Y axis", &gameData.sceneModel.rotationAxis.y, -1.0f, 1.0f, "%.2f");
  ImGui::SliderFloat("Z axis", &gameData.sceneModel.rotationAxis.z, -1.0f, 1.0f, "%.2f");
  ImGui::SliderFloat("Angle", &gameData.sceneModel.rotationAngle, 0.0f, 360.0f, "%.2f");
  gameData.sceneModel.rotationAxis = normalize(gameData.sceneModel.rotationAxis);
  
  ImGui::Dummy(ImVec2(0.0f, 32.0f));
  
  ImGui::Text("New object parameters");
  ImGui::SliderFloat("r constant", &gameData.newModelParams.r, 0.1f, 5.0f, "%.2f");
  ImGui::SliderFloat("c constant", &gameData.newModelParams.c, 0.1f, 5.0f, "%.2f");
  ImGui::SliderFloat("d constant", &gameData.newModelParams.d, 0.1f, 5.0f, "%.2f");
  ImGui::SliderFloat("theta angle", &gameData.newModelParams.theta, 0.0f, 360.0f, "%.2f");
  if(ImGui::Button("Regenerate", ImVec2(0.0f, 0.0f)))
  {
    generateMesh();
  }
  
  ImGui::End();
}

static void generateViewportWindow(Application* app)
{
  ImGui::Begin(gameData.viewportWindowName, nullptr);
  ImVec2 size = ImGui::GetWindowSize();
  size.y -= 35;
  if(gameData.viewportSize.x != size.x || gameData.viewportSize.y != size.y)
  {
    updateViewportFramebuffer(size);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, gameData.viewportFramebufferHandle);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, size.x, size.y);

  glUseProgram(gameData.shaderProgramHandle);
  glBindVertexArray(gameData.sceneModel.meshVAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, applicationGetScreenWidth(), applicationGetScreenHeight());

  ImGui::Image((void*)gameData.viewportTextureHandle, gameData.viewportSize, ImVec2(1.0, 0.0), ImVec2(0.0f, -1.0f));
  
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
