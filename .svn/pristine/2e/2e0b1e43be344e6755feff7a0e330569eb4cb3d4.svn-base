/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <string.h>

#include "print_and_exit.h"
#include "string_util.h"
#include "xform.h"
#include "xform_legacy.h"

/* -----------------------------------------------------------------------
   Utility functions
   ----------------------------------------------------------------------- */
static int
get_parms (FILE* fp, itk::Array<double>* parms, int num_parms)
{
    float f;
    int r, s;

    s = 0;
    while ((r = fscanf (fp, "%f",&f))) {
	(*parms)[s++] = (double) f;
	if (s == num_parms) break;
	if (!fp) break;
    }
    return s;
}

/* This is an older xform format that plastimatch used to use.  Maybe 
   some users still have some of these, so we can let them load them. */
void
xform_legacy_load (Xform *xf, FILE* fp)
{
    char buf[1024];
    rewind (fp);
    fgets (buf,1024,fp);
    if (plm_strcmp (buf,"ObjectType = MGH_XFORM_TRANSLATION") == 0) {
	TranslationTransformType::Pointer trn = TranslationTransformType::New();
	TranslationTransformType::ParametersType xfp(12);
	int num_parms;

	num_parms = get_parms (fp, &xfp, 3);
	if (num_parms != 3) {
	    print_and_exit ("Wrong number of parameters in xf_in file.\n");
	} else {
	    trn->SetParameters(xfp);
#if defined (commentout)
	    std::cout << "Initial translation parms = " << trn << std::endl;
#endif
	}
	xf->set_trn (trn);
    } else if (plm_strcmp(buf,"ObjectType = MGH_XFORM_VERSOR")==0) {
	VersorTransformType::Pointer vrs = VersorTransformType::New();
	VersorTransformType::ParametersType xfp(6);
	int num_parms;

	num_parms = get_parms (fp, &xfp, 6);
	if (num_parms != 6) {
	    print_and_exit ("Wrong number of parameters in xf_in file.\n");
	} else {
	    vrs->SetParameters(xfp);
#if defined (commentout)
	    std::cout << "Initial versor parms = " << vrs << std::endl;
#endif
	}
	xf->set_vrs (vrs);
    } else if (plm_strcmp(buf,"ObjectType = MGH_XFORM_AFFINE")==0) {
	AffineTransformType::Pointer aff = AffineTransformType::New();
	AffineTransformType::ParametersType xfp(12);
	int num_parms;

	num_parms = get_parms (fp, &xfp, 12);
	if (num_parms != 12) {
	    print_and_exit ("Wrong number of parameters in xf_in file.\n");
	} else {
	    aff->SetParameters(xfp);
#if defined (commentout)
	    std::cout << "Initial affine parms = " << aff << std::endl;
#endif
	}
	xf->set_aff (aff);
    } else if (plm_strcmp (buf, "ObjectType = MGH_XFORM_BSPLINE") == 0) {
	int s[3];
	float p[3];
	BsplineTransformType::RegionType::SizeType bsp_size;
	BsplineTransformType::RegionType bsp_region;
	BsplineTransformType::SpacingType bsp_spacing;
	BsplineTransformType::OriginType bsp_origin;
	BsplineTransformType::DirectionType bsp_direction;

	/* Initialize direction cosines to identity */
	bsp_direction[0][0] = bsp_direction[1][1] = bsp_direction[2][2] = 1.0;

	/* Create the bspline structure */
	xform_itk_bsp_init_default (xf);

	/* Skip 2 lines */
	fgets(buf,1024,fp);
	fgets(buf,1024,fp);

	/* Load bulk transform, if it exists */
	fgets(buf,1024,fp);
	if (!strncmp ("BulkTransform", buf, strlen("BulkTransform"))) {
	    TranslationTransformType::Pointer trn = TranslationTransformType::New();
	    VersorTransformType::Pointer vrs = VersorTransformType::New();
	    AffineTransformType::Pointer aff = AffineTransformType::New();
	    itk::Array <double> xfp(12);
	    float f;
	    int n, num_parm = 0;
	    char *p = buf + strlen("BulkTransform = ");
	    while (sscanf (p, " %g%n", &f, &n) > 0) {
		if (num_parm>=12) {
		    print_and_exit ("Error loading bulk transform\n");
		}
		xfp[num_parm] = f;
		p += n;
		num_parm++;
	    }
	    if (num_parm == 12) {
		aff->SetParameters(xfp);
#if defined (commentout)
		std::cout << "Bulk affine = " << aff;
#endif
		xf->get_itk_bsp()->SetBulkTransform (aff);
	    } else if (num_parm == 6) {
		vrs->SetParameters(xfp);
#if defined (commentout)
		std::cout << "Bulk versor = " << vrs;
#endif
		xf->get_itk_bsp()->SetBulkTransform (vrs);
	    } else if (num_parm == 3) {
		trn->SetParameters(xfp);
#if defined (commentout)
		std::cout << "Bulk translation = " << trn;
#endif
		xf->get_itk_bsp()->SetBulkTransform (trn);
	    } else {
		print_and_exit ("Error loading bulk transform\n");
	    }
	    fgets(buf,1024,fp);
	}

	/* Load origin, spacing, size */
	if (3 != sscanf(buf,"Offset = %g %g %g",&p[0],&p[1],&p[2])) {
	    print_and_exit ("Unexpected line in xform_in file.\n");
	}
	bsp_origin[0] = p[0]; bsp_origin[1] = p[1]; bsp_origin[2] = p[2];

	fgets(buf,1024,fp);
	if (3 != sscanf(buf,"ElementSpacing = %g %g %g",&p[0],&p[1],&p[2])) {
	    print_and_exit ("Unexpected line in xform_in file.\n");
	}
	bsp_spacing[0] = p[0]; bsp_spacing[1] = p[1]; bsp_spacing[2] = p[2];

	fgets(buf,1024,fp);
	if (3 != sscanf(buf,"DimSize = %d %d %d",&s[0],&s[1],&s[2])) {
	    print_and_exit ("Unexpected line in xform_in file.\n");
	}
	bsp_size[0] = s[0]; bsp_size[1] = s[1]; bsp_size[2] = s[2];

#if defined (commentout)
	std::cout << "Offset = " << origin << std::endl;
	std::cout << "Spacing = " << spacing << std::endl;
	std::cout << "Size = " << size << std::endl;
#endif

	fgets(buf,1024,fp);
	if (plm_strcmp (buf, "ElementDataFile = LOCAL")) {
	    print_and_exit ("Error: bspline xf_in failed sanity check\n");
	}

	/* Set the BSpline grid to specified parameters */
	bsp_region.SetSize (bsp_size);
	xform_itk_bsp_set_grid (xf, bsp_origin, bsp_spacing, 
	    bsp_region, bsp_direction);

	/* Read bspline coefficients from file */
	const unsigned int num_parms 
	    = xf->get_itk_bsp()->GetNumberOfParameters();
	BsplineTransformType::ParametersType bsp_coeff;
	bsp_coeff.SetSize (num_parms);
	for (unsigned int i = 0; i < num_parms; i++) {
	    float d;
	    if (!fgets(buf,1024,fp)) {
		print_and_exit ("Missing bspline coefficient from xform_in file.\n");
	    }
	    if (1 != sscanf(buf,"%g",&d)) {
		print_and_exit ("Bad bspline parm in xform_in file.\n");
	    }
	    bsp_coeff[i] = d;
	}

	/* Copy into bsp structure */
	xf->get_itk_bsp()->SetParametersByValue (bsp_coeff);
    } else {
        /* Not a legacy format */
    }
}

/* This is the older xform_save routine for itk b-splines.  It is not used */
void
xform_legacy_save_itk_bsp (
    BsplineTransformType::Pointer transform, 
    const char* filename
)
{
    FILE* fp = fopen (filename,"w");
    if (!fp) {
	printf ("Error: Couldn't open file %s for write\n", filename);
	return;
    }

    fprintf (fp,
	"ObjectType = MGH_XFORM_BSPLINE\n"
	"NDims = 3\n"
	"BinaryData = False\n");
    if (transform->GetBulkTransform()) {
	if ((!strcmp ("TranslationTransform", transform->GetBulkTransform()->GetNameOfClass())) ||
	    (!strcmp ("AffineTransform", transform->GetBulkTransform()->GetNameOfClass())) ||
	    (!strcmp ("VersorTransform", transform->GetBulkTransform()->GetNameOfClass())) ||
	    (!strcmp ("VersorRigid3DTransform", transform->GetBulkTransform()->GetNameOfClass()))) {
	    fprintf (fp, "BulkTransform =");
	    for (unsigned int i = 0; i < transform->GetBulkTransform()->GetNumberOfParameters(); i++) {
		fprintf (fp, " %g", transform->GetBulkTransform()->GetParameters()[i]);
	    }
	    fprintf (fp, "\n");
	} else if (strcmp("IdentityTransform", transform->GetBulkTransform()->GetNameOfClass())) {
	    printf ("Warning!!! BulkTransform exists. Type=%s\n", transform->GetBulkTransform()->GetNameOfClass());
	    printf (" # of parameters=%d\n", transform->GetBulkTransform()->GetNumberOfParameters());
	    printf (" The code currently does not know how to handle this type and will not write the parameters out!\n");
	}
    }

    fprintf (fp,
	"Offset = %f %f %f\n"
	"ElementSpacing = %f %f %f\n"
	"DimSize = %lu %lu %lu\n"
	"ElementDataFile = LOCAL\n",
	transform->GetGridOrigin()[0],
	transform->GetGridOrigin()[1],
	transform->GetGridOrigin()[2],
	transform->GetGridSpacing()[0],
	transform->GetGridSpacing()[1],
	transform->GetGridSpacing()[2],
	transform->GetGridRegion().GetSize()[0],
	transform->GetGridRegion().GetSize()[1],
	transform->GetGridRegion().GetSize()[2]
    );
    for (unsigned int i = 0; i < transform->GetNumberOfParameters(); i++) {
	fprintf (fp, "%g\n", transform->GetParameters()[i]);
    }
    fclose (fp);
}
