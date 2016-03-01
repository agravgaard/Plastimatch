//

#ifndef ORAINTERPOLATEFUNCTION1D_H_
#define ORAINTERPOLATEFUNCTION1D_H_

#include <vtkSmartPointer.h>
#include <vtkTupleInterpolator.h>

namespace ora
{

/** \class InterpolateFunction1D
 * \brief Simple interpolator for 1D functions.
 *
 * Simple wrapper class for interpolating 1D functions.
 *
 * Currently available interpolation modes: <br>
 * LINEAR ... linear spline, <br>
 * CARDINAL_SPLINE ... cardinal spline <br>
 * KOCHANEK_SPLINE ... Kochanek spline <br>
 * CARDINAL_SPLINE_HYBRID ... hybrid: cardinal spline, but linear at the edges<br>
 * KOCHANEK_SPLINE_HYBRID ... hybrid: Kochanek spline, but linear at the edges<br>
 *
 * NOTE: Internally we use VTK-based interpolation.
 *
 * <b>Tests</b>:<br>
 * TestInterpolateFunction1D.cxx <br>
 * Test1DTupleInterpolation.h <br>
 *
 * @see vtkTupleInterpolator
 * @see vtkCardinalSpline
 * @see vtkKochanekSpline
 *
 * @author phil 
 * @version 1.2
 *
 * \ingroup Maths
 */
class InterpolateFunction1D
{
public:
  /** Interpolation mode types. **/
  typedef enum
  {
    /** use simple piece-wise linear interpolation **/
    LINEAR = 0,
    /** use interpolation based on cardinal splines **/
    CARDINAL_SPLINE = 1,
    /**
     * use interpolation based on Kochanek splines
     * @see SetKochanekBias()
     * @see SetKochanekContinuity()
     * @see SetKochanekTension()
     **/
    KOCHANEK_SPLINE = 2,
    /**
     * use interpolation based on cardinal splines if the point (x) that should
     * be interpolated is within the 'uncritical region'; points in the critical
     * region are interpolated with linear interpolation
     * @see SetHybridCriticalEdgeWidth()
     **/
    CARDINAL_SPLINE_HYBRID = 3,
    /**
     * use interpolation based on Kochanek splines if the point (x) that should
     * be interpolated is within the 'uncritical region'; points in the critical
     * region are interpolated with linear interpolation
     * @see SetHybridCriticalEdgeWidth()
     **/
    KOCHANEK_SPLINE_HYBRID = 4
  } InterpolationModeType;

  /** Default constructor. **/
  InterpolateFunction1D();
  /**
   * Constructor with supporting points arguments. NOTE: the number of x- and
   * y-values must match. The specified values are copied (not only referenced)
   * internally.
   * @param x x-values (positions of supporting points)
   * @param y y-values (values supporting points)
   * @param N number of x-/y-pairs
   * @param mode interpolation mode
   * @see SetSupportingPoints()
   * @see SetInterpolationMode()
   **/
      InterpolateFunction1D(double *x, double *y, int N,
          InterpolationModeType mode);
  /** Destructor. **/
  ~InterpolateFunction1D();

  /**
   * Set the supporting points (x-/y-pairs). NOTE: the number of x- and
   * y-values must match. The specified values are copied (not only referenced)
   * internally.<br>
   * NOTE: We need at least 2 valid x/y-pairs! Furthermore, duplicated x/y-
   * pairs will be collapsed (the last identical pair will override the
   * previous ones).<br>
   * If you would like to delete the supporting points, apply NULL for x and y,
   * and 0 for N!
   * @param x x-values (positions of supporting points)
   * @param y y-values (values supporting points)
   * @param N number of x-/y-pairs
   */
  void SetSupportingPoints(double *x, double *y, int N);
  /** @return current number of supporting points **/
  int GetN() const
  {
    return m_N;
  }
  /** @return const pointer to internal supporting points (x-values) **/
  const double *GetX() const
  {
    return m_X;
  }
  /** @return const pointer to internal supporting points (y-values) **/
  const double *GetY() const
  {
    return m_Y;
  }
  /**
   * @return current interpolation mode (the mode that determines how
   * Interpolate() works).
   * @see Interpolate()
   **/
  InterpolationModeType GetInterpolationMode()
  {
    return m_InterpolationMode;
  }
  /**
   * Set current interpolation mode (the mode that determines how Interpolate()
   * works).
   * @see Interpolate()
   **/
  void SetInterpolationMode(InterpolationModeType mode)
  {
    m_InterpolationMode = mode;
    if (m_N > 0)
    {
      int n = m_N;
      double *x = new double[n];
      double *y = new double[n];
      memcpy(x, m_X, sizeof(double) * n); // copy
      memcpy(y, m_Y, sizeof(double) * n);
      SetSupportingPoints(x, y, n);
      delete[] x;
      delete[] y;
    }
  }

  /** Get bias parameter of Kochanek spline (default: 0.0) **/
  double GetKochanekBias()
  {
    return m_KochanekBias;
  }
  /** Set bias parameter of Kochanek spline (default: 0.0) **/
  void SetKochanekBias(double bias)
  {
    m_KochanekBias = bias;
    if ((m_InterpolationMode == KOCHANEK_SPLINE ||
         m_InterpolationMode == KOCHANEK_SPLINE_HYBRID) && m_N > 0)
    {
      int n = m_N;
      double *x = new double[n];
      double *y = new double[n];
      memcpy(x, m_X, sizeof(double) * n); // copy
      memcpy(y, m_Y, sizeof(double) * n);
      SetSupportingPoints(x, y, n);
      delete[] x;
      delete[] y;
    }
  }
  /** Get continuity parameter of Kochanek spline (default: 0.0) **/
  double GetKochanekContinuity()
  {
    return m_KochanekContinuity;
  }
  /** Set continuity parameter of Kochanek spline (default: 0.0) **/
  void SetKochanekContinuity(double continuity)
  {
    m_KochanekContinuity = continuity;
    if ((m_InterpolationMode == KOCHANEK_SPLINE ||
         m_InterpolationMode == KOCHANEK_SPLINE_HYBRID) && m_N > 0)
    {
      int n = m_N;
      double *x = new double[n];
      double *y = new double[n];
      memcpy(x, m_X, sizeof(double) * n); // copy
      memcpy(y, m_Y, sizeof(double) * n);
      SetSupportingPoints(x, y, n);
      delete[] x;
      delete[] y;
    }
  }
  /** Get tension parameter of Kochanek spline (default: 0.0) **/
  double GetKochanekTension()
  {
    return m_KochanekTension;
  }
  /** Set tension parameter of Kochanek spline (default: 0.0) **/
  void SetKochanekTension(double tension)
  {
    m_KochanekTension = tension;
    if ((m_InterpolationMode == KOCHANEK_SPLINE ||
         m_InterpolationMode == KOCHANEK_SPLINE_HYBRID) && m_N > 0)
    {
      int n = m_N;
      double *x = new double[n];
      double *y = new double[n];
      memcpy(x, m_X, sizeof(double) * n); // copy
      memcpy(y, m_Y, sizeof(double) * n);
      SetSupportingPoints(x, y, n);
      delete[] x;
      delete[] y;
    }
  }

  /**
   * Get flag indicating whether or not to use closed spline interpolation; this
   * can be useful for enhancing interpolation quality at the edges if the
   * function is periodic in reality. Default: FALSE.
   **/
  bool GetUseClosedSplineInterpolation()
  {
    return m_UseClosedSplineInterpolation;
  }
  /**
   * Set flag indicating whether or not to use closed spline interpolation; this
   * can be useful for enhancing interpolation quality at the edges if the
   * function is periodic in reality. Default: FALSE.
   **/
  void SetUseClosedSplineInterpolation(bool closed)
  {
    m_UseClosedSplineInterpolation = closed;
    if (m_InterpolationMode != LINEAR && m_N > 0)
    {
      int n = m_N;
      double *x = new double[n];
      double *y = new double[n];
      memcpy(x, m_X, sizeof(double) * n); // copy
      memcpy(y, m_Y, sizeof(double) * n);
      SetSupportingPoints(x, y, n);
      delete[] x;
      delete[] y;
    }
  }

  /**
   * Get the critical edge width where linear interpolation should be applied
   * instead of spline interpolation (hybrid modes).
   **/
  double GetHybridCriticalEdgeWidth()
  {
    return m_HybridCriticalEdgeWidth;
  }
  /**
   * Set the critical edge width. If a point to be interpolated lies within this
   * region (from the borders), linear interpolation is applied instead of
   * spline interpolation (hybrid modes). This means if point x<(x_min+edge) or
   * x>(x_max-edge), linear interpolation is applied in order to overcome
   * problems with inaccuracies of spline-interpolation methods in regions where
   * we have only a few supporting points (at the edges).
   **/
  void SetHybridCriticalEdgeWidth(double edge)
  {
    m_HybridCriticalEdgeWidth = edge;
  }

  /**
   * Interpolate the y-value according to specified x-value, current set
   * supporting points and current set interpolation mode.
   **/
  double Interpolate(double x);

protected:
  /** current stored x-values (copy) **/
  double *m_X;
  /** current stored y-values (copy) **/
  double *m_Y;
  /** current number of value-pairs **/
  int m_N;
  /** current interpolation mode **/
  InterpolationModeType m_InterpolationMode;
  /** Interpolator for linear interpolation (and hybrid modes). **/
  vtkSmartPointer<vtkTupleInterpolator> m_LinearInterpolator;
  /** Interpolator for cardinal spine interpolation. **/
  vtkSmartPointer<vtkTupleInterpolator> m_CardinalSplineInterpolator;
  /** Interpolator for Kochanek spine interpolation. **/
  vtkSmartPointer<vtkTupleInterpolator> m_KochanekSplineInterpolator;
  /** Bias parameter of Kochanek spline (default: 0.0) **/
  double m_KochanekBias;
  /** Continuity parameter of Kochanek spline (default: 0.0) **/
  double m_KochanekContinuity;
  /** Tension parameter of Kochanek spline (default: 0.0) **/
  double m_KochanekTension;
  /** Edge with for hybrid modes **/
  double m_HybridCriticalEdgeWidth;
  /**
   * Flag indicating whether or not to use closed spline interpolation; this
   * can be useful for enhancing interpolation quality at the edges if the
   * function is periodic in reality. Default: FALSE.
   **/
  bool m_UseClosedSplineInterpolation;

  /** Object member initializer. **/
  void Initialize();
  /** Object member deinitializer. **/
  void Deinitialize();

};

}

#endif /* ORAINTERPOLATEFUNCTION1D_H_ */
