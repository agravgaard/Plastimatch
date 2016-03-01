//

#ifndef ORATRANSLATIONSCALEFLEXMAPCORRECTION_H_
#define ORATRANSLATIONSCALEFLEXMAPCORRECTION_H_

#include "oraFlexMapCorrection.h"
#include "oraInterpolateFunction1D.h"

#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

namespace ora
{

/** \class TranslationScaleFlexMapCorrection
 * \brief A simple flex map considering x/y/z-translation of image plane.
 *
 * A very basic flex map correction approach originally implemented in open
 * radART. This method considers a grantry-angle and rotation-direction-
 * dependent translation of the flat panel along its row and column vectors
 * ("x-/y-offset") and along its plane normal ("isotropic scaling"). This class
 * does not account for plane tilts and/or source focal spot position changes.
 *
 * The parameters usually have a sinusoidal curve over the gantry angle
 * following a mechanical hysteresis. The input for this class are supporting
 * points that define the offset curves for a specific rotation direction.
 * Values between these points are interpolated (linearly or spline-based). By
 * default all curves for each correction parameter are constantly zero (no
 * correction). NOTE: If spline interpolation is configured, cardinal spline
 * interpolation with a closed interval is applied!
 *
 * NOTE: For compatibility reasons with ora::ProjectionProperties, this class
 * is templated over the pixel type of the fixed image and the mask pixel type.
 *
 * <b>Tests</b>:<br>
 * TestPortalImagingDeviceProjectionProperties.cxx <br>
 *
 * @see ora::FlexMapCorrection
 *
 * @author phil 
 * @author Markus 
 * @version 1.0.1
 */
template<class TPixelType, class TMaskPixelType = unsigned char>
class TranslationScaleFlexMapCorrection:
    public FlexMapCorrection<TPixelType, TMaskPixelType>
{
public:
  /** Standard class typedefs. **/
  typedef TranslationScaleFlexMapCorrection Self;
  typedef FlexMapCorrection<TPixelType, TMaskPixelType> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Communication types. **/
  typedef typename Superclass::LinacProjectionPropertiesType
      LinacProjectionPropertiesType;
  typedef typename Superclass::GenericProjectionPropertiesType
      GenericProjectionPropertiesType;

  /** Project the specified angle into [0;2PI[-range. **/
  static double ProjectAngleInto0To2PIRange(double angle);

  /** Method for creation through the object factory. **/
  itkNewMacro(Self)

  /** Run-time type information (and related methods). **/
  itkTypeMacro(TranslationScaleFlexMapCorrection, FlexMapCorrection)

  /**
   * Do translational correction of the projection plane dependent on the
   * gantry-angle, gantry-direction and the set correction curves.
   * @see FlexMapCorrrection#Correct()
   */
  virtual bool Correct();

  itkSetMacro(EssentialsOnly, bool)
  itkGetMacro(EssentialsOnly, bool)
  itkBooleanMacro(EssentialsOnly)

  virtual void SetUseSplineInterpolation(bool splineInterpolation)
  {
    m_UseSplineInterpolation = splineInterpolation;
    InterpolateFunction1D::InterpolationModeType intMode;
    if (m_UseSplineInterpolation)
      intMode = InterpolateFunction1D::CARDINAL_SPLINE;
    else
      intMode = InterpolateFunction1D::LINEAR;
    if (m_XTranslationCW)
      m_XTranslationCW->SetInterpolationMode(intMode);
    if (m_XTranslationCCW)
      m_XTranslationCCW->SetInterpolationMode(intMode);
    if (m_YTranslationCW)
      m_YTranslationCW->SetInterpolationMode(intMode);
    if (m_YTranslationCCW)
      m_YTranslationCCW->SetInterpolationMode(intMode);
    if (m_ZTranslationCW)
      m_ZTranslationCW->SetInterpolationMode(intMode);
    if (m_ZTranslationCCW)
      m_ZTranslationCCW->SetInterpolationMode(intMode);
  }
  itkGetMacro(UseSplineInterpolation, bool)
  itkBooleanMacro(UseSplineInterpolation)

/** Generic macro for setting/getting supporting points information. **/
#define SetGetInterpolatorMacro(name) \
  /** Set supporting points directly. **/ \
  virtual void Set##name##SupportingPoints(double *x, double *y, double N) \
  { \
    if (m_##name) \
      delete m_##name; \
    m_##name = NULL; \
    this->Modified(); \
    if (N <= 0) \
      return; \
    m_##name = new InterpolateFunction1D(); \
    if (m_UseSplineInterpolation) \
      m_##name->SetInterpolationMode(InterpolateFunction1D::LINEAR); \
    else \
      m_##name->SetInterpolationMode(InterpolateFunction1D::CARDINAL_SPLINE); \
    m_##name->SetUseClosedSplineInterpolation(true); \
    m_##name->SetSupportingPoints(x, y, N); \
  } \
  /** Get the supporting points x-values. **/ \
  virtual const double *Get##name##X() const \
  { \
    if (m_##name) \
      return m_##name->GetX(); \
    else \
      return NULL; \
  } \
  /** Get the supporting points y-values. **/ \
  virtual const double *Get##name##Y() const \
  { \
    if (m_##name) \
      return m_##name->GetY(); \
    else \
      return NULL; \
  } \
  /** Get the number of supporting points. **/ \
  virtual const int Get##name##N() const \
  { \
    if (m_##name) \
      return m_##name->GetN(); \
    else \
      return NULL; \
  } \
  /** Get interpolated y-value at specified x-position. **/ \
  virtual double GetInterpolated##name##YAtX(double x) const \
  { \
    if (m_##name) \
    { \
      return m_##name->Interpolate(x); \
    } \
    else \
      return 0.; \
  } \
  /**
   * Set interpolation supporting points by a typical ORA string.
   * @param oraString a comma-separated list with alternating gantry angles and
   * correction factors, e.g. "0,20.26,10,20.23,20,20.20,..." (ORA
   * stores the gantry angles in degrees, and the correction factor values
   * (offsets) in cm; ORA relates to the central axis position!; basically the
   * gantry angles are expected to cover the [0;2PI]-range!)
   * @param nominalFactor nominal (ideal) value of the central axis position in
   * plane (in cm)
   * @param addOffset additional offset of central axis (in cm)
   **/ \
  virtual void Set##name##ByORAString(std::string oraString, \
      double nominalFactor, double addOffset) \
  { \
    std::vector<double> xys; \
    GenerateXYList(oraString, xys); \
    if (xys.size() % 2 == 0) \
    { \
      const int N = static_cast<int>(xys.size() / 2); \
	  double *x = new double[N];\
	  double *y = new double[N];\
      for (int i = 0, c = 0; i < N; i++) \
      { \
        x[i] = xys[c++] / 180. * M_PI; /* deg -> rad */ \
        y[i] = (nominalFactor - xys[c++] + addOffset) * 10.; /* cm -> mm */ \
      } \
      Set##name##SupportingPoints(x, y, N); \
	  delete [] x;\
    delete [] y;\
    } \
  }

  SetGetInterpolatorMacro(XTranslationCW)
  SetGetInterpolatorMacro(XTranslationCCW)
  SetGetInterpolatorMacro(YTranslationCW)
  SetGetInterpolatorMacro(YTranslationCCW)
  SetGetInterpolatorMacro(ZTranslationCW)
  SetGetInterpolatorMacro(ZTranslationCCW)

protected:
  /** 2*PI constant **/
	static const double PI2;

  /** Clockwise translational x-correction interpolator. **/
  InterpolateFunction1D *m_XTranslationCW;
  /** Counter-clockwise translational x-correction interpolator. **/
  InterpolateFunction1D *m_XTranslationCCW;
  /** Clockwise translational y-correction interpolator. **/
  InterpolateFunction1D *m_YTranslationCW;
  /** Counter-clockwise translational y-correction interpolator. **/
  InterpolateFunction1D *m_YTranslationCCW;
  /** Clockwise translational z-correction interpolator. **/
  InterpolateFunction1D *m_ZTranslationCW;
  /** Counter-clockwise translational z-correction interpolator. **/
  InterpolateFunction1D *m_ZTranslationCCW;
  /**
   * Flag indicating whether only the essential props are updated in the
   * CorrectedProjProps-member. If TRUE, only projection origin, projection
   * orientation and source focal spot position will be available in the
   * CorrectedProjProps-member. This is suggested if performance plays a crucial
   * role. If FALSE, the CorrectedProjProps-member will contain ALL props.
   **/
  bool m_EssentialsOnly;
  /**
   * Flag indicating that closed cardinal spline interpolation should be used
   * for correction (TRUE), otherwise linear interpolation will be used.
   **/
  bool m_UseSplineInterpolation;

  /** Default constructor. **/
  TranslationScaleFlexMapCorrection();
  /** Default constructor. **/
  virtual ~TranslationScaleFlexMapCorrection();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /** Generate a vector of x/y-pairs from a comma-separated string. **/
  void GenerateXYList(const std::string &str, std::vector<double> &xys);

private:
  /** Purposely not implemented. **/
  TranslationScaleFlexMapCorrection(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#include "oraTranslationScaleFlexMapCorrection.txx"

#endif /* ORATRANSLATIONSCALEFLEXMAPCORRECTION_H_ */
