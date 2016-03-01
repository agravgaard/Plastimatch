/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_point_h_
#define _itk_point_h_

#include "plmbase_config.h"
#include <vector>
#include "itkPoint.h"
#include "itkVector.h"

/* Points & vectors */
typedef itk::Point < float, 2 > FloatPoint2DType;
typedef itk::Point < double, 2 > DoublePoint2DType;

typedef itk::Point < float, 3 > FloatPoint3DType;
typedef itk::Point < double, 3 > DoublePoint3DType;

typedef itk::Vector < float, 2 > FloatVector2DType;
typedef itk::Vector < double, 2 > DoubleVector2DType;

typedef itk::Vector < float, 3 > FloatVector3DType;
typedef itk::Vector < double, 3 > DoubleVector3DType;

#endif
