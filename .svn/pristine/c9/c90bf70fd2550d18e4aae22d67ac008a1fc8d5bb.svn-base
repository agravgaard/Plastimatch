/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_type_h_
#define _itk_image_type_h_

#include "plmbase_config.h"

/* itkImage.h emits warnings on gcc when used with itkKernelTransform */
//#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6)
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
//#endif
#include "itkImage.h"
//#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6)
//#pragma GCC diagnostic pop
//#endif

#include "itkImageIOBase.h"
#include "itkVectorImage.h"

#include "itk_point.h"


/* 4D images */
typedef itk::Image < unsigned char, 4 > UCharImage4DType;

/* 3D images */
typedef itk::Image < char, 3 > CharImageType;
typedef itk::Image < unsigned char, 3 > UCharImageType;
typedef itk::Image < short, 3 > ShortImageType;
typedef itk::Image < unsigned short, 3 > UShortImageType;
#if (CMAKE_SIZEOF_UINT == 4)
typedef itk::Image < int, 3 > Int32ImageType;
typedef itk::Image < unsigned int, 3 > UInt32ImageType;
#else
typedef itk::Image < long, 3 > Int32ImageType;
typedef itk::Image < unsigned long, 3 > UInt32ImageType;
#endif
typedef itk::Image < float, 3 > FloatImageType;
typedef itk::Image < double, 3 > DoubleImageType;

typedef itk::VectorImage < unsigned char, 3 > UCharVecImageType;

/* 2D images */
typedef itk::Image < unsigned char, 2 > UCharImage2DType;
typedef itk::Image < short, 2 > ShortImage2DType;
typedef itk::Image < unsigned short, 2 > UShortImage2DType;
#if (CMAKE_SIZEOF_UINT == 4)
typedef itk::Image < int, 2 > Int32Image2DType;
typedef itk::Image < unsigned int, 2 > UInt32Image2DType;
#else
typedef itk::Image < long, 2 > Int32Image2DType;
typedef itk::Image < unsigned long, 2 > UInt32Image2DType;
#endif
typedef itk::Image < float, 2 > FloatImage2DType;
typedef itk::Image < double, 2 > DoubleImage2DType;

typedef itk::VectorImage < unsigned char, 2 > UCharVecImage2DType;

/* Vector field */
typedef itk::Image < FloatVector3DType, 3 > DeformationFieldType;

#endif
