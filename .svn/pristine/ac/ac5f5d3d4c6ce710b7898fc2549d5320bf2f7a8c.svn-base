/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "itkImageRegionIteratorWithIndex.h"

#include "compiler_warnings.h"
#include "itk_directions.h"
#include "itk_image_type.h"
#include "plm_image.h"
#include "plm_image_header.h"
#include "plm_int.h"
#include "plm_math.h"
#include "threshbox.h"

void do_threshbox( Threshbox_parms *parms) {

    float spacing_in[3], origin_in[3];
    plm_long dim_in[3];
    Plm_image_header pih;
    unsigned char label_uchar, label_box;

    FloatImageType::Pointer img_in = parms->img_in->itk_float();

    pih.set_from_itk_image (img_in);
    pih.get_dim (dim_in);
    pih.get_origin (origin_in );
    pih.get_spacing (spacing_in );
    // direction cosines??

    /* Create ITK image for labelmap */
    FloatImageType::SizeType sz;
    FloatImageType::IndexType st;
    FloatImageType::RegionType rg;
    FloatImageType::PointType og;
    FloatImageType::SpacingType sp;
    FloatImageType::DirectionType itk_dc;
    for (int d1 = 0; d1 < 3; d1++) {
	st[d1] = 0;
	sz[d1] = dim_in[d1];
	sp[d1] = spacing_in[d1];
	og[d1] = origin_in[d1];
    }
    rg.SetSize (sz);
    rg.SetIndex (st);
    itk_direction_from_dc (&itk_dc, parms->dc);

    // labelmap thresholded image
    UCharImageType::Pointer uchar_img = UCharImageType::New();
    uchar_img->SetRegions (rg);
    uchar_img->SetOrigin (og);
    uchar_img->SetSpacing (sp);
    uchar_img->Allocate();

    // box image
    UCharImageType::Pointer box_img = UCharImageType::New();
    box_img->SetRegions (rg);
    box_img->SetOrigin (og);
    box_img->SetSpacing (sp);
    box_img->Allocate();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > UCharIteratorType;
    UCharIteratorType uchar_img_iterator;
    uchar_img_iterator = UCharIteratorType (uchar_img, uchar_img->GetLargestPossibleRegion());
    uchar_img_iterator.GoToBegin();

    UCharIteratorType box_img_iterator;
    box_img_iterator = UCharIteratorType (box_img, box_img->GetLargestPossibleRegion());
    box_img_iterator.GoToBegin();

    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > FloatIteratorType;
    FloatIteratorType img_in_iterator (img_in, img_in->GetLargestPossibleRegion());

    FloatImageType::IndexType k, k_max;
    FloatPoint3DType phys, phys_max;
    phys_max[0] = phys_max[1] = phys_max[2] = 0.f;
    float level, maxlevel=-1e20;
    for (img_in_iterator.GoToBegin(); !img_in_iterator.IsAtEnd(); ++img_in_iterator) {
	    
	k=img_in_iterator.GetIndex();
//	    img_in->TransformIndexToPhysicalPoint( k, phys );

	level = img_in_iterator.Get();

	//NSh 2012-05-30 - find position of the max over the entire image, disregarding the box
	/*
          if ( (parms->center[0]- parms->boxsize[0]/2 <= k[0] && k[0] < parms->center[0]+parms->boxsize[0]/2) &&
          (parms->center[1]- parms->boxsize[1]/2 <= k[1] && k[1] < parms->center[1]+parms->boxsize[1]/2) &&
          (parms->center[2]- parms->boxsize[2]/2 <= k[2] && k[2] < parms->center[2]+parms->boxsize[2]/2) )
	*/
	{
            if (level> maxlevel) { maxlevel = level;
                k_max = img_in_iterator.GetIndex(); 
                img_in->TransformIndexToPhysicalPoint(k_max, phys_max);
            }
	
	
	}
    }
	
    //char fn_tmp[1024];
    //sprintf(fn_tmp, "c:\\cygwin\\home\\nadya\\%s", parms->max_coord_fn_out);

    FILE *fp0 = fopen( parms->max_coord_fn_out, "w");
    //FILE *fp0 = fopen( fn_tmp, "w");
    if (fp0) { 
	fprintf(fp0, "max_coord\n%.3f %.3f %.3f\n", phys_max[0],phys_max[1],phys_max[2]);
	fclose(fp0);
    }
    
    int label_onecount=0;

    for (img_in_iterator.GoToBegin(); !img_in_iterator.IsAtEnd(); ++img_in_iterator) {
	    
	level = img_in_iterator.Get();
	k=img_in_iterator.GetIndex();
	//img_in->TransformIndexToPhysicalPoint( k, phys );

	label_uchar = 0;
	label_box = 0;
            
	if ( (parms->center[0]- parms->boxsize[0]/2 <= k[0] && k[0] < parms->center[0]+parms->boxsize[0]/2) &&
	    (parms->center[1]- parms->boxsize[1]/2 <= k[1] && k[1] < parms->center[1]+parms->boxsize[1]/2) &&
	    (parms->center[2]- parms->boxsize[2]/2 <= k[2] && k[2] < parms->center[2]+parms->boxsize[2]/2) ) 
	{ /* label_uchar = 2; */ label_box=1; } 
            
	if ( (parms->center[0] - parms->boxsize[0]/2 <= k[0] && k[0] < parms->center[0]+parms->boxsize[0]/2) &&
	    (parms->center[1] - parms->boxsize[1]/2 <= k[1] && k[1] < parms->center[1]+parms->boxsize[1]/2) &&
	    (parms->center[2] - parms->boxsize[2]/2 <= k[2] && k[2] < parms->center[2]+parms->boxsize[2]/2) )	
	    if	(level > parms->threshold/100.*maxlevel) { label_uchar = 1; label_onecount++;  }
	    
	uchar_img_iterator.Set ( label_uchar );
	box_img_iterator.Set ( label_box );
	++uchar_img_iterator;
	++box_img_iterator;
    }

    parms->img_out = new Plm_image;
    parms->img_out->set_itk( uchar_img);

    parms->img_box = new Plm_image;
    parms->img_box->set_itk( box_img);

}

void do_overlap_fraction( Threshbox_parms *parms) {

    double vol_img1=0, vol_img2=0, vol_min=0, vol_overlap=0, max_sep;
    int level1, level2, level11, level22;

    int vol_img1_int = 0, vol_img2_int=0;

    FloatPoint3DType phys_max1, phys_max2;
    UCharImageType::IndexType k_max1, k_max2;

    UCharImageType::IndexType kcurr;
    FloatPoint3DType phys;

    UCharImageType::Pointer img1 = parms->overlap_labelmap1->itk_uchar();
    UCharImageType::Pointer img2 = parms->overlap_labelmap2->itk_uchar();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > UCharIteratorType;

    UCharIteratorType img1_iterator;
    img1_iterator = UCharIteratorType (img1, img1->GetLargestPossibleRegion());
    img1_iterator.GoToBegin();
    
    UCharIteratorType img2_iterator;
    img2_iterator = UCharIteratorType (img2, img2->GetLargestPossibleRegion());
    img2_iterator.GoToBegin();

    for (img1_iterator.GoToBegin(); !img1_iterator.IsAtEnd(); ++img1_iterator) {
        level1 = img1_iterator.Get();
        if ( level1>0 ) { vol_img1++; vol_img1_int++;}
    }

    for (img2_iterator.GoToBegin(); !img2_iterator.IsAtEnd(); ++img2_iterator) {
        level2 = img2_iterator.Get();
        if ( level2>0 ) { vol_img2++; vol_img2_int++;} 
    }


// diameter of the labelmap thresholded image

    float *xone = (float *)malloc(vol_img1_int*sizeof(float));
    float *yone = (float *)malloc(vol_img1_int*sizeof(float));
    float *zone = (float *)malloc(vol_img1_int*sizeof(float));

    int j=0;
    for (img1_iterator.GoToBegin(); !img1_iterator.IsAtEnd(); ++img1_iterator) {

	int level = img1_iterator.Get();

	if (level>0) {
            kcurr=img1_iterator.GetIndex();
            img1->TransformIndexToPhysicalPoint( kcurr, phys );
            xone[j]=phys[0];yone[j]=phys[1];zone[j]=phys[2];
            if (j>vol_img1_int) { fprintf(stderr,"inconsistent labelmap vox count!\n"); exit(1);}
            j++;		
        }
    }


    int i1,i2;
    float diam1=-1;
    for(i1=0;i1<j;i1++) for(i2=i1+1;i2<j;i2++) {
            float d=sqrt( (xone[i1]-xone[i2])*(xone[i1]-xone[i2])+
                (yone[i1]-yone[i2])*(yone[i1]-yone[i2])+
                (zone[i1]-zone[i2])*(zone[i1]-zone[i2])				
            );
            if (d>diam1) diam1=d;		
        }	

    free(xone);free(yone);free(zone);

    xone = (float *)malloc(vol_img2_int*sizeof(float));
    yone = (float *)malloc(vol_img2_int*sizeof(float));
    zone = (float *)malloc(vol_img2_int*sizeof(float));

    j=0;
    for (img2_iterator.GoToBegin(); !img2_iterator.IsAtEnd(); ++img2_iterator) {

	int level = img2_iterator.Get();

	if (level>0) {
            kcurr=img2_iterator.GetIndex();
            img2->TransformIndexToPhysicalPoint( kcurr, phys );
            xone[j]=phys[0];yone[j]=phys[1];zone[j]=phys[2];
            if (j>vol_img2_int) { fprintf(stderr,"inconsistent labelmap vox count!\n"); exit(1);}
            j++;		
        }
    }


//	int i1,i2;
    float diam2=-1;
    for(i1=0;i1<j;i1++) for(i2=i1+1;i2<j;i2++) {
            float d=sqrt(   (xone[i1]-xone[i2])*(xone[i1]-xone[i2])+
                (yone[i1]-yone[i2])*(yone[i1]-yone[i2])+
                (zone[i1]-zone[i2])*(zone[i1]-zone[i2])				
            );
            if (d>diam2) diam2=d;		
        }	

    free(xone);free(yone);free(zone);


    printf("%s\n%s\n", parms->max_coord_fn_in1, parms->max_coord_fn_in2);

    FILE *fp1 = fopen( parms->max_coord_fn_in1, "r");
    char tmpbuf[1024];
    if (fp1) {
	fscanf(fp1, "%s", tmpbuf);
	fscanf(fp1, "%f %f %f", &phys_max1[0], &phys_max1[1], &phys_max1[2]);
	printf("read %f %f %f from %s\n", phys_max1[0],phys_max1[1],phys_max1[2], parms->max_coord_fn_in1);
	fclose(fp1);
    }

    FILE *fp2 = fopen( parms->max_coord_fn_in2, "r");
    //char tmpbuf[1024];
    if (fp2) {
	fscanf(fp2, "%s", tmpbuf);
	fscanf(fp2, "%f %f %f", &phys_max2[0], &phys_max2[1], &phys_max2[2]);
	printf("read %f %f %f from %s\n", phys_max2[0],phys_max2[1],phys_max2[2], parms->max_coord_fn_in2);
	fclose(fp2);
    }
		
    max_sep = (phys_max1[0] - phys_max2[0])*(phys_max1[0] - phys_max2[0])+
        (phys_max1[1] - phys_max2[1])*(phys_max1[1] - phys_max2[1])+
        (phys_max1[2] - phys_max2[2])*(phys_max1[2] - phys_max2[2]);
    max_sep = sqrt(max_sep);
    printf("max_sep %f\n", max_sep);

//    FloatPoint3DType phys;
    bool in_image;
    UCharImageType::IndexType k1;
    UCharImageType::IndexType k2;
    
    img1_iterator = UCharIteratorType (img1, img1->GetLargestPossibleRegion());
    img1_iterator.GoToBegin();
    
    for (img1_iterator.GoToBegin(); !img1_iterator.IsAtEnd(); ++img1_iterator) {
        
	/*make sure we can process images with different offsets etc*/
	k1=img1_iterator.GetIndex();
	img1->TransformIndexToPhysicalPoint( k1, phys );
	in_image = img2->TransformPhysicalPointToIndex( phys, k2) ;
	
	if (in_image) {
            level1 = img1->GetPixel(k1);
            level2 = img2->GetPixel(k2);
            if (level1 >0 && level2 > 0) vol_overlap++;
        }

    }

    if (vol_img1<vol_img2) vol_min=vol_img1; else vol_min=vol_img2;	

    //check if phys_max1 is in overlap
    int max1_in;
    img1->TransformPhysicalPointToIndex(phys_max1, k_max1);
    img2->TransformPhysicalPointToIndex(phys_max1, k_max2);
    level11 = img1->GetPixel(k_max1);
    level22 = img2->GetPixel(k_max2);
    if (level11 > 0 && level22 > 0 ) max1_in = 1; else max1_in = 0;

    int max2_in;
    img1->TransformPhysicalPointToIndex(phys_max2, k_max1);
    img2->TransformPhysicalPointToIndex(phys_max2, k_max2);
    level11 = img1->GetPixel(k_max1);
    level22 = img2->GetPixel(k_max2);
    if (level11 > 0 && level22 > 0 ) max2_in = 1; else max2_in = 0;

    int over_max = -1;
    UNUSED_VARIABLE (over_max);

    if ( max1_in==1 && max2_in==1 )  over_max = 0; // two suv max are in the overlap volume
    if ( max1_in==1 && max2_in==0 )  over_max = 1; // suv max 1 is in the overlap volume, suv max 2 is outside
    if ( max1_in==0 && max2_in==1 )  over_max = 2; // suv max 2 is in the overlap volume, suv max 1 is outside
    if ( max1_in==0 && max2_in==0 )  over_max = 3; // both suv max are outside overlap volume

    printf("trying to write %s\n", parms->overlap_fn_out);
    //return;
    FILE *fpout = fopen( parms->overlap_fn_out, "w");
    if (fpout) { 
	fprintf(fpout, "Vol1 %.1f  Vol2 %.1f  Volmin %.1f  Vover %.1f  (voxels)\n", 
            vol_img1,vol_img2, vol_min, vol_overlap);
	/*fprintf(fpout, "Vover/Volmin = %.3f  MaxSep = %.3f SUV max location = %d\n", 
          vol_overlap/vol_min, max_sep, over_max );*/

	fprintf(fpout, "Vover/Volmin = %.3f", vol_overlap/vol_min );

	float voxvol1;
	float spacing1[3];
	Plm_image_header pih_overlap1 ( parms->overlap_labelmap1 );
	pih_overlap1.get_spacing(spacing1);
	voxvol1=spacing1[0]*spacing1[1]*spacing1[2];	

	float voxvol2;
	float spacing2[3];
	Plm_image_header pih_overlap2 ( parms->overlap_labelmap2 );
	pih_overlap2.get_spacing(spacing2);
	voxvol2=spacing2[0]*spacing2[1]*spacing2[2];	

	fprintf(fpout, "Vol1cm3 %.4f Vol2cm3 %.4f\n", vol_img1*voxvol1/1000., vol_img2*voxvol2/1000. );
	fprintf(fpout, "diam1mm %f diam2mm %f\n",diam1,diam2);
	fclose(fpout);
    }
}

/*
void remove_face_connected_regions(img...)
http://www.itk.org/Doxygen314/html/classitk_1_1ConnectedThresholdImageFilter.html

iterate over src image
{
if (! on_threshbox_face) continue;
if GetPix(src)==0 continue;

seed on the threshbox face = current voxel

create itk::ConnectedThresholdImageFilter

virtual void itk::ImageToImageFilter< TInputImage, TOutputImage >::SetInput  
    ( const InputImageType *  image   )  [virtual] 

virtual void itk::ConnectedThresholdImageFilter< TInputImage, TOutputImage >::SetReplaceValue  
    ( OutputImagePixelType  _arg   )  [virtual] 
//Set/Get value to replace thresholded pixels. Pixels that lie * within Lower and Upper 
//(inclusive) will be replaced with this value. The default is 1. 

void itk::ConnectedThresholdImageFilter< TInputImage, TOutputImage >::SetSeed  
	    ( const IndexType &  seed   )  
virtual void itk::ConnectedThresholdImageFilter< TInputImage, TOutputImage >::SetUpper  
	    ( InputImagePixelType    )  [virtual] 
//also SetLower?  -> set both to 1.

someFilter->Update();
image = someFilter->GetOutput();

iterate over src image
 if ( GetPix( src)>0 && GetPix(filtered)==filterlabel ) Pix(src)=0

delete filter

} 

subtract 1 from nonzero pixels in src

*/






/* thresholders for dose comparison plugin*/

static void do_single_threshold( Threshbox_parms *parms ,int thresh_id ) 
    {

    float spacing_in[3], origin_in[3];
    plm_long dim_in[3];
    Plm_image_header pih;
    unsigned char label_uchar;
    float cutoff = 0;

    if (thresh_id == 1 ) cutoff = parms->isodose_value1;
    if (thresh_id == 2 ) cutoff = parms->isodose_value2;
    if (thresh_id == 3 ) cutoff = parms->isodose_value3;
    if (thresh_id == 4 ) cutoff = parms->isodose_value4;
    if (thresh_id == 5 ) cutoff = parms->isodose_value5;

    FloatImageType::Pointer img_in = parms->img_in->itk_float();

    pih.set_from_itk_image (img_in);
    pih.get_dim (dim_in);
    pih.get_origin (origin_in );
    pih.get_spacing (spacing_in );
    // direction cosines??

    /* Create ITK image for labelmap */
    FloatImageType::SizeType sz;
    FloatImageType::IndexType st;
    FloatImageType::RegionType rg;
    FloatImageType::PointType og;
    FloatImageType::SpacingType sp;
    FloatImageType::DirectionType itk_dc;
    for (int d1 = 0; d1 < 3; d1++) {
	st[d1] = 0;
	sz[d1] = dim_in[d1];
	sp[d1] = spacing_in[d1];
	og[d1] = origin_in[d1];
    }
    rg.SetSize (sz);
    rg.SetIndex (st);
    itk_direction_from_dc (&itk_dc, parms->dc);

    // labelmap thresholded image
    UCharImageType::Pointer uchar_img = UCharImageType::New();
    uchar_img->SetRegions (rg);
    uchar_img->SetOrigin (og);
    uchar_img->SetSpacing (sp);
    uchar_img->Allocate();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > UCharIteratorType;
    UCharIteratorType uchar_img_iterator;
    uchar_img_iterator = UCharIteratorType (uchar_img, uchar_img->GetLargestPossibleRegion());
    uchar_img_iterator.GoToBegin();

    typedef itk::ImageRegionIteratorWithIndex< FloatImageType > FloatIteratorType;
    FloatIteratorType img_in_iterator (img_in, img_in->GetLargestPossibleRegion());

    FloatImageType::IndexType k;
    FloatPoint3DType phys;
    float level;

    for (img_in_iterator.GoToBegin(); !img_in_iterator.IsAtEnd(); ++img_in_iterator) {
	    
	level = img_in_iterator.Get();
	k=img_in_iterator.GetIndex();
	//img_in->TransformIndexToPhysicalPoint( k, phys );

	label_uchar = 0;

	if (level > cutoff ) label_uchar = thresh_id;
	    
	uchar_img_iterator.Set ( label_uchar );
	++uchar_img_iterator;
	
    }


    if (thresh_id == 1 ) {
    parms->dose_labelmap1 = new Plm_image; 
    parms->dose_labelmap1->set_itk( uchar_img); }

    if (thresh_id == 2 ) {
    parms->dose_labelmap2 = new Plm_image; 
    parms->dose_labelmap2->set_itk( uchar_img); }

    if (thresh_id == 3 ) {
    parms->dose_labelmap3 = new Plm_image; 
    parms->dose_labelmap3->set_itk( uchar_img); }

    if (thresh_id == 4 ) {
    parms->dose_labelmap4 = new Plm_image; 
    parms->dose_labelmap4->set_itk( uchar_img); }

    if (thresh_id == 5 ) {
    parms->dose_labelmap5 = new Plm_image; 
    parms->dose_labelmap5->set_itk( uchar_img); }

}

void do_composite_labelmap( Threshbox_parms *parms)
{
    float spacing_in[3], origin_in[3];
    plm_long dim_in[3];
    Plm_image_header pih;

    unsigned char label_uchar;
    unsigned char level[5];

    UCharImageType::Pointer map1 = parms->dose_labelmap1->itk_uchar();
    UCharImageType::Pointer map2 = parms->dose_labelmap2->itk_uchar();
    UCharImageType::Pointer map3 = parms->dose_labelmap3->itk_uchar();
    UCharImageType::Pointer map4 = parms->dose_labelmap4->itk_uchar();
    UCharImageType::Pointer map5 = parms->dose_labelmap5->itk_uchar();

    pih.set_from_itk_image (map1);
    pih.get_dim (dim_in);
    pih.get_origin (origin_in );
    pih.get_spacing (spacing_in );
    // direction cosines??

    /* Create ITK image for labelmap */
    FloatImageType::SizeType sz;
    FloatImageType::IndexType st;
    FloatImageType::RegionType rg;
    FloatImageType::PointType og;
    FloatImageType::SpacingType sp;
    FloatImageType::DirectionType itk_dc;
    for (int d1 = 0; d1 < 3; d1++) {
	st[d1] = 0;
	sz[d1] = dim_in[d1];
	sp[d1] = spacing_in[d1];
	og[d1] = origin_in[d1];
    }
    rg.SetSize (sz);
    rg.SetIndex (st);
    itk_direction_from_dc (&itk_dc, parms->dc);

    // labelmap thresholded image
    UCharImageType::Pointer uchar_img = UCharImageType::New();
    uchar_img->SetRegions (rg);
    uchar_img->SetOrigin (og);
    uchar_img->SetSpacing (sp);
    uchar_img->Allocate();

    typedef itk::ImageRegionIteratorWithIndex< UCharImageType > UCharIteratorType;
    UCharIteratorType it1, it2, it3, it4, it5, it_compo;
    it1 = UCharIteratorType (map1, map1->GetLargestPossibleRegion());
    it2 = UCharIteratorType (map2, map2->GetLargestPossibleRegion());
    it3 = UCharIteratorType (map3, map3->GetLargestPossibleRegion());
    it4 = UCharIteratorType (map4, map4->GetLargestPossibleRegion());
    it5 = UCharIteratorType (map5, map5->GetLargestPossibleRegion());

    it_compo = UCharIteratorType (uchar_img, uchar_img->GetLargestPossibleRegion());

    it1.GoToBegin(); it2.GoToBegin();it3.GoToBegin();it4.GoToBegin();it5.GoToBegin();
    it_compo.GoToBegin();

        for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1) {
	    
	level[0] = it1.Get();
	level[1] = it2.Get();
	level[2] = it3.Get();
	level[3] = it4.Get();
	level[4] = it5.Get();

	label_uchar = level[0];

	int i;
	for(i=0; i<5; i++) { if (level[i]>label_uchar) label_uchar = level[i]; }

	it_compo.Set ( label_uchar );	
	++it2; ++it3; ++it4; ++it5;
	++it_compo;

    }

    parms->composite_labelmap = new Plm_image; 
    parms->composite_labelmap->set_itk( uchar_img); 

}

void do_multi_threshold (Threshbox_parms *parms) 
{ 

do_single_threshold( parms, 1);
do_single_threshold( parms, 2);
do_single_threshold( parms, 3);
do_single_threshold( parms, 4);
do_single_threshold( parms, 5);

/* assumes that threshold increases with thresh_id!! */
do_composite_labelmap( parms );

}
