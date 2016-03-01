//
#ifndef VTKIMPLICITTRIANGULATEDPOLYDATA_H
#define VTKIMPLICITTRIANGULATEDPOLYDATA_H

#include <vtkImplicitFunction.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkTriangleFilter.h>
#include <vtkCellLocator.h>

/**
 * FIXME:
 * @author phil 
 * @version 1.0
 */
class vtkImplicitTriangulatedPolyData : public vtkImplicitFunction
{
public:
  static vtkImplicitTriangulatedPolyData *New() {
    return new vtkImplicitTriangulatedPolyData;
  };
  const char *GetClassName() {return "vtkImplicitTriangulatedPolyData";};
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkImplicitTriangulatedPolyData();
  ~vtkImplicitTriangulatedPolyData();

  // Description:
  // Return the MTime also considering the Input dependency.
  unsigned long GetMTime();
  
  // Description
  // Evaluate plane equation of nearest triangle to point x[3].
  double EvaluateFunction(double x[3]);

  // Description
  // Evaluate function gradient of nearest triangle to point x[3].
  void EvaluateGradient(double x[3], double g[3]);

  // Description:
  // Set the input polydata used for the implicit function evaluation.
  // Passes input through an internal instance of vtkTriangleFilter to remove
  // vertices and lines, leaving only triangular polygons for evaluation as
  // implicit planes
  void SetInput(vtkPolyData *input);

  vtkSetMacro(UsePointNormals, bool)
  vtkGetMacro(UsePointNormals, bool)
  vtkBooleanMacro(UsePointNormals, bool)

protected:
  vtkSmartPointer<vtkTriangleFilter> TriangleFilter;
  vtkSmartPointer<vtkPolyData> InputPD;
  vtkSmartPointer<vtkCellLocator> CellLocator;
  bool UsePointNormals;
  
};
#endif /* VTKIMPLICITTRIANGULATEDPOLYDATA_H */


