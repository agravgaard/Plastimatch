//
#ifndef VTKCONTOURSTOSURFACEFILTER_H_
#define VTKCONTOURSTOSURFACEFILTER_H_

#include <vtkPolyDataAlgorithm.h>

class vtkTransform;

// constants for surface decimation definition
#define VTK_CTSF_NO_DECIMATION 0
#define VTK_CTSF_PRO_DECIMATION 1
#define VTK_CTSF_QUADRIC_DECIMATION 2

/** \class vtkContoursToSurfaceFilter
 * \brief Converts a set of parallel 2D contours in 3D space into a 3D surface.
 *
 * Converts a set of parallel 2D contours in 3D space into a 3D surface based
 * on an internal discretized volume model.
 *
 * This filter really needs contours that are parallel to each other; these
 * contours can however be arbitrarily oriented in 3D space (no requirement for
 * parallelity with the x/y-plane!). The distance between the contours does not
 * have to be constant (but see below, this can have an impact on the surface
 * generation).
 *
 * Moreover, this filter assumes the contours lying on at least 2 different
 * levels (projected coordinates) along their common normal. Certainly,
 * two or more contours are allowed to share the same level (e.g. bifurcations).
 *
 * The contours are not required being sorted w.r.t. their levels; this filter
 * is capable of sorting them on its own.
 *
 * As this filter relies on a discretized internal volume model, a spacing of
 * the underlying voxels should be chosen. If not specified, the in-plane
 * spacing (x,y) is set to 0.5 units, and the z spacing is set to the
 * encountered minimum level distance between the input contours. Setting the
 * spacing can be, however, very essential as it controls the 3D surface
 * generation. For example, a set of contours with non-constant contour
 * distances (along the normal) in combination with a too small z spacing
 * could cause a split volume - be careful!
 *
 * Optionally, a user can determine to apply an additional mesh decimation in
 * order to generate more simple surfaces.
 * 
 * <b>Tests</b>:<br>
 * TestORAStructureReader.cxx <br>
 *
 * @author phil 
 * @version 1.1
 */
class vtkContoursToSurfaceFilter : public vtkPolyDataAlgorithm
{
public:
  /** Standard new macro. **/
  static vtkContoursToSurfaceFilter *New();

  vtkTypeRevisionMacro(vtkContoursToSurfaceFilter, vtkPolyDataAlgorithm);

  /** Print object information. **/
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetVector3Macro(Spacing, double)
  vtkGetVector3Macro(Spacing, double)
  vtkGetMacro(MinimumDeltaZ, double)
  vtkGetVector3Macro(AppliedSpacing, double)

  vtkSetClampMacro(MeshDecimationMode, int, VTK_CTSF_NO_DECIMATION,
      VTK_CTSF_QUADRIC_DECIMATION)
  vtkGetMacro(MeshDecimationMode, int);
  void SetMeshDecimationModeToNone();
  void SetMeshDecimationModeToPro();
  void SetMeshDecimationModeToQuadric();
  const char *GetMeshDecimationModeAsString();

  vtkSetClampMacro(MeshReductionRatio, double, 0.0, 1.0)
  vtkGetMacro(MeshReductionRatio, double)

protected:
  /** Internal helper holding a reference normal vector **/
  double *RefNormal;
  /** Internal helper holding a reference point of the contours-collection **/
  double RefPoint[3];
  /**
   * Implicit voxel spacing for surface generation: each component which is not
   * set (<= 0.0) is replaced by a default value. Default for x and y spacing
   * (in-plane) is 0.5. Default for the z spacing is the minimum distance which
   * was detected during contour checks.
   **/
  double Spacing[3];
  /** Internal helper holding the minimum distance between contours. **/
  double MinimumDeltaZ;
  /** Holds the spacing that was really applied. **/
  double AppliedSpacing[3];
  /**
   * Optional mesh decimation of the reconstructed 3D structure.<br>
   * VTK_CTSF_NO_DECIMATION (0) ... no decimation<br>
   * VTK_CTSF_PRO_DECIMATION (1) ... simple pro-decimation<br>
   * VTK_CTSF_QUADRIC_DECIMATION (2) ... more advanced quadric decimation<br>
   */
  int MeshDecimationMode;
  /**
   * Mesh reduction ratio (for all decimation algorithms): the desired amount
   * of reduction [0.0;1.0].
   */
  double MeshReductionRatio;

  /** Default constructor **/
  vtkContoursToSurfaceFilter();
  /** Destructor **/
  ~vtkContoursToSurfaceFilter();

  /**
   * This is the real information update-method where the filter generates the
   * output poly data from the input poly data.
   **/
  int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
        vtkInformationVector *outputVector);

  /**
   * Compute the normals of the input poly data and check whether all contours
   * (polys!) are parallel.
   * @param pd poly data object to be checked
   * @return true if all contours (polys) of pd are parallel
   **/
  bool CheckParallelity(vtkPolyData *pd);

  /**
   * Examine the contours: be sure that we have contours on at least 2 different
   * levels w.r.t. to the normal direction. Basically, we assume the contours
   * being sorted from - to + relative to the normal. The sort order can,
   * however, be auto-corrected later.
   * @param pd poly data object to be checked
   * @param needsCorrection return true if the sort order needs to be auto-
   * corrected
   * @return true if we have at least two different levels (and the sort order
   * can be auto-corrected)
   */
  bool CheckLevelsAndOrder(vtkPolyData *pd, bool &needsCorrection);

  /**
   * Auto-correct the input poly data w.r.t. the sort order of their projected
   * coordinates along the normal direction.
   * @param uncorrected uncorrected input poly data
   * @param corrected corrected output poly data (NOTE: the object must be
   * created!)
   * @return true if the correction was successful
   */
  bool CorrectSortOrder(vtkPolyData *uncorrected, vtkPolyData *corrected);

  /**
   * Apply a transform that a) corrects the general tilt of the contours w.r.t.
   * the world coordinate system's z-axis, and b) transforms the nodes to the
   * discrete ijk-coordinates for the voxel-based contours to surface filter.
   * @param uncorrected uncorrected input poly data
   * @param corrected corrected output poly data (NOTE: the object must be
   * created!)
   * @param forwardTransform returned forward transform (NOTE: the object must
   * be created!)
   * @param bounds returned bounds of geometrically transformed (but not
   * discretized) object - needed for back-transform later
   * @param center returned center of geometrically transformed (but not
   * discretized) object - needed for back-transform later
   * @return true if the transform correction was successful
   */
  bool ApplyForwardTransform(vtkPolyData *uncorrected, vtkPolyData *corrected,
      vtkTransform *forwardTransform, double *&bounds, double *&center);

  /**
   * Apply a transform that back-transforms the poly data to its original
   * pose.
   * @param uncorrected uncorrected input poly data
   * @param corrected corrected output poly data (NOTE: the object must be
   * created!)
   * @param forwardTransform recently applied forward transform
   * @param center center of the untransformed input
   * @param bounds bounds of the untransformed input
   * @return true if successful
   */
  bool ApplyBackwardTransform(vtkPolyData *uncorrected, vtkPolyData *corrected,
      vtkTransform *forwardTransform, double *center, double *bounds);

private:
  /** Purposely not implemented. **/
  vtkContoursToSurfaceFilter(const vtkContoursToSurfaceFilter&);
  /** Purposely not implemented. **/
  void operator=(const vtkContoursToSurfaceFilter&);

};

#endif /* VTKCONTOURSTOSURFACEFILTER_H_ */
