#include <vector>
#include <unordered_map>

#include "model3d.h"
#include "renderer.h"
#include "shader_manager.h"
#include "renderer_utils.h"
#include "billboard_system.h"

using std::pair;
using std::vector;
using std::unordered_map;

const static uint32 RECT_VRT_BUFFER_SLOT  = 0;
const static uint32 RECT_INST_BUFFER_SLOT = 1;
const static uint32 MAX_BATCH_SIZE        = 128;

const static float32 rectangleVertexData[] =
{
  // Coordinates are provided in our RHS system (x - left, y - up, z - forward)
   1.0f, -1.0f, 0.0f, /* Left, bottom, near  */ 0.0f, 0.0f, /* UV left bottom  */
  -1.0f, -1.0f, 0.0f, /* Right, bottom, near */ 1.0f, 0.0f, /* UV right bottom */
  -1.0f,  1.0f, 0.0f, /* Right, top, near    */ 1.0f, 1.0f, /* UV right top    */
   1.0f,  1.0f, 0.0f, /* Left, top, near     */ 0.0f, 1.0f  /* UV left top     */
};

const static uint32 rectangleIndexData[] =
{
  0, 1, 2, 0, 2, 3
};

struct BillboardData
{
  float2 scale;
  float3 offset;  
  float3 position;
  float4 color;
  float4 uvRect;
};

struct BillboardSystemData
{
  bool8 initiailized;
  
  GLuint ldrFramebuffer;
  Model3DPtr rectangleModel;
  ShaderProgram* program;
  
  vector<pair<GLuint, BillboardData>> painterOrderedBatches;
  unordered_map<GLuint, vector<BillboardData>> zOrderedBatches;
};

static BillboardSystemData data;

static bool8 createLDRFramebuffer()
{
  GLuint framebuffer = 0;
  
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rendererGetResourceHandle(RR_LDR_MAP_TEXTURE), 0);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, rendererGetResourceHandle(RR_DISTANCES_MAP_TEXTURE), 0); 
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    return FALSE;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  data.ldrFramebuffer = framebuffer;
  
  return TRUE;
}

static bool8 createProgram()
{
  createShaderProgram(&data.program);
  shaderProgramAttachShader(data.program, shaderManagerLoadShader(GL_VERTEX_SHADER, "shaders/billboard.vert"));
  shaderProgramAttachShader(data.program, shaderManagerLoadShader(GL_FRAGMENT_SHADER, "shaders/billboard.frag"));

  if(linkShaderProgram(data.program) == FALSE)
  {
    destroyShaderProgram(data.program);
    return FALSE;
  }

  return TRUE;
}

static void createRectangleModel()
{
  Model3D* model = nullptr;
  assert(createModel3DEmpty(&model));

  model3DAttachIndexBuffer(model, rectangleIndexData, sizeof(rectangleIndexData), GL_STATIC_DRAW);
  model3DAttachBuffer(model, RECT_VRT_BUFFER_SLOT, rectangleVertexData, sizeof(rectangleVertexData), GL_STATIC_DRAW);
  model3DAttachBuffer(model, RECT_INST_BUFFER_SLOT, NULL, sizeof(BillboardData) * MAX_BATCH_SIZE, GL_DYNAMIC_DRAW);

  // Per-vertex input description
  model3DDescribeInput(model, 0, RECT_VRT_BUFFER_SLOT, 3, GL_FLOAT, 5 * sizeof(float32), 0);
  model3DDescribeInput(model, 1, RECT_VRT_BUFFER_SLOT, 2, GL_FLOAT, 5 * sizeof(float32), sizeof(float32) * 3);

  // Per-instance input description
  model3DDescribeInput(model, 2, RECT_INST_BUFFER_SLOT, 2, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, scale), FALSE);    // Scale  
  model3DDescribeInput(model, 3, RECT_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, offset), FALSE);   // Offset  
  model3DDescribeInput(model, 4, RECT_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, position), FALSE); // Position
  model3DDescribeInput(model, 5, RECT_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, color), FALSE);    // Color
  model3DDescribeInput(model, 6, RECT_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, uvRect), FALSE);   // UV Rect
  
  data.rectangleModel = Model3DPtr(model);
}

bool8 initializeBillboardSystem()
{
  assert(data.initiailized == FALSE);

  if(createLDRFramebuffer() == FALSE)
  {
    return FALSE;
  }

  if(createProgram() == FALSE)
  {
    return FALSE;
  }
  
  createRectangleModel();
  
  data.initiailized = TRUE;

  return TRUE;
}

void shutdownBillboardSystem()
{
  assert(data.initiailized == TRUE);

  data.initiailized = FALSE;
}

void billboardSystemDrawImage(ImagePtr image,
                              float3 worldPosition,
                              float2 uvMin,
                              float2 uvMax,
                              float4 color,                              
                              float2 scale,
                              float3 offset,
                              bool8 usePainterOrder)
{
  BillboardData billboardData =
  {
    scale,
    offset,        
    worldPosition,
    color,
    float4(uvMin.x, uvMin.y, uvMax.x, uvMax.y)
  };

  if(usePainterOrder == TRUE)
  {
    data.painterOrderedBatches.push_back(std::make_pair(imageGetGLHandle(image), billboardData));
  }
  else
  {
    data.zOrderedBatches[imageGetGLHandle(image)].push_back(billboardData);
  }
}

void billboardSystemDrawImagePix(ImagePtr image,
                                 float3 worldPosition,
                                 uint2 pixelSize,
                                 uint2 pixelOffset,
                                 float4 color,
                                 float2 scale,
                                 float3 offset,
                                 bool8 usePainterOrder)
{
  float32 invWidth = 1.0f / float32(imageGetWidth(image));
  float32 invHeight = 1.0f / float32(imageGetHeight(image));

  float2 uvMin = float2(float32(pixelOffset.x) * invWidth, float32(pixelOffset.y) * invHeight);
  float2 uvMax = float2(float32(pixelSize.x) * invWidth, float32(pixelSize.y) * invHeight) + uvMin;

  billboardSystemDrawImage(image, worldPosition, uvMin, uvMax, color, scale, offset, usePainterOrder);
}


void billboardSystemPresent()
{
  glBindFramebuffer(GL_FRAMEBUFFER, data.ldrFramebuffer);
  shaderProgramUse(data.program);
  glBindVertexArray(model3DGetVAOHandle(data.rectangleModel));

  
  glEnable(GL_BLEND);
  pushBlend(GL_FUNC_ADD, GL_FUNC_ADD,
            GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  
  for(auto batch: data.zOrderedBatches)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, batch.first);
    
    model3DUpdateBuffer(data.rectangleModel, RECT_INST_BUFFER_SLOT,
                        0, &batch.second[0], sizeof(BillboardData) * batch.second.size());

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, batch.second.size());
  }
  
  //glDisable(GL_DEPTH_TEST);
  
  for(auto batch: data.painterOrderedBatches)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, batch.first);
    
    model3DUpdateBuffer(data.rectangleModel, RECT_INST_BUFFER_SLOT,
                        0, &batch.second, sizeof(BillboardData));

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1);
  }
  
  glBindVertexArray(0);
  shaderProgramUse(nullptr);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  popBlend();
  
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  data.painterOrderedBatches.clear();  
  data.zOrderedBatches.clear();
}


