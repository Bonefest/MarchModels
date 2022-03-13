#include "image.h"

struct Image
{
  GLuint textureHandle;

  uint32 width;
  uint32 height;
};

bool8 createImage(GLuint texture, uint32 width, uint32 height, Image** outImage)
{
  Image* image = engineAllocObject<Image>(MEMORY_TYPE_GENERAL);
  *outImage = image;

  image->textureHandle = texture;
  image->width = width;
  image->height = height;

  return TRUE;
}

void destroyImage(Image* image)
{
  if(image->textureHandle != 0)
  {
    glDeleteTextures(1, &image->textureHandle);
    image->textureHandle = 0;
  }
  
  engineFreeObject(image, MEMORY_TYPE_GENERAL);
}

uint32 imageGetWidth(Image* image)
{
  return image->width;
}

uint32 imageGetHeight(Image* image)
{
  return image->height;
}

uint2 imageGetSize(Image* image)
{
  return uint2(image->width, image->height);
}

GLuint imageGetGLHandle(Image* image)
{
  return image->textureHandle;
}
