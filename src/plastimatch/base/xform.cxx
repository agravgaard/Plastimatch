/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdlib.h>
#include <string.h>
#include "itkArray.h"
#include "itkResampleImageFilter.h"
#include "itkBSplineResampleImageFunction.h"
#if ITK_VERSION_MAJOR > 3
#include "itkTransformFactory.h"
#endif
#include "itkTransformFileWriter.h"
#include "itkTransformFileReader.h"

#include "bspline_interpolate.h"
#include "bspline_xform.h"
#include "file_util.h"
#include "itk_directions.h"
#include "itk_image_create.h"
#include "itk_image_load.h"
#include "itk_image_save.h"
#include "itk_resample.h"
#include "logfile.h"
#include "mha_io.h"
#include "plm_image_header.h"
#include "plm_math.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "volume.h"
#include "volume_header.h"
#include "volume_resample.h"
#include "xform.h"
#include "xform_legacy.h"

static void
itk_bsp_set_grid_img (Xform *xf,
    const Plm_image_header* pih,
    const float* grid_spac);
static void load_gpuit_bsp (Xform *xf, const char* fn);
static void itk_xform_load (Xform *xf, const char* fn);

/* Xform_private is currently a stub... */
class Xform_private {
public:
    Bspline_xform::Pointer m_bsp;
    Volume::Pointer m_vf;
public:
    Xform_private () {
        m_bsp = Bspline_xform::New ();
        m_vf = Volume::New ();
    }
};

Xform::Xform ()
{
    d_ptr = new Xform_private;
    clear ();
}

Xform::~Xform () {
    clear ();
    delete d_ptr;
}

Xform::Xform (const Xform& xf) {
    d_ptr = new Xform_private;
    *this = xf;
}

Xform& Xform::operator= (const Xform& xf) {
    d_ptr->m_bsp = xf.d_ptr->m_bsp;
    d_ptr->m_vf = xf.d_ptr->m_vf;

    m_type = xf.m_type;
    m_trn = xf.m_trn;
    m_vrs = xf.m_vrs;
    m_quat = xf.m_quat;
    m_similarity = xf.m_similarity;
    m_aff = xf.m_aff;
    m_itk_vf = xf.m_itk_vf;
    m_itk_bsp = xf.m_itk_bsp;
    m_itk_tps = xf.m_itk_tps;
//    m_gpuit = xf.m_gpuit;                  /* Shallow copy */
    return *this;
}

void
Xform::clear ()
{
    d_ptr->m_bsp.reset();
    d_ptr->m_vf.reset();

    m_type = XFORM_NONE;
    m_trn = 0;
    m_vrs = 0;
    m_quat = 0;
    m_aff = 0;
    m_similarity= 0;
    m_itk_bsp = 0;
    m_itk_tps = 0;
    m_itk_vf = 0;
}

/* -----------------------------------------------------------------------
   Type casting
   ----------------------------------------------------------------------- */
TranslationTransformType::Pointer
Xform::get_trn () const
{
    if (m_type != XFORM_ITK_TRANSLATION) {
        print_and_exit ("Typecast error in get_trn()\n");
    }
    return m_trn;
}

VersorTransformType::Pointer
Xform::get_vrs () const
{
    if (m_type != XFORM_ITK_VERSOR) {
        printf ("Got type = %d\n", m_type);
        print_and_exit ("Typecast error in get_vrs ()\n");
    }
    return m_vrs;
}

QuaternionTransformType::Pointer
Xform::get_quat () const
{
    if (m_type != XFORM_ITK_QUATERNION) {
        print_and_exit ("Typecast error in get_quat()\n");
    }
    return m_quat;
}

AffineTransformType::Pointer
Xform::get_aff () const
{
    if (m_type != XFORM_ITK_AFFINE) {
        print_and_exit ("Typecast error in get_aff()\n");
    }
    return m_aff;
}


SimilarityTransformType::Pointer
Xform::get_similarity() const
{
    if (m_type != XFORM_ITK_SIMILARITY) {
        print_and_exit ("Typecast error in get_similarity()\n");
    }
    return m_similarity;
}

BsplineTransformType::Pointer
Xform::get_itk_bsp () const
{
    if (m_type != XFORM_ITK_BSPLINE) {
        print_and_exit ("Typecast error in get_itk_bsp()\n");
    }
    return m_itk_bsp;
}

TpsTransformType::Pointer
Xform::get_itk_tps () const
{
    if (m_type != XFORM_ITK_TPS) {
        print_and_exit ("Typecast error in get_tps()\n");
    }
    return m_itk_tps;
}

DeformationFieldType::Pointer
Xform::get_itk_vf () const
{
    if (m_type != XFORM_ITK_VECTOR_FIELD) {
        print_and_exit ("Typecast error in get_itk_vf()\n");
    }
    return m_itk_vf;
}

Bspline_xform*
Xform::get_gpuit_bsp () const
{
    if (m_type != XFORM_GPUIT_BSPLINE) {
        print_and_exit ("Typecast error in get_gpuit_bsp()\n");
    }

    return d_ptr->m_bsp.get();
}

Volume::Pointer&
Xform::get_gpuit_vf () const
{
    if (m_type != XFORM_GPUIT_VECTOR_FIELD) {
        print_and_exit ("Typecast error in get_gpuit_vf()\n");
    }

    return d_ptr->m_vf;
}

void
Xform::set_trn (TranslationTransformType::Pointer trn)
{
    clear ();
    m_type = XFORM_ITK_TRANSLATION;
    m_trn = trn;
}

void
Xform::set_trn (const itk::Array<double>& trn)
{
    typedef TranslationTransformType XfType;
    XfType::Pointer transform = XfType::New ();
    transform->SetParametersByValue (trn);
    this->set_trn (transform);
}

void
Xform::set_vrs (VersorTransformType::Pointer vrs)
{
    clear ();
    m_type = XFORM_ITK_VERSOR;
    m_vrs = vrs;
}

void
Xform::set_vrs (const itk::Array<double>& vrs)
{
    typedef VersorTransformType XfType;
    XfType::Pointer transform = XfType::New ();
    transform->SetParametersByValue (vrs);
    this->set_vrs (transform);
}

void
Xform::set_quat (QuaternionTransformType::Pointer quat)
{
    clear ();
    m_type = XFORM_ITK_QUATERNION;
    m_quat = quat;
}

void
Xform::set_quat (const itk::Array<double>& quat)
{
    typedef QuaternionTransformType XfType;
    XfType::Pointer transform = XfType::New ();
    transform->SetParametersByValue (quat);
    this->set_quat (transform);
}

void
Xform::set_aff (AffineTransformType::Pointer aff)
{
    clear ();
    m_type = XFORM_ITK_AFFINE;
    m_aff = aff;
}

void
Xform::set_aff (const itk::Array<double>& aff)
{
    typedef AffineTransformType XfType;
    XfType::Pointer transform = XfType::New ();
    transform->SetParametersByValue (aff);
    this->set_aff (transform);
}

void
Xform::set_similarity(SimilarityTransformType::Pointer sim)
{
    clear ();
    m_type = XFORM_ITK_SIMILARITY;
    m_similarity = sim;
}

void
Xform::set_similarity (const itk::Array<double>& sim)
{
    typedef SimilarityTransformType XfType;
    XfType::Pointer transform = XfType::New ();
    transform->SetParametersByValue (sim);
    this->set_similarity (transform);
}

void
Xform::set_itk_bsp (BsplineTransformType::Pointer bsp)
{
    /* Do not clear */
    m_type = XFORM_ITK_BSPLINE;
    m_itk_bsp = bsp;
}

void
Xform::set_itk_tps (TpsTransformType::Pointer tps)
{
    clear ();
    m_type = XFORM_ITK_TPS;
    m_itk_tps = tps;
}

void
Xform::set_itk_vf (DeformationFieldType::Pointer vf)
{
    clear ();
    m_type = XFORM_ITK_VECTOR_FIELD;
    m_itk_vf = vf;
}

void
Xform::set_gpuit_bsp (Bspline_xform* xgb)
{
    clear ();
    m_type = XFORM_GPUIT_BSPLINE;

    //m_gpuit = (void*) xgb;
    d_ptr->m_bsp.reset (xgb);
}

void
Xform::set_gpuit_vf (const Volume::Pointer& vf)
{
    clear ();
    m_type = XFORM_GPUIT_VECTOR_FIELD;
    //m_gpuit = (void*) vf;
    //d_ptr->m_vf.reset (vf);
    d_ptr->m_vf = vf;
}


/* -----------------------------------------------------------------------
   Load/save functions
   ----------------------------------------------------------------------- */
void
Xform::load (const char* fn)
{
    char buf[1024];
    FILE* fp;

    fp = fopen (fn, "r");
    if (!fp) {
        print_and_exit ("Error: xf_in file %s not found\n", fn);
    }
    if (!fgets(buf,1024,fp)) {
        print_and_exit ("Error reading from xf_in file.\n");
    }

    if (plm_strcmp (buf, "#Insight Transform File V1.0") == 0) {
        fclose(fp);
        itk_xform_load (this, fn);
    } else if (plm_strcmp (buf,"ObjectType = MGH_XFORM") == 0) {
        xform_legacy_load (this, fp);
        fclose(fp);
    } else if (plm_strcmp(buf,"MGH_GPUIT_BSP <experimental>")==0) {
        fclose (fp);
        load_gpuit_bsp (this, fn);
    } else {
        /* Close the file and try again, it is probably a vector field */
        fclose (fp);
        DeformationFieldType::Pointer vf = DeformationFieldType::New();
        vf = itk_image_load_float_field (fn);
        if (!vf) {
            print_and_exit ("Unexpected file format for xf_in file.\n");
        }
        this->set_itk_vf (vf);
    }
}

void
Xform::load (const std::string& fn)
{
    this->load (fn.c_str());
}

void
xform_load (Xform *xf, const char* fn)
{
    xf->load (fn);
}

void
xform_load (Xform *xf, const std::string& fn)
{
    xf->load (fn);
}

Xform::Pointer xform_load (const char* fn)
{
    Xform::Pointer xf = Xform::New ();
    xf->load (fn);
    return xf;
}

Xform::Pointer xform_load (const std::string& fn)
{
    return xform_load (fn.c_str());
}

static void
load_gpuit_bsp (Xform *xf, const char* fn)
{
    Bspline_xform* bxf;

    bxf = bspline_xform_load ((char*)fn);
    if (!bxf) {
        print_and_exit ("Error loading bxf format file: %s\n", fn);
    }

    /* Tell Xform that it is gpuit_bsp */
    xf->set_gpuit_bsp (bxf);
}

template< class T >
static void
itk_xform_save (T transform, const char *filename)
{
    typedef itk::TransformFileWriter TransformWriterType;
    TransformWriterType::Pointer outputTransformWriter;

    make_parent_directories (filename);
    outputTransformWriter = TransformWriterType::New();
    outputTransformWriter->SetFileName(filename);
    outputTransformWriter->SetInput(transform);
    try
    {
        outputTransformWriter->Update();
    }
    catch (itk::ExceptionObject &err)
    {
        std::cerr << err << std::endl;
        print_and_exit ("Error writing file: %s\n", filename);
    }
}

static void
itk_xform_load (Xform *xf, const char* fn)
{
#if (ITK_VERSION_MAJOR > 3)
    itk::TransformFactory< itk::BSplineDeformableTransform< double, 3, 3 > >::RegisterTransform ();
#endif

    /* Load from file to into reader */
    itk::TransformFileReader::Pointer transfReader;
    transfReader = itk::TransformFileReader::New();
    transfReader->SetFileName(fn);
    try {
        transfReader->Update();
    }
    catch (itk::ExceptionObject& excp) {
        std::cerr << excp << std::endl;
        print_and_exit ("Error reading ITK transform file: %s\n", fn);
    }
 
    /* Confirm that there is only one xform in the file */
    typedef itk::TransformFileReader::TransformListType* TransformListType;
    TransformListType transfList = transfReader->GetTransformList();
    if (transfList->size() != 1) {
        print_and_exit ("Error. ITK transform file has multiple "
            "(%d) transforms: %s\n", transfList->size(), fn);
    }

    /* Deduce transform type, and copy into Xform structure */
    itk::TransformFileReader::TransformListType::const_iterator 
        itTrasf = transfList->begin();
    if (!strcmp((*itTrasf)->GetNameOfClass(), "TranslationTransform")) {
        TranslationTransformType::Pointer itk_xf
            = TranslationTransformType::New();
        itk_xf = static_cast<TranslationTransformType*>(
            (*itTrasf).GetPointer());
        xf->set_trn (itk_xf);
    }
    else if (!strcmp((*itTrasf)->GetNameOfClass(), "VersorRigid3DTransform"))
    {
        VersorTransformType::Pointer itk_xf
            = VersorTransformType::New();
        VersorTransformType::InputPointType cor;
        cor.Fill(12);
        itk_xf->SetCenter(cor);
        itk_xf = static_cast<VersorTransformType*>(
            (*itTrasf).GetPointer());
        xf->set_vrs (itk_xf);
    }
    else if (!strcmp((*itTrasf)->GetNameOfClass(), "QuaternionRigidTransform"))
    {
        QuaternionTransformType::Pointer quatTransf 
            = QuaternionTransformType::New();
        QuaternionTransformType::InputPointType cor;
        cor.Fill(12);
        quatTransf->SetCenter(cor);
        quatTransf = static_cast<QuaternionTransformType*>(
            (*itTrasf).GetPointer());
        //quatTransf->Print(std::cout);
        xf->set_quat (quatTransf);
    }
    else if (!strcmp((*itTrasf)->GetNameOfClass(), "AffineTransform"))
    {
        AffineTransformType::Pointer affineTransf
            = AffineTransformType::New();
        AffineTransformType::InputPointType cor;
        cor.Fill(12);
        affineTransf->SetCenter(cor);
        affineTransf = static_cast<AffineTransformType*>(
            (*itTrasf).GetPointer());
        //affineTransf->Print(std::cout);
        xf->set_aff (affineTransf);
    }
    else if (!strcmp((*itTrasf)->GetNameOfClass(), 
            "BSplineDeformableTransform"))
    {
        BsplineTransformType::Pointer bsp
            = BsplineTransformType::New();
        bsp = static_cast<BsplineTransformType*>(
            (*itTrasf).GetPointer());
        bsp->Print(std::cout);
        xf->set_itk_bsp (bsp);
    }
}

void
Xform::save_gpuit_vf (const char* fn)
{
    //write_mha (fn, this->get_gpuit_vf().get());

    DeformationFieldType::Pointer itk_vf = 
        xform_gpuit_vf_to_itk_vf (this->get_gpuit_vf().get(), 0);
    itk_image_save (itk_vf, fn);
}

void
Xform::save (const char* fn)
{
    switch (this->m_type) {
    case XFORM_ITK_TRANSLATION:
        itk_xform_save (this->get_trn(), fn);
        break;
    case XFORM_ITK_VERSOR:
        itk_xform_save (this->get_vrs(), fn);
        break;
    case XFORM_ITK_QUATERNION:
        itk_xform_save (this->get_quat(), fn);
        break;
    case XFORM_ITK_AFFINE:
        itk_xform_save (this->get_aff(), fn);
        break;
    case XFORM_ITK_SIMILARITY:
        itk_xform_save (this->get_similarity(), fn);
        break;
    case XFORM_ITK_BSPLINE:
        itk_xform_save (this->get_itk_bsp(), fn);
        break;
    case XFORM_ITK_VECTOR_FIELD:
        itk_image_save (this->get_itk_vf(), fn);
        break;
    case XFORM_GPUIT_BSPLINE:
        this->get_gpuit_bsp()->save(fn);
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        this->save_gpuit_vf (fn);
        break;
    case XFORM_NONE:
        print_and_exit ("Error trying to save null transform\n");
        break;
    default:
        print_and_exit ("Unhandled case trying to save transform\n");
        break;
    }
}

void
Xform::save (const std::string& fn)
{
    this->save (fn.c_str());
}

void
xform_save (Xform *xf, const char* fn)
{
    xf->save (fn);
}

void
xform_save (Xform *xf, const std::string& fn)
{
    xf->save (fn.c_str());
}

/* -----------------------------------------------------------------------
   Defaults
   ----------------------------------------------------------------------- */
void
Xform::init_trn ()
{
    TranslationTransformType::Pointer trn = TranslationTransformType::New();
    this->set_trn (trn);
}

static void
init_versor_default (Xform *xf_out)
{
    VersorTransformType::Pointer vrs = VersorTransformType::New();
    xf_out->set_vrs (vrs);
}

static void
init_quaternion_default (Xform *xf_out)
{
    QuaternionTransformType::Pointer quat = QuaternionTransformType::New();
    xf_out->set_quat (quat);
}

static void
init_affine_default (Xform *xf_out)
{
    AffineTransformType::Pointer aff = AffineTransformType::New();
    xf_out->set_aff (aff);
}

static void
init_similarity_default (Xform *xf_out)
{
    SimilarityTransformType::Pointer sim = SimilarityTransformType::New();
    xf_out->set_similarity (sim);
}

void
xform_itk_bsp_init_default (Xform *xf)
{
    BsplineTransformType::Pointer bsp = BsplineTransformType::New();
    xf->set_itk_bsp (bsp);
}

/* -----------------------------------------------------------------------
   Conversions for trn, vrs, aff
   ----------------------------------------------------------------------- */
static void
xform_trn_to_vrs (Xform *xf_out, const Xform* xf_in)
{
    init_versor_default (xf_out);
    xf_out->get_vrs()->SetOffset(xf_in->get_trn()->GetOffset());
}

static void
xform_trn_to_aff (Xform *xf_out, const Xform* xf_in)
{
    init_affine_default (xf_out);
    xf_out->get_aff()->SetOffset(xf_in->get_trn()->GetOffset());
}

void
xform_trn_to_sim (Xform *xf_out, const Xform* xf_in)
{
    init_similarity_default (xf_out);
    xf_out->get_similarity()->SetOffset(xf_in->get_trn()->GetOffset());
}

static void
xform_vrs_to_quat (Xform *xf_out, const Xform* xf_in)
{
    init_quaternion_default (xf_out);
#if ITK_VERSION_MAJOR == 3
    xf_out->get_quat()->SetMatrix(xf_in->get_vrs()->GetRotationMatrix());
#else /* ITK 4 */
    xf_out->get_quat()->SetMatrix(xf_in->get_vrs()->GetMatrix());
#endif
    xf_out->get_quat()->SetOffset(xf_in->get_vrs()->GetOffset());
}

static void
xform_vrs_to_aff (Xform *xf_out, const Xform* xf_in)
{
    init_affine_default (xf_out);
#if ITK_VERSION_MAJOR == 3
    xf_out->get_aff()->SetMatrix(xf_in->get_vrs()->GetRotationMatrix());
#else /* ITK 4 */
    xf_out->get_aff()->SetMatrix(xf_in->get_vrs()->GetMatrix());
#endif
    xf_out->get_aff()->SetOffset(xf_in->get_vrs()->GetOffset());
}

static void
xform_vrs_to_sim (Xform *xf_out, const Xform* xf_in)
{
    init_similarity_default (xf_out);
#if ITK_VERSION_MAJOR == 3
    xf_out->get_similarity()->SetMatrix(xf_in->get_vrs()->GetRotationMatrix());
#else /* ITK 4 */
    xf_out->get_similarity()->SetMatrix(xf_in->get_vrs()->GetMatrix());
#endif
    xf_out->get_similarity()->SetOffset(xf_in->get_vrs()->GetOffset());
}

void
xform_sim_to_aff (Xform *xf_out, const Xform* xf_in)
{
    init_affine_default (xf_out);
#if ITK_VERSION_MAJOR == 3
    xf_out->get_aff()->SetMatrix(xf_in->get_similarity()->GetRotationMatrix());
#else /* ITK 4 */
    xf_out->get_aff()->SetMatrix(xf_in->get_similarity()->GetMatrix());
#endif
    xf_out->get_aff()->SetOffset(xf_in->get_similarity()->GetOffset());
}

/* -----------------------------------------------------------------------
   Conversion to itk_bsp
   ----------------------------------------------------------------------- */
/* Initialize using bspline spacing */
void
xform_itk_bsp_set_grid (
    Xform *xf,
    const BsplineTransformType::OriginType bsp_origin,
    const BsplineTransformType::SpacingType bsp_spacing,
    const BsplineTransformType::RegionType bsp_region,
    const BsplineTransformType::DirectionType bsp_direction)
{
    /* Set grid specifications to bsp struct */
    xf->get_itk_bsp()->SetGridSpacing (bsp_spacing);
    xf->get_itk_bsp()->SetGridOrigin (bsp_origin);
    xf->get_itk_bsp()->SetGridRegion (bsp_region);

    /* Allocate and initialize a buffer for the BSplineTransform */
    unsigned int num_parameters = xf->get_itk_bsp()->GetNumberOfParameters();
    itk::Array<double> parameters (num_parameters);
    xf->get_itk_bsp()->SetParametersByValue (parameters);
    xf->get_itk_bsp()->SetIdentity ();

    /* GCS FIX: Assume direction cosines orthogonal */
    xf->get_itk_bsp()->SetGridDirection (bsp_direction);
}

/* Initialize using image spacing */
static void
bsp_grid_from_img_grid (
    BsplineTransformType::OriginType& bsp_origin,       /* Output */
    BsplineTransformType::SpacingType& bsp_spacing,     /* Output */
    BsplineTransformType::RegionType& bsp_region,       /* Output */
    BsplineTransformType::DirectionType& bsp_direction, /* Output */
    const Plm_image_header* pih,                        /* Input */
    const float* grid_spac)                                   /* Input */
{
    BsplineTransformType::RegionType::SizeType bsp_size;

    /* Convert image specifications to grid specifications */
    for (int d=0; d<3; d++) {
        float img_ext = (pih->dim(d) - 1) * fabs (pih->spacing(d));
        bsp_origin[d] = pih->origin(d);
        bsp_spacing[d] = grid_spac[d];
        bsp_size[d] = 4 + (int) floor (img_ext / grid_spac[d]);
    }
    bsp_direction = pih->GetDirection();
    bsp_region.SetSize (bsp_size);

    /* Adjust origin based on direction cosines */
    for (int d1=0; d1<3; d1++) {
        for (int d2=0; d2<3; d2++) {
            bsp_origin[d2] -= grid_spac[d1] * bsp_direction[d1][d2];
        }
    }
}

/* Initialize using image spacing */
static void
itk_bsp_set_grid_img (
    Xform *xf,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    BsplineTransformType::OriginType bsp_origin;
    BsplineTransformType::SpacingType bsp_spacing;
    BsplineTransformType::RegionType bsp_region;
    BsplineTransformType::DirectionType bsp_direction;

    /* Compute bspline grid specifications */
    bsp_grid_from_img_grid (bsp_origin, bsp_spacing, bsp_region, 
        bsp_direction, pih, grid_spac);

    /* Set grid specifications into xf structure */
    xform_itk_bsp_set_grid (xf, bsp_origin, bsp_spacing, bsp_region, 
        bsp_direction);
}

static void
xform_trn_to_itk_bsp_bulk (
    Xform *xf_out, 
    const Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    xform_itk_bsp_init_default (xf_out);
    itk_bsp_set_grid_img (xf_out, pih, grid_spac);
    xf_out->get_itk_bsp()->SetBulkTransform (xf_in->get_trn());
}

static void
xform_vrs_to_itk_bsp_bulk (
    Xform *xf_out, 
    const Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    xform_itk_bsp_init_default (xf_out);
    itk_bsp_set_grid_img (xf_out, pih, grid_spac);
    xf_out->get_itk_bsp()->SetBulkTransform (xf_in->get_vrs());
}

static void
xform_quat_to_itk_bsp_bulk (
    Xform *xf_out, 
    const Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    xform_itk_bsp_init_default (xf_out);
    itk_bsp_set_grid_img (xf_out, pih, grid_spac);
    xf_out->get_itk_bsp()->SetBulkTransform (xf_in->get_quat());
}

static void
xform_aff_to_itk_bsp_bulk (
    Xform *xf_out, 
    const Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    xform_itk_bsp_init_default (xf_out);
    itk_bsp_set_grid_img (xf_out, pih, grid_spac);
    xf_out->get_itk_bsp()->SetBulkTransform (xf_in->get_aff());
}

/* Convert xf to vector field to bspline */
static void
xform_any_to_itk_bsp_nobulk (
    Xform *xf_out, Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    int d;
    Xform xf_tmp;
    Plm_image_header pih_bsp;

    /* Set bsp grid parameters in xf_out */
    xform_itk_bsp_init_default (xf_out);
    itk_bsp_set_grid_img (xf_out, pih, grid_spac);
    BsplineTransformType::Pointer bsp_out = xf_out->get_itk_bsp();

    /* Create temporary array for output coefficients */
    const unsigned int num_parms = bsp_out->GetNumberOfParameters();
    BsplineTransformType::ParametersType bsp_coeff;
    bsp_coeff.SetSize (num_parms);

    /* Compute bspline grid specifications */
    BsplineTransformType::OriginType bsp_origin;
    BsplineTransformType::SpacingType bsp_spacing;
    BsplineTransformType::RegionType bsp_region;
    BsplineTransformType::DirectionType bsp_direction;
    bsp_grid_from_img_grid (bsp_origin, bsp_spacing, 
        bsp_region, bsp_direction, pih, grid_spac);

    /* Make a vector field at bspline grid spacing */
    pih_bsp.set (bsp_region, bsp_origin, bsp_spacing, bsp_direction);
    xform_to_itk_vf (&xf_tmp, xf_in, &pih_bsp);

    /* Vector field is interleaved.  We need planar for decomposition. */
    FloatImageType::Pointer img = itk_image_create<float> (pih_bsp);

    /* Loop through planes */
    unsigned int counter = 0;
    DeformationFieldType::Pointer vf = xf_tmp.get_itk_vf();
    for (d = 0; d < 3; d++) {
        /* Copy a single VF plane into img */
        typedef itk::ImageRegionIterator< FloatImageType > FloatIteratorType;
        typedef itk::ImageRegionIterator< DeformationFieldType > VFIteratorType;
        FloatIteratorType img_it (img, img->GetLargestPossibleRegion());
        VFIteratorType vf_it (vf, vf->GetLargestPossibleRegion());
        for (img_it.GoToBegin(), vf_it.GoToBegin(); 
             !img_it.IsAtEnd(); 
             ++img_it, ++vf_it) 
        {
            img_it.Set(vf_it.Get()[d]);
        }

        /* Decompose into bpline coefficient image */
        typedef itk::BSplineDecompositionImageFilter <FloatImageType, 
            DoubleImageType> DecompositionType;
        DecompositionType::Pointer decomposition = DecompositionType::New();
        decomposition->SetSplineOrder (SplineOrder);
        decomposition->SetInput (img);
        decomposition->Update ();

        /* Copy the coefficients into a temporary parameter array */
        typedef BsplineTransformType::ImageType ParametersImageType;
        ParametersImageType::Pointer newCoefficients 
            = decomposition->GetOutput();
        typedef itk::ImageRegionIterator<ParametersImageType> Iterator;
        Iterator co_it (newCoefficients, bsp_out->GetGridRegion());
        co_it.GoToBegin();
        while (!co_it.IsAtEnd()) {
            bsp_coeff[counter++] = co_it.Get();
            ++co_it;
        }
    }

    /* Finally fixate coefficients into recently created bsp structure */
    bsp_out->SetParametersByValue (bsp_coeff);
}

/* This function extends the B-spline grid, padding with zeros, 
   so that the grid contains the specified Region "roi." 
   This is sometimes needed so that the B-spline can warp an 
   image or be rendered into a vector field. 
   It may sometimes be needed for going from lower resolution 
   to higher resolutions as well.  */
static void
itk_bsp_extend_to_region (
    Xform* xf, 
    const Plm_image_header* pih, 
    const RegionType* roi)
{
    unsigned long i, j, k;
    int extend_needed = 0;
    BsplineTransformType::Pointer bsp = xf->get_itk_bsp();
    BsplineTransformType::OriginType bsp_origin = bsp->GetGridOrigin();
    BsplineTransformType::RegionType bsp_region = bsp->GetGridRegion();
    BsplineTransformType::RegionType::SizeType bsp_size = bsp->GetGridRegion().GetSize();
    BsplineTransformType::SpacingType bsp_spacing = bsp->GetGridSpacing();
    BsplineTransformType::DirectionType bsp_direction = bsp->GetGridDirection();
    Plm_image_header bsp_pih (bsp_region, bsp_origin, bsp_spacing, 
        bsp_direction);

    /* # of control points to "extend before" and "extend after" 
       existing grid */
    int eb[3], ea[3];

#if PLM_CONFIG_LEGACY_BSPLINE_EXTEND
    /* Figure out if we need to extend the bspline grid.  If so, compute 
       ea & eb, as well as new values of bsp_region and bsp_origin. */
    for (int d = 0; d < 3; d++) {
        float old_roi_origin = bsp->GetGridOrigin()[d] + bsp->GetGridSpacing()[d];
        float old_roi_corner = old_roi_origin + (bsp->GetGridRegion().GetSize()[d] - 3) * bsp->GetGridSpacing()[d];
        float new_roi_origin = pih->origin(d) + roi->GetIndex()[d] * pih->spacing(d);
        float new_roi_corner = new_roi_origin + (roi->GetSize()[d] - 1) * pih->spacing(d);
        ea[d] = eb[d] = 0;
        if (old_roi_origin > new_roi_origin) {
            float diff = old_roi_origin - new_roi_origin;
            eb[d] = (int) ceil (diff / bsp->GetGridSpacing()[d]);
            bsp_origin[d] -= eb[d] * bsp->GetGridSpacing()[d];
            bsp_size[d] += eb[d];
            extend_needed = 1;
        }
        if (old_roi_corner < new_roi_corner) {
            float diff = new_roi_origin - old_roi_origin;
            ea[d] = (int) ceil (diff / bsp->GetGridSpacing()[d]);
            bsp_size[d] += ea[d];
            extend_needed = 1;
        }
    }

#else
    /* Figure out if we need to extend the bspline grid.  If so, compute 
       ea & eb, as well as new values of bsp_region and bsp_origin. */
    float new_roi_origin_idx[3], new_roi_corner_idx[3];
    for (int d = 0; d < 3; d++) {
        new_roi_origin_idx[d] = 0.f;
        new_roi_corner_idx[d] = roi->GetSize()[d] - 1;
    }
    FloatPoint3DType new_roi_origin = pih->get_position (new_roi_origin_idx);
    FloatPoint3DType new_roi_corner = pih->get_position (new_roi_corner_idx);

    FloatPoint3DType new_roi_origin_idx_in_old = 
        bsp_pih.get_index (new_roi_origin);
    FloatPoint3DType new_roi_corner_idx_in_old = 
        bsp_pih.get_index (new_roi_corner);

#if defined (commentout)
    printf ("-- BSP PIH\n");
    bsp_pih.print();
    printf ("-- PIH\n");
    pih->print();
    printf ("New ROI Or Idx: %g %g %g\n",
        new_roi_origin_idx[0], new_roi_origin_idx[1], new_roi_origin_idx[2]);
    printf ("New ROI Co Idx: %g %g %g\n",
        new_roi_corner_idx[0], new_roi_corner_idx[1], new_roi_corner_idx[2]);
    printf ("New ROI Or: %g %g %g\n",
        new_roi_origin[0], new_roi_origin[1], new_roi_origin[2]);
    printf ("New ROI Co: %g %g %g\n",
        new_roi_corner[0], new_roi_corner[1], new_roi_corner[2]);
    printf ("New ROI Or Idx (inold): %g %g %g\n",
        new_roi_origin_idx_in_old[0], new_roi_origin_idx_in_old[1], new_roi_origin_idx_in_old[2]);
    printf ("New ROI Co Idx (inold): %g %g %g\n",
        new_roi_corner_idx_in_old[0], new_roi_corner_idx_in_old[1], new_roi_corner_idx_in_old[2]);
#endif

    for (int d = 0; d < 3; d++) {
        eb[d] = ea[d] = 0;
        new_roi_origin_idx_in_old[d] = floorf (new_roi_origin_idx_in_old[d]);
        new_roi_corner_idx_in_old[d] = ceilf (new_roi_corner_idx_in_old[d]);
        if (new_roi_origin_idx_in_old[d] < 1) {
            eb[d] = 1 - (int) new_roi_origin_idx_in_old[d];
            new_roi_corner_idx[d] -= eb[d];
            extend_needed = 1;
        }
        if (new_roi_corner_idx_in_old[d] > bsp_size[d] - 2) {
            ea[d] = new_roi_corner_idx_in_old[d] - (bsp_size[d] - 1);
            extend_needed = 1;
        }
    }

    if (extend_needed) {
        /* Figure out new origin */
        bsp_origin = bsp_pih.get_position (new_roi_corner_idx);
        /* Figure out new size */
        for (int d = 0; d < 3; d++) {
            bsp_size[d] += eb[d] + ea[d];
        }
    }
#endif

    if (extend_needed) {
        /* Allocate new parameter array */
        BsplineTransformType::Pointer bsp_new = BsplineTransformType::New();
        BsplineTransformType::RegionType old_region = bsp->GetGridRegion();
        bsp_region.SetSize (bsp_size);
        bsp_new->SetGridOrigin (bsp_origin);
        bsp_new->SetGridRegion (bsp_region);
        bsp_new->SetGridSpacing (bsp->GetGridSpacing());
        bsp_new->SetGridDirection (bsp->GetGridDirection());

        /* Copy current parameters in... */
        const unsigned int num_parms = bsp_new->GetNumberOfParameters();
        BsplineTransformType::ParametersType bsp_coeff;
        bsp_coeff.SetSize (num_parms);
        bsp_coeff.Fill (0.f);
        for (int old_idx = 0, d = 0; d < 3; d++) {
            for (k = 0; k < old_region.GetSize()[2]; k++) {
                for (j = 0; j < old_region.GetSize()[1]; j++) {
                    for (i = 0; i < old_region.GetSize()[0]; i++, old_idx++) {
                        int new_idx;
                        new_idx = ((((d * bsp_size[2]) + k + eb[2]) * bsp_size[1] + (j + eb[1])) * bsp_size[0]) + (i + eb[0]);
                        bsp_coeff[new_idx] = bsp->GetParameters()[old_idx];
                    }
                }
            }
        }

        /* Copy coefficients into recently created bsp struct */
        bsp_new->SetParametersByValue (bsp_coeff);

        /* Fixate xf with new bsp */
        xf->set_itk_bsp (bsp_new);
    }
}

static void
xform_itk_bsp_to_itk_bsp (
    Xform *xf_out, 
    const Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    BsplineTransformType::Pointer bsp_old = xf_in->get_itk_bsp();

    xform_itk_bsp_init_default (xf_out);
    itk_bsp_set_grid_img (xf_out, pih, grid_spac);

    /* Need to copy the bulk transform */
    BsplineTransformType::Pointer bsp_out = xf_out->get_itk_bsp();
    bsp_out->SetBulkTransform (bsp_old->GetBulkTransform());

    /* Create temporary array for output coefficients */
    const unsigned int num_parms 
        = xf_out->get_itk_bsp()->GetNumberOfParameters();
    BsplineTransformType::ParametersType bsp_coeff;
    bsp_coeff.SetSize (num_parms);

    /* GCS May 12, 2008.  I feel like the below algorithm suggested 
       by ITK is wrong.  If BSplineResampleImageFunction interpolates the 
       coefficient image, the resulting resampled B-spline will be smoother 
       than the original.  But maybe the algorithm is OK, just a problem with 
       the lack of proper ITK documentation.  Need to test this.

       What this code does is this:
       1) Resample coefficient image using B-Spline interpolator
       2) Pass resampled image to decomposition filter
       3) Copy decomposition filter output into new coefficient image

       This is the original comment from the ITK code:
       Now we need to initialize the BSpline coefficients of the higher 
       resolution transform. This is done by first computing the actual 
       deformation field at the higher resolution from the lower 
       resolution BSpline coefficients. Then a BSpline decomposition 
       is done to obtain the BSpline coefficient of the higher 
       resolution transform. */
    unsigned int counter = 0;
    for (unsigned int k = 0; k < 3; k++) {
        typedef BsplineTransformType::ImageType ParametersImageType;
        typedef itk::ResampleImageFilter<
            ParametersImageType, ParametersImageType> ResamplerType;
        ResamplerType::Pointer resampler = ResamplerType::New();

        typedef itk::BSplineResampleImageFunction<
            ParametersImageType, double> FunctionType;
        FunctionType::Pointer fptr = FunctionType::New();

        typedef itk::IdentityTransform<double, 3> IdentityTransformType;
        IdentityTransformType::Pointer identity = IdentityTransformType::New();

#if ITK_VERSION_MAJOR == 3
        resampler->SetInput (bsp_old->GetCoefficientImage()[k]);
#else /* ITK 4 */
        resampler->SetInput (bsp_old->GetCoefficientImages()[k]);
#endif
        resampler->SetInterpolator (fptr);
        resampler->SetTransform (identity);
        resampler->SetSize (bsp_out->GetGridRegion().GetSize());
        resampler->SetOutputSpacing (bsp_out->GetGridSpacing());
        resampler->SetOutputOrigin (bsp_out->GetGridOrigin());
        resampler->SetOutputDirection (bsp_out->GetGridDirection());

        typedef itk::BSplineDecompositionImageFilter<
            ParametersImageType, ParametersImageType> DecompositionType;
        DecompositionType::Pointer decomposition = DecompositionType::New();

        decomposition->SetSplineOrder (SplineOrder);
        decomposition->SetInput (resampler->GetOutput());
        decomposition->Update();

        ParametersImageType::Pointer newCoefficients 
            = decomposition->GetOutput();

        // copy the coefficients into a temporary parameter array
        typedef itk::ImageRegionIterator<ParametersImageType> Iterator;
        Iterator it (newCoefficients, bsp_out->GetGridRegion());
        while (!it.IsAtEnd()) {
            bsp_coeff[counter++] = it.Get();
            ++it;
        }
    }

    /* Finally fixate coefficients into recently created bsp structure */
    bsp_out->SetParametersByValue (bsp_coeff);
}

/* Compute itk_bsp grid specifications from gpuit specifications */
static void
gpuit_bsp_grid_to_itk_bsp_grid (
    BsplineTransformType::OriginType& bsp_origin,        /* Output */
    BsplineTransformType::SpacingType& bsp_spacing,      /* Output */
    BsplineTransformType::RegionType& bsp_region,        /* Output */
    BsplineTransformType::DirectionType& bsp_direction,  /* Output */
    Bspline_xform* bxf)                                  /* Input */
{
    BsplineTransformType::SizeType bsp_size;

    /* Convert bxf specifications to grid specifications */
    for (int d = 0; d < 3; d++) {
        bsp_size[d] = bxf->cdims[d];
        bsp_origin[d] = bxf->img_origin[d];
        bsp_spacing[d] = bxf->grid_spac[d];
    }
    itk_direction_from_dc (&bsp_direction, bxf->dc);
    bsp_region.SetSize (bsp_size);

    /* Adjust origin based on direction cosines */
    for (int d1=0; d1<3; d1++) {
        for (int d2=0; d2<3; d2++) {
            bsp_origin[d2] -= bxf->grid_spac[d1] * bsp_direction[d1][d2];
        }
    }
}

static void
gpuit_bsp_to_itk_bsp_raw (
    Xform *xf_out, 
    const Xform* xf_in, 
    const Plm_image_header* pih)
{
    typedef BsplineTransformType::ImageType ParametersImageType;
    typedef itk::ImageRegionIterator<ParametersImageType> Iterator;
    Bspline_xform* bxf = xf_in->get_gpuit_bsp();

    BsplineTransformType::OriginType bsp_origin;
    BsplineTransformType::SpacingType bsp_spacing;
    BsplineTransformType::RegionType bsp_region;
    BsplineTransformType::DirectionType bsp_direction = pih->GetDirection();

    /* Convert bspline grid geometry from gpuit to itk */
    gpuit_bsp_grid_to_itk_bsp_grid (bsp_origin, bsp_spacing, bsp_region, 
        bsp_direction, bxf);

    /* Create itk bspline structure */
    xform_itk_bsp_init_default (xf_out);
    xform_itk_bsp_set_grid (xf_out, bsp_origin, bsp_spacing, 
        bsp_region, bsp_direction);

    /* RMK: bulk transform is Identity (not supported by GPUIT) */

    /* Create temporary array for output coefficients */
    const unsigned int num_parms 
        = xf_out->get_itk_bsp()->GetNumberOfParameters();
    BsplineTransformType::ParametersType bsp_coeff;
    bsp_coeff.SetSize (num_parms);

    /* Copy from GPUIT coefficient array to ITK coefficient array */
    int k = 0;
    for (int d = 0; d < 3; d++) {
        for (int i = 0; i < bxf->num_knots; i++) {
            bsp_coeff[k] = bxf->coeff[3*i+d];
            k++;
        }
    }

    /* Fixate coefficients into bsp structure */
    xf_out->get_itk_bsp()->SetParametersByValue (bsp_coeff);
}

/* If grid_spac is null, then don't resample */
static void
xform_gpuit_bsp_to_itk_bsp (
    Xform *xf_out, 
    const Xform* xf_in,
    const Plm_image_header* pih,
    const float* grid_spac)
{
    Xform xf_tmp;

    if (grid_spac) {
        /* Convert to itk data structure */
        gpuit_bsp_to_itk_bsp_raw (&xf_tmp, xf_in, pih);

        /* Then, resample the xform to the desired grid spacing */
        xform_itk_bsp_to_itk_bsp (xf_out, &xf_tmp, pih, grid_spac);
    } else {
        /* Convert to itk data structure only */
        gpuit_bsp_to_itk_bsp_raw (xf_out, xf_in, pih);
    }
}

/* -----------------------------------------------------------------------
   Conversion to itk_vf
   ----------------------------------------------------------------------- */
static DeformationFieldType::Pointer
xform_itk_any_to_itk_vf (
    itk::Transform<double,3,3>* xf,
    const Plm_image_header* pih)
{
#if defined (commentout)
    DeformationFieldType::Pointer itk_vf = DeformationFieldType::New();
    itk_vf->SetOrigin (pih->m_origin);
    itk_vf->SetSpacing (pih->m_spacing);
    itk_vf->SetRegions (pih->m_region);
    itk_vf->SetDirection (pih->m_direction);
    itk_vf->Allocate ();
#endif
    DeformationFieldType::Pointer itk_vf 
        = itk_image_create<FloatVector3DType> (*pih);

    typedef itk::ImageRegionIteratorWithIndex< 
        DeformationFieldType > FieldIterator;
    FieldIterator fi (itk_vf, itk_vf->GetLargestPossibleRegion());

    fi.GoToBegin();

    DoublePoint3DType fixed_point;
    DoublePoint3DType moving_point;
    DeformationFieldType::IndexType index;

    FloatVector3DType displacement;

    while (!fi.IsAtEnd()) {
        index = fi.GetIndex();
        itk_vf->TransformIndexToPhysicalPoint (index, fixed_point);
        moving_point = xf->TransformPoint (fixed_point);
        for (int r = 0; r < 3; r++) {
            displacement[r] = moving_point[r] - fixed_point[r];
        }
        fi.Set (displacement);
        ++fi;
    }
    return itk_vf;
}

/* ITK bsp is different from itk_any, because additional control points 
   might be needed */
static DeformationFieldType::Pointer 
xform_itk_bsp_to_itk_vf (Xform* xf_in, const Plm_image_header* pih)
{
    int d;
    Xform xf_tmp;
    float grid_spac[3];

    /* Deep copy of itk_bsp */
    for (d = 0; d < 3; d++) {
        grid_spac[d] = xf_in->get_itk_bsp()->GetGridSpacing()[d];
    }
    xform_itk_bsp_to_itk_bsp (&xf_tmp, xf_in, pih, grid_spac);

    /* Extend bsp control point grid */
    RegionType region = pih->GetRegion();
    itk_bsp_extend_to_region (&xf_tmp, pih, &region);

    /* Convert extended bsp to vf */
    return xform_itk_any_to_itk_vf (xf_tmp.get_itk_bsp(), pih);
}

static DeformationFieldType::Pointer 
xform_itk_vf_to_itk_vf (
    const DeformationFieldType::Pointer& vf, 
    Plm_image_header* pih)
{
    return vector_resample_image (vf, (const Plm_image_header*) pih);
}

/* Here what we're going to do is use GPUIT library to interpolate the 
    B-Spline at its native resolution, then convert gpuit_vf -> itk_vf. 

    GCS: Aug 6, 2008.  The above idea doesn't work, because the native 
    resolution might not encompass the image.  Here is what we will do:
    1) Convert to ITK B-Spline
    2) Extend ITK B-Spline to encompass image
    3) Render vf.
*/
static DeformationFieldType::Pointer
xform_gpuit_bsp_to_itk_vf (Xform* xf_in, Plm_image_header* pih)
{
    DeformationFieldType::Pointer itk_vf;

    Xform xf_tmp;
    OriginType img_origin;
    SpacingType img_spacing;
    RegionType img_region;

    /* Copy from GPUIT coefficient array to ITK coefficient array */
    gpuit_bsp_to_itk_bsp_raw (&xf_tmp, xf_in, pih);

    /* Resize itk array to span image */
    RegionType region = pih->GetRegion();
    itk_bsp_extend_to_region (&xf_tmp, pih, &region);

    /* Render to vector field */
    itk_vf = xform_itk_any_to_itk_vf (xf_tmp.get_itk_bsp(), pih);

    return itk_vf;
}

/* 1) convert gpuit -> itk at the native resolution, 
   2) convert itk -> itk to change resolution.  */
DeformationFieldType::Pointer 
xform_gpuit_vf_to_itk_vf (
    Volume* vf,            /* Input */
    Plm_image_header* pih    /* Input, can be null */
)
{
    DeformationFieldType::Pointer itk_vf = DeformationFieldType::New();
    FloatVector3DType displacement;

    /* Copy header & allocate data for itk */
    itk_image_set_header (itk_vf, Plm_image_header (vf));
    itk_vf->Allocate();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (itk_vf, itk_vf->GetLargestPossibleRegion());

    if (vf->pix_type == PT_VF_FLOAT_INTERLEAVED) {
        float* img = (float*) vf->img;
        int i = 0;
        for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi) {
            for (int r = 0; r < 3; r++) {
                displacement[r] = img[i++];
            }
            fi.Set (displacement);
        }
    }
    else if (vf->pix_type == PT_VF_FLOAT_PLANAR) {
        float** img = (float**) vf->img;
        int i = 0;
        for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi, ++i) {
            for (int r = 0; r < 3; r++) {
                displacement[r] = img[r][i];
            }
            fi.Set (displacement);
        }
    } else {
        print_and_exit ("Irregular pix_type used converting gpuit_xf -> itk\n");
    }

    /* Resample to requested resolution */
    if (pih) {
        itk_vf = xform_itk_vf_to_itk_vf (itk_vf, pih);
    }

    return itk_vf;
}

/* -----------------------------------------------------------------------
   Conversion to gpuit_bsp
   ----------------------------------------------------------------------- */
static Bspline_xform*
create_gpuit_bxf (Plm_image_header* pih, const float* grid_spac)
{
    int d;
    Bspline_xform* bxf = new Bspline_xform;
    float img_origin[3];
    float img_spacing[3];
    plm_long img_dim[3];
    plm_long roi_offset[3];
    plm_long roi_dim[3];
    plm_long vox_per_rgn[3];
    float direction_cosines[9];

    pih->get_origin (img_origin);
    pih->get_dim (img_dim);
    pih->get_spacing (img_spacing);
    pih->get_direction_cosines (direction_cosines);

    for (d = 0; d < 3; d++) {
        /* Old ROI was whole image */
        roi_offset[d] = 0;
        roi_dim[d] = img_dim[d];
        /* Compute vox_per_rgn */
        vox_per_rgn[d] = ROUND_INT (grid_spac[d] / fabs(img_spacing[d]));
        if (vox_per_rgn[d] < 4) {
            lprintf ("Warning: vox_per_rgn was less than 4.\n");
            vox_per_rgn[d] = 4;
        }
    }
    bxf->initialize (img_origin, img_spacing, img_dim, 
        roi_offset, roi_dim, vox_per_rgn, direction_cosines);
    return bxf;
}

void
xform_any_to_gpuit_bsp (
    Xform* xf_out, Xform* xf_in, 
    Plm_image_header* pih, 
    const float* grid_spac)
{
    Xform xf_tmp;

    /* Initialize gpuit bspline data structure */
    Bspline_xform* bxf_new = create_gpuit_bxf (pih, grid_spac);

    if (xf_in->m_type != XFORM_NONE) {
        /* Output ROI is going to be whole image */
        RegionType roi = pih->GetRegion ();

        /* Create itk_bsp xf using image specifications */
        xform_any_to_itk_bsp_nobulk (&xf_tmp, xf_in, pih, bxf_new->grid_spac);

        /* Copy from ITK coefficient array to gpuit coefficient array */
        int k = 0;
        for (int d = 0; d < 3; d++) {
            for (int i = 0; i < bxf_new->num_knots; i++) {
                bxf_new->coeff[3*i+d] = xf_tmp.get_itk_bsp()->GetParameters()[k];
                k++;
            }
        }
    }

    /* Fixate gpuit bsp to xf */
    xf_out->set_gpuit_bsp (bxf_new);
}

void
xform_gpuit_bsp_to_gpuit_bsp (
    Xform* xf_out, 
    Xform* xf_in, 
    Plm_image_header* pih, 
    const float* grid_spac
)
{
    Xform xf_tmp;

    /* Initialize gpuit bspline data structure */
    Bspline_xform* bxf_new = create_gpuit_bxf (pih, grid_spac);

    /* Create itk_bsp xf using image specifications */
    xform_gpuit_bsp_to_itk_bsp (&xf_tmp, xf_in, pih, bxf_new->grid_spac);

    /* Copy from ITK coefficient array to gpuit coefficient array */
    int k = 0;
    for (int d = 0; d < 3; d++) {
        for (int i = 0; i < bxf_new->num_knots; i++) {
            bxf_new->coeff[3*i+d] = xf_tmp.get_itk_bsp()->GetParameters()[k];
            k++;
        }
    }

    /* Fixate gpuit bsp to xf */
    xf_out->set_gpuit_bsp (bxf_new);
}

void
xform_gpuit_vf_to_gpuit_bsp (
    Xform* xf_out, 
    Xform* xf_in, 
    Plm_image_header* pih, 
    const float* grid_spac
)
{
    /* Convert gpuit_vf to itk_vf, then convert itk_vf to gpuit_bsp */
    Xform tmp;
    xform_to_itk_vf (&tmp, xf_in, pih);
    xform_any_to_gpuit_bsp (xf_out, &tmp, pih, grid_spac);
}

/* -----------------------------------------------------------------------
   Conversion to gpuit_vf
   ----------------------------------------------------------------------- */
static Volume::Pointer
xform_itk_any_to_gpuit_vf (
    itk::Transform<double,3,3>* xf,
    const Plm_image_header* pih)
{
    Volume_header vh (pih);
    Volume::Pointer vf_out = Volume::New (vh, PT_VF_FLOAT_INTERLEAVED, 3);
    float* img = (float*) vf_out->img;

    DoublePoint3DType fixed_point;
    DoublePoint3DType moving_point;

    int i = 0;
    plm_long fijk[3] = {0};
    float fxyz[3];
    LOOP_Z(fijk,fxyz,vf_out) {
        LOOP_Y(fijk,fxyz,vf_out) {
            LOOP_X(fijk,fxyz,vf_out) {
                fixed_point[0] = fxyz[0];
                fixed_point[1] = fxyz[1];
                fixed_point[2] = fxyz[2];
                moving_point = xf->TransformPoint (fixed_point);
                for (int r = 0; r < 3; r++) {
                    img[i++] = moving_point[r] - fixed_point[r];
                }
            }
        }
    }
    return vf_out;
}

static Volume::Pointer
xform_gpuit_vf_to_gpuit_vf (
    const Volume::Pointer& vf_in, 
    const Plm_image_header *pih)
{
    Volume::Pointer vf_out;
    Volume_header vh (pih);
    vf_out = volume_resample (vf_in, &vh);
    return vf_out;
}

static Volume::Pointer
xform_gpuit_bsp_to_gpuit_vf (const Xform *xf_in, const Plm_image_header *pih)
{
    Bspline_xform* bxf = xf_in->get_gpuit_bsp();
    Volume::Pointer vf_out;

    Volume_header vh (pih);
    vf_out = Volume::New (vh, PT_VF_FLOAT_INTERLEAVED, 3);
    bspline_interpolate_vf (vf_out.get(), bxf);
    return vf_out;
}

Volume::Pointer
xform_itk_vf_to_gpuit_vf (
    DeformationFieldType::Pointer itk_vf, const Plm_image_header *pih)
{
    Volume_header vh (pih);
    Volume::Pointer vf_out = Volume::New (vh, PT_VF_FLOAT_INTERLEAVED, 3);
    float* img = (float*) vf_out->img;
    FloatVector3DType displacement;

    int i = 0;
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (itk_vf, itk_vf->GetLargestPossibleRegion());
    for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi) {
        displacement = fi.Get ();
        for (int r = 0; r < 3; r++) {
            img[i++] = displacement[r];
        }
    }
    return vf_out;
}


/* -----------------------------------------------------------------------
   Selection routines to convert from X to Y
   ----------------------------------------------------------------------- */
void
xform_to_trn (
    Xform *xf_out, 
    const Xform *xf_in, 
    Plm_image_header *pih)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        xf_out->init_trn ();
        break;
    case XFORM_ITK_TRANSLATION:
        *xf_out = *xf_in;
        break;
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert to trn\n");
        break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

void
xform_to_vrs (
    Xform *xf_out, 
    const Xform *xf_in, 
    Plm_image_header *pih)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        init_versor_default (xf_out);
        break;
    case XFORM_ITK_TRANSLATION:
        xform_trn_to_vrs (xf_out, xf_in);
        break;
    case XFORM_ITK_VERSOR:
        *xf_out = *xf_in;
        break;
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert to vrs\n");
        break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

void
xform_to_quat (
    Xform *xf_out, 
    const Xform *xf_in, 
    Plm_image_header *pih)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        init_quaternion_default (xf_out);
        break;
    case XFORM_ITK_TRANSLATION:
        print_and_exit ("Sorry, couldn't convert to quaternion\n");
        break;
    case XFORM_ITK_VERSOR:
        xform_vrs_to_quat (xf_out, xf_in);
        break;
    case XFORM_ITK_QUATERNION:
        *xf_out = *xf_in;
        break;
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_SIMILARITY:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert to quaternion\n");
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

void
xform_to_aff (
    Xform *xf_out, 
    const Xform *xf_in, 
    Plm_image_header *pih)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        init_affine_default (xf_out);
        break;
    case XFORM_ITK_TRANSLATION:
        xform_trn_to_aff (xf_out, xf_in);
        break;
    case XFORM_ITK_VERSOR:
        xform_vrs_to_aff (xf_out, xf_in);
        break;
    case XFORM_ITK_SIMILARITY:
        xform_sim_to_aff (xf_out, xf_in);
        break;
    case XFORM_ITK_QUATERNION:
        print_and_exit ("Sorry, couldn't convert to aff\n");
        break;
    case XFORM_ITK_AFFINE:
        *xf_out = *xf_in;
        break;
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert to aff\n");
        break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

void
xform_to_similarity (
    Xform *xf_out,
    const Xform *xf_in,
    Plm_image_header *pih)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        init_similarity_default (xf_out);
        break;
    case XFORM_ITK_TRANSLATION:
        xform_trn_to_sim (xf_out, xf_in);
        break;
    case XFORM_ITK_VERSOR:
        xform_vrs_to_sim(xf_out,xf_in);
        break;
    case XFORM_ITK_SIMILARITY:
        *xf_out = *xf_in;
        break;
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert to aff\n");
        break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

void
xform_to_itk_bsp (
    Xform *xf_out, 
    const Xform *xf_in, 
    Plm_image_header* pih,
    const float* grid_spac
)
{
    BsplineTransformType::Pointer bsp;

    switch (xf_in->m_type) {
    case XFORM_NONE:
        xform_itk_bsp_init_default (xf_out);
        itk_bsp_set_grid_img (xf_out, pih, grid_spac);
        break;
    case XFORM_ITK_TRANSLATION:
        xform_trn_to_itk_bsp_bulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_VERSOR:
        xform_vrs_to_itk_bsp_bulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_QUATERNION:
        xform_quat_to_itk_bsp_bulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_AFFINE:
        xform_aff_to_itk_bsp_bulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_BSPLINE:
        xform_itk_bsp_to_itk_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_TPS:
        print_and_exit ("Sorry, couldn't convert itk_tps to itk_bsp\n");
        break;
    case XFORM_ITK_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert itk_vf to itk_bsp\n");
        break;
    case XFORM_GPUIT_BSPLINE: 
        xform_gpuit_bsp_to_itk_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit ("Sorry, couldn't convert gpuit_vf to itk_bsp\n");
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

Xform::Pointer
xform_to_itk_bsp (
    const Xform::Pointer& xf_in, 
    Plm_image_header* pih,
    const float* grid_spac
)
{
    Xform::Pointer xf_out = Xform::New ();
    xform_to_itk_bsp (xf_out.get(), xf_in.get(), pih, grid_spac);
    return xf_out;
}

void
xform_to_itk_bsp_nobulk (
    Xform *xf_out, Xform *xf_in, 
    Plm_image_header* pih,
    const float* grid_spac)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        xform_itk_bsp_init_default (xf_out);
        itk_bsp_set_grid_img (xf_out, pih, grid_spac);
        break;
    case XFORM_ITK_TRANSLATION:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_VERSOR:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_QUATERNION:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_AFFINE:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_BSPLINE:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_TPS:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_VECTOR_FIELD:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_GPUIT_BSPLINE:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        xform_any_to_itk_bsp_nobulk (xf_out, xf_in, pih, grid_spac);
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

Xform::Pointer
xform_to_itk_bsp_nobulk (
    const Xform::Pointer& xf_in, 
    Plm_image_header* pih,
    const float* grid_spac
)
{
    Xform::Pointer xf_out = Xform::New ();
    xform_to_itk_bsp_nobulk (xf_out.get(), xf_in.get(), pih, grid_spac);
    return xf_out;
}

void
xform_to_itk_vf (Xform* xf_out, Xform *xf_in, Plm_image_header* pih)
{
    DeformationFieldType::Pointer vf;

    switch (xf_in->m_type) {
    case XFORM_NONE:
        print_and_exit ("Sorry, couldn't convert to vf\n");
        break;
    case XFORM_ITK_TRANSLATION:
        vf = xform_itk_any_to_itk_vf (xf_in->get_trn(), pih);
        break;
    case XFORM_ITK_VERSOR:
        vf = xform_itk_any_to_itk_vf (xf_in->get_vrs(), pih);
        break;
    case XFORM_ITK_QUATERNION:
        vf = xform_itk_any_to_itk_vf (xf_in->get_quat(), pih);
        break;
    case XFORM_ITK_AFFINE:
        vf = xform_itk_any_to_itk_vf (xf_in->get_aff(), pih);
        break;
    case XFORM_ITK_SIMILARITY:
        vf = xform_itk_any_to_itk_vf (xf_in->get_similarity(), pih);
        break;
    case XFORM_ITK_BSPLINE:
        vf = xform_itk_bsp_to_itk_vf (xf_in, pih);
        break;
    case XFORM_ITK_TPS:
        vf = xform_itk_any_to_itk_vf (xf_in->get_itk_tps(), pih);
        break;
    case XFORM_ITK_VECTOR_FIELD:
        vf = xform_itk_vf_to_itk_vf (xf_in->get_itk_vf(), pih);
        break;
    case XFORM_GPUIT_BSPLINE:
        vf = xform_gpuit_bsp_to_itk_vf (xf_in, pih);
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        vf = xform_gpuit_vf_to_itk_vf (xf_in->get_gpuit_vf().get(), pih);
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
    xf_out->set_itk_vf (vf);
}

/* Overloaded fn. GCS FIX -- maybe this function is not needed */
void
xform_to_itk_vf (Xform* xf_out, Xform *xf_in, FloatImageType::Pointer image)
{
    Plm_image_header pih;
    pih.set_from_itk_image (image);
    xform_to_itk_vf (xf_out, xf_in, &pih);
}

Xform::Pointer
xform_to_itk_vf (const Xform::Pointer& xf_in, Plm_image_header* pih)
{
    Xform::Pointer xf_out = Xform::New ();
    xform_to_itk_vf (xf_out.get(), xf_in.get(), pih);
    return xf_out;
}

void
xform_to_gpuit_bsp (Xform* xf_out, Xform* xf_in, Plm_image_header* pih, 
    const float* grid_spac)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_TRANSLATION:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_VERSOR:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_QUATERNION:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_AFFINE:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_BSPLINE:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_TPS:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_ITK_VECTOR_FIELD:
        xform_any_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_GPUIT_BSPLINE:
        xform_gpuit_bsp_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        xform_gpuit_vf_to_gpuit_bsp (xf_out, xf_in, pih, grid_spac);
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }
}

Xform::Pointer
xform_to_gpuit_bsp (
    const Xform::Pointer& xf_in, 
    Plm_image_header* pih,
    float* grid_spac
)
{
    Xform::Pointer xf_out = Xform::New ();
    xform_to_gpuit_bsp (xf_out.get(), xf_in.get(), pih, grid_spac);
    return xf_out;
}

void
xform_to_gpuit_vf (
    Xform* xf_out, const Xform *xf_in, const Plm_image_header* pih)
{
    Volume::Pointer vf = Volume::New();
    switch (xf_in->m_type) {
    case XFORM_NONE:
        print_and_exit ("Sorry, couldn't convert NONE to gpuit_vf\n");
        break;
    case XFORM_ITK_TRANSLATION:
        vf = xform_itk_any_to_gpuit_vf (xf_in->get_trn(), pih);
        break;
    case XFORM_ITK_VERSOR:
        vf = xform_itk_any_to_gpuit_vf (xf_in->get_vrs(), pih);
        break;
    case XFORM_ITK_QUATERNION:
        vf = xform_itk_any_to_gpuit_vf (xf_in->get_quat(), pih);
        break;
    case XFORM_ITK_AFFINE:
        vf = xform_itk_any_to_gpuit_vf (xf_in->get_aff(), pih);
        break;
    case XFORM_ITK_BSPLINE:
        vf = xform_itk_any_to_gpuit_vf (xf_in->get_itk_bsp(), pih);
        break;
    case XFORM_ITK_TPS:
        vf = xform_itk_any_to_gpuit_vf (xf_in->get_itk_tps(), pih);
        break;
    case XFORM_ITK_VECTOR_FIELD:
        vf = xform_itk_vf_to_gpuit_vf (xf_in->get_itk_vf(), pih);
        break;
    case XFORM_GPUIT_BSPLINE:
        vf = xform_gpuit_bsp_to_gpuit_vf (xf_in, pih);
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        vf = xform_gpuit_vf_to_gpuit_vf (xf_in->get_gpuit_vf(), pih);
        break;
    default:
        print_and_exit ("Program error.  Bad xform type.\n");
        break;
    }

    //xf_out->set_gpuit_vf (Volume::Pointer(vf));
    xf_out->set_gpuit_vf (vf);
}

Xform::Pointer
xform_to_gpuit_vf (
    const Xform::Pointer& xf_in, const Plm_image_header* pih)
{
    Xform::Pointer xf_out = Xform::New ();
    xform_to_gpuit_vf (xf_out.get(), xf_in.get(), pih);
    return xf_out;
}


Xform_type
Xform::get_type () const
{
    return this->m_type;
}

void
Xform::get_volume_header (Volume_header *vh)
{
    switch (this->m_type) {
    case XFORM_NONE:
    case XFORM_ITK_TRANSLATION:
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
        /* Do nothing */
        break;
    case XFORM_ITK_VECTOR_FIELD:
        itk_image_get_volume_header (vh, this->get_itk_vf());
        break;
    case XFORM_GPUIT_BSPLINE: {
        Bspline_xform* bxf = this->get_gpuit_bsp();
        bxf->get_volume_header (vh);
        break;
    }
    case XFORM_GPUIT_VECTOR_FIELD:
        print_and_exit (
            "Sorry, didn't implement get_volume_header (type = %d)\n",
            this->m_type);
        break;
    default:
        print_and_exit ("Sorry, couldn't get_volume_header (type = %d)\n",
            this->m_type);
        break;
    }
}

Plm_image_header
Xform::get_plm_image_header ()
{
    Volume_header vh;
    this->get_volume_header (&vh);
    return Plm_image_header (vh);
}

void
Xform::get_grid_spacing (float grid_spacing[3])
{
    switch (this->m_type) {
    case XFORM_NONE:
    case XFORM_ITK_TRANSLATION:
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
        /* Do nothing */
        break;
    case XFORM_ITK_BSPLINE:
        print_and_exit (
            "Sorry, didn't implement get_grid_spacing (type = %d)\n",
            this->m_type);
        break;
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
        /* Do nothing */
        break;
    case XFORM_GPUIT_BSPLINE: {
        Bspline_xform* bxf = this->get_gpuit_bsp();
        for (int d = 0; d < 3; d++) {
            grid_spacing[d] = bxf->grid_spac[d];
        }
        break;
    }
    case XFORM_GPUIT_VECTOR_FIELD:
        /* Do nothing */
        break;
    default:
        print_and_exit ("Sorry, couldn't get_volume_header (type = %d)\n",
            this->m_type);
        break;
    }
}

void
Xform::print ()
{
    switch (this->m_type) {
    case XFORM_NONE:
        lprintf ("XFORM_NONE\n");
        break;
    case XFORM_ITK_TRANSLATION:
        lprintf ("XFORM_ITK_TRANSLATION\n");
        std::cout << this->get_trn ();
        break;
    case XFORM_ITK_VERSOR:
        lprintf ("XFORM_ITK_VERSOR\n");
        std::cout << this->get_vrs ();
        break;
    case XFORM_ITK_QUATERNION:
        lprintf ("XFORM_ITK_QUATERNION\n");
        break;
    case XFORM_ITK_AFFINE:
        lprintf ("XFORM_ITK_AFFINE\n");
        std::cout << this->get_aff ();
        break;
    case XFORM_ITK_BSPLINE:
        lprintf ("XFORM_ITK_BSPLINE\n");
        break;
    case XFORM_ITK_TPS:
        lprintf ("XFORM_ITK_TPS\n");
        break;
    case XFORM_ITK_VECTOR_FIELD:
        lprintf ("XFORM_ITK_VECTOR_FIELD\n");
        break;
    case XFORM_GPUIT_BSPLINE:
        lprintf ("XFORM_GPUIT_BSPLINE\n");
        break;
    case XFORM_GPUIT_VECTOR_FIELD:
        lprintf ("XFORM_GPUIT_VECTOR_FIELD\n");
        break;
    default:
        print_and_exit ("Sorry, couldn't print xform (type = %d)\n",
            this->m_type);
        break;
    }
}
