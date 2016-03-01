/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "itkImage.h"
#include "itkImageRegionConstIterator.h"

#define BUFLEN 1024

char header_pat[] = 
    "ObjectType = Image\n"
    "NDims = 3\n"
    "BinaryData = True\n"
    "BinaryDataByteOrderMSB = False\n"
	"TransformMatrix = 1 0 0 0 1 0 0 0 1\n"
    "Offset = 0 0 0\n"
	"CenterOfRotation = 0 0 0\n"
    "ElementSpacing = %f %f %f\n"
    "DimSize = %d %d %d\n"
	"AnatomicalOrientation = RPI\n"
	"ElementNumberOfChannels = 1\n"
    "ElementType = MET_FLOAT\n"
    "ElementDataFile = %s\n"
    ;


int main (int argc, char* argv[])
{
    // File pointers
    FILE *ofpx, *ofpy, *ofpz;
    Plm_image_header pih;
    FloatImageType::Pointer img_fixed;
    DeformationFieldType::Pointer vf;
    // input xform from bspline
    Xform xf_in, xf_out;

    if (argc!=4)  {
        std::cerr << "Wrong Parameters " << std::endl;
        std::cerr << "Usage: " << argv[0];
        std::cerr << " <input xform> ";
        std::cerr << " <fixed image> ";
        std::cerr << " <outputVectorFieldDir>" << std::endl;;
        return 1;
    }

    xform_load (&xf_in, argv[1]);
    img_fixed = itk_image_load_float (argv[2], 0);
    xform_to_itk_vf (&xf_out, &xf_in, img_fixed);
    vf = xf_out.get_itk_vf();

    // need to write to 3 separate files, named as defX.mhd defY.mhd and defZ.mhd
    char ofn1[255], ofn2[255], ofn3[255];

    sprintf(ofn1, "%s%s", argv[3], "defX.mhd");
    ofpx = fopen(ofn1, "w");
    if (ofpx == NULL) 
    {
        fprintf(stdout, "open file %s failed\n", ofn1);
        exit(-1);
    }
    sprintf(ofn2, "%s%s", argv[3], "defY.mhd");
    ofpy = fopen(ofn2, "w");
    if (ofpx == NULL) 
    {
        fprintf(stdout, "open file %s failed\n", ofn2);
        exit(-1);
    }

    sprintf(ofn3, "%s%s", argv[3], "defZ.mhd");
    ofpz = fopen(ofn3, "w");
    if (ofpz == NULL) 
    {
        fprintf(stdout, "open file %s failed\n", ofn3);
        exit(-1);
    }

    // print file header
    pih.set_from_itk_image (img_fixed);

    // write deformation field to separate binary files
    fprintf(ofpx, header_pat, pih.m_spacing[0], pih.m_spacing[1], pih.m_spacing[2], 
        pih.m_region.GetSize()[0], pih.m_region.GetSize()[1], pih.m_region.GetSize()[2], 
        "defX.raw");
    fclose(ofpx);
    fprintf(ofpy, header_pat, pih.m_spacing[0], pih.m_spacing[1], pih.m_spacing[2], 
        pih.m_region.GetSize()[0], pih.m_region.GetSize()[1], pih.m_region.GetSize()[2], 
        "defY.raw");
    fclose(ofpy);
    fprintf(ofpz, header_pat, pih.m_spacing[0], pih.m_spacing[1], pih.m_spacing[2], 
        pih.m_region.GetSize()[0], pih.m_region.GetSize()[1], pih.m_region.GetSize()[2], 
        "defZ.raw");
    fclose(ofpz);

    sprintf(ofn1, "%s%s", argv[3], "defX.raw");
    ofpx = fopen(ofn1, "wb");
    sprintf(ofn2, "%s%s", argv[3], "defY.raw");
    ofpy = fopen(ofn2, "wb");
    sprintf(ofn3, "%s%s", argv[3], "defZ.raw");
    ofpz = fopen(ofn3, "wb");

    // iterate through the deformation field pixel by pixel and write each x, y, z
    // component to the corresponding binary files
    typedef itk::ImageRegionConstIterator< DeformationFieldType > RegionIteratorType;

    fprintf (stdout, "requested region size %ld %ld %ld\n",
        vf->GetRequestedRegion().GetSize()[0],
        vf->GetRequestedRegion().GetSize()[1],
        vf->GetRequestedRegion().GetSize()[2]);

    RegionIteratorType  vf_it(vf, vf->GetRequestedRegion() );
    
    vf_it.GoToBegin();
    while(! vf_it.IsAtEnd() )
    {
        DeformationFieldType::PixelType v(vf_it.Get());        
        fwrite(&v[0], 4, 1, ofpx);
        fwrite(&v[1], 4, 1, ofpy);
        fwrite(&v[2], 4, 1, ofpz);
        ++ vf_it;
    }

    fclose(ofpx);
    fclose(ofpy);
    fclose(ofpz);

    return 0;
}
