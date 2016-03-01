//
#include "oraImageConsumer.h"

namespace ora
{

ImageConsumer::ImageConsumer()
{
  m_Image = NULL;
  m_DeleteImageAtEnd = false;
}

ImageConsumer::ImageConsumer(bool deleteImageAtEnd)
{
  m_Image = NULL;
  m_DeleteImageAtEnd = deleteImageAtEnd;
}

ImageConsumer::~ImageConsumer()
{
  UnconsumeImage();
}

void ImageConsumer::ConsumeImage(ImageType *image)
{
  m_Image = image;
}

void ImageConsumer::UnconsumeImage()
{
  if (m_DeleteImageAtEnd)
  {
    if (m_Image)
      delete m_Image;
  }
  m_Image = NULL;
}

ImageConsumer::ImageType *ImageConsumer::ProduceImage()
{
  return m_Image;
}

}
