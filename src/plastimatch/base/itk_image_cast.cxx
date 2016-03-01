/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkOrientImageFilter.h"

#include "itk_image_type.h"
#include "itkClampCastImageFilter.h"

/* -----------------------------------------------------------------------
   Casting image types
   ----------------------------------------------------------------------- */
template<class T> 
CharImageType::Pointer
cast_char (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, CharImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New ();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in CastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
UCharImageType::Pointer
cast_uchar (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, UCharImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
UShortImageType::Pointer
cast_ushort (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, UShortImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
ShortImageType::Pointer
cast_short (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, ShortImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
Int32ImageType::Pointer
cast_int32 (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, Int32ImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
UInt32ImageType::Pointer
cast_uint32 (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, UInt32ImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
FloatImageType::Pointer
cast_float (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, FloatImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

template<class T> 
DoubleImageType::Pointer
cast_double (T image)
{
    typedef typename T::ObjectType ImageType;
    typedef itk::ClampCastImageFilter <
	ImageType, DoubleImageType > ClampCastFilterType;

    typename ClampCastFilterType::Pointer caster = ClampCastFilterType::New();
    caster->SetInput(image);
    try {
	caster->Update();
    }
    catch (itk::ExceptionObject & ex) {
	printf ("ITK exception in ClampCastFilter.\n");
	std::cout << ex << std::endl;
	exit(1);
    }
    return caster->GetOutput();
}

/* Explicit instantiations */
template PLMBASE_API CharImageType::Pointer cast_char (UCharImageType::Pointer);
template PLMBASE_API CharImageType::Pointer cast_char (ShortImageType::Pointer);
template PLMBASE_API CharImageType::Pointer cast_char (FloatImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (CharImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (UShortImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (ShortImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (UInt32ImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (Int32ImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (FloatImageType::Pointer);
template PLMBASE_API UCharImageType::Pointer cast_uchar (DoubleImageType::Pointer);
template PLMBASE_API ShortImageType::Pointer cast_short (UCharImageType::Pointer);
template PLMBASE_API ShortImageType::Pointer cast_short (UShortImageType::Pointer);
template PLMBASE_API ShortImageType::Pointer cast_short (ShortImageType::Pointer);
template PLMBASE_API ShortImageType::Pointer cast_short (Int32ImageType::Pointer);
template PLMBASE_API ShortImageType::Pointer cast_short (UInt32ImageType::Pointer);
template PLMBASE_API ShortImageType::Pointer cast_short (FloatImageType::Pointer);
template PLMBASE_API UShortImageType::Pointer cast_ushort (ShortImageType::Pointer);
template PLMBASE_API UShortImageType::Pointer cast_ushort (FloatImageType::Pointer);
template PLMBASE_API Int32ImageType::Pointer cast_int32 (ShortImageType::Pointer);
template PLMBASE_API Int32ImageType::Pointer cast_int32 (FloatImageType::Pointer);
template PLMBASE_API UInt32ImageType::Pointer cast_uint32 (UCharImageType::Pointer);
template PLMBASE_API UInt32ImageType::Pointer cast_uint32 (ShortImageType::Pointer);
template PLMBASE_API UInt32ImageType::Pointer cast_uint32 (FloatImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (UCharImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (CharImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (UShortImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (ShortImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (UInt32ImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (Int32ImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (FloatImageType::Pointer);
template PLMBASE_API FloatImageType::Pointer cast_float (DoubleImageType::Pointer);
template PLMBASE_API DoubleImageType::Pointer cast_double (UCharImageType::Pointer);
template PLMBASE_API DoubleImageType::Pointer cast_double (ShortImageType::Pointer);
template PLMBASE_API DoubleImageType::Pointer cast_double (UInt32ImageType::Pointer);
template PLMBASE_API DoubleImageType::Pointer cast_double (FloatImageType::Pointer);
