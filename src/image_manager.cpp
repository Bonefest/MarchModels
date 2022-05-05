#include <string>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "image_manager.h"

using std::string;
using std::unordered_map;

struct ImageManagerData
{
  bool8 initialized;
  
  unordered_map<string, ImagePtr> loadedImages;
};

static ImageManagerData data;

bool8 initializeImageManager()
{
  assert(data.initialized == FALSE);

  data.initialized = TRUE;

  return TRUE;
}

void shutdownImageManager()
{
  assert(data.initialized == TRUE);

  data.initialized = FALSE;
}

ImagePtr imageManagerLoadImage(const char* path)
{
  auto it = data.loadedImages.find(string(path));
  if(it != data.loadedImages.end())
  {
    return it->second;
  }

  int32 width, height, channelsCount;
  uint8* pixels = stbi_load(path, &width, &height, &channelsCount, 0);
  if(pixels == NULL)
  {
    LOG_ERROR("Image manager cannot load image located at '%s'", path);
    return ImagePtr(nullptr);
  }

  GLuint textureHandle;
  glGenTextures(1, &textureHandle);
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  if(channelsCount == 3)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  }
  else if(channelsCount == 4)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);    
  }
  else
  {
    assert(FALSE && "Given image's format is not supported...");
  }


  glGenerateMipmap(GL_TEXTURE_2D);
  
  glBindTexture(GL_TEXTURE_2D, 0);
  
  Image* image;
  createImage(textureHandle, width, height, &image);
  imageSetName(image, path);
  
  ImagePtr imagePtr = ImagePtr(image);

  data.loadedImages[string(path)] = imagePtr;
  
  return imagePtr;
}

bool8 imageManagerHasImage(const char* path)
{
  auto it = data.loadedImages.find(string(path));
  return it != data.loadedImages.end();
}

bool8 imageManagerFreeImage(const char* path)
{
  auto it = data.loadedImages.find(string(path));
  if(it != data.loadedImages.end())
  {
    data.loadedImages.erase(it);

    return TRUE;
  }

  return FALSE;
}

void imageManagerFreeImages()
{
  data.loadedImages.clear();
}
