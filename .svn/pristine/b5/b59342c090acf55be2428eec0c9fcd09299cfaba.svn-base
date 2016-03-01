//

#include "oraScalarFieldManager.h"

#include <vtkAbstractMapper.h>
#include <vtkFloatArray.h>

namespace ora
{

ScalarFieldManager::ScalarFieldManager()
{
  this->TextureId = 0;
  this->Loaded = false;
  this->SupportFloatTextures = false;
  this->LoadedTableRange[0] = 0.0;
  this->LoadedTableRange[1] = 1.0;
  this->LoadedExtent[0] = VTK_INT_MAX;
  this->LoadedExtent[1] = VTK_INT_MIN;
  this->LoadedExtent[2] = VTK_INT_MAX;
  this->LoadedExtent[3] = VTK_INT_MIN;
  this->LoadedExtent[4] = VTK_INT_MAX;
  this->LoadedExtent[5] = VTK_INT_MIN;
}

ScalarFieldManager::~ScalarFieldManager()
{
  if (this->TextureId != 0)
  {
    glDeleteTextures(1, &this->TextureId);
    this->TextureId = 0;
  }
}

vtkTimeStamp ScalarFieldManager::GetBuildTime()
{
  return this->BuildTime;
}

void ScalarFieldManager::Bind()
{
  glBindTexture(vtkgl::TEXTURE_3D, this->TextureId);
}

void ScalarFieldManager::Update(vtkImageData *input, int textureExtent[6],
    int scalarMode, int arrayAccessMode, int arrayId, const char *arrayName,
    bool linearInterpolation, double tableRange[2], int maxMemoryInBytes)
{
  bool needUpdate = false;
  bool modified = false;

  if (this->TextureId == 0)
  {
    glGenTextures(1, &this->TextureId);
    needUpdate = true;
  }
  glBindTexture(vtkgl::TEXTURE_3D, this->TextureId);

  int obsolete = needUpdate || !this->Loaded || input->GetMTime()
      > this->BuildTime;
  if (!obsolete)
  {
    int i = 0;
    while (!obsolete && i < 6)
    {
      obsolete = obsolete || this->LoadedExtent[i] > textureExtent[i];
      ++i;
      obsolete = obsolete || this->LoadedExtent[i] < textureExtent[i];
      ++i;
    }
  }

  if (!obsolete)
  {
    obsolete = this->LoadedTableRange[0] != tableRange[0]
        || this->LoadedTableRange[1] != tableRange[1];
  }

  if (obsolete)
  {
    this->Loaded = false;
    int dim[3];
    input->GetDimensions(dim);

    GLint internalFormat = 0;
    GLenum format = 0;
    GLenum type = 0;
    // shift then scale: y:=(x+shift)*scale
    double shift = 0.0;
    double scale = 1.0;
    int needTypeConversion = 0;
    vtkDataArray *sliceArray = NULL;

    int cf; // cell-flag not used
    vtkDataArray *scalars = vtkAbstractMapper::GetScalars(input, scalarMode,
        arrayAccessMode, arrayId, arrayName, cf);

    // DONT USE GetScalarType() or GetNumberOfScalarComponents() on
    // ImageData as it deals only with point data...

    int scalarType = scalars->GetDataType();
    switch (scalarType) {
      case VTK_FLOAT:
        if (this->SupportFloatTextures)
          internalFormat = vtkgl::INTENSITY16F_ARB;
        else
          internalFormat = GL_INTENSITY16;
        format = GL_RED; // 1 channel
        type = GL_FLOAT;
        shift = -tableRange[0];
        scale = 1 / (tableRange[1] - tableRange[0]);
        break;
      case VTK_UNSIGNED_CHAR:
        internalFormat = GL_INTENSITY8;
        format = GL_RED;
        type = GL_UNSIGNED_BYTE;
        shift = -tableRange[0] / VTK_UNSIGNED_CHAR_MAX;
        scale = VTK_UNSIGNED_CHAR_MAX / (tableRange[1] - tableRange[0]);
        break;
      case VTK_SIGNED_CHAR:
        internalFormat = GL_INTENSITY8;
        format = GL_RED;
        type = GL_BYTE;
        shift = -(2 * tableRange[0] + 1) / VTK_UNSIGNED_CHAR_MAX;
        scale = VTK_SIGNED_CHAR_MAX / (tableRange[1] - tableRange[0]);
        break;
      case VTK_CHAR:
        vtkstd::cerr << "VTK_CHAR scalar field type not supported!\n";
        break;
      case VTK_BIT:
        vtkstd::cerr << "VTK_BIT scalar field type not supported!\n";
        break;
      case VTK_ID_TYPE:
        vtkstd::cerr << "VTK_ID_TYPE scalar field type not supported!\n";
        break;
      case VTK_INT:
        internalFormat = GL_INTENSITY16;
        format = GL_RED;
        type = GL_INT;
        shift = -(2 * tableRange[0] + 1) / VTK_UNSIGNED_INT_MAX;
        scale = VTK_INT_MAX / (tableRange[1] - tableRange[0]);
        break;
      case VTK_DOUBLE:
      case VTK___INT64:
      case VTK_LONG:
      case VTK_LONG_LONG:
      case VTK_UNSIGNED___INT64:
      case VTK_UNSIGNED_LONG:
      case VTK_UNSIGNED_LONG_LONG:
        needTypeConversion = 1; // --> float
        if (this->SupportFloatTextures)
          internalFormat = vtkgl::INTENSITY16F_ARB;
        else
          internalFormat = GL_INTENSITY16;
        format = GL_RED;
        type = GL_FLOAT;
        shift = -tableRange[0];
        scale = 1 / (tableRange[1] - tableRange[0]);
        sliceArray = vtkFloatArray::New();
        break;
      case VTK_SHORT:
        internalFormat = GL_INTENSITY16;
        format = GL_RED;
        type = GL_SHORT;
        shift = -(2 * tableRange[0] + 1) / VTK_UNSIGNED_SHORT_MAX;
        scale = VTK_SHORT_MAX / (tableRange[1] - tableRange[0]);
        break;
      case VTK_STRING:
        vtkstd::cerr << "VTK_STRING scalar field type not supported!\n";
        break;
      case VTK_UNSIGNED_SHORT:
        internalFormat = GL_INTENSITY16;
        format = GL_RED;
        type = GL_UNSIGNED_SHORT;
        shift = -tableRange[0] / VTK_UNSIGNED_SHORT_MAX;
        scale = VTK_UNSIGNED_SHORT_MAX / (tableRange[1] - tableRange[0]);
        break;
      case VTK_UNSIGNED_INT:
        internalFormat = GL_INTENSITY16;
        format = GL_RED;
        type = GL_UNSIGNED_INT;

        shift = -tableRange[0] / VTK_UNSIGNED_INT_MAX;
        scale = VTK_UNSIGNED_INT_MAX / (tableRange[1] - tableRange[0]);
        break;
      default:
        vtkstd::cerr << "Scalar field type not possible!\n";
        break;
    }

    // enough memory?
    int textureSize[3];
    int i = 0;
    while (i < 3)
    {
      textureSize[i] = textureExtent[2 * i + 1] - textureExtent[2 * i] + 1;
      ++i;
    }

    GLint width;
    glGetIntegerv(vtkgl::MAX_3D_TEXTURE_SIZE, &width);
    this->Loaded = textureSize[0] <= width && textureSize[1] <= width
        && textureSize[2] <= width;
    if (this->Loaded)
    {
      // the texture size is theoretically small enough for OpenGL
      vtkgl::TexImage3D(vtkgl::PROXY_TEXTURE_3D, 0, internalFormat,
          textureSize[0], textureSize[1], textureSize[2], 0, format, type, 0);
      glGetTexLevelParameteriv(vtkgl::PROXY_TEXTURE_3D, 0, GL_TEXTURE_WIDTH,
          &width);
      this->Loaded = width != 0;

      if (this->Loaded)
      {
        // but some cards always succeed with a proxy texture let's try to
        // actually allocate ...
        vtkgl::TexImage3D(vtkgl::TEXTURE_3D, 0, internalFormat, textureSize[0],
            textureSize[1], textureSize[2], 0, format, type, 0);
        GLenum errorCode = glGetError();
        this->Loaded = errorCode != GL_OUT_OF_MEMORY;
        if (this->Loaded)
        {
          // actual allocation succeeded
          if (errorCode != GL_NO_ERROR)
          {
            vtkstd::cerr << "after try to load the texture";
            vtkstd::cerr << " ERROR (x" << hex << errorCode << ") " << dec;
            vtkstd::cerr << endl;
          }
          // but some cards don't report allocation error
          this->Loaded = textureSize[0] * textureSize[1] * textureSize[2]
              * vtkAbstractArray::GetDataTypeSize(scalarType)
              * scalars->GetNumberOfComponents() <= maxMemoryInBytes;

          if (this->Loaded)
          {
            // consider the allocation above succeeded ...
            // if it actually didn't, the only to fix it for the user
            // is to increase the value of MaxMemoryInBytes.
            // enough memory! can load the scalars!
            double bias = shift * scale;

            // we don't clamp to edge because for the computation of the
            // gradient on the border we need some external value.
            glTexParameterf(vtkgl::TEXTURE_3D, vtkgl::TEXTURE_WRAP_R,
                vtkgl::CLAMP_TO_EDGE);
            glTexParameterf(vtkgl::TEXTURE_3D, GL_TEXTURE_WRAP_S,
                vtkgl::CLAMP_TO_EDGE);
            glTexParameterf(vtkgl::TEXTURE_3D, GL_TEXTURE_WRAP_T,
                vtkgl::CLAMP_TO_EDGE);

            GLfloat borderColor[4] = { 0.0, 0.0, 0.0, 0.0 };
            glTexParameterfv(vtkgl::TEXTURE_3D, GL_TEXTURE_BORDER_COLOR,
                borderColor);

            if (needTypeConversion)
            {
              // Convert and send to the GPU, z-slice by z-slice.
              // Allocate memory on the GPU (NULL data pointer with the right
              // dimensions)
              // Here we are assuming that GL_ARB_texture_non_power_of_two is
              // available
              glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

              // memory allocation is already done.
              // Send the slices: allocate CPU memory for a slice.
              sliceArray->SetNumberOfComponents(1);
              sliceArray->SetNumberOfTuples(textureSize[0] * textureSize[1]);

              void *slicePtr = sliceArray->GetVoidPointer(0);
              int k = 0;
              int kInc = dim[0] * dim[1];
              int kOffset = (textureExtent[4] * dim[1] + textureExtent[2])
                  * dim[0] + textureExtent[0];
              while (k < textureSize[2])
              {
                int j = 0;
                int jOffset = 0;
                int jDestOffset = 0;
                while (j < textureSize[1])
                {
                  i = 0;
                  while (i < textureSize[0])
                  {
                    sliceArray->SetTuple1(jDestOffset + i, (scalars->GetTuple1(
                        kOffset + jOffset + i) + shift) * scale);
                    ++i;
                  }
                  ++j;
                  jOffset += dim[0];
                  jDestOffset += textureSize[0];
                }
                vtkgl::TexSubImage3D(vtkgl::TEXTURE_3D, 0, 0, 0, k,
                    textureSize[0], textureSize[1], 1, // depth is 1, not 0!
                    format, type, slicePtr);
                ++k;
                kOffset += kInc;
              }
              sliceArray->Delete();
            }
            else
            {
              // One chunk of data to the GPU.
              // Here we are assuming that GL_ARB_texture_non_power_of_two is
              // available
              //  make sure any previous OpenGL call is executed and will not
              // be disturbed by our PixelTransfer value
              glFinish();
              glPixelTransferf(GL_RED_SCALE, static_cast<GLfloat> (scale));
              glPixelTransferf(GL_RED_BIAS, static_cast<GLfloat> (bias));
              glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

              if (!(textureExtent[1] - textureExtent[0] == dim[0]))
                glPixelStorei(GL_UNPACK_ROW_LENGTH, dim[0]);

              if (!(textureExtent[3] - textureExtent[2] == dim[1]))
                glPixelStorei(vtkgl::UNPACK_IMAGE_HEIGHT_EXT, dim[1]);

              void *dataPtr =
                  scalars->GetVoidPointer(((textureExtent[4] * dim[1]
                      + textureExtent[2]) * dim[0] + textureExtent[0]) * 1);

              vtkgl::TexImage3D(vtkgl::TEXTURE_3D, 0, internalFormat,
                  textureSize[0], textureSize[1], textureSize[2], 0, format,
                  type, dataPtr);

              // make sure TexImage3D is executed with our PixelTransfer mode
              glFinish();
              // Restore the default values.
              glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
              glPixelStorei(vtkgl::UNPACK_IMAGE_HEIGHT_EXT, 0);
              glPixelTransferf(GL_RED_SCALE, 1.0);
              glPixelTransferf(GL_RED_BIAS, 0.0);
            }

            i = 0;
            while (i < 6)
            {
              this->LoadedExtent[i] = textureExtent[i];
              ++i;
            }

            double spacing[3];
            double origin[3];
            input->GetSpacing(spacing);
            input->GetOrigin(origin);
            int swapBounds[3];
            swapBounds[0] = (spacing[0] < 0);
            swapBounds[1] = (spacing[1] < 0);
            swapBounds[2] = (spacing[2] < 0);

            // slabsPoints[i]=(slabsDataSet[i] - origin[i/2]) / spacing[i/2];
            // in general, x=o+i*spacing.
            // if spacing is positive min extent match the min of the
            // bounding box
            // and the max extent match the max of the bounding box
            // if spacing is negative min extent match the max of the
            // bounding box
            // and the max extent match the min of the bounding box

            // if spacing is negative, we may have to rethink the equation
            // between real point and texture coordinate...
            this->LoadedBounds[0] = origin[0]
                + static_cast<double> (this->LoadedExtent[0 + swapBounds[0]])
                    * spacing[0];
            this->LoadedBounds[2] = origin[1]
                + static_cast<double> (this->LoadedExtent[2 + swapBounds[1]])
                    * spacing[1];
            this->LoadedBounds[4] = origin[2]
                + static_cast<double> (this->LoadedExtent[4 + swapBounds[2]])
                    * spacing[2];
            this->LoadedBounds[1] = origin[0]
                + static_cast<double> (this->LoadedExtent[1 - swapBounds[0]])
                    * spacing[0];
            this->LoadedBounds[3] = origin[1]
                + static_cast<double> (this->LoadedExtent[3 - swapBounds[1]])
                    * spacing[1];
            this->LoadedBounds[5] = origin[2]
                + static_cast<double> (this->LoadedExtent[5 - swapBounds[2]])
                    * spacing[2];

            this->LoadedTableRange[0] = tableRange[0];
            this->LoadedTableRange[1] = tableRange[1];
            modified = true;
          } // if enough memory
        } // load fail with out of memory
      } // proxy ok
    } // else: out of theoretical limitations
  } // if obsolete

  if (this->Loaded && (needUpdate || modified || linearInterpolation
      != this->LinearInterpolation))
  {
    this->LinearInterpolation = linearInterpolation;
    if (this->LinearInterpolation)
    {
      glTexParameterf(vtkgl::TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameterf(vtkgl::TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
      glTexParameterf(vtkgl::TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(vtkgl::TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    modified = true;
  }

  if (modified)
    this->BuildTime.Modified();
}

double *
ScalarFieldManager::GetLoadedBounds()
{
  return this->LoadedBounds;
}

vtkIdType *
ScalarFieldManager::GetLoadedExtent()
{
  return this->LoadedExtent;
}

bool ScalarFieldManager::IsLoaded()
{
  return this->Loaded;
}

bool ScalarFieldManager::GetSupportFloatTextures()
{
  return this->SupportFloatTextures;
}

void ScalarFieldManager::SetSupportFloatTextures(bool value)
{
  this->SupportFloatTextures = value;
}

}
