//

#ifndef ORASCALARFIELDMANAGER_H_
#define ORASCALARFIELDMANAGER_H_

#include <vtkTimeStamp.h>
#include <vtkImageData.h>
#include <vtkgl.h>
#include <vtkstd/map>

namespace ora
{

/**
 * A port to internal VTK-class vtkKWScalarField from
 * vtkOpenGLGPUVolumeRayCastMapper class. This class helps converting scalar
 * fields to openGL textures (3D).
 *
 * @see vtkOpenGLGPUVolumeRayCastMapper
 *
 * @author VTK-community
 * @author phil 
 * @version 1.0
 */
class ScalarFieldManager
{
public:
  /** Default constructor **/
  ScalarFieldManager();
  /** Destructor **/
  ~ScalarFieldManager();

  /** Get build time. **/
  vtkTimeStamp GetBuildTime();

  /** Bind the texture. **/
  void Bind();

  /** Update texture from scalar field. **/
  void Update(vtkImageData *input, int textureExtent[6], int scalarMode,
      int arrayAccessMode, int arrayId, const char *arrayName,
      bool linearInterpolation, double tableRange[2], int maxMemoryInBytes);

  /** Get loaded bounds. **/
  double *GetLoadedBounds();

  /** Get loaded extent. **/
  vtkIdType *GetLoadedExtent();

  /** Return whether texture is loaded. **/
  bool IsLoaded();

  /** Return whether float textures are supported. **/
  bool GetSupportFloatTextures();

  /** Set whether float textures are supported. **/
  void SetSupportFloatTextures(bool value);

protected:
  /** texture ID **/
  GLuint TextureId;
  /** helper time stamp **/
  vtkTimeStamp BuildTime;
  /** loaded bounds of scalar field **/
  double LoadedBounds[6];
  /** loaded extent of scalar field **/
  vtkIdType LoadedExtent[6];
  /** flag indicating whether loaded **/
  bool Loaded;
  /** use linar or nearest interpolation **/
  bool LinearInterpolation;
  /** flag indicating whether float textures are supported **/
  bool SupportFloatTextures;
  /** scalar range **/
  double LoadedTableRange[2];

};

/**
 * A port to internal VTK-class vtkMapDataArrayTextureId from
 * vtkOpenGLGPUVolumeRayCastMapper class.
 * This class helps indexing scalar field textures.
 *
 * @see vtkOpenGLGPUVolumeRayCastMapper
 *
 * @author VTK-community
 * @author phil 
 * @version 1.0
 */
class ScalarFieldTextureMapper
{
public:
  /** Mapping of image data and scalar field managers. **/
  vtkstd::map<vtkImageData *, ScalarFieldManager *> Map;

  /** Constructor. **/
  ScalarFieldTextureMapper()
  {
  }
  ;

private:
  /** Undefined copy constructor **/
  ScalarFieldTextureMapper(const ScalarFieldTextureMapper &other);
  /** undefined assignment operator **/
  ScalarFieldTextureMapper &operator=(const ScalarFieldTextureMapper &other);

};

}

#endif /* ORASCALARFIELDMANAGER_H_ */
