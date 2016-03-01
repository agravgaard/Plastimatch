//

#ifndef ORARGBTABLEMANAGER_H_
#define ORARGBTABLEMANAGER_H_

#include <vtkColorTransferFunction.h>
#include <vtkTimeStamp.h>
#include <vtkgl.h>

namespace ora
{

/**
 * A port to internal VTK-class vtkRGBTable from vtkOpenGLGPUVolumeRayCastMapper
 * class. This class helps converting color transfer functions to openGL
 * textures (1D).
 *
 * @see vtkOpenGLGPUVolumeRayCastMapper
 *
 * @author VTK-community
 * @author phil 
 * @version 1.1
 */
class RGBTableManager
{
public:

  /** Default constructor **/
  RGBTableManager();
  /** Destructor **/
  ~RGBTableManager();

  /** @return TRUE if table is readily loaded as texture **/
  bool IsLoaded();

  /** Bind the texture. **/
  void Bind();

  /** Update texture from transfer function. **/
  void Update(vtkColorTransferFunction *scalarRGB, double range[2],
      bool linearInterpolation);

protected:
  /** internal openGL texture ID **/
  GLuint TextureId;
  /** time helper **/
  vtkTimeStamp BuildTime;
  /** internal table representation **/
  float *Table;
  /** flag indicating whether texture is loaded **/
  bool Loaded;
  /** help flag for interpolation **/
  bool LastLinearInterpolation;

};

}

#endif /* ORARGBTABLEMANAGER_H_ */
