/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmutil_config.h"
#include <string.h>
#include <stddef.h>
#include <itkScaleInvariantFeatureImageFilter.h>
//#include <itkImageSeriesReader.h>
//#include <itkNumericSeriesFileNames.h>
//#include <itkAffineTransform.h>
//#include <itkLinearInterpolateImageFunction.h>
//#include <itkResampleImageFilter.h>

#include "itk_image_type.h"
#include "plm_image.h"
#include "print_and_exit.h"
#include "sift.h"

typedef itk::ScaleInvariantFeatureImageFilter<
    FloatImageType, 3> SiftFilterType;

class Sift_private {
public:
    Sift_private () {
        image_doubled = false;
        octave = 3;
        initial_sigma1 = 2;
        initial_sigma2 = 2;
        descriptor_dimension = 8;
        contrast = 0.03;
        curvature = 172.3025;
        flag_curve = true;
        normalization = true;
        match_ratio = 0.9;

        image = 0;
    }
    ~Sift_private () {
        delete image;
    }
public:
    bool image_doubled;   //false: no doubling; true: doubling
    unsigned int octave;  //number of octave
    float initial_sigma1; //float	initial_sigma_sy = 1.5;
    float initial_sigma2;
    float descriptor_dimension;
    float contrast;       //if we assume image pixel value in the range [0,1]
    float curvature;      //if we assume image pixel value in the range [0,1]
    bool flag_curve;      //1: curvature; 0: no curvature;
    bool normalization;   //true: normalization of input image for 
                          //contrast&curvature thresholds definition 
    float match_ratio;    //from 0 (no matches) to 1 (all matches)


    Plm_image *image;
    SiftFilterType::PointSetTypePointer keypoints;
    SiftFilterType sift_filter;
};

Sift::Sift () {
    d_ptr = new Sift_private;
}

Sift::~Sift () {
    delete d_ptr;
}

void
Sift::set_image (const char* image_fn)
{
    d_ptr->image = new Plm_image (image_fn);
}

void
Sift::set_image (const std::string& image_fn)
{
    this->set_image (image_fn.c_str());
}

void
Sift::set_contrast_threshold (float contrast_threshold)
{
    d_ptr->contrast = contrast_threshold;
}

void
Sift::set_curvature_threshold (float curvature_threshold)
{
    if (curvature_threshold > 0.f) {
        /* Use curvature */
        d_ptr->flag_curve = true;
    } else {
        /* Don't use curvature */
        d_ptr->flag_curve = false;
    }
    d_ptr->curvature = curvature_threshold;
}

void
Sift::run ()
{
    if (!d_ptr->image) {
        print_and_exit ("Error: image not defined for Sift::run()\n");
    }
    d_ptr->sift_filter.SetDoubling(d_ptr->image_doubled);
    d_ptr->sift_filter.SetNumScales(d_ptr->octave);
    d_ptr->sift_filter.SetInitialSigma(d_ptr->initial_sigma1);
    d_ptr->sift_filter.SetContrast(d_ptr->contrast);
    d_ptr->sift_filter.SetCurvature(d_ptr->curvature);
    d_ptr->sift_filter.SetDescriptorDimension(d_ptr->descriptor_dimension);
    d_ptr->sift_filter.SetMatchRatio(d_ptr->match_ratio);

    printf ("Contrast threshold = %f\n", d_ptr->contrast);
    printf ("Curvature threshold = %f\n", d_ptr->curvature);

    /* output keypoints from image */
    d_ptr->keypoints = d_ptr->sift_filter.getSiftFeatures (
        d_ptr->image->itk_float(), 
        d_ptr->flag_curve,
        d_ptr->normalization, 
        "", "", "", "", "", "");
/*
    d_ptr->keypoints = d_ptr->sift_filter.getSiftFeatures (
        d_ptr->image->itk_float(), 
        d_ptr->flag_curve,
        d_ptr->normalization, 
        "phy_max1.fcsv",
        "phy_max2.fcsv",
        "imagecoord_max1.txt",
        "imagecoord_min1.txt",
        "point_rej_contrast1.fcsv",
        "point_rej_curvature1.fcsv"
    );
*/
}

itk::ScaleInvariantFeatureImageFilter<FloatImageType,3>::PointSetTypePointer
Sift::get_keypoints ()
{
    return d_ptr->keypoints;
}

void
Sift::save_pointset (const char* filename)
{
    d_ptr->sift_filter.save_pointset (filename);
}

void
Sift::match_features (
    Sift& sift1, Sift& sift2,
    const char* filename1, const char* filename2, 
    float match_ratio)
{
    itk::ScaleInvariantFeatureImageFilter<FloatImageType,3>
        ::PointSetTypePointer keypoints1, keypoints2;
    keypoints1 = sift1.get_keypoints ();
    keypoints2 = sift2.get_keypoints ();

    SiftFilterType::MatchKeypointsFeatures (
        keypoints1, keypoints2, filename1, filename2, match_ratio);
}


/* ***********************************************************************
 * The below is left here for reference.
 *************************************************************************/
#if defined (commentout)

#define DIMENSION 3
// Command Line Arguments
int ARG_IMG1=2;
int ARG_IMG2=3;

int main( int argc, char *argv[] ) 
{
    const unsigned int Dimension = DIMENSION;
	 
    // ---- DEFAULT PARAMETERS:
    bool image_doubled = false; //false: no doubling; true: doubling
    unsigned int octave = 3; //number of octave
    float initial_sigma1 = 2; //float	initial_sigma_sy = 1.5;
    float initial_sigma2 = 2;
    float descriptor_dimension = 8;
    float contrast = 0.03; //if we assume image pixel value in the range [0,1]
    float curvature = 172.3025; //if we assume image pixel value in the range [0,1]
    bool flag_curve = true; //1: curvature; 0: no curvature;
    bool normalization = true;	//true: normalization of input image for 
    //contrast&curvature thresholds definition 
    float match_ratio = 0.9; //from 0 (no matches) to 1 (all matches)

    double test_scale = 1.0; // Default scale is 1.0
    float test_rotate = 0.0;  // 0 degrees
    float test_translate = 0.0; //0 mm
    //float test_rotate = 0.0874;  // 5 degrees
    //float test_rotate = 0.1748;  // 10 degrees
   
    int mode = 'i';  /* defaults to comparing 2 images */;
    int transform_middle=0; /* defaults to applying transformations around the origin */
  
    //output:
    char *point_match1="phy_match1.fcsv";
    char *point_match2="phy_match2.fcsv";
    char *point_max1="phy_max1.fcsv";
    char *point_max2="phy_max2.fcsv";
    char *point_min1="phy_min1.fcsv";
    char *point_min2="phy_min2.fcsv";

    // ---- OPTIONS:
#define OPT_OCTAVE 'o'
#define OPT_DOUBLING 'di'
#define OPT_INITIAL_SIGMA1 'z1'
#define OPT_INITIAL_SIGMA2 'z2'
#define OPT_DESCRIPTOR 'de'
#define OPT_CONTRAST 'co'
#define OPT_CURVATURE 'cu'
#define OPT_MATCH 'm'
#define OPT_SCALE 'x'
#define OPT_ROTATE 'r'
#define OPT_TRANSLATE 't'
    //#define OPT_DIM 'd'
    
    while(1) {
        static struct option long_options[] =
            {
                // Modalities (These options set a flag).
                {"synthetic", 0, &mode, 's'},
                {"image", 0, &mode, 'i'},
                {"transform-middle", 0, &transform_middle, 1},
                // Parameters (These options don't set a flag)
                {"double", required_argument, 0, OPT_DOUBLING},
                {"octave", required_argument, 0, OPT_OCTAVE},
                {"initial-sigma1", required_argument, 0, OPT_INITIAL_SIGMA1},
                {"initial-sigma2", required_argument, 0, OPT_INITIAL_SIGMA2},
                {"contrast", required_argument, 0, OPT_CONTRAST},
                {"curvature", required_argument, 0, OPT_CURVATURE},
                {"descr-dim", required_argument, 0, OPT_DESCRIPTOR},
                {"match-ratio", required_argument, 0, OPT_MATCH},
                //{"dimension", required_argument, 0, OPT_DIM},
                {"scale", required_argument, 0, OPT_SCALE},
                {"rotate", required_argument, 0, OPT_ROTATE},
                {"translate", required_argument, 0, OPT_TRANSLATE},
                //Output
                {"out-match1", required_argument, 0, 1},
                {"out-match2", required_argument, 0, 2},
                {"out-max1", required_argument, 0, 3},
                {"out-min1", required_argument, 0, 4},
                {"out-max2", required_argument, 0, 5},
                {"out-min2", required_argument, 0, 6},
                {0, 0, 0, 0}
            };    

        int optindex;
        int val = getopt_long(argc, argv, "", long_options, &optindex);

        if (val == -1)
            break;

        switch(val) {
	case OPT_DOUBLING:
            image_doubled = atof(optarg);
            break;
        case OPT_OCTAVE:
            octave = atof(optarg);
            break;
        case OPT_INITIAL_SIGMA1:
            initial_sigma1 = atof(optarg);
            break;
	case OPT_INITIAL_SIGMA2:
            initial_sigma2 = atof(optarg);
            break;
	case OPT_CONTRAST:
            contrast = atof(optarg);
            if(contrast<0.0|| contrast>1.0)
            {normalization=false;
		flag_curve=false;}
            break;
	case OPT_CURVATURE:
            if(atof(optarg)==0)
                flag_curve=false;
            if(!normalization && atof(optarg)!=0)
                flag_curve=true;
            curvature = atof(optarg);
            break;
	case OPT_DESCRIPTOR:
            descriptor_dimension = atof(optarg);
            break;
	case OPT_MATCH:
            match_ratio = atof(optarg);
            break;
            //case OPT_DIM:
            //	Dimension = atoi(optarg);
            //     break;
        case OPT_SCALE:
            test_scale = atof(optarg);
            break;
        case OPT_ROTATE:
            if (atof(optarg) >= 0.0 && atof(optarg) <= 360.0)
                test_rotate = atof(optarg) * PI * 2.0 / 360.0;
            break;
	case OPT_TRANSLATE:
            test_translate = atof(optarg);
            break;
            //Output:
	case 1:
            point_match1=optarg;
            break;
	case 2:
            point_match2=optarg;
            break;
	case 3:
            point_max1=optarg;
            break;
	case 4:
            point_min1=optarg;
            break;
	case 5:
            point_max2=optarg;
            break;
	case 6:
            point_min2=optarg;
            break;
        }
    }

    ARG_IMG1 = optind;
    ARG_IMG2 = optind+1;

    FILE* match1=0;match1=fopen(point_match1,"w");fclose(match1);
    FILE* match2=0;match2=fopen(point_match2,"w");fclose(match2);
    FILE* max1=0;max1=fopen(point_max1,"w");fclose(max1);
    FILE* min1=0;min1=fopen(point_min1,"w");fclose(min1);
    FILE* max2=0;max2=fopen(point_max2,"w");fclose(max2);
    FILE* min2=0;min2=fopen(point_min2,"w");fclose(min2);

    typedef  float  PixelType;
    typedef itk::Image< PixelType, Dimension >  FixedImageType;
    typedef itk::ScaleInvariantFeatureImageFilter<FixedImageType, Dimension> SiftFilterType;

    typedef itk::ImageSource< FixedImageType > ImageSourceType;
    ImageSourceType::Pointer fixedImageReader, fixedImageReader2;
 
    // ---- USAGE:
    if( argc <= ARG_IMG1 || (mode == 'i' && argc <= ARG_IMG2))
    {
        std::cerr << "Incorrect number of parameters " << std::endl;
        std::cerr << std::endl;

        std::cerr << "USAGE: \n";
        std::cerr << argv[0] << " [options] ImageFile [ImageFile2]\n"; 
        std::cerr << "This program takes as input 3D images and generates Scale Invariant Feature Transform" << std::endl;
        std::cerr << std::endl;
        std::cerr << "**IMAGE PROCESSING OPTIONS (Choose ONE):" << std::endl;
        std::cerr << "--image <ImageFile ImageFile2>" << std::endl;
        std::cerr << "	compare ImageFile.mha and ImageFile2.mha" << std::endl;
        std::cerr << "OR\n" << std::endl;
        std::cerr << "--synthetic <ImageFile>" << std::endl;
        std::cerr << "	compare ImageFile to synthetically generated version" << std::endl;
        std::cerr << "	return the synthetic image as output (image_transform.mha)" << std::endl;
        std::cerr << "	Synthetic Image Options:" << std::endl;
        std::cerr << "	--rotate <arg>"  << std::endl;		
        std::cerr << "		rotate synthetic image on first axis [degree]" << std::endl;
        std::cerr << "	--translate <arg>"  << std::endl;		
        std::cerr << "		translate synthetic image [mm]" << std::endl;
        std::cerr << "	--scale <arg>"  << std::endl;
        std::cerr << "		scale all axes of synthetic image" << std::endl;
        std::cerr << "	--transform-middle"<< std::endl;
        std::cerr << "		center of transformation: center of the image (default origin)" << std::endl;
        std::cerr << "\n**PARAMETERS:\n" << std::endl;
        /*std::cerr << "--dimension <arg>" << std::endl;
	  std::cerr << "	image dimension (default 3)" << std::endl;*/
        std::cerr << "--double <arg>" << std::endl;
        std::cerr << "	image doubling -> yes:1 or no:0 (default 0)" << std::endl;
        std::cerr << "--octave <arg>" << std::endl;
        std::cerr << "	set number of octaves (default 3)" << std::endl;
        //the number of octave is a function of the image dimension!
        std::cerr << "--initial-sigma1 <arg>" << std::endl;
        std::cerr << "	set Gaussian blur initial sigma for ImageFile (default 2mm)" << std::endl;
        std::cerr << "--initial-sigma2 <arg>" << std::endl;
        std::cerr << "	set Gaussian blur initial sigma for ImageFile2/synthetic version" << std::endl; 
        std::cerr << "	(default 2mm)" << std::endl;
        std::cerr << "--contrast <arg>" << std::endl;
        std::cerr << "	threshold on image contrast (default 0.03 for image value in [0,1])" << std::endl;
        std::cerr << "	if contrast value is 0, the contrast threshold is not performed" << std::endl;
        std::cerr << "	if contrast value is greater than 1, the curvature threshold" << std::endl; 
        std::cerr << "	is not performed in default modality" << std::endl;
        std::cerr << "--curvature <arg>" << std::endl;
        std::cerr << "	threshold on image curvature (default 172.3 for image value in [0,1])" << std::endl;
        std::cerr << "	if curvature value is 0, the curvature threshold is not performed" << std::endl;
        std::cerr << "--descr-dim <arg>" << std::endl;
        std::cerr << "	half of the keypoint descriptor region size" << std::endl;
        std::cerr << "	(default 8 voxels -> 16x16x16 region size)" << std::endl;
        std::cerr << "--match-ratio <arg>" << std::endl;
        std::cerr << "	set matching ratio in the range [0,1] (default 0.9)" << std::endl;
        std::cerr << "\n**OUTPUT:\n" << std::endl;
        std::cerr << "Default: Feature in physical coordinates in RAS system (file format .fcsv)" << std::endl;
        std::cerr << "--out-max1 <arg>" << std::endl;
        std::cerr << "	maxima keypoints of ImageFile (default phy_max1.fcsv)" << std::endl;
        std::cerr << "--out-min1 <arg>" << std::endl;
        std::cerr << "	minima keypoints of ImageFile (default phy_min1.fcsv)" << std::endl;
        std::cerr << "--out-max2 <arg>" << std::endl;
        std::cerr << "	maxima keypoints of ImageFile2 (default phy_max2.fcsv)" << std::endl;
        std::cerr << "--out-min2 <arg>" << std::endl;
        std::cerr << "	minima keypoints of ImageFile2 (default phy_max2.fcsv)" << std::endl;
        std::cerr << "--out-match1 <arg>" << std::endl;
        std::cerr << "	matching keypoints of ImageFile (default point_match1.fcsv)" << std::endl;
        std::cerr << "--out-match2 <arg>" << std::endl;
        std::cerr << "	matching keypoints of ImageFile2 (default point_match2.fcsv)" << std::endl;
        std::cerr << std::endl;
        return 1;
    }

    std::cerr << "Dimension = " << Dimension << "\n";
    /*std::cerr << "Test Scale = " << test_scale << "\n";
      std::cerr << "Test Rotate = " << test_rotate << "\n";
      std::cerr << "Test Translate = " << test_translate << "\n";*/
    std::cerr << "Mode = " << (char) mode << "\n";
    std::cerr << "ImageFile1 = " << argv[optind] << "\n";
    /*std::cerr << "curvature = " << curvature << "\n";
      std::cerr << "flag_curve = " << flag_curve << "\n";
      std::cerr << "contrast = " << contrast << "\n";
      std::cerr << "normalization = " << normalization << "\n";*/

  
    std::cerr << "SIFT Feature\n" << std::endl;
  
    //Read Input Image1
    typedef itk::ImageFileReader< FixedImageType  > FixedImageReaderType;
    FixedImageReaderType::Pointer tmpImageReader  = FixedImageReaderType::New();
    tmpImageReader  = FixedImageReaderType::New();
    tmpImageReader->SetFileName(  argv[ARG_IMG1] );
    fixedImageReader=tmpImageReader;
    fixedImageReader->Update();
    FixedImageType::Pointer fixedImage= FixedImageType::New();
    try{
        fixedImage = fixedImageReader->GetOutput();
    }
    catch (itk::ExceptionObject &err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        return -1;
    }

    SiftFilterType::PointSetTypePointer keypoints1, keypoints2;
    SiftFilterType siftFilter1, siftFilter2;
  
    //siftFilter1.writeImage(fixedImage, "InputImage1.mha");
  
    //set parameters:
    siftFilter1.SetDoubling(image_doubled);
    siftFilter1.SetNumScales(octave);
    siftFilter1.SetInitialSigma(initial_sigma1);
    siftFilter1.SetContrast(contrast);
    siftFilter1.SetCurvature(curvature);
    siftFilter1.SetDescriptorDimension(descriptor_dimension);
    siftFilter1.SetMatchRatio(match_ratio);
    //output keypoints from first Image 
    keypoints1 = siftFilter1.getSiftFeatures(fixedImage, flag_curve,normalization, point_max1,point_min1,"imagecoord_max1.txt","imagecoord_min1.txt","point_rej_contrast1.fcsv","point_rej_curvature1.fcsv");

    typedef itk::AffineTransform< double, Dimension > TestTransformType;
    typedef TestTransformType::InputVectorType VectorType;  
    typedef TestTransformType::ParametersType ParametersType;
    TestTransformType::Pointer test_transform = TestTransformType::New();
    test_transform->SetIdentity();
    FixedImageType::Pointer scaledImage = FixedImageType::New();
 
    // ---- SYNTHETIC TEST IMAGE:
    if (mode=='s') {
        std::cerr << std::endl << "Synthetic image mode\n";  
   
	const unsigned int np = test_transform->GetNumberOfParameters();
        ParametersType parameters( np ); // Number of parameters
        TestTransformType::InputPointType translate_vector;
	
	FixedImageType::PointType origin = fixedImage->GetOrigin();
	FixedImageType::SpacingType spacing = fixedImage->GetSpacing();
	FixedImageType::SizeType size = fixedImage->GetLargestPossibleRegion().GetSize();

	if (transform_middle) {
            std::cerr << "Transformation centred at middle of image." << std::endl;    
            /* Cycle through each dimension and shift by half, taking into account the element spacing*/
            for (int k = 0; k < Dimension; ++k)
                translate_vector[k] = origin[k]+(size[k]/2.0)*spacing[k];
            test_transform->SetCenter(translate_vector);
            std::cout<<"Center of Transformation: "<<translate_vector<<std::endl;
            std::cout<<"Origin: "<<fixedImage->GetOrigin()<<std::endl;
        } 
	else {
            std::cerr << "Transformation centred at origin." << std::endl;   
            test_transform->SetCenter( origin );
            std::cout<<"Center of Transformation: "<<test_transform->GetCenter()<<std::endl;
            std::cout<<"Origin: "<<fixedImage->GetOrigin()<<std::endl;
        }
	
        //ROTATION (around z):
        TestTransformType::OutputVectorType rot;
        rot[0]=0;
        rot[1]=0;
        rot[2]=1;
        //test_transform->Rotate(0,1,test_rotate);
        test_transform->Rotate3D(rot,test_rotate);
	
        //TRANSLATION:
        TestTransformType::OutputVectorType tr;
        tr[0]=tr[1]=tr[2]=test_translate;
        test_transform->Translate(tr);

        //SCALING:
        TestTransformType::OutputVectorType scaling;
        scaling[0]=scaling[1]=scaling[2]= test_scale;
        test_transform->Scale(scaling);

        std::cout << "Transform Parms: " << std::endl;
        std::cout << test_transform->GetParameters() << std::endl;
        /*std::cout << "MATRIX: " << std::endl;
	  std::cout << test_transform->GetMatrix() << std::endl;*/

        FixedImageType::Pointer scaledImage;
        typedef itk::ResampleImageFilter<FixedImageType,FixedImageType> ResampleFilterType;
        ResampleFilterType::Pointer scaler = ResampleFilterType::New();
        scaler->SetInput(fixedImage);
        //scaler->SetSize(size);
        //scaler->SetOutputSpacing(spacing);
        //scaler->SetOutputOrigin(origin);
        //scaler->SetOutputDirection( fixedImage->GetDirection() );
      
        FixedImageType::SizeType newsize;
        FixedImageType::PointType offset;
	  	  
        for (int k = 0; k < Dimension; ++k)
            newsize[k] = (unsigned int) size[k] / test_scale;
				
        scaler->SetSize( newsize );
        std::cout << "New size: " << newsize << std::endl;
        scaler->SetOutputSpacing(spacing);
	  	  
        if(newsize!=size && transform_middle)  //scaling centred at middle of image
        {
            for (int k = 0; k < Dimension; ++k)
                offset[k]=translate_vector[k]-(newsize[k]/2.0)*spacing[k];
            std::cout<<"New Origin: "<<offset<<std::endl;
            scaler->SetOutputOrigin(offset);
        }
        else
        {scaler->SetOutputOrigin(origin);}

        scaler->SetOutputDirection( fixedImage->GetDirection() );

        //INTERPOLATION:
        // Linear Interpolation:
        typedef itk::LinearInterpolateImageFunction< FixedImageType, double >  InterpolatorType;
        InterpolatorType::Pointer interpolator = InterpolatorType::New();
        // B-spline Interpolation:
        /* typedef itk::BSplineInterpolateImageFunction< FixedImageType, double >  InterpolatorType;
           InterpolatorType::Pointer interpolator = InterpolatorType::New();
           interpolator->SetSplineOrder(3);
           interpolator->SetInputImage(fixedImage);
           interpolator->UseImageDirectionOn();*/
        scaler->SetInterpolator( interpolator );
        scaler->SetDefaultPixelValue( (PixelType) -1200 );
        scaler->SetTransform(test_transform);
        scaler->Update();
        scaledImage = scaler->GetOutput();	
      
        //set parameters for synthetic image
        siftFilter2.SetDoubling(image_doubled);
        siftFilter2.SetNumScales(octave);
        siftFilter2.SetDescriptorDimension(descriptor_dimension);
        siftFilter2.SetInitialSigma(initial_sigma2);
        siftFilter2.SetContrast(contrast);
        siftFilter2.SetCurvature(curvature);
        siftFilter2.SetMatchRatio(match_ratio);
        siftFilter2.writeImage(scaledImage, "image_transform.mha");
        //output keypoints of synthetic image
        keypoints2 = siftFilter2.getSiftFeatures(scaledImage,flag_curve,normalization,point_max2,point_min2,"imagecoord_max2.txt","imagecoord_min2.txt","point_rej_contrast2.fcsv","point_rej_curvature2.fcsv");  
    
        /*std::cerr << "Test Image Scale: " << test_scale << std::endl;
          std::cerr << "Test Translate: " << test_translate << std::endl;
          std::cerr << "Test Image Rotate: " << test_rotate << std::endl;*/    
    } 
    // ---- IMAGE COMPARISON MODE:
    else if (mode == 'i') {
        std::cerr << std::endl << "Image Comparison mode\n";  

        //Read ImageFile2
        typedef itk::ImageFileReader< FixedImageType  > FixedImageReaderType;
        FixedImageReaderType::Pointer tmpImageReader  = FixedImageReaderType::New();
        tmpImageReader  = FixedImageReaderType::New();
        tmpImageReader->SetFileName(  argv[ARG_IMG2] );
        fixedImageReader2 = tmpImageReader;
        fixedImageReader2->Update();
        FixedImageType::Pointer fixedImage2 = fixedImageReader2->GetOutput(); 

        //set parameters for ImageFile2
        siftFilter2.SetDoubling(image_doubled);
        siftFilter2.SetNumScales(octave);
        siftFilter2.SetDescriptorDimension(descriptor_dimension);
        siftFilter2.SetInitialSigma(initial_sigma2);
        siftFilter2.SetContrast(contrast);
        siftFilter2.SetCurvature(curvature);
        siftFilter2.SetMatchRatio(match_ratio);

        //output keypoints from ImageFile2
        keypoints2 = siftFilter2.getSiftFeatures(fixedImage2,flag_curve,normalization,point_max2,point_min2,"imagecoord_max2.txt","imagecoord_min2.txt","point_rej_contrast2.fcsv","point_rej_curvature2.fcsv");
    }
  
    // ---- MATCHING:
    std::cerr << std::endl << "Matching Keypoints\n";  
    siftFilter2.MatchKeypointsFeatures(keypoints1, keypoints2, point_match1, point_match2);

    return 0;

}

#endif
