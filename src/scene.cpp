#include <algorithm>

#include "memory_manager.h"

#include "scene.h"

using std::vector;

struct Scene
{
  vector<Shape*> shapesArray;
};

bool8 createScene(Scene** outScene)
{
  *outScene = engineAllocObject<Scene>(MEMORY_TYPE_GENERAL);

  return TRUE;
}

void destroyScene(Scene* scene)
{
  engineFreeObject(scene, MEMORY_TYPE_GENERAL);
}

void sceneAddShape(Scene* scene, Shape* shape)
{
  scene->shapesArray.push_back(shape);
}

bool8 sceneRemoveShape(Scene* scene, Shape* shape)
{
  auto it = std::find(scene->shapesArray.begin(), scene->shapesArray.end(), shape);
  if(it == scene->shapesArray.end())
  {
    return FALSE;
  }

  scene->shapesArray.erase(it);
  
  return TRUE;
}

const vector<Shape*>& sceneGetShapes(Scene* scene)
{
  return scene->shapesArray;
}

void sceneAddLightSource(Scene* scene, LightSource* lightSource)
{

}

void sceneRemoveLightSource(Scene* scene, LightSource* lightSource)
{

}

// const std::vector<LightSource*>& sceneGetLightSources(Scene* scene)
// {

// }

IntersectionDetails sceneFindIntersection(Scene* scene, Ray ray)
{

}

