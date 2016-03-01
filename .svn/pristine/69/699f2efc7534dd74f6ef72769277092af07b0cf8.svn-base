//

#include "oraInterpolateFunction1D.h"

#include <string>

#include <vtkCardinalSpline.h>
#include <vtkKochanekSpline.h>

namespace ora
{

InterpolateFunction1D::InterpolateFunction1D()
{
  Initialize();
}

InterpolateFunction1D::InterpolateFunction1D(double *x, double *y, int N,
    InterpolationModeType mode)
{
  Initialize();
  SetInterpolationMode(mode);
  SetSupportingPoints(x, y, N);
}

InterpolateFunction1D::~InterpolateFunction1D()
{
  Deinitialize();
  m_LinearInterpolator = NULL;
  m_CardinalSplineInterpolator = NULL;
  m_KochanekSplineInterpolator = NULL;
}

void InterpolateFunction1D::Initialize()
{
  m_X = NULL;
  m_Y = NULL;
  m_N = 0;
  m_InterpolationMode = LINEAR;
  m_LinearInterpolator = vtkSmartPointer<vtkTupleInterpolator>::New();
  m_CardinalSplineInterpolator = vtkSmartPointer<vtkTupleInterpolator>::New();
  m_KochanekSplineInterpolator = vtkSmartPointer<vtkTupleInterpolator>::New();
  m_KochanekBias = 0;
  m_KochanekContinuity = 0;
  m_KochanekTension = 0;
  m_HybridCriticalEdgeWidth = 0;
  m_UseClosedSplineInterpolation = false;
}

void InterpolateFunction1D::Deinitialize()
{
  if (m_X)
    delete[] m_X;
  if (m_Y)
    delete[] m_Y;
  m_X = NULL;
  m_Y = NULL;
  m_N = 0;
}

void InterpolateFunction1D::SetSupportingPoints(double *x, double *y, int N)
{
  if (m_N > 0) // deinitialize all interpolators
  {
    m_LinearInterpolator->Initialize();
    m_CardinalSplineInterpolator->Initialize();
    m_KochanekSplineInterpolator->Initialize();
  }
  Deinitialize();
  if (N > 1) // at least 2 supporting points required!
  {
    // add the tuples to the interpolators (duplicates are automatically
    // eliminated, the order does not matter):
    if (m_InterpolationMode == CARDINAL_SPLINE || m_InterpolationMode
        == CARDINAL_SPLINE_HYBRID)
    {
      m_CardinalSplineInterpolator->SetInterpolationTypeToSpline();
      vtkSmartPointer<vtkCardinalSpline> cs =
          vtkSmartPointer<vtkCardinalSpline>::New();
      cs->SetClosed(m_UseClosedSplineInterpolation);
      m_CardinalSplineInterpolator->SetInterpolatingSpline(cs);
      m_CardinalSplineInterpolator->SetNumberOfComponents(1);
      for (int i = 0; i < N; i++)
        m_CardinalSplineInterpolator->AddTuple(x[i], y + i);
    }
    else if (m_InterpolationMode == KOCHANEK_SPLINE || m_InterpolationMode
        == KOCHANEK_SPLINE_HYBRID)
    {
      m_KochanekSplineInterpolator->SetInterpolationTypeToSpline();
      vtkSmartPointer<vtkKochanekSpline> ks =
          vtkSmartPointer<vtkKochanekSpline>::New();
      ks->SetDefaultBias(m_KochanekBias);
      ks->SetDefaultContinuity(m_KochanekContinuity);
      ks->SetDefaultTension(m_KochanekTension);
      ks->SetClosed(m_UseClosedSplineInterpolation);
      m_KochanekSplineInterpolator->SetInterpolatingSpline(ks);
      m_KochanekSplineInterpolator->SetNumberOfComponents(1);
      for (int i = 0; i < N; i++)
        m_KochanekSplineInterpolator->AddTuple(x[i], y + i);
    }

    // needed in linear case and hybrid cases:
    if (m_InterpolationMode == LINEAR || m_InterpolationMode
        == CARDINAL_SPLINE_HYBRID  || m_InterpolationMode
        == KOCHANEK_SPLINE_HYBRID)
    {
      m_LinearInterpolator->SetInterpolationTypeToLinear();
      m_LinearInterpolator->SetNumberOfComponents(1);
      for (int i = 0; i < N; i++)
        m_LinearInterpolator->AddTuple(x[i], y + i);
    }

    m_N = N;
    m_X = new double[m_N];
    m_Y = new double[m_N];
    memcpy(m_X, x, sizeof(double) * N); // copy
    memcpy(m_Y, y, sizeof(double) * N);
  }
}

double InterpolateFunction1D::Interpolate(double x)
{
  double y = 0.;
  if (m_InterpolationMode == LINEAR)
  {
    m_LinearInterpolator->InterpolateTuple(x, &y);
  }
  else if (m_InterpolationMode == CARDINAL_SPLINE)
  {
    m_CardinalSplineInterpolator->InterpolateTuple(x, &y);
  }
  else if (m_InterpolationMode == KOCHANEK_SPLINE)
  {
    m_KochanekSplineInterpolator->InterpolateTuple(x, &y);
  }
  else // HYBRID modes -> further checks
  {
    vtkTupleInterpolator *interpolator = NULL;
    if (m_InterpolationMode == CARDINAL_SPLINE_HYBRID)
      interpolator = m_CardinalSplineInterpolator;
    else
      interpolator = m_KochanekSplineInterpolator;
    bool useLinear = (m_HybridCriticalEdgeWidth > 0) &&
        ((x < (interpolator->GetMinimumT() + m_HybridCriticalEdgeWidth)) ||
         (x > (interpolator->GetMaximumT() - m_HybridCriticalEdgeWidth)));
    if (!useLinear)
      interpolator->InterpolateTuple(x, &y);
    else
      m_LinearInterpolator->InterpolateTuple(x, &y);
  }

  return y;
}

}
