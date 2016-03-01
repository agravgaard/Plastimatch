

#ifndef VTKORAMEMORYSTRUCTURESOURCE_H_
#define VTKORAMEMORYSTRUCTURESOURCE_H_


#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>


/**
 * This filter generates an open radART structure in vtkPolyData-representation
 * outgoing from an existing open radART shared memory blocks that must have
 * been initialized adequately.
 *
 * NOTE: this class is not responsible for generating a vtkActor.
 *
 * @see FIXME (actor-creator)
 *
 * @author phil 
 * @version 1.0
 */
class vtkORAMemoryStructureSource
  : public vtkPolyDataAlgorithm
{
public:
  /** Structure type enumeration **/
  typedef enum
  {
    /** undefined **/
    ST_UNDEFINED = 0,
    /** external (body outline) **/
    ST_EXTERNAL = 1,
    /** internal (DICOM avoidance, organ, contrast agent, cavity) **/
    ST_INTERNAL = 2,
    /** target (DICOM PTV) **/
    ST_TARGET = 3,
    /** landmarks **/
    ST_LANDMARKS = 4,
    /** bolus **/
    ST_BOLUS = 5,
    /** reference points **/
    ST_REF_POINTS = 6,
    /** markers (DICOM marker, registration) **/
    ST_MARKERS = 7,
    /** iso markers **/
    ST_ISO_MARK = 8,
    /** DICOM CTV **/
    ST_CTV = 9,
    /** DICOM GTV **/
    ST_GTV = 10,
    /** beam shape visualization **/
    ST_BEAM_VISUALIZATION = 11,
    /** axis visualization **/
    ST_AXIS = 12,
    /** 3D surface scanner **/
    ST_3D_SCAN = 13
  } StructureTypeType;

  /** RTTI **/
  vtkTypeRevisionMacro(vtkORAMemoryStructureSource, vtkPolyDataAlgorithm);
  /** Standard information output **/
  void PrintSelf(ostream& os, vtkIndent indent);

  /** Default VTK-instantiator. **/
  static vtkORAMemoryStructureSource *New();

  vtkSetMacro(NumberOfPolys, int);
  vtkGetMacro(NumberOfPolys, int);

  vtkSetMacro(VertexCoords, float *);
  vtkGetMacro(VertexCoords, float *);

  vtkSetMacro(TextureCoords, int *);
  vtkGetMacro(TextureCoords, int *);

  vtkSetMacro(VertexColor, unsigned char *);
  vtkGetMacro(VertexColor, unsigned char *);

  vtkSetMacro(VertexNormal, float *);
  vtkGetMacro(VertexNormal, float *);

  vtkSetMacro(StructureUID, std::string);
  vtkGetMacro(StructureUID, std::string);

  vtkSetMacro(StructureName, std::string);
  vtkGetMacro(StructureName, std::string);

  vtkSetMacro(StructureSetUID, std::string);
  vtkGetMacro(StructureSetUID, std::string);

  vtkSetMacro(StructureType, StructureTypeType);
  vtkGetMacro(StructureType, StructureTypeType);

  vtkSetVector3Macro(DFColor, float);
  vtkGetVector3Macro(DFColor, float);

  vtkSetMacro(DFAlpha, float);
  vtkGetMacro(DFAlpha, float);

  vtkSetVector3Macro(AFColor, float);
  vtkGetVector3Macro(AFColor, float);

  vtkSetVector3Macro(SFColor, float);
  vtkGetVector3Macro(SFColor, float);

  vtkSetVector3Macro(ContourColor, float);
  vtkGetVector3Macro(ContourColor, float);

  vtkSetMacro(ContourWidth, int);
  vtkGetMacro(ContourWidth, int);

  vtkSetMacro(ORAModifiedTimeStamp, unsigned int);
  vtkGetMacro(ORAModifiedTimeStamp, unsigned int);

  /**
   * Convenience method for checking whether ORA shared memory information
   * is sufficient to create a valid polydata object.
   * @return TRUE if data is sufficient
   */
  virtual bool CanCreateStructure();

protected:
  /**
   * ORA shared mem: basic number of polygons (triangles)
   * <br>MANDATORY for object creation!
   **/
  int NumberOfPolys;
  /**
   * ORA shared mem: array of vertex coordinates of the polygons
   * (consistently with NumberOfPolys)
   * <br>MANDATORY for object creation!
   **/
  float *VertexCoords;
  /**
   * ORA shared mem: array of 2D texture coordinates for the vertices
   * (consistently with NumberOfPolys)
   **/
  int *TextureCoords;
  /**
   * ORA shared mem: array of RGBA-color for the vertices
   * (consistently with NumberOfPolys)
   **/
  unsigned char *VertexColor;
  /**
   * ORA shared mem: array of normal-vectors for the vertices
   * (consistently with NumberOfPolys)
   **/
  float *VertexNormal;
  /** ORA shared mem: structure's unique identifier **/
  std::string StructureUID;
  /** ORA shared mem: structure's name **/
  std::string StructureName;
  /** ORA shared mem: structure's parent structure set name **/
  std::string StructureSetUID;
  /** ORA shared mem: structure type **/
  StructureTypeType StructureType;
  /** ORA shared mem: diffuse color **/
  float DFColor[3];
  /** ORA shared mem: diffuse alpha channel **/
  float DFAlpha;
  /** ORA shared mem: ambient color **/
  float AFColor[3];
  /** ORA shared mem: specular color **/
  float SFColor[3];
  /** ORA shared mem: contour color **/
  float ContourColor[3];
  /** ORA shared mem: contour width **/
  int ContourWidth;
  /** ORA shared mem: ORA modification time stamp (in ms since start of day) **/
  unsigned int ORAModifiedTimeStamp;


  /** Default constructor **/
  vtkORAMemoryStructureSource();
  /** Default destructor **/
  ~vtkORAMemoryStructureSource();

  /** Default filter data request. **/
  int RequestData(vtkInformation *, vtkInformationVector **,
      vtkInformationVector *);
  /** Default filter information request. **/
  int RequestInformation(vtkInformation *, vtkInformationVector **,
      vtkInformationVector *);

private:
  /** Purposely not implemented. **/
  vtkORAMemoryStructureSource(const vtkORAMemoryStructureSource&);
  /** Purposely not implemented. **/
  void operator=(const vtkORAMemoryStructureSource&);

};


#endif /* VTKORAMEMORYSTRUCTURESOURCE_H_ */

