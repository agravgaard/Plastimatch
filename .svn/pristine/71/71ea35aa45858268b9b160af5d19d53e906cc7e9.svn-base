/*=========================================================================
Program: ITK nSIFT Implemention (Template Source)
Module: $RCSfile: itkScaleInvariantFeatureImageFilter.txx,v $
Language: C++
Date: $Date: 2007/11/25 15:51:48 $
Version: $Revision: 1.0 $
Copyright (c) 2005,2006,2007 Warren Cheung
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
* The name of the Insight Consortium, nor the names of any consortium members,
nor of any contributors, may be used to endorse or promote products derived
from this software without specific prior written permission.
* Modified source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This routine was modified by Chiara Paganelli and Marta Peroni from the one 
proposed by Warren Cheung in:
http://www.insight-journal.org/browse/publication/207

=========================================================================*/

#define VERBOSE 
#ifndef SIFTKEY_CLASS
#define SIFTKEY_CLASS

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "itkScaleInvariantFeatureImageFilter.h"
#include <cstdio>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include <itkLinearInterpolateImageFunction.h>
#include <itkHessianRecursiveGaussianImageFilter.h> 
#include "itkRescaleIntensityImageFilter.h"

/* GCS 2012-09-27: The original code had a #ifdef which included 
   legal C++ for MSVC, but illegal C++ for other compilers.
   I've replaced this so it uses the following ifdef instead.
   If you want the illegal code, uncomment the below. */
//#define USE_ORIGINAL_BROKEN_CODE 1

const float PI=3.14159265;

namespace itk
{  
// ---- PARAMETERS: 
template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetNumScales ( unsigned int tmp) //number of octaves
{
    m_ImageScalesTestedNumber = tmp; 
    //m_ImageScalesTestedNumber = 3;
}

template <class TFixedImageType, int VDimension> 
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>::ScaleInvariantFeatureImageFilter() 
{
    m_ScalingFactor = 2.0; //scaling factor
    m_DifferenceOfGaussianTestsNumber = 3; //2    
    m_ErrorThreshold = 0.0;	    
    // Derived from above:
    m_DifferenceOfGaussianImagesNumber = m_DifferenceOfGaussianTestsNumber+2;
    m_GaussianImagesNumber = m_DifferenceOfGaussianImagesNumber+1;
    m_IdentityTransform = IdentityTransformType::New();
}

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetInitialSigma( float InitialSigma) //initial sigma for Gaussian blur
{
    m_GaussianSigma = InitialSigma;
}   

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetContrast( float contrast) //contrast threshold value 
{
    m_MinKeypointValue = contrast; //0.03; // if we assume image pixel value in the range [0,1] (otherwise 72)
}  

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetCurvature( float curvature) //curvature threshold value
{
    m_ThresholdPrincipalCurve = curvature; //172.3025; //if we assume image pixel value in the range [0,1]
}  

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetDescriptorDimension( float descriptor_dim) //descriptor dimension
{
    m_SIFTHalfWidth = descriptor_dim; //12.0; //8.0;  // This MUST be a multiple of m_SIFTSubfeatureWidth: 
    // it is half the size of the descriptor
    // for phantom: region of the descriptor=16 so m_SIFTHalfWidth=8.0
    // for image: region of the descriptor=24 so m_SIFTHalfWidth=12.0
    m_SIFTSubfeatureWidth = 4; //number of subregions along the dimension of the descriptor
    m_SIFTSubfeatureBins = 8; //bins of the descriptor
}  

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetMatchRatio ( float tmp) //Matching threshold
{
    m_MaxFeatureDistanceRatio = tmp;
}

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SetDoubling (bool tmp) //Image Doubling
{
    m_DoubleOriginalImage = tmp;
}
      
template <class TFixedImageType, int VDimension> 
double
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::GetGaussianScale( int j ) //Sigma update
{	
    /*sigma{i}=k*sigma{i-1};
      with k=2^(1/s);
      but for indipendent variables:
      sigma_{total}^2 = sigma_{i}^2 + sigma_{i-1}^2;
      so sigma_{i}^2 = sigma_{total}^2 - sigma_{i-1}^2;*/

    double k=0;
    k = pow( 2.0, 1.0 / (double) m_DifferenceOfGaussianTestsNumber);
    if (j==0){
        return m_GaussianSigma;}
    else{
        double sig_prev=0;
        double sig_total=0;
        double variance_gauss=0;
        sig_prev = pow( k, j-1 ) * m_GaussianSigma;
        sig_total = sig_prev * k;
        variance_gauss = sqrt( sig_total * sig_total - sig_prev * sig_prev );
        return variance_gauss;
    }
}

template <class TFixedImageType, int VDimension> 
typename ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>::GradientImageType::Pointer
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::GetHypersphericalCoordinates(typename GradientImageType::Pointer inputImg) 
// Compute hypersferical coordinates (spherycal coordinates for 3D) from Gradient Image
// Returns an image in which each voxel is described by its hypersferical coordinates
{
    typedef itk::ImageRegionIteratorWithIndex< GradientImageType > ImageIteratorType;
    typedef itk::ImageRegionConstIteratorWithIndex< GradientImageType > ConstImageIteratorType;
    typedef itk::ImageRegionConstIteratorWithIndex< TFixedImageType > ConstFixedImageIteratorType;
    typename GradientImageType::Pointer outputImg = GradientImageType::New();
    // Copy attributes
    outputImg->SetRegions(inputImg->GetLargestPossibleRegion());
    outputImg->CopyInformation( inputImg );
    outputImg->Allocate();

    ConstImageIteratorType inputIt(inputImg, inputImg->GetLargestPossibleRegion());
    ImageIteratorType outputIt(outputImg, inputImg->GetLargestPossibleRegion());
      
    for ( inputIt.GoToBegin(), outputIt.GoToBegin(); !inputIt.IsAtEnd();
	  ++inputIt, ++outputIt)
    {
        typename GradientImageType::PixelType x =  inputIt.Get();
        typename GradientImageType::PixelType p;

        // position 0 is the norm
        p[0] = x.GetNorm();
        // position 1 is arctan (x0 / x1) (azimuth)
        p[1] = atan2( x[0],x[1] );
        // Iterate over all the positions
        // position k  is arctan (x_k-1 / (x_k * cos p_k))	  
        for (unsigned int k = 2; k < x.Size(); ++k)
        {
            p[k] = atan2( x[k-1], x[k] * cos(p[k-1]));
            //p[2]: elevation
        }
        outputIt.Set(p);
    }
    return outputImg;
}

template <class TFixedImageType, int VDimension> 
unsigned int
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::SiftFeatureSize() 
// compute: 4^n*8^(n-1), with n=dimension
//	example for 3D: in a region of 16*16*16 we build sub-regions 4*4*4 
//	with 8*8 bin (for each orientation - azimuth and elevation)
{
    unsigned int size = 1;
    for (unsigned int i = 0; i < VDimension; ++i)
    {
	size *= (m_SIFTHalfWidth * 2 / m_SIFTSubfeatureWidth );
	if (i > 0)
            size *= m_SIFTSubfeatureBins;
    }
    return size;
}
  
template <class TFixedImageType, int VDimension> 
unsigned int
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::DeltaToSiftIndex (int delta[]) 
// Convert the delta iterator into index to the 
// start of the SIFT histogram
{
    unsigned int bin = 0;
    unsigned int binpos = 1;
    //std::cerr << "Converting delta: ";

    for (unsigned int i = 0; i < VDimension; ++i) {
        //std::cerr << delta[i];
        unsigned int tmp =  (delta[i] + m_SIFTHalfWidth) / m_SIFTSubfeatureWidth;
        bin += tmp * binpos;
        binpos *= (m_SIFTHalfWidth * 2 / m_SIFTSubfeatureWidth );     
    }
    for (unsigned int i = 1; i < VDimension; ++i)
    { bin *= m_SIFTSubfeatureBins;}

    return bin;
}


template <class TFixedImageType, int VDimension> 
typename ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>::FeatureType
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::GetSiftKey(typename GradientImageType::Pointer inputImg,
    FixedImagePointer multImg,
    IndexType pixelIndex) 
//Computes ORIENTATION HISTOGRAM
{
    //std::cerr << "GetSiftKey..." << std::endl; 
    FeatureType sifthistogram(this->SiftFeatureSize());
    sifthistogram.Fill(0);
	
    // delta iterates from  -m_SIFTHalfWidth to m_SIFTHalfWidth-1 
    // in each dimensions
    int delta[VDimension];
    for (int k = 0; k < VDimension; ++k) {
        delta[k] = -m_SIFTHalfWidth;    
    }

    typename GradientImageType::SizeType regionSize = 
        inputImg->GetLargestPossibleRegion().GetSize();	
    
    while(1) {
        unsigned int siftbin = this->DeltaToSiftIndex(delta);
        // std::cerr << "Siftbin:" << siftbin << std::endl; 
        // Get pixel index. Clamp to image edges
        IndexType tmpIndex;
        for (int k=0; k < VDimension; ++k) {
            if ((pixelIndex[k] + delta[k]) < 0) {
                tmpIndex[k] = 0;
            } else {
                tmpIndex[k] = pixelIndex[k] + delta[k];
                if (((SizeValueType) tmpIndex[k]) >= regionSize[k])
                    tmpIndex[k] = regionSize[k]-1;
            }
            //std::cout << "tmpIndex: "<< tmpIndex[k] << std::endl;
        }
      
        //std::cerr << "Pixel:" << tmpIndex << std::endl; 

        typename GradientImageType::PixelType x = inputImg->GetPixel(tmpIndex);
      
        // Get histogram bin
        // Iterate over all the positions
        unsigned int bin = 0;
        unsigned int binpos = 1;
        for (unsigned int k = 1; k < x.Size(); ++k)
        {
            // Rescale from -PI to PI ->  0 to m_HistogramBinsNumber-1
            float p;
            p = (x[k] + PI)  * (float) m_SIFTSubfeatureBins / (2.0 * PI); 
            if (p < 0 || p >= m_SIFTSubfeatureBins) 
                p = 0;
            bin += (unsigned int) p * binpos;
            binpos *= m_SIFTSubfeatureBins;
            //std::cout << "p: "<< p << std::endl;
        }
        bin += siftbin;
             
        // Fill Sift Index bin
        if (bin > this->SiftFeatureSize()) {
            // VERY BAD
            std::cerr << bin << " > " << this->SiftFeatureSize() << " Warning -- Overload2\n";
        }
        sifthistogram[bin] += x[0] * multImg->GetPixel(tmpIndex); //fill histogram bin
        //std::cerr << "x0: "<<x[0]<<std::endl;
        //std::cerr << "GaussCircle: "<<multImg->GetPixel(tmpIndex)<<std::endl;
        //std::cerr << "hist: "<<sifthistogram[bin]<<std::endl;
			  
        // Increment delta
        bool resetdelta=false;
        for(int k=0; k <= VDimension; ++k) {
            //std::cerr << delta[k];
            if (k == VDimension) {
                //std::cerr << "done\n";
                resetdelta = true;
                break; // done
            }
            // Don't want to go past m_SIFTHalfWidth-1
            if (++delta[k] < (int) m_SIFTHalfWidth) {
                break;
            }
            delta[k] = -m_SIFTHalfWidth; // reset and increment the next pos
        }
        if(resetdelta) break;	  
    }
	 
    //std::cerr << "SIFT key: " << sifthistogram << "\n";

    return(sifthistogram);
}
  
template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::CheckLocalExtrema(FixedImagePointer image, IndexType pixelIndex,
    PixelType pixelValue,
    bool &isMax, bool &isMin,
    bool checkCentre)
// Computes Local Extrema (Maxima & Minima)
{
    int delta[VDimension];
    for (int k = 0; k < VDimension; ++k) {
        delta[k] = -1;
    }
    while(1) {
        bool isZero=true;
        if (!checkCentre) {
            for (int k=0; k < VDimension; ++k) {
                if(delta[k] != 0) {
                    isZero = false;
                    break;
                }
            }
        }
      
        if (checkCentre || !isZero) {
            // Check if not the centre
            IndexType tmpIndex;
            for (int k=0; k < VDimension; ++k) {
                tmpIndex[k] = pixelIndex[k] + delta[k];
            }
	
            typename TFixedImageType::PixelType tmpValue = 
                image->GetPixel(tmpIndex);
	  	
            //std::cout << "...Comparing to ( ";
            //
            //for (int k = 0; k < VDimension; ++k)
            //{
            //  std::cout << tmpIndex[k] << " ";
            //  std::cout << pixelIndex[k] << " ";
            //  std::cout << delta[k] << "\n";
            ////std::cout << ") = " << tmpValue << "\n";
            //}
 
            // Treat as equality if within the error bound
            //Finite difference derivative
            if (((tmpValue - pixelValue) <= m_ErrorThreshold) && 
                ((tmpValue - pixelValue) >= -m_ErrorThreshold)) {
                isMax = false;
                isMin = false;
            } else	
                if (tmpValue > pixelValue) {
                    isMax = false;
                } else
                    if (tmpValue < pixelValue) {
                        isMin = false;
                    }
            if (!isMax && !isMin) break;
        }
        // Increment delta
        bool resetdelta=false;
        for(int k=0; k <= VDimension; ++k) {
            if (k == VDimension) {
                resetdelta = true;
                break; // done
            }
            if (delta[k] < 1) {
                ++delta[k];
                //std::cout << "...increment delta ( ";
                //std::cout << delta[k] << " \n";
                break;
            }
            delta[k] = -1; // reset and increment the next pos
        }
        if(resetdelta) break;	  
    }  
}
  
template <class TFixedImageType, int VDimension> 
typename ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>::FeatureType
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::GetFeatures( FixedImagePointer fixedImage, 
    typename GradientImageType::Pointer hgradImage,
    PointType &point, float currScale) 
{
    // KEYPOINT DESCRIPTOR:
    // Input:  Image, Gradient Image, Point
    // Output:  Vector of direction

    // Generate the Gaussian
    typedef GaussianImageSource<TFixedImageType> GaussianImageSourceType;
    typename GaussianImageSourceType::Pointer gaussImgSource;
    typename GaussianImageSourceType::ArrayType sigma;
	
    gaussImgSource = GaussianImageSourceType::New();
    gaussImgSource->SetNormalized(true);
    gaussImgSource->SetSpacing(fixedImage->GetSpacing());
    gaussImgSource->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
    gaussImgSource->SetDirection(fixedImage->GetDirection());
    gaussImgSource->SetOrigin(fixedImage->GetOrigin());
    gaussImgSource->SetMean(point);
       
    // Simulate the 16x16 Gaussian window descriptor
    // use sigma equal to half the descriptor window width
    for (int i = 0; i < VDimension; ++i)
        sigma[i]=m_SIFTHalfWidth; //sigma[i] = 8.00;
    gaussImgSource->SetSigma(sigma);
    gaussImgSource->Update();

    /*char filename[256];
      sprintf(filename, "GaussianCircle.mha");
      this->writeImage(gaussImgSource->GetOutput(), filename);*/
    
    IndexType pixelIndex;
    fixedImage->TransformPhysicalPointToIndex(point, pixelIndex);
     
    // return the Gaussian weighted Histogram
    return this->GetSiftKey(hgradImage, gaussImgSource->GetOutput(),
        pixelIndex);
}

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::writeImage(FixedImagePointer fixedImage, const char *filename)
{
    typedef itk::Image< float, VDimension >    myImageType;
    typedef itk::ImageFileWriter< myImageType >  myWriterType;
    typedef  itk::ResampleImageFilter< TFixedImageType,  myImageType    >    myResampleFilterType;   
    typename myResampleFilterType::Pointer resampler = myResampleFilterType::New();
    resampler->SetInput(fixedImage);
    resampler->SetReferenceImage(fixedImage);
    resampler->SetUseReferenceImage(true);
    typename myWriterType::Pointer writer = myWriterType::New();
    writer->SetFileName( filename ); //outputFilename
    writer->SetInput( resampler->GetOutput());
  
    std::cout << "[Writing file << " << filename << "]";

    try 
    { 
        writer->Update(); 
    } 
    catch( itk::ExceptionObject & err ) 
    { 
        std::cerr << "ExceptionObject caught !" << std::endl; 
        std::cerr << err << std::endl; 
    } 
}
 
template <class TFixedImageType, int VDimension> 
typename ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>::ResampleFilterType::Pointer
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::getScaleResampleFilter ( typename TFixedImageType::Pointer fixedImage, float scale )
// Create a filter that resamples the image (scale up or down)
{
    typename ResampleFilterType::Pointer scaler = ResampleFilterType::New();
    scaler->SetInput( fixedImage );
   	
    // Change the size of the image
    typename TFixedImageType::SizeType size = fixedImage->GetLargestPossibleRegion().GetSize();
    for (int k = 0; k < VDimension; ++k)
        size[k] = (unsigned int) floor(size[k] * scale);
    scaler->SetSize( size );
    //std::cout << " VDIM: " << VDimension << std::endl;
    //std::cout << " SIZE AFTER RASAMPLE: " << size[0] << " " << size[1] << std::endl;

    // Change the spacing of the image
    typename TFixedImageType::SpacingType spacing = fixedImage->GetSpacing();
    for (int k = 0; k < VDimension; ++k)
        spacing[k] = (spacing[k] / scale);
    scaler->SetOutputSpacing( spacing );
    //std::cout << " SPACING AFTER RESAMPLE: " << spacing[0] << " " << spacing[1] << std::endl;
    
    //Origin
//    typename TFixedImageType::PointType origin = fixedImage->GetOrigin();
    scaler->SetOutputOrigin( fixedImage->GetOrigin() );
   
    //Orientation
    typename TFixedImageType::DirectionType direction;
    direction.SetIdentity();
    direction = fixedImage->GetDirection();
    scaler->SetOutputDirection( fixedImage->GetDirection() );
        
    //Interpolation: linear interpolation
    typedef itk::LinearInterpolateImageFunction< TFixedImageType, double >  InterpolatorType;
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    scaler->SetInterpolator( interpolator );
    scaler->SetDefaultPixelValue( 0 );
    scaler->SetTransform( m_IdentityTransform );
    scaler->Update();
    
    return scaler;
}
  
template <class TFixedImageType, int VDimension> 
unsigned int
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::GetHessian(typename myHessianImageType::Pointer ImgInput, IndexType pixelIndex, bool isMax, bool isMin, int i, int j) 
{
    // (GLOBAL) HESSIAN - for threshold on curvature
    typename HessianGaussianImageFunctionType::TensorType hessian;
    hessian.Fill(0.0);
    hessian=ImgInput->GetPixel(pixelIndex);
    /*std::cout<<"Global Hessian "<<std::endl;
      std::cout<<" ,"<<hessian<<std::endl;*/
	
    /*FILE* pFileH;
      pFileH=fopen("curve_parameter.txt","a");*/	
    float Tr=0;
    float Det=1;
    float PrincipalCurve=0;
    float Minors=0;
    float Product=0;
    unsigned int Curvature=0;
//    unsigned int P=0;
//    unsigned int M=0;
//    unsigned int C=0;
//    bool maxmin=0;
//    if(isMax) maxmin=1;
//    if(isMin) maxmin=0;
	
    if(VDimension==2)
    {
	Tr=hessian[0]+hessian[2];
	Det=hessian[0]*hessian[2]-hessian[1]*hessian[1];
	Product=Tr*Det;
//        if (Product>0){P=1;}
        PrincipalCurve=(Tr*Tr)/Det; 
//        if (PrincipalCurve<m_ThresholdPrincipalCurve){C=1;} 
	Minors=Det;
//	if (Minors>0){M=1;}
    }
		
    if(VDimension==3)
    {
	Tr=hessian[0]+hessian[3]+hessian[5];
	Det=hessian[0]*hessian[3]*hessian[5]+2*hessian[1]*hessian[4]*hessian[2]-hessian[0]*hessian[4]*hessian[4]-hessian[3]*(hessian[2])*hessian[2]-hessian[5]*(hessian[1])*hessian[1];
        Product=Tr*Det;

//        if (Product>0){P=1;}
        PrincipalCurve=(Tr*Tr*Tr)/Det; 
//        if (PrincipalCurve<m_ThresholdPrincipalCurve){C=1;} 
        Minors=hessian[3]*hessian[5]-(hessian[4])*hessian[4]+hessian[5]*hessian[0]-(hessian[2])*hessian[2]+hessian[0]*hessian[3]-(hessian[1])*hessian[1];
//        if (Minors>0){M=1;}
    }
    
    if ((Product>0) && (Minors>0)&&(PrincipalCurve<m_ThresholdPrincipalCurve)) //threshold on curvature
	Curvature=1;
	 
    /*fprintf(pFileH, " %d,%d,%d, %f, %f, %f, %d\n",i,j,maxmin,Product,Minors,PrincipalCurve,Curvature);
      fclose(pFileH);*/
    /*std::cout << "P:" << Product << " M:" << Minors << " C:" << PrincipalCurve << " cond:" << Curvature << std::endl;
      std::cout << "P:" << P << " M:" << M << " C:" << C << " cond:" << Curvature << std::endl;*/
			
    return Curvature; 
   
}


// ----- PRINCIPAL CLASS
template <class TFixedImageType, int VDimension> 
typename ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>::PointSetTypePointer
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::getSiftFeatures (
    FixedImagePointer fixedImageInput, 
    bool flag_curve,
    bool normalization, 
    const char *filename_phy_max, 
    const char *filename_phy_min,
    const char *filename_im_max, 
    const char *filename_im_min,
    const char *filename_rej_contrast,
    const char *filename_rej_curvature)
{
    unsigned int numMin = 0, numMax = 0, numReject = 0;
    m_KeypointSet = PointSetType::New();
    m_PointsCount = 0;  

    // Image normalization [0,1]
    typedef itk::RescaleIntensityImageFilter< TFixedImageType, TFixedImageType > RescaleFilterType;
    typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(fixedImageInput);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(1);
 
    typename TFixedImageType::Pointer fixedImage = TFixedImageType::New();
    if (!normalization)
    {fixedImage=fixedImageInput;} //no normalization -> implies no curvature threshold
    else{fixedImage=rescaleFilter->GetOutput();} //normalization
	
    //fixedImage->DisconnectPipeline();

    /*char filename[256];
      sprintf(filename, "InputImage.mha");
      this->writeImage(fixedImage, filename);*/

    // Declare Gaussian 
    typedef itk::DiscreteGaussianImageFilter<TFixedImageType, TFixedImageType > GaussianFilterType;
    // Declare DoG 
    typedef itk::SubtractImageFilter<TFixedImageType, TFixedImageType, TFixedImageType> DifferenceFilterType;

#if USE_ORIGINAL_BROKEN_CODE
    typename TFixedImageType::Pointer	 dogImage[m_DifferenceOfGaussianImagesNumber];
#else
    std::vector<typename TFixedImageType::Pointer> dogImage(m_DifferenceOfGaussianImagesNumber);
#endif
    typename TFixedImageType::Pointer gaussianImage_ref = TFixedImageType::New();      


    // Declare Gradient
#if USE_ORIGINAL_BROKEN_CODE
    typename GradientImageType::Pointer gradImage[m_ImageScalesTestedNumber];
    typename GradientImageType::Pointer hgradImage[m_ImageScalesTestedNumber];
    FixedImagePointer gradMagImage[m_ImageScalesTestedNumber];
#else
    std::vector<typename GradientImageType::Pointer> gradImage(m_ImageScalesTestedNumber);
    std::vector<typename GradientImageType::Pointer> hgradImage(m_ImageScalesTestedNumber);
    std::vector<FixedImagePointer> gradMagImage(m_ImageScalesTestedNumber);
#endif

    float currScale = 0.5;

    // FOR EACH OCTAVE i
    for (unsigned int i = 0; i < m_ImageScalesTestedNumber; ++i) {
        std::cout << "Computing Octave Level " << i << "... (";

	typename TFixedImageType::Pointer scaleImage = TFixedImageType::New();
	typename ResampleFilterType::Pointer scaler = ResampleFilterType::New();
	typename GaussianFilterType::Pointer tmpGaussianFilter = GaussianFilterType::New();  //this is a new gaussian filter for aliasing correction
        tmpGaussianFilter->ReleaseDataFlagOn();
    
        //if(i>0 )
        //{
        //char filename[256];
	//sprintf(filename, "gauss_ref-%d.mha", i);
	//this->writeImage(gaussianImage_ref, filename);}

        if (i == 0 && !m_DoubleOriginalImage) { //NO DOUBLING && FIRST OCTAVE
            //ANTI_ALIASING FILTER: sigma=0.5
            double variance_anti = 0.5*0.5;
            tmpGaussianFilter->SetVariance(variance_anti);
            tmpGaussianFilter->SetInput( fixedImage );
            //Image Spacing-wise smoothing (physical space)
            tmpGaussianFilter->SetUseImageSpacing(true); 
            //pixel-wise smoothing (pixel space)
            //tmpGaussianFilter->SetUseImageSpacing(false); 
            try {
                tmpGaussianFilter->Update();
            }
            catch( itk::ExceptionObject & excep ) {
                std::cerr << "Exception caught !" << std::endl;
                std::cerr << excep << std::endl;
            }

            scaleImage = tmpGaussianFilter->GetOutput();
            scaleImage->DisconnectPipeline();

            //scaleImage = fixedImage;
            //gaussianImage_ref=fixedImage;
	
        } else { 
            if (i == 0 && m_DoubleOriginalImage) {// DOUBLING && FIRST OCTAVE
                // Input is the fixed Image.  
                //ANTI_ALIASING FILTER: sigma=0.5;
                //this->writeImage(fixedImage, "Input.mha");
			
                //gaussianImage_ref=fixedImage;	 
				
                //double variance = (double)m_SigmaAliasing*m_SigmaAliasing;
                double variance_anti = 0.5*0.5;
                tmpGaussianFilter->SetVariance(variance_anti);
                tmpGaussianFilter->SetInput( fixedImage );
                //Image Spacing-wise smoothing
                tmpGaussianFilter->SetUseImageSpacing(true); 
                try {
                    tmpGaussianFilter->Update();
                }
                catch( itk::ExceptionObject & excep ) {
                    std::cerr << "Exception caught !" << std::endl;
                    std::cerr << excep << std::endl;
                }

                scaleImage = tmpGaussianFilter->GetOutput();
                scaleImage->DisconnectPipeline();

                scaler = getScaleResampleFilter ( scaleImage, m_ScalingFactor );  //doubled image
			
                typename GaussianFilterType::Pointer tmpGaussianFilter = GaussianFilterType::New();  
                //now we need to filter with sigma == 1 because we doubled the size of the image
                //ANTI_ALIASING FILTER: sigma=1;
                double variance1 = 1*1;
                tmpGaussianFilter->SetVariance(variance1);
                tmpGaussianFilter->SetInput( scaler->GetOutput() );
                //Image Spacing-wise smoothing
                tmpGaussianFilter->SetUseImageSpacing(true); 

                try {
                    tmpGaussianFilter->Update();
                }
                catch( itk::ExceptionObject & excep ) {
                    std::cerr << "Exception caught !" << std::endl;
                    std::cerr << excep << std::endl;
                }

                scaleImage = tmpGaussianFilter->GetOutput();
                scaleImage->DisconnectPipeline();

                char filename[256];
                sprintf(filename, "double_image.mha");
                this->writeImage(scaleImage, filename);
	  
            } else { //DOWN_SAMPLING (by a factor of 2) FOR THE NEXT OCTAVE
                // Input is the 2*sigma smoothed image from the previous octave
                scaler = getScaleResampleFilter ( gaussianImage_ref , 1.0 / m_ScalingFactor );
                scaleImage = scaler->GetOutput();
            }
	
//            typename TFixedImageType::SpacingType spacing = scaleImage->GetSpacing();
//            typename TFixedImageType::PointType origin = scaleImage->GetOrigin();
//            typename TFixedImageType::DirectionType direction = scaleImage->GetDirection();

//std::cout << " SPACING AFTER ALIAS: " << spacing[0] << " " << spacing[1] <<" "<< spacing[2] <<std::endl;
//std::cout << " ORIGIN AFTER ALIAS: " << origin[0] << " " << origin[1] <<" "<< origin[2] <<std::endl;	
//std::cout << " DIRECTION AFTER ALIAS: " << direction <<std::endl;
        } //else

        {
            typename TFixedImageType::SizeType gsize = 
                scaleImage->GetLargestPossibleRegion().GetSize();
            for (int j = 0; j < VDimension; ++j)
                std::cout << gsize[j] << " ";
        }

        //COMPUTE GRADIENT
        std::cout << "...Computing Gradient...";
        typename GradientFilterType::Pointer tmpGradFilter = GradientFilterType::New();
        tmpGradFilter->ReleaseDataFlagOn();
        tmpGradFilter->SetInput(scaleImage);
        // Do this in physical space
        tmpGradFilter->SetUseImageSpacing(true);
        tmpGradFilter->Update();
        gradImage[i] = tmpGradFilter->GetOutput();
        hgradImage[i] = this->GetHypersphericalCoordinates(gradImage[i]);
            
        typename GradientMagFilterType::Pointer tmpGradMagFilter = GradientMagFilterType::New();
        tmpGradMagFilter->ReleaseDataFlagOn();
        tmpGradMagFilter->SetInput(scaleImage);
        // Do this in physical space
        tmpGradMagFilter->SetUseImageSpacing(true);
        tmpGradMagFilter->Update();
        gradMagImage[i] = tmpGradMagFilter->GetOutput();
        std::cout << "...Done\n";

        typename TFixedImageType::Pointer gaussianImage_old = TFixedImageType::New();
        typename TFixedImageType::Pointer gaussianImage_new = TFixedImageType::New();
        typename TFixedImageType::Pointer gaussianImage_start = TFixedImageType::New();

        // COMPUTE GAUSSIAN FOR EACH SCALE j IN EACH OCTAVE i
        for (unsigned int j = 0; j < m_GaussianImagesNumber; ++j) {
#ifdef VERBOSE
            std::cout << "Setting Up Gaussian Filter " << i << "-" << j << "...";
            std::cout.flush();
#endif
	
            if (j==0)
            {
		typename GaussianFilterType::Pointer tmpGaussianFilter = GaussianFilterType::New();
		/* Variance is square of the sigma
                 * sigma = (2^(j/s)*sigma)
                 */
		double variance = this->GetGaussianScale(j);
		std::cout<<"scale: "<<j<<" sigma: "<<variance<<std::endl;
		variance *= variance;
		tmpGaussianFilter->ReleaseDataFlagOn();
		tmpGaussianFilter->SetVariance(variance);
		tmpGaussianFilter->SetInput( scaleImage );
		// physical-wise smoothing
		tmpGaussianFilter->SetUseImageSpacing(true); 
		try {
                    tmpGaussianFilter->Update();
                }
		catch( itk::ExceptionObject & excep ) {
                    std::cerr << "Exception caught !" << std::endl;
                    std::cerr << excep << std::endl;
                }
		gaussianImage_start= tmpGaussianFilter->GetOutput();
		gaussianImage_start->DisconnectPipeline();
		/*char filename[256];
                  sprintf(filename, "gauss-%d-%d.mha", i, j);
                  this->writeImage(gaussianImage_start, filename);*/
		//tmpGaussianFilter->ReleaseDataFlagOff();
            }
		
            if(j>0)
            {	
		if(j==1)
                    gaussianImage_old=gaussianImage_start;
		
		typename GaussianFilterType::Pointer tmpGaussianFilter = GaussianFilterType::New();
		/* Variance is square of the sigma
		 * sigma = (2^(j/s)*sigma)
                 */
		double variance = this->GetGaussianScale(j);
		std::cout<<"scale: "<<j<<" sigma: "<<variance<<std::endl;
		
		variance *= variance;
		tmpGaussianFilter->ReleaseDataFlagOn();
		tmpGaussianFilter->SetVariance(variance);
		tmpGaussianFilter->SetInput( scaleImage );
		// physical-wise smoothing
		tmpGaussianFilter->SetUseImageSpacing(true); 
		try {
                    tmpGaussianFilter->Update();
                }
		catch( itk::ExceptionObject & excep ) {
                    std::cerr << "Exception caught !" << std::endl;
                    std::cerr << excep << std::endl;
                }
		gaussianImage_new = tmpGaussianFilter->GetOutput();
		gaussianImage_new->DisconnectPipeline();
		//tmpGaussianFilter->ReleaseDataFlagOff();
	
		if(j==m_DifferenceOfGaussianTestsNumber)
		{
                    gaussianImage_ref=gaussianImage_new;
		}

//		typename TFixedImageType::SpacingType spacing = gaussianImage_new->GetSpacing();
	
#ifdef VERBOSE
                std::cout << "Done\n";
                std::cout.flush();
#endif

#ifdef VERBOSE
                std::cout << "Setting Up DoG Filter " << i << "-" << j << "...";
                std::cout.flush();
#endif

		if (j<m_DifferenceOfGaussianImagesNumber+1)
		{
                    typename DifferenceFilterType::Pointer tmpDogFilter = DifferenceFilterType::New();
                    tmpDogFilter->SetInput1( gaussianImage_old );
                    tmpDogFilter->SetInput2( gaussianImage_new );
                    tmpDogFilter->Update();	
                    dogImage[j-1] = tmpDogFilter->GetOutput();
                    //tmpDogFilter->ReleaseDataFlagOff();
		
                    //typename TFixedImageType::SpacingType spacing = dogImage[j-1]->GetSpacing();
                    //std::cout << " SPACING DOG IMAGE: " << spacing[0] << " " << spacing[1] << std::endl;
	
                    /*char filename[256];
                      sprintf(filename, "dog-%d-%d.mha", i, j-1);
                      this->writeImage(dogImage[j-1], filename);*/
	
                }
	
#ifdef VERBOSE
                std::cout << "Done\n";
                std::cout.flush();
#endif 
    
                //char filename[256];
                //sprintf(filename, "gauss-%d-%d.mha", i, j);
                //this->writeImage(gaussianImage_new, filename);
	
                gaussianImage_old=gaussianImage_new; //update
    
            } //if (j>=0)
    
        } //for gaussian

	// SEARCH MAXIMA & MINIMA EXTREMA IN ADJACENT DOGs
        for (unsigned int j=1; j < (m_DifferenceOfGaussianImagesNumber - 1); ++j) {
            // Search the dogImages for local maxima,  w.r.t. corresponding point in the scale above and below
            // Only use the middle dogs (ones with both neighbours above and below)
            // Iterate over each position in the dog filter
            typedef itk::ImageRegionIteratorWithIndex< TFixedImageType > ImageIteratorType;

            IndexType regionStart;
            // Avoid the edges
            for (int k=0; k < VDimension; ++k)
                regionStart[k] = 1;

            typename TFixedImageType::SizeType regionSize = 
                dogImage[j]->GetLargestPossibleRegion().GetSize();
	  
            //HESSIAN OF CURRENT DOG
            typedef itk::HessianRecursiveGaussianImageFilter< TFixedImageType >  myFilterType;
            typedef typename myFilterType::OutputImageType myHessianImageType;
            typename myHessianImageType::Pointer HessianImage = myHessianImageType::New();
            typename myFilterType::Pointer filter_hess = myFilterType::New();
            filter_hess->SetInput( dogImage[j] ); 
            filter_hess->SetSigma( 2.0 ); 
            filter_hess->Update();
            HessianImage = filter_hess->GetOutput();
            HessianImage->DisconnectPipeline();
            filter_hess->ReleaseDataFlagOn();  
    
#ifdef VERBOSE
            std::cout << "Searching for Extrema in DoG Image " << i << "-" << j;
            std::cout << " ( ";
            for (int k=0; k < VDimension; ++k)
                std::cout << regionSize[k] << " ";
            std::cout << ") Scale " << currScale << "\n";
            std::cout.flush();
#endif

            // Avoid far edge
            for (int k=0; k < VDimension; ++k)
                regionSize[k] -=  2;
	  
            typename TFixedImageType::RegionType itregion;
            itregion.SetIndex(regionStart);
            itregion.SetSize(regionSize);
	      
            ImageIteratorType pixelIt(dogImage[j],itregion);
      
            // this iterates on the pixels of dogImage[j]
            for ( pixelIt.GoToBegin(); !pixelIt.IsAtEnd(); ++pixelIt) {
                // Make sure to start sufficiently into the image so that all neighbours are present
                IndexType pixelIndex = pixelIt.GetIndex();
                typename TFixedImageType::PixelType pixelValue = pixelIt.Get();
                PointType point;
                //typename TFixedImageType::SpacingType spacing;
                //typename TFixedImageType::OriginType origin;
                PointType vertex;
                dogImage[j]->TransformIndexToPhysicalPoint (pixelIndex, point); 
		  
                // Compare to the 8 immediate neighbours in the current DOG 
                bool isMax=true;
                bool isMin=true;

                this->CheckLocalExtrema(dogImage[j], 
                    pixelIndex, pixelValue, isMax, isMin, false);

                if (!isMax && !isMin) continue;
	  
                // Compare to scale above
                if (j < (m_GaussianImagesNumber-1)) {
                    this->CheckLocalExtrema(dogImage[j+1], 
                        pixelIndex, pixelValue, isMax, isMin, true);
                }
                if (!isMax && !isMin) continue;

                // Compare to scale below
                if (j > 0) {
                    this->CheckLocalExtrema(dogImage[j-1], 
                        pixelIndex, pixelValue, isMax, isMin, true);
		}
	
                /*if(isMax) 
                  std::cout<< "MAX:" <<std::endl;
                  if(isMin)
                  std:: cout<< "min:" <<std::endl;*/
	  
                if (!isMax && !isMin) continue;
	   
                // THRESHOLDING ON IMAGE CONTRAST: Check if it is sufficiently large (absolute value)
	  
                //FILE* pFileRejectContrast;
                if (fabs(pixelValue) < m_MinKeypointValue) {
                    //if (fabs(pixelValue) > 0.3 || abs(pixelValue) < 0.005) {
                    //std::cout<< "module: "<<fabs(pixelValue)<<std::endl;
                    ++numReject;
                    //pFileRejectContrast=fopen(filename_rej_contrast,"a");
                    //// we work in LPS, now we pass in RAS system, in order to load coordinates in Slicer3D (module Fiducial)
                    //point[0]=-1.0*point[0];
                    //point[1]=-1.0*point[1];
                    //if(isMax){
                    //	fprintf(pFileRejectContrast,"M");
                    //	}
                    //if(isMin){
                    //	fprintf(pFileRejectContrast,"m");
                    //	}
                    //fprintf(pFileRejectContrast, "-%d-%d-%d,",numReject,i,j);
                    //for(int k=0; k<VDimension; k++)
                    // 	  {
                    // 		fprintf(pFileRejectContrast,"%.3f, ",point[k]);
                    // 		} 	  
                    // 		fprintf(pFileRejectContrast,"\n");
                    // 		fclose(pFileRejectContrast);
                    continue;
                }
	  
	  
                // THRESHOLDING ON IMAGE CURVATURE
    
                if (flag_curve){
                    //FILE* pFileRejectCurvature;
                    unsigned int Curvature=0;
                    Curvature=this->GetHessian(HessianImage, pixelIndex, isMax, isMin, i, j);
                    if ( Curvature==0){
                        ++numReject;
                        /*point[0]=-1.0*point[0];
                          point[1]=-1.0*point[1];
                          pFileRejectCurvature=fopen(filename_rej_curvature,"a");
				
                          if(isMax){
                          fprintf(pFileRejectCurvature,"M");
                          }
                          if(isMin){
                          fprintf(pFileRejectCurvature,"m");
                          }	
                          fprintf(pFileRejectCurvature, "-%d-%d-%d,",numReject,i,j);
                          for(int k=0; k<VDimension; k++)
                          {
                          fprintf(pFileRejectCurvature,"%.3f, ",point[k]);
                          } 	  
                          fprintf(pFileRejectCurvature,"\n");
                          fclose(pFileRejectCurvature);*/
                        continue;
                    }
                }
  
                // Passed all checks:
	 
                m_KeypointSet->SetPoint( m_PointsCount, point);
                m_KeypointSet->SetPointData( m_PointsCount, this->GetFeatures( gaussianImage_start, 
                        hgradImage[i], point,
                        this->GetGaussianScale(j)));
                ++m_PointsCount;

                if (isMax) {
                    // Maxima detected.  
                    ++numMax;
                    //std::cout << "max phys coord: "<< point << std::endl;
                    //std::cout << "max image coord: "<< pixelIndex << std::endl;
	     
                    if (filename_phy_max && filename_phy_max[0]) {
                        FILE* pFile;
                        pFile=fopen(filename_phy_max,"a");
                        //FILE* pFile1;
                        //pFile1=fopen(filename_im_max,"a");
                        //physical coordinates
                        point[0]=-1.0*point[0];
                        point[1]=-1.0*point[1];	
                        fprintf(pFile, "M-%d-%d-%d,", numMax,i,j);
                        for(int k=0; k<VDimension; k++)
                        {	  		
                            fprintf(pFile,"%.3f, ",point[k]);
                        } 	  
                        fprintf(pFile,"\n");
                        fclose(pFile);
                    }
	  		
                    //image coordinates
                    /*point[0]=-1.0*point[0];
                      point[1]=-1.0*point[1];
	  		 	  
                      fprintf(pFile1, "%d,", numMax);
                      for(int k=0; k<VDimension; k++)
                      {
                      vertex[k]= (point[k]-fixedImage->GetOrigin()[k])/fixedImage->GetSpacing()[k];
                      fprintf(pFile1,"%.3f, ",vertex[k]);
                      }
                      fprintf(pFile1,"\n");
                      fclose(pFile1);*/
                }
	 
                if (isMin) {
                    // Minima detected.  
                    ++numMin;
                    //std::cout << "min phys coord: "<< point << std::endl;
                    //std::cout << "min image coord: "<< pixelIndex << std::endl;

                    if (filename_phy_min && filename_phy_min[0]) {
                        FILE* pFile;
                        pFile=fopen(filename_phy_min,"a");
                        //pFile1=fopen(filename_im_min,"a");
                        //physical coordinates
                        point[0]=-1.0*point[0];
                        point[1]=-1.0*point[1];	
                        fprintf(pFile, "m-%d-%d-%d,", numMin,i,j);
                        for(int k=0; k<VDimension; k++)
                        {
                            fprintf(pFile,"%.3f, ",point[k]);
                        } 	  
                        fprintf(pFile,"\n");
                        fclose(pFile);
                    }
                    //image coordinates
                    /*point[0]=-1.0*point[0];
                      point[1]=-1.0*point[1];
	  		  		
                      fprintf(pFile1, "%d,", numMin);
                      for(int k=0; k<VDimension; k++)
                      {
                      vertex[k]= (point[k]-fixedImage->GetOrigin()[k])/fixedImage->GetSpacing()[k];
                      fprintf(pFile1,"%.3f, ",vertex[k]);
                      }
                      fprintf(pFile1,"\n");
                      fclose(pFile1);*/
                }
                //std::cout << "current scale: "<< currScale << std::endl;
            }
#ifdef VERBOSE
            std::cout << "Acc. Num Max: " << numMax 
                << "\nAcc. Num Min: " << numMin 
                << "\nAcc. Num Reject: " << numReject 
                << std::endl;
            std::cout.flush();
#endif
        }
        currScale *= m_ScalingFactor;
    }
	
#ifdef VERBOSE
    std::cout << "Total Num Max: " << numMax 
        << "\nTotal Num Min: " << numMin 
        << "\nTotal Num Reject: " << numReject
        << std::endl;	
    std::cout.flush();
#endif
	
    return m_KeypointSet;
} 

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::save_pointset (const char* filename)
{
    FILE *fp = fopen (filename, "w");
    unsigned long num_points = m_KeypointSet->GetNumberOfPoints();
    for (unsigned int i = 0; i < num_points; ++i) {
        PointType point;
        point.Fill(0.0);
        m_KeypointSet->GetPoint(i, &point);
        fprintf (fp, "p-%03d,%f,%f,%f\n", i, - point[0], - point[1], point[2]);
//        FeatureType ft2;
//        m_KeypointSet->GetPointData(i, &ft2);
//        std::cout << ft2 << "\n";
    }
    fclose (fp);
}

template <class TFixedImageType, int VDimension> 
void
ScaleInvariantFeatureImageFilter<TFixedImageType,VDimension>
::MatchKeypointsFeatures(
    PointSetTypePointer keypoints1, PointSetTypePointer keypoints2,
    const char *filename_phy_match1, const char *filename_phy_match2,
    float max_feature_distance_ratio)
{
    // Compare Keypoints.  Check Coverage and Accuracy
    // This does the comparison based on position of the keypoints
    // Find:  
    // # of points that match which will tell us
    // # of points that did not scale
    // # of points created by the scale

    FILE* fp1 = 0;
    FILE* fp2 = 0;
//    unsigned int numMatches;
    unsigned int numMatchesTried;
//    unsigned int numMatches2;
//    unsigned int numMatches5;
    typename PointSetType::PointsContainer::Pointer keyps1, keyps2;

    /* Open files, we will stream into them */
    if (filename_phy_match1 && filename_phy_match1[0]) {
        fp1 = fopen (filename_phy_match1, "w");
    }
    if (filename_phy_match2 && filename_phy_match2[0]) {
        fp2 = fopen (filename_phy_match2, "w");
    }

    unsigned long numpoints1, numpoints2;
    numpoints1 = keypoints1->GetNumberOfPoints();
    std::cout << "Keypoints1 Found: " << numpoints1 << std::endl;
    numpoints2 = keypoints2->GetNumberOfPoints();
    std::cout << "Keypoints2 Found: " << numpoints2 << std::endl;

    std::cout << "***Keypoint Matches***\n";
	
//    numMatches = 0;
//    numMatches2 = 0;
//    numMatches5 = 0;
    numMatchesTried = 0;
    for (unsigned int i = 0; i < numpoints2; ++i) {
        PointType pp2;
        pp2.Fill(0.0);
        keypoints2->GetPoint(i, &pp2);	
        FeatureType ft2;
        keypoints2->GetPointData(i, &ft2);	
	
        FeatureType bestft;
        float bestdist = -1.0;
        float nextbestdist = -1.0;
        unsigned int bestj=0;
        for (unsigned int j = 0; j < numpoints1; ++j) {
            PointType pp;
            keypoints1->GetPoint(j, &pp);
            FeatureType ft;
            keypoints1->GetPointData(j, &ft);	

            /* Compute feature distance */
            float dist = 0.0;
            for (unsigned int k = 0; k < ft.Size(); ++k)
            {
		dist += (ft[k] - ft2[k])*(ft[k] - ft2[k]);
            }

            //std::cout<<"dist: "<<dist<<std::endl;
	  
            if (nextbestdist < 0.0 || dist < bestdist)
            {
                nextbestdist = bestdist;
                bestdist=dist;
                bestft = ft;
                bestj = j;
            }	 

            //std::cout << "bestdist= "<<bestdist<<std::endl;
            //std::cout << "nextbestdist= "<<nextbestdist<<std::endl;
        }

        /* Reject "too close" matches */
//        if ((bestdist / nextbestdist) >  m_MaxFeatureDistanceRatio)
        if ((bestdist / nextbestdist) > max_feature_distance_ratio)
        {
            //std::cout << "MATCH REJECTED 1:" << std::endl;
            //PointType pp;
            //keypoints1->GetPoint(bestj, &pp);
            //std::cout << pp << std::endl;

            //matches rejected on Image1:
            /*FILE* pFileMatch1_rej;
              FILE* pFileMatch2_rej;
              pFileMatch1_rej=fopen("point_match1_rej.fcsv","a");
              pp[0]=-1.0*pp[0];
              pp[1]=-1.0*pp[1];
              fprintf(pFileMatch1_rej, "p1-%d,",bestj);
              for(int k=0; k<VDimension; k++)
              {
              fprintf(pFileMatch1_rej,"%.3f, ",pp[k]);
              } 	  
              fprintf(pFileMatch1_rej,"\n");
              fclose(pFileMatch1_rej);*/


            ////std::cout << "MATCH REJECTED 2:" << std::endl;
            ////std::cout << pp2 << std::endl;
		
            //matches rejected on Image2:
            /*pFileMatch2_rej=fopen("point_match2_rej.fcsv","a");
              pp2[0]=-1.0*pp2[0];
              pp2[1]=-1.0*pp2[1];
              fprintf(pFileMatch2_rej, "p2-%d,",bestj);
              for(int k=0; k<VDimension; k++)
              {
              fprintf(pFileMatch2_rej,"%.3f, ",pp2[k]);
              } 	  
              fprintf(pFileMatch2_rej,"\n");
              fclose(pFileMatch2_rej);*/

            ////std::cout << "MATCH REJECTED:" << std::endl;
            ////std::cout << pp << " => " << pp2 << std::endl;

            continue;
        }	

        /* NEW IDEA: Bi-directional mapping -- look to make sure it is a 
           reciprocal best match */
        /* Take the best feature found,  see if pp2 makes the cut */
        bestdist = -1.0;
        nextbestdist = -1.0;
        FeatureType bestft2;
//        unsigned int bestj2 = -1;

        for (unsigned int j = 0; j < numpoints2; ++j) {
            PointType pp;
            keypoints2->GetPoint(j, &pp);
            FeatureType ft;
            keypoints2->GetPointData(j, &ft);	

            float dist = 0.0;
            for (unsigned int k = 0; k < ft.Size(); ++k)
                dist += (ft[k] - bestft[k])*(ft[k] - bestft[k]);
	  
            if (nextbestdist < 0.0 || dist < bestdist)
            {
                nextbestdist = bestdist;
                bestdist=dist;
                bestft2 = ft;
//                bestj2 = j;
            }
        }

        /* Reject if not reciprocal best hit or "too close" matches */
//        if ( bestft2 != ft2 || ((bestdist / nextbestdist) >  m_MaxFeatureDistanceRatio))
        if (bestft2 != ft2 
            || (bestdist / nextbestdist) > max_feature_distance_ratio)
        {
            continue;
        }
        /* END NEW IDEA */

        ++numMatchesTried;

        // Check goodness of best feature
        PointType tmpp, pp;
        tmpp.Fill (0.0);
        pp.Fill (0.0);
        keypoints1->GetPoint(bestj, &tmpp);

        // Print the match
        std::cout << tmpp << " => " << pp2 << std::endl;

        if (fp1) {
            fprintf (fp1, "p1-%d,%.3f,%.3f,%.3f\n",
                bestj, - tmpp[0], - tmpp[1], tmpp[2]);
        }
        
        if (fp2) {
            fprintf (fp2, "p1-%d,%.3f,%.3f,%.3f\n",
                bestj, - pp2[0], - pp2[1], pp2[2]);
        }
    }

    std::cout << "\n***Features Matches: " << numMatchesTried << std::endl;
    if (fp1) {
        fclose (fp1);
    }
    if (fp2) {
        fclose (fp2);
    }
}

} // end namespace itk

#endif
