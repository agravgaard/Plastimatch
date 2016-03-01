//
#ifndef VTKORASTRUCTUREREADER_H_
#define VTKORASTRUCTUREREADER_H_


#include <vtkPolyDataAlgorithm.h>

namespace ora
{
  class IniAccess;
  class ContourReconstructionInfo;
}

class vtkContoursToSurfaceFilter;

/** \class vtkORAStructureReader
 * \brief VTK-compatible reader for open radART structures.
 *
 * VTK-compatible reader for open radART structures. This reader is capable of
 * reading both 2D-contours-based ORA structures and 3D-vertex-tris-based
 * ORA structures. The structures are stored in the typical ORA-INI-format.
 *
 * 2D-contour-based structures can be converted into 3D representation by
 * invoking an instance of vtkContoursToSurfaceFilter (which can be accessed
 * and configured outside). If a user does not want 3D reconstruction, the
 * contours are simply exported as contour-stack (points and lines).
 *
 * Optionally, a user can decide to apply a normals generate filter in order
 * to enable 'nice' visualization.
 *
 * // FIXME: The color compatibility -> structure meta information and so on
 * must be implemented soon - at the moment, we concentrate on the polygonal
 * shape only!
 *
 * @see vtkContoursToSurfaceFilter
 *
 * <b>Tests</b>:<br>
 * TestORAStructureReader.cxx <br>
 *
 * @author phil 
 * @author Markus 
 * @version 1.2
 */
class vtkORAStructureReader
  : public vtkPolyDataAlgorithm
{
public:
  /** RTTI info **/
  vtkTypeRevisionMacro(vtkORAStructureReader, vtkPolyDataAlgorithm);

  /** Object information output. **/
  void PrintSelf(ostream& os, vtkIndent indent);

  /** Default instantiation method. **/
  static vtkORAStructureReader *New();

  vtkSetStringMacro(FileName)
  vtkGetStringMacro(FileName)

  vtkSetMacro(ReadColorsAsWell, bool)
  vtkGetMacro(ReadColorsAsWell, bool)
  vtkBooleanMacro(ReadColorsAsWell, bool)

  vtkSetMacro(GenerateSurfaceNormals, bool)
  vtkGetMacro(GenerateSurfaceNormals, bool)
  vtkBooleanMacro(GenerateSurfaceNormals, bool)

  vtkSetClampMacro(SurfaceNormalsFeatureAngle, double, 0.0, 180.0)
  vtkGetMacro(SurfaceNormalsFeatureAngle, double)

  vtkSetMacro(Generate3DSurfaceFrom2DContoursIfApplicable, bool)
  vtkGetMacro(Generate3DSurfaceFrom2DContoursIfApplicable, bool)
  vtkBooleanMacro(Generate3DSurfaceFrom2DContoursIfApplicable, bool)

  /** Minimal check whether the specified file can be read. **/
  int CanReadFile(const char *fileName);

  vtkGetObjectMacro(ContoursToSurfaceFilter, vtkContoursToSurfaceFilter)

  /** @return a pointer to the internal INI access **/
  ora::IniAccess *GetInfoFileAccess()
  {
    return Ini;
  }

protected:
  /** Name of ORA structure info file **/
  char *FileName;
  /** INI file accessor **/
  ora::IniAccess *Ini;
  /** Flag indicating that colors from ORA structure file should be read, too. **/
  bool ReadColorsAsWell;
  /** Flag indicating that surface normals should be generated **/
  bool GenerateSurfaceNormals;
  /** Feature angle for surface normals generation [0;180] **/
  double SurfaceNormalsFeatureAngle;
  /**
   * Generate 3D surface from source 2D contours is applicable (otherwise, the
   * poly data output generated from 2D ORA contours will comprise single lines)
   **/
  bool Generate3DSurfaceFrom2DContoursIfApplicable;
  /** Contours to surface filter to be used for 3D surface reconstruction **/
  vtkContoursToSurfaceFilter *ContoursToSurfaceFilter;

  /** Default constructor **/
  vtkORAStructureReader();
  /** Destructor **/
  ~vtkORAStructureReader();

  /**
   * This is the real information update-method where the reader generates the
   * poly data from the file information.
   **/
  int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
      vtkInformationVector *outputVector);

  /** Default filter information request. **/
  int RequestInformation(vtkInformation *, vtkInformationVector **,
      vtkInformationVector *);

  /**
   * Read 3D raw data (real 3D poly data: vertices & triangle connections) from
   * currently loaded INI file.
   * @return true if successful
   **/
  bool Read3DStructureRawData(vtkPolyData *output);
  /**
   * Read 2D raw data (2D contours) from currently loaded INI file.
   * @return true if successful
   **/
  bool Read2DContourRawData(vtkPolyData *output);

private:
  /** Purposely not implemented. **/
  vtkORAStructureReader(const vtkORAStructureReader&);
  /** Purposely not implemented. **/
  void operator=(const vtkORAStructureReader&);

};


#endif /* VTKORASTRUCTUREREADER_H_ */
