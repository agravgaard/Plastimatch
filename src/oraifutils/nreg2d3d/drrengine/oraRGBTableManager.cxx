//

#include "oraRGBTableManager.h"

namespace ora
{

RGBTableManager::RGBTableManager()
{
  this->TextureId = 0;
  this->Table = 0;
  this->Loaded = false;
  this->LastLinearInterpolation = false;
}

RGBTableManager::~RGBTableManager()
{
  if (this->TextureId != 0)
  {
    glDeleteTextures(1, &this->TextureId);
    this->TextureId = 0;
  }
  if (this->Table != 0)
  {
    delete[] this->Table;
    this->Table = 0;
  }
}

bool RGBTableManager::IsLoaded()
{
  return this->Loaded;
}

void RGBTableManager::Bind()
{
  glBindTexture(GL_TEXTURE_1D, this->TextureId);
}

void RGBTableManager::Update(vtkColorTransferFunction *scalarRGB,
    double range[2], bool linearInterpolation)
{
  const int TABLE_SIZE = 1024;
  bool needUpdate = false;

  if (this->TextureId == 0)
  {
    glGenTextures(1, &this->TextureId);
    needUpdate = true;
  }

  glBindTexture(GL_TEXTURE_1D, this->TextureId);

  if (needUpdate)
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, vtkgl::CLAMP_TO_EDGE);

  if (scalarRGB->GetMTime() > this->BuildTime || needUpdate || !this->Loaded)
  {
    this->Loaded = false;
    if (this->Table == 0)
      this->Table = new float[TABLE_SIZE * 3];

    scalarRGB->GetTable(range[0], range[1], TABLE_SIZE, this->Table);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16, TABLE_SIZE, 0, GL_RGB, GL_FLOAT,
        this->Table);

    this->Loaded = true;
    this->BuildTime.Modified();
  }

  needUpdate = needUpdate || this->LastLinearInterpolation
      != linearInterpolation;
  if (needUpdate)
  {
    this->LastLinearInterpolation = linearInterpolation;
    GLint value;
    if (linearInterpolation)
      value = GL_LINEAR;
    else
      value = GL_NEAREST;
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, value);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, value);
  }
}

}
