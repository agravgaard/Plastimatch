/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _itk_image_load_txx_
#define _itk_image_load_txx_

#include "plmbase_config.h"
#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkMetaDataDictionary.h"
#include "itkOrientImageFilter.h"

#include "file_util.h"
#include "itk_dicom_load.h"
#include "itk_image.h"
#include "itk_image_load.h"
#include "plm_image_type.h"
#include "print_and_exit.h"
#include "string_util.h"

/* -----------------------------------------------------------------------
   Loading Images
   ----------------------------------------------------------------------- */
template<class T>
static
typename T::Pointer
itk_image_load (const char *fn)
{
    typedef typename itk::ImageFileReader < T > ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(fn);
    try {
	reader->Update();
    }
    catch(itk::ExceptionObject & ex) {
	printf ("ITK exception reading image file: %s!\n",fn);
	std::cout << ex << std::endl;
	getchar();
	exit(1);
    }
    typename T::Pointer img = reader->GetOutput();
    img->SetMetaDataDictionary (reader->GetMetaDataDictionary());
    return img;
}

template<class T, class U>
static
typename itk::Image< U, 3 >::Pointer
load_any_2 (const char* fname, T, U)
{
    typedef typename itk::Image < T, 3 > TImageType;
    typedef typename itk::Image < U, 3 > UImageType;
    typedef itk::ImageFileReader < TImageType > TReaderType;
    typedef typename itk::CastImageFilter < 
		TImageType, UImageType > CastFilterType;

    /* Load image as type T */
    typename TImageType::Pointer input_image 
	= itk_image_load<TImageType> (fname);

    /* Convert images to type U */
    typename CastFilterType::Pointer caster = CastFilterType::New();
    caster->SetInput (input_image);
    typename UImageType::Pointer image = caster->GetOutput();
    image->Update();

    /* Copy metadata */
    image->SetMetaDataDictionary (input_image->GetMetaDataDictionary());

    /* Return type U */
    return image;
}

static void
set_original_type (Plm_image_type *original_type,
		   Plm_image_type t)
{
    if (original_type) {
	*original_type = t;
    }
}

template<class U>
static
typename itk::Image< U, 3 >::Pointer
itk_image_load_any (
    const char* fname,
    Plm_image_type* original_type, 
    U otype)
{
    if (!file_exists (fname) && !string_starts_with (fname, "slicer:"))
    {
	print_and_exit ("Can't open file \"%s\" for read\n", fname);
    }

    int num_dimensions;
    itk::ImageIOBase::IOPixelType pixelType;
    itk::ImageIOBase::IOComponentType componentType;
    int num_components;
    try {
	itk_image_get_props (std::string (fname), &num_dimensions, 
	    &pixelType, &componentType, &num_components);
	switch (componentType) {
	case itk::ImageIOBase::UCHAR:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_UCHAR);
	    return load_any_2 (fname, static_cast<unsigned char>(0), otype);
	case itk::ImageIOBase::CHAR:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_CHAR);
	    return load_any_2 (fname, static_cast<char>(0), otype);
	case itk::ImageIOBase::USHORT:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_USHORT);
	    return load_any_2 (fname, static_cast<unsigned short>(0), otype);
	case itk::ImageIOBase::SHORT:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_SHORT);
	    return load_any_2 (fname, static_cast<short>(0), otype);
	case itk::ImageIOBase::UINT:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_ULONG);
	    return load_any_2 (fname, static_cast<unsigned int>(0), otype);
	case itk::ImageIOBase::INT:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_LONG);
	    return load_any_2 (fname, static_cast<int>(0), otype);
	case itk::ImageIOBase::ULONG:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_ULONG);
	    return load_any_2 (fname, static_cast<unsigned long>(0), otype);
	case itk::ImageIOBase::LONG:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_LONG);
	    return load_any_2 (fname, static_cast<long>(0), otype);
	case itk::ImageIOBase::FLOAT:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_FLOAT);
	    return load_any_2 (fname, static_cast<float>(0), otype);
	case itk::ImageIOBase::DOUBLE:
	    set_original_type (original_type, PLM_IMG_TYPE_ITK_DOUBLE);
	    return load_any_2 (fname, static_cast<double>(0), otype);
	case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
	default:
	    fprintf (stderr, 
		"Error: unhandled file type for loading image (%d) %s\n", 
		componentType, fname);
	    exit (-1);
	    break;
	}
    }
    catch (itk::ExceptionObject &excep) {
	std::cerr << "ITK xception loading image: " << fname << std::endl;
	std::cerr << excep << std::endl;
	exit (-1);
    }
}

template<class T>
T
orient_image (T img)
{
    typedef typename T::ObjectType ImageType;
    typedef typename itk::OrientImageFilter<ImageType,ImageType> OrienterType;

    try {    
	typename OrienterType::Pointer orienter = OrienterType::New();
	orienter->UseImageDirectionOn ();
	orienter->SetDesiredCoordinateOrientation (
	    itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
	orienter->SetInput (img);
	orienter->Update ();
	T output_img = orienter->GetOutput ();
	output_img->SetMetaDataDictionary (img->GetMetaDataDictionary());
	return output_img;
    } catch (itk::ExceptionObject &e) {
	std::cerr << "ITK exception orienting image." << std::endl;
	std::cerr << e << std::endl;
	exit (-1);
    }
}

template<class T>
T
itk_image_load_postprocess (T img)
{
//    img = orient_image (img);
    img = itk_image_fix_negative_spacing (img);
    return img;
}

#endif
