/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/*  Warp one or more dij matrices based on a vector field */
#include "plmcli_config.h"
#include <fstream>
#include <math.h>
#include <time.h>
#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkInterpolateImagePointsFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkVectorLinearInterpolateImageFunction.h"

#include "itk_image_create.h"
#include "itk_image_save.h"
#include "itk_image_type.h"
#include "itk_warp.h"
#include "logfile.h"
#include "pcmd_warp.h"
#include "plm_image_header.h"
#include "print_and_exit.h"
#include "string_util.h"
#include "warp_parms.h"
#include "xform.h"

typedef unsigned short ushort;
typedef unsigned long ulong;

class Dij_header {
public:
    plm_long dose_dim[3];
    float dose_origin[3];
    float dose_spacing[3];
public:
    Dij_header () {
        for (int d = 0; d < 3; d++) {
            dose_dim[d] = 0;
            dose_origin[d] = 0.;
            dose_spacing[d] = 0.;
        }
    }
};

typedef struct __Pencil_Beam Pencil_Beam;
struct __Pencil_Beam {
    float energy;
    float spot_x;
    float spot_y;
    int nvox;
    ushort* vox;
};

typedef struct __Dij_Matrix Dij_Matrix;
struct __Dij_Matrix {
    float gantry_angle;
    float table_angle;
    float collimator_angle;
    float spot_spacing_dx;
    float spot_spacing_dy;
    float voxel_size_dx;
    float voxel_size_dy;
    float voxel_size_dz;
    int dose_cube_size[3];
    int num_pencil_beams;
    float absolute_dose_coefficient;
};

void
dij_parse_error (void)
{
    fprintf (stderr, "Parse error in reading dij_matrix\n");
    exit (-1);
}

void
dij_write_error (void)
{
    fprintf (stderr, "Error writing dij_matrix\n");
    exit (-1);
}

void
ctatts_parse_error (void)
{
    fprintf (stderr, "Parse error in reading ctatts file\n");
    exit (-1);
}

void
dif_parse_error (void)
{
    fprintf (stderr, "Parse error in reading dif file\n");
    exit (-1);
}

void
load_dif (Dij_header* dijh, const char* dif_in)
{
    int i;
    float f;
    FILE* fp;
    const int BUFLEN = 1024;
    char buf[BUFLEN];

    fp = fopen (dif_in, "rt");
    if (!fp) {
	fprintf (stderr, "Error opening dif file for read: %s\n", dif_in);
	exit (-1);
    }

    /* GCS FIX: I should give an error if not all lines are found */
    /* N.b. This converts from IEC coordinats to DICOM coordinates 
       during read */
    while (1) {
	if (!fgets (buf, BUFLEN, fp)) {
	    break;
	}
	if (buf[0] == '\0' || buf[0] == '\n') {
	    /* Empty lines are ok */
	}
	else if (sscanf (buf, "Delta-X %f", &f)) {
	    dijh->dose_spacing[0] = f;
	}
	else if (sscanf (buf, "Delta-Y %f", &f)) {
	    dijh->dose_spacing[2] = f;
	}
	else if (sscanf (buf, "Delta-Z %f", &f)) {
	    dijh->dose_spacing[1] = f;
	}
	else if (sscanf (buf, "Dimension-Dose-X %d", &i)) {
	    dijh->dose_dim[0] = i;
	}
	else if (sscanf (buf, "Dimension-Dose-Y %d", &i)) {
	    dijh->dose_dim[2] = i;
	}
	else if (sscanf (buf, "Dimension-Dose-Z %d", &i)) {
	    dijh->dose_dim[1] = i;
	}
	else if (sscanf (buf, "Position-Dose-X %f", &f)) {
	    dijh->dose_origin[0] = f;
	}
	else if (sscanf (buf, "Position-Dose-Y %f", &f)) {
	    dijh->dose_origin[2] = f;
	}
	else if (sscanf (buf, "Position-Dose-Z %f", &f)) {
	    dijh->dose_origin[1] = -f;
	}
	else {
            /* Ignore other lines */
	}
    }
    fclose (fp);
}

FloatImageType::Pointer
make_dose_image (
    Dij_header *dijh,
    Dij_Matrix *dij_matrix)
{
    /* GCS FIX: Should check that this value matches the one in 
       the dij header */
#if defined (commentout)
    FloatImageType::SizeType size;
    size[0]  = dij_matrix->dose_cube_size[0];
    size[1]  = dij_matrix->dose_cube_size[1];
    size[2]  = dij_matrix->dose_cube_size[2];
#endif
    return itk_image_create<float> (Plm_image_header (
            dijh->dose_dim, 
            dijh->dose_origin,
            dijh->dose_spacing));
}

void
set_pencil_beam_to_image (
    FloatImageType::Pointer& img,
    const Pencil_Beam *pb)
{
    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > ImageIterator;

    /* Clear image */
    ImageIterator it1 (img, img->GetBufferedRegion());
    it1.GoToBegin();
    while (!it1.IsAtEnd()) {
        FloatImageType::IndexType idx = it1.GetIndex();
	//it1.Set ((float) idx[0]);
	it1.Set (0.0);
	++it1;
    }

    /* Set non-zero voxels into image */
    SizeType sz = img->GetLargestPossibleRegion().GetSize();
    FloatImageType::IndexType itk_index;
    for (long i = 0; i < pb->nvox; i++) {
	/* Get index and value; convert index to DICOM */
	long pb_index = (pb->vox[i*3+1] << 16) + pb->vox[i*3];
	ushort value = pb->vox[i*3+2];
        long iec_index[3];
        iec_index[0] = pb_index % sz[0];
        long tmp = pb_index / sz[0];
        iec_index[1] = tmp % sz[2];
        iec_index[2] = tmp / sz[2];

	/* Convert index to DICOM */
        itk_index[2] = (sz[2] - iec_index[1] - 1);
        itk_index[1] = iec_index[2];
        itk_index[0] = iec_index[0];

#if defined (commentout)
        printf ("%d -> %d %d %d\n", pb_index, 
            itk_index[0],  itk_index[1], itk_index[2]);
        break;
#endif

        /* Set voxel */
        img->SetPixel (itk_index, (float) value);
    }
}

void
read_pencil_beam (Pencil_Beam* pb, FILE* fp)
{
    int rc;
    rc = fread (&pb->energy, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&pb->spot_x, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&pb->spot_y, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&pb->nvox, sizeof(int), 1, fp);
    if (rc != 1) dij_parse_error();

    pb->vox = (ushort*) malloc (3*pb->nvox*sizeof(ushort));

    rc = fread (pb->vox, sizeof(ushort), 3*pb->nvox, fp);
    if (rc != 3*pb->nvox) dij_parse_error();
}

void
write_pencil_beam (Pencil_Beam* pb, FloatImageType::Pointer img, FILE* fp)
{
    int rc;
    int nvox = 0;
    long index = 0;
    long nvox_loc, eob_loc;

    rc = fwrite (&pb->energy, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&pb->spot_x, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&pb->spot_y, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    nvox_loc = ftell (fp);
    rc = fwrite (&pb->nvox, sizeof(int), 1, fp);
    if (rc != 1) dij_write_error();

    SizeType sz = img->GetLargestPossibleRegion().GetSize();
    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > ImageIterator;
    ImageIterator it1 (img, img->GetBufferedRegion());
    it1.GoToBegin();
    while (!it1.IsAtEnd()) {
	ushort vox[3];
	vox[2] = (ushort) it1.Get();
	if (vox[2] > 0) {
            ImageIterator::IndexType idx = it1.GetIndex();
            long iec_index
                = idx[1]*sz[2]*sz[0] + (sz[2]-idx[2]-1)*sz[0] + idx[0];
	    vox[0] = (ushort) (iec_index & 0xFFFF);
	    vox[1] = (ushort) (iec_index >> 16);
#if defined (commentout)
            printf ("%d %d %d -> %d, 0x%x -> 0x%02x 0x%02x\n", 
                idx[0], idx[1], idx[2], 
                iec_index, iec_index, vox[0], vox[1]);
            break;
#endif
	    rc = fwrite (vox, sizeof(ushort), 3, fp);
	    if (rc != 3) dij_write_error();
	    nvox++;
	}
	++index;
	++it1;
    }
    eob_loc = ftell (fp);
    fseek (fp, nvox_loc, SEEK_SET);
    rc = fwrite (&nvox, sizeof(int), 1, fp);
    if (rc != 1) dij_write_error();
    fseek (fp, eob_loc, SEEK_SET);
}

void
read_dij_header (Dij_Matrix* dij_matrix, FILE* fp)
{
    int rc;

    rc = fread (&dij_matrix->gantry_angle, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->table_angle, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->collimator_angle, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->spot_spacing_dx, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->spot_spacing_dy, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->voxel_size_dx, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->voxel_size_dy, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->voxel_size_dz, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->dose_cube_size, sizeof(int), 3, fp);
    if (rc != 3) dij_parse_error();
    rc = fread (&dij_matrix->num_pencil_beams, sizeof(int), 1, fp);
    if (rc != 1) dij_parse_error();
    rc = fread (&dij_matrix->absolute_dose_coefficient, sizeof(float), 1, fp);
    if (rc != 1) dij_parse_error();
}

void
write_dij_header (Dij_Matrix* dij_matrix, FILE* fp)
{
    int rc;

    rc = fwrite (&dij_matrix->gantry_angle, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->table_angle, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->collimator_angle, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->spot_spacing_dx, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->spot_spacing_dy, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->voxel_size_dx, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->voxel_size_dy, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->voxel_size_dz, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->dose_cube_size, sizeof(int), 3, fp);
    if (rc != 3) dij_write_error();
    rc = fwrite (&dij_matrix->num_pencil_beams, sizeof(int), 1, fp);
    if (rc != 1) dij_write_error();
    rc = fwrite (&dij_matrix->absolute_dose_coefficient, sizeof(float), 1, fp);
    if (rc != 1) dij_write_error();
}

FloatImageType::Pointer
warp_pencil_beam (
    DeformationFieldType::Pointer& vf, 
    FloatImageType::Pointer& pb_img, 
    Dij_Matrix *dij_matrix,
    Dij_header *dijh, 
    Pencil_Beam *pb)
{
    return itk_warp_image (pb_img, vf, 1, 0.f);
}

void
convert_vector_field (
    Xform::Pointer& xf,
    Dij_header* dijh,
    Warp_parms *parms)
{
    int i;
    FILE *fp_in, *fp_out;
    Dij_Matrix dij_matrix;
    Pencil_Beam pb;

    fp_in = fopen (parms->input_fn.c_str(), "rb");
    if (!fp_in) {
	fprintf (stderr, "Error opening dij file for read: %s\n", 
            parms->input_fn.c_str());
	exit (-1);
    }
    fp_out = fopen (parms->output_dij_fn.c_str(), "wb");
    if (!fp_out) {
	fprintf (stderr, "Error opening dij file for write: %s\n", 
            parms->output_dij_fn.c_str());
	exit (-1);
    }

    /* Load the header */
    read_dij_header (&dij_matrix, fp_in);
    printf ("Found %d pencil beams\n", dij_matrix.num_pencil_beams);
    write_dij_header (&dij_matrix, fp_out);

    /* Create a new image to hold the input_image (gets re-used 
       for each pencil beam) */
    FloatImageType::Pointer pb_img = make_dose_image (
        dijh, &dij_matrix);

    /* Resample vector field to match dose image */
    printf ("Resampling vector field (please be patient)\n");
    Plm_image_header pih (pb_img);
    Xform::Pointer xf2 = xform_to_itk_vf (xf, &pih);
    DeformationFieldType::Pointer vf = xf2->get_itk_vf ();

    /* For each pencil beam, load, warp, and write warped */
    for (i = 0; i < dij_matrix.num_pencil_beams; i++) {
        printf ("Warping PB %03d\n", i);
	read_pencil_beam (&pb, fp_in);
        set_pencil_beam_to_image (pb_img, &pb);
        if (parms->output_dij_dose_volumes) {
            std::string fn = string_format ("PB_%03d.nrrd", i);
            itk_image_save (pb_img, fn);
        }
        FloatImageType::Pointer warped_pb_img 
            = warp_pencil_beam (vf, pb_img, &dij_matrix, dijh, &pb);
        if (parms->output_dij_dose_volumes) {
            std::string fn = string_format ("PBw_%03d.nrrd", i);
            itk_image_save (warped_pb_img, fn);
        }
	write_pencil_beam (&pb, warped_pb_img, fp_out);
	free (pb.vox);
    }

    /* Done! */
    fclose (fp_in);
    fclose (fp_out);
}

void
warp_dij_main (Warp_parms* parms)
{
    lprintf ("Loading xf...\n");
    Xform::Pointer xf = xform_load (parms->xf_in_fn);

    lprintf ("Loading dif...\n");
    Dij_header dijh;
    load_dif (&dijh, parms->dif_in_fn.c_str());

    convert_vector_field (xf, &dijh, 
        parms);
}
