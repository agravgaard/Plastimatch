/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xform_h_
#define _xform_h_

#include "plmbase_config.h"
#include "itkTranslationTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkThinPlateSplineKernelTransform.h"
#include "itkSimilarity3DTransform.h"

#include "itk_image_type.h"
#include "smart_pointer.h"
#include "volume.h"

class Bspline_xform;
class Plm_image_header;
class Volume_header;
class Xform;
class Xform_private;

enum Xform_type {
    XFORM_NONE                  = 0,
    XFORM_ITK_TRANSLATION       = 1,
    XFORM_ITK_VERSOR            = 2,
    XFORM_ITK_QUATERNION        = 3,
    XFORM_ITK_AFFINE            = 4,
    XFORM_ITK_BSPLINE           = 5,
    XFORM_ITK_TPS               = 6,
    XFORM_ITK_VECTOR_FIELD      = 7,
    XFORM_GPUIT_BSPLINE         = 8,
    XFORM_GPUIT_VECTOR_FIELD    = 9,
    XFORM_ITK_SIMILARITY        = 10
};

/* itk basic transforms */
typedef itk::TranslationTransform < double, 3 > TranslationTransformType;
typedef itk::VersorRigid3DTransform < double > VersorTransformType;
typedef itk::QuaternionRigidTransform < double > QuaternionTransformType;
typedef itk::AffineTransform < double, 3 > AffineTransformType;
typedef itk::Similarity3DTransform<double> SimilarityTransformType;

/* itk B-spline transforms */
const unsigned int SplineDimension = 3;
const unsigned int SplineOrder = 3;
typedef itk::BSplineDeformableTransform <
    double, SplineDimension, SplineOrder > BsplineTransformType;

/* itk thin-plate transforms */
typedef itk::ThinPlateSplineKernelTransform < 
    float, 3 > FloatTpsTransformType;
typedef itk::ThinPlateSplineKernelTransform < 
    double, 3 > DoubleTpsTransformType;
typedef DoubleTpsTransformType TpsTransformType;


class PLMBASE_API Xform {
public:
    SMART_POINTER_SUPPORT (Xform);
    Xform_private *d_ptr;
public:
    Xform ();
    ~Xform ();
    Xform (const Xform& xf);

public:
    Xform_type m_type;
    
    /* The actual xform is one of the following. */
    TranslationTransformType::Pointer m_trn;
    VersorTransformType::Pointer m_vrs;
    AffineTransformType::Pointer m_aff;
    QuaternionTransformType::Pointer m_quat;
    DeformationFieldType::Pointer m_itk_vf;
    BsplineTransformType::Pointer m_itk_bsp;
    TpsTransformType::Pointer m_itk_tps;
    SimilarityTransformType::Pointer m_similarity;

public:
    void clear ();

    void load (const char* fn);
    void load (const std::string& fn);
    void save (const char* fn);
    void save (const std::string& fn);

    TranslationTransformType::Pointer get_trn () const;
    VersorTransformType::Pointer get_vrs () const;
    QuaternionTransformType::Pointer get_quat () const;
    AffineTransformType::Pointer get_aff () const;
    SimilarityTransformType::Pointer get_similarity() const;
    BsplineTransformType::Pointer get_itk_bsp () const;
    TpsTransformType::Pointer get_itk_tps () const;
    DeformationFieldType::Pointer get_itk_vf () const;
    Bspline_xform* get_gpuit_bsp () const;
    Volume::Pointer& get_gpuit_vf () const;

    void init_trn ();

    void set_trn (const itk::Array<double>& trn);
    void set_trn (TranslationTransformType::Pointer trn);
    void set_vrs (const itk::Array<double>& vrs);
    void set_vrs (VersorTransformType::Pointer vrs);
    void set_quat (const itk::Array<double>& quat);
    void set_quat (QuaternionTransformType::Pointer quat);
    void set_aff (const itk::Array<double>& aff);
    void set_aff (AffineTransformType::Pointer aff);
    void set_similarity(SimilarityTransformType::Pointer sim);
    void set_similarity(const itk::Array<double>& sim);
    void set_itk_bsp (BsplineTransformType::Pointer bsp);
    void set_itk_tps (TpsTransformType::Pointer tps);
    void set_itk_vf (DeformationFieldType::Pointer vf);
    void set_gpuit_bsp (Bspline_xform* xgb);
    void set_gpuit_vf (const Volume::Pointer& vf);

    Xform_type get_type () const;
    void get_volume_header (Volume_header *vh);
    Plm_image_header get_plm_image_header ();
    void get_grid_spacing (float grid_spacing[3]);

    void print ();

protected:
    void save_gpuit_vf (const char* fn);

public:
    Xform& operator= (const Xform& xf);
};


PLMBASE_API Xform::Pointer xform_load (const std::string& fn);
PLMBASE_API Xform::Pointer xform_load (const char* fn);
PLMBASE_API void xform_load (Xform *xf, const std::string& fn);
PLMBASE_API void xform_load (Xform *xf, const char* fn);
PLMBASE_API void xform_save (Xform *xf, const std::string& fn);
PLMBASE_API void xform_save (Xform *xf, const char* fn);
PLMBASE_API void xform_itk_bsp_init_default (Xform *xf);
PLMBASE_API void xform_itk_bsp_set_grid (Xform *xf,
    const BsplineTransformType::OriginType bsp_origin,
    const BsplineTransformType::SpacingType bsp_spacing,
    const BsplineTransformType::RegionType bsp_region,
    const BsplineTransformType::DirectionType bsp_direction);
PLMBASE_API void xform_to_trn (
    Xform *xf_out, const Xform *xf_in, Plm_image_header* pih);
PLMBASE_API void xform_to_vrs (
    Xform *xf_out, const Xform *xf_in, Plm_image_header* pih);
PLMBASE_API void xform_to_quat (
    Xform *xf_out, const Xform *xf_in, Plm_image_header* pih);
PLMBASE_API void xform_to_aff (
    Xform *xf_out, const Xform *xf_in, Plm_image_header* pih);
PLMBASE_API void xform_to_similarity (
    Xform *xf_out, const Xform *xf_in, Plm_image_header* pih);
PLMBASE_API DeformationFieldType::Pointer xform_gpuit_vf_to_itk_vf (
    Volume* vf,              /* Input */
    Plm_image_header* pih    /* Input, can be null */
);
PLMBASE_API void xform_to_itk_bsp (Xform *xf_out, const Xform *xf_in, 
    Plm_image_header* pih, const float* grid_spac);
PLMBASE_API void xform_to_itk_bsp_nobulk (Xform *xf_out, Xform *xf_in, Plm_image_header* pih, const float* grid_spac);
PLMBASE_API void xform_to_itk_vf (Xform* xf_out, Xform *xf_in, Plm_image_header* pih);
PLMBASE_API void xform_to_itk_vf (Xform* xf_out, Xform *xf_in, FloatImageType::Pointer image);
PLMBASE_API void xform_to_gpuit_bsp (Xform* xf_out, Xform* xf_in, Plm_image_header* pih, const float* grid_spac);
PLMBASE_API void xform_to_gpuit_vf (Xform* xf_out, const Xform *xf_in, const Plm_image_header* pih);

PLMBASE_API Xform::Pointer xform_to_itk_bsp (const Xform::Pointer& xf_in, 
    Plm_image_header* pih, const float* grid_spac);
PLMBASE_API Xform::Pointer xform_to_itk_bsp_nobulk (const Xform::Pointer& xf_in, Plm_image_header* pih, const float* grid_spac);
PLMBASE_API Xform::Pointer xform_to_itk_vf (const Xform::Pointer& xf_in, Plm_image_header* pih);
PLMBASE_API Xform::Pointer xform_to_gpuit_bsp (const Xform::Pointer& xf_in, Plm_image_header* pih, float* grid_spac);
PLMBASE_API Xform::Pointer xform_to_gpuit_vf (const Xform::Pointer& xf_in, const Plm_image_header* pih);

#endif
