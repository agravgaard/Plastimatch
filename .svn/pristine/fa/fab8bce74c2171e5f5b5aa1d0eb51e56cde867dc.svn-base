/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkAffineTransform.h"
#include "itkVectorResampleImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itk_image_type.h"
#include "itk_resample.h"
#include "logfile.h"
#include "plm_image_header.h"
#include "ss_img_extract.h"

template <class T>
T
vector_resample_image (const T& vf_image, const Plm_image_header* pih)
{
    typedef typename T::ObjectType VFImageType;
    typedef itk::VectorResampleImageFilter < VFImageType, VFImageType > FilterType;
    typedef itk::VectorLinearInterpolateImageFunction< 
            VFImageType, double >  InterpolatorType;

    typename FilterType::Pointer filter = FilterType::New();

    filter->SetOutputOrigin (pih->GetOrigin());
    filter->SetOutputSpacing (pih->GetSpacing());
    filter->SetSize (pih->GetSize());
    filter->SetOutputDirection (pih->GetDirection());

    typedef itk::AffineTransform< double, 3 > TransformType;
    TransformType::Pointer transform = TransformType::New();
    filter->SetTransform (transform);

    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    filter->SetInterpolator (interpolator);

    FloatVector3DType v;
    v[0] = v[1] = v[2] = 0;
    filter->SetDefaultPixelValue (v);

    filter->SetInput (vf_image);
    try {
        filter->Update();
    }
    catch(itk::ExceptionObject & ex) {
        printf ("Exception running vector resample filter!\n");
        std::cout << ex << std::endl;
        getchar();
        exit(1);
    }

    T out_image = filter->GetOutput();
    return out_image;
}

template <class T>
static 
T
resample_image (
    T& image, 
    DoublePoint3DType origin, 
    DoubleVector3DType spacing, 
    SizeType size, 
    const DirectionType& direction, 
    float default_val, 
    int interp_lin)
{
    typedef typename T::ObjectType ImageType;
    typedef typename T::ObjectType::PixelType PixelType;
    typedef itk::ResampleImageFilter < ImageType, ImageType > FilterType;
    typedef itk::LinearInterpolateImageFunction< 
        ImageType, double >  LinInterpType;
    typedef itk::NearestNeighborInterpolateImageFunction < ImageType, double >  NNInterpType;

    typename FilterType::Pointer filter = FilterType::New();

    filter->SetOutputOrigin (origin);
    filter->SetOutputSpacing (spacing);
    filter->SetSize (size);
    filter->SetOutputDirection (direction);

    typedef itk::AffineTransform< double, 3 > TransformType;
    TransformType::Pointer transform = TransformType::New();
    filter->SetTransform (transform);

    typename LinInterpType::Pointer l_interpolator = LinInterpType::New();
    typename NNInterpType::Pointer nn_interpolator = NNInterpType::New();

    if (interp_lin) {
        filter->SetInterpolator (l_interpolator);
    } else {
        filter->SetInterpolator (nn_interpolator);
    }

    filter->SetDefaultPixelValue ((PixelType) default_val);

    filter->SetInput (image);
    try {
        filter->Update();
    }
    catch(itk::ExceptionObject & ex) {
        printf ("Exception running image resample filter!\n");
        std::cout << ex << std::endl;
        getchar();
        exit(1);
    }

    T out_image = filter->GetOutput();
    return out_image;
}

template <class T>
T
resample_image (
    T& image, 
    const Plm_image_header* pih, 
    float default_val, 
    int interp_lin)
{
    return resample_image (image, pih->GetOrigin(), pih->GetSpacing(),
        pih->GetSize(), pih->GetDirection(),
        default_val, interp_lin);
}

template <class T>
T
resample_image (
    T& image, 
    const Plm_image_header& pih, 
    float default_val, 
    int interp_lin)
{
    return resample_image (image, pih.GetOrigin(), pih.GetSpacing(),
        pih.GetSize(), pih.GetDirection(), default_val, interp_lin);
}

template <class T>
T
resample_image (T& image, float spacing[3])
{
    typedef typename T::ObjectType ImageType;
    typedef typename T::ObjectType::PixelType PixelType;
    typedef itk::ResampleImageFilter < ImageType, ImageType > FilterType;
    typedef itk::LinearInterpolateImageFunction< 
        ImageType, double >  InterpolatorType;

    const typename ImageType::SpacingType& old_spacing = image->GetSpacing();
    const typename ImageType::PointType& old_origin = image->GetOrigin();
    typename ImageType::SizeType old_size 
        = image->GetLargestPossibleRegion().GetSize();

    float old_coverage[3];
    float coverage[3];
    typename ImageType::SpacingType itk_spacing;
    typename ImageType::SizeType size;
    typename ImageType::PointType origin;

    for (int d = 0; d < 3; d++) {
        itk_spacing[d] = spacing[d];
        old_coverage[d] = old_size[d] * old_spacing[d];
        size[d] = (unsigned long) (old_coverage[d] / spacing[d]);
        coverage[d] = size[d] * spacing[d];
        origin[d] = old_origin[d];
    }

    lprintf ("New spacing at %f %f %f\n", spacing[0], spacing[1], spacing[2]);
    lprintf ("Resample size was %ld %ld %ld\n", 
        old_size[0], old_size[1], old_size[2]);
    lprintf ("Resample size will be %ld %ld %ld\n", 
        size[0], size[1], size[2]);
    lprintf ("Resample coverage was %g %g %g\n", 
        old_coverage[0], old_coverage[1], old_coverage[2]);
    lprintf ("Resample coverage will be %g %g %g\n", 
        coverage[0], coverage[1], coverage[2]);

    typename FilterType::Pointer filter = FilterType::New();
    filter->SetSize (size);
    filter->SetOutputOrigin (origin);
    filter->SetOutputSpacing (itk_spacing);
    filter->SetOutputDirection (image->GetDirection());

    typedef itk::AffineTransform< double, 3 > TransformType;
    TransformType::Pointer transform = TransformType::New();
    filter->SetTransform (transform);

    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    filter->SetInterpolator (interpolator);
    const PixelType default_val = 0;  /* Hard coded... */
    filter->SetDefaultPixelValue ((PixelType) default_val);

    filter->SetInput (image);
    try {
        filter->Update();
    }
    catch(itk::ExceptionObject & ex) {
        printf ("Exception running resample image filter!\n");
        std::cout << ex << std::endl;
        getchar();
        exit(1);
    }

    T out_image = filter->GetOutput();
    return out_image;
}

UCharVecImageType::Pointer
resample_image (UCharVecImageType::Pointer image, float spacing[3])
{
    typedef UCharVecImageType ImageType;
    const ImageType::SpacingType& old_spacing = image->GetSpacing();
    const ImageType::PointType& old_origin = image->GetOrigin();
    ImageType::SizeType old_size 
        = image->GetLargestPossibleRegion().GetSize();

    float old_coverage[3];
    ImageType::SpacingType itk_spacing;
    ImageType::SizeType size;
    ImageType::PointType origin;
    ImageType::RegionType rg;

    for (int d = 0; d < 3; d++) {
        itk_spacing[d] = spacing[d];
        old_coverage[d] = old_size[d] * old_spacing[d];
        size[d] = (unsigned long) (old_coverage[d] / spacing[d]);
        origin[d] = old_origin[d];
    }
    rg.SetSize (size);
    unsigned int num_uchar = image->GetVectorLength();

    UCharVecImageType::Pointer im_out = UCharVecImageType::New();
    im_out->SetOrigin (origin);
    im_out->SetSpacing (itk_spacing);
    im_out->SetRegions (rg);
    im_out->SetDirection (image->GetDirection());
    im_out->SetVectorLength (image->GetVectorLength());
    im_out->Allocate ();

    for (unsigned int uchar_no = 0; uchar_no < num_uchar; uchar_no++) {
	UCharImageType::Pointer uchar_img 
	    = ss_img_extract_uchar (image, uchar_no);
	UCharImageType::Pointer uchar_img_resampled
	    = resample_image (uchar_img, Plm_image_header (im_out), 0.f, 0);
	ss_img_insert_uchar (im_out, uchar_img_resampled, uchar_no);
    }
    return im_out;
}


template <class T>
T
subsample_image (T& image, int x_sampling_rate,
                int y_sampling_rate, int z_sampling_rate,
                float default_val)
{
    typedef typename T::ObjectType ImageType;
    typedef typename T::ObjectType::PixelType PixelType;
    typedef itk::ResampleImageFilter < ImageType, ImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    typedef itk::LinearInterpolateImageFunction< ImageType, double >  InterpolatorType;
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

    int sampling_rate[3];
    
    sampling_rate[0] = x_sampling_rate;
    sampling_rate[1] = y_sampling_rate;
    sampling_rate[2] = z_sampling_rate;

    /* GCS TODO: Separate out default pixel value stuff */
    filter->SetInterpolator (interpolator);
    filter->SetDefaultPixelValue ((PixelType) default_val);

    const typename ImageType::SpacingType& spacing1 = image->GetSpacing();
    const typename ImageType::PointType& origin1 = image->GetOrigin();
    const typename ImageType::SizeType size1 = image->GetLargestPossibleRegion().GetSize();

    typename ImageType::SpacingType spacing;
    typename ImageType::SizeType size;
    typename ImageType::PointType origin;
    for (int i = 0; i < 3; i++) {
        spacing[i] = spacing1[i] * sampling_rate[i];
        origin[i] = origin1[i] + 0.5 * (sampling_rate[i]-1) * spacing1[i];
        size[i] = (int) ceil(((float) size1[i] / sampling_rate[i]) - 0.5);
    }

    //compute_origin_and_size (origin, size, spacing, origin1, spacing1, size1);

    filter->SetOutputOrigin (origin);
    filter->SetOutputSpacing (spacing);
    filter->SetSize (size);

    // GCS FIX: Assume direction cosines orthogonal
    filter->SetOutputDirection (image->GetDirection());

    typedef itk::AffineTransform< double, 3 > TransformType;
    TransformType::Pointer transform = TransformType::New();
    filter->SetTransform( transform );
    filter->SetInput( image ); 
    try {
        filter->Update();
    }
    catch(itk::ExceptionObject & ex) {
        printf ("Exception running image subsample filter!\n");
        std::cout << ex << std::endl;
        getchar();
        exit(1);
    }

    T out_image = filter->GetOutput();
    return out_image;
}

/* Explicit instantiations */
template PLMBASE_API DeformationFieldType::Pointer vector_resample_image (const DeformationFieldType::Pointer&, const Plm_image_header*);

template PLMBASE_API UCharImageType::Pointer resample_image (UCharImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API CharImageType::Pointer resample_image (CharImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API UShortImageType::Pointer resample_image (UShortImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API ShortImageType::Pointer resample_image (ShortImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API UInt32ImageType::Pointer resample_image (UInt32ImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API Int32ImageType::Pointer resample_image (Int32ImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API FloatImageType::Pointer resample_image (FloatImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);
template PLMBASE_API DoubleImageType::Pointer resample_image (DoubleImageType::Pointer&, const Plm_image_header*, float default_val, int interp_lin);

template PLMBASE_API UCharImageType::Pointer resample_image (UCharImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API CharImageType::Pointer resample_image (CharImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API UShortImageType::Pointer resample_image (UShortImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API ShortImageType::Pointer resample_image (ShortImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API UInt32ImageType::Pointer resample_image (UInt32ImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API Int32ImageType::Pointer resample_image (Int32ImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API FloatImageType::Pointer resample_image (FloatImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);
template PLMBASE_API DoubleImageType::Pointer resample_image (DoubleImageType::Pointer&, const Plm_image_header&, float default_val, int interp_lin);

template PLMBASE_API FloatImageType::Pointer resample_image (FloatImageType::Pointer&, float spacing[3]);

template PLMBASE_API UCharImageType::Pointer subsample_image (UCharImageType::Pointer&, int, int, int, float);
template PLMBASE_API CharImageType::Pointer subsample_image (CharImageType::Pointer&, int, int, int, float);
template PLMBASE_API UShortImageType::Pointer subsample_image (UShortImageType::Pointer&, int, int, int, float);
template PLMBASE_API ShortImageType::Pointer subsample_image (ShortImageType::Pointer&, int, int, int, float);
template PLMBASE_API UInt32ImageType::Pointer subsample_image (UInt32ImageType::Pointer&, int, int, int, float);
template PLMBASE_API Int32ImageType::Pointer subsample_image (Int32ImageType::Pointer&, int, int, int, float);
template PLMBASE_API FloatImageType::Pointer subsample_image (FloatImageType::Pointer&, int, int, int, float);
template PLMBASE_API DoubleImageType::Pointer subsample_image (DoubleImageType::Pointer&, int, int, int, float);
