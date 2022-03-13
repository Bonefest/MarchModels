#include <vector>
#include <unordered_map>

#include "billboard_system.h"

using std::vector;
using std::unordered_map;

const static uint32 RECT_VRT_BUFFER_SLOT  = 0;
const static uint32 RECT_INST_BUFFER_SLOT = 1;
const static uint32 MAX_BATCH_SIZE        = 128;

const static float32 rectangleVertexData[] =
{
  // Coordinates are provided in our RHS system (x - left, y - up, z - forward)
   1.0f, -1.0f, 0.0f, // Left, bottom, near
  -1.0f, -1.0f, 0.0f, // Right, bottom, near
  -1.0f,  1.0f, 0.0f, // Right, top, near
   1.0f,  1.0f, 0.0f, // Left, top, near    
};

const static uint32 rectangleIndexData[] =
{
  0, 1, 2, 0, 2, 3
};

struct BillboardData
{
  float2 size;
  float3 worldPosition;
  float4 color;
  float4 uvMinMax;
};

struct BillboardSystemData
{
  bool8 initiailized;
  
  GLuint ldrFramebuffer;
  Model3DPtr rectangleModel;
  
  unordered_map<GLuint, vector<BillboardData>> batches;
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

static void createRectangleModel()
{
  Model3D* model = nullptr;
  assert(createModel3DEmpty(&model));

  model3DAttachIndexBuffer(model, rectangleIndexData, sizeof(rectangleIndexData), GL_STATIC_DRAW);
  model3DAttachBuffer(model, RECT_VRT_BUFFER_SLOT, rectangleVertexData, sizeof(rectangleVertexData), GL_STATIC_DRAW);
  model3DAttachBuffer(model, RECT_INST_BUFFER_SLOT, NULL, sizeof(BillboardData) * MAX_BATCH_SIZE, GL_DYNAMIC_DRAW);

  model3DDescribeInput(model, 0, RECT_VRT_BUFFER_SLOT, 3, GL_FLOAT, 3 * sizeof(float32), 0);
  model3DDescribeInput(model, 1, RECT_INST_BUFFER_SLOT, 2, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, size), FALSE);
  model3DDescribeInput(model, 2, CUBE_INST_BUFFER_SLOT, 3, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, position), FALSE);
  model3DDescribeInput(model, 3, CUBE_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, color), FALSE);
  model3DDescribeInput(model, 4, CUBE_INST_BUFFER_SLOT, 4, GL_FLOAT, sizeof(BillboardData), offsetof(BillboardData, uvMinMax), FALSE);    
  
  data.rectangleModel = Model3DPtr(model);
}

bool8 initializeBillboardSystem()
{
  assert(data.initiailized == FALSE);

  data.initiailized = TRUE;

  return TRUE;
}

void shutdownBillboardSystem()
{
  assert(data.initiailized == TRUE);

  data.initiailized = FALSE;
}

void billboardDrawSprite(GLuint imageHandle,
                         float3 worldPosition,
                         float2 size,
                         float4 color,
                         float2 uvMin,
                         float2 uvMax)
{
  BillboardData billboardData =
  {
    size,
    worldPosition,
    color,
    float4(uvMin.x, uvMin.y, uvMin.x, uvMin.y)
  };

  batches[imageHandle].push_back(billboardData);
}

void billboardPresent()
{
  for(auto batch: data.batches)
  {
    
  }
}


