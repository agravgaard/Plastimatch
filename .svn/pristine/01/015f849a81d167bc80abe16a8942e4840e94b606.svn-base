//
#ifndef VTKSURFACETOPERSPECTIVEPROJECTIONCONTOURFILTER_H_
#define VTKSURFACETOPERSPECTIVEPROJECTIONCONTOURFILTER_H_

#include "vtkSurfaceToPerspectiveProjectionImageFilter.h"

#include <vtkSmartPointer.h>

// forward declarations:
class vtkPolyData;
class vtkContourFilter;
class vtkTransformPolyDataFilter;
class vtkTransform;


/** \class vtkSurfaceToPerspectiveProjectionContourFilter
 * \brief Generates a perspective projection contour representation from input poly data surface(s).
 *
 * Generates a perspective projection contour representation from input poly data
 * surface(s). A set of input surfaces can be added (poly data representation).
 *
 * This class is directly derived from vtkSurfaceToPerspectiveProjectionImageFilter.
 * Although it should rather be a vtkPolyDataAlgorithm subclass, it was easier
 * to implement, and we did not have to define the setters/getters twice.
 *
 * Use the GetOutputPolyData() method to retrieve the generated contours.
 *
 * Moreover, the perspective projection geometry must be set: image plane size
 * and pose, and projection source position. NOTE: At the moment, only a
 * homogeneous spacing in row and column direction of the image plane is
 * supported!
 *
 * @see vtkSurfaceToPerspectiveProjectionImageFilter
 *
 * <b>Tests</b>:<br>
 * TestSurfaceToPerspectiveProjectionContourFilter.cxx <br>
 *
 * @author phil 
 * @version 1.0
 */
class vtkSurfaceToPerspectiveProjectionContourFilter :
    public vtkSurfaceToPerspectiveProjectionImageFilter
{
public:
  /** Standard new macro. **/
  static vtkSurfaceToPerspectiveProjectionContourFilter *New();

vtkTypeRevisionMacro(vtkSurfaceToPerspectiveProjectionContourFilter, vtkSurfaceToPerspectiveProjectionImageFilter)
  ;

  /** Print object information. **/
  void PrintSelf(ostream& os, vtkIndent indent);

  /**
   * Get the output poly data object. Here, we output the perspective projection
   * contour representation of the input poly data surfaces.
   **/
  vtkPolyData* GetOutputPolyData();

protected:
  /** Flag indicating that update is currently running **/
  bool Updating;
  /** Output poly data object **/
  vtkSmartPointer<vtkPolyData> OutputPolyData;
  /** Contouring filter **/
  vtkSmartPointer<vtkContourFilter> ContourFilter;
  /** Transformation filter **/
  vtkSmartPointer<vtkTransformPolyDataFilter> TransformFilter;
  /** Transformation **/
  vtkSmartPointer<vtkTransform> Transform;

  /** Default constructor **/
  vtkSurfaceToPerspectiveProjectionContourFilter();
  /** Destructor **/
  ~vtkSurfaceToPerspectiveProjectionContourFilter();

  /**
   * Additionally, generate output poly data object.
   * @see vtkSurfaceToPerspectiveProjectionImageFilter#FillOutputPortInformation()
   **/
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  /** @see vtkAlgorithm#RequestData() **/
  virtual void RequestData(vtkInformation *, vtkInformationVector **,
      vtkInformationVector *);

private:
  /** Purposely not implemented. **/
  vtkSurfaceToPerspectiveProjectionContourFilter(
      const vtkSurfaceToPerspectiveProjectionContourFilter&);
  /** Purposely not implemented. **/
  void operator=(const vtkSurfaceToPerspectiveProjectionContourFilter&);

};

#endif /* VTKSURFACETOPERSPECTIVEPROJECTIONCONTOURFILTER_H_ */
