/*=========================================================================
Program: ITK nSIFT Implemention (Header)
Module: $RCSfile: itkScaleInvariantFeatureImageFilter.h,v $
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

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifndef __itkScaleInvariantFeatureImageFilter_h
#define __itkScaleInvariantFeatureImageFilter_h

#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkGaussianImageSource.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkPointSet.h"
#include "itkVector.h"
#include "itkDiscreteHessianGaussianImageFunction.h"
#include <itkHessianRecursiveGaussianImageFilter.h> 
#include "itkRescaleIntensityImageFilter.h"

#include <cstdio>


namespace itk
{  

/** \class ScaleInvariantFeatureImageFilter
 * \brief Generate and match scale invariant features from an image input.
 *
 * This class takes the input image and locates scale invariant features in
 * the image.  The result is a set of keypoints, each with an associated
 * vector of values (the corresponding feature).  Two sets of keypoints
 * can then be matched, to determine which points in the sets are likely
 * to correspond, by comparing feature vectors.
 *
 */

template <class TFixedImageType, int VDimension=3> 
class ITK_EXPORT ScaleInvariantFeatureImageFilter
{
public:

    /** typedefs to facilitate access to the image-related types */
    typedef typename TFixedImageType::PixelType PixelType;
    typedef typename TFixedImageType::Pointer  FixedImagePointer;
    typedef typename TFixedImageType::IndexType  IndexType;
    typedef typename TFixedImageType::SizeType::SizeValueType SizeValueType;

    /** used for validating results from synthetic images */
    typedef typename itk::AffineTransform< double,VDimension> TransformType;

    /** multidimensional histogram for the features */
    typedef Array< float > FeatureType;

    /** keypoints, storing the associated feature as well */
    typedef PointSet< FeatureType, VDimension,
                      DefaultStaticMeshTraits< FeatureType, VDimension, VDimension, double > > PointSetType;
    typedef typename PointSetType::PointType PointType;
    typedef typename PointSetType::Pointer PointSetTypePointer;

    /** Filters for scaling and resampling images for the multiple scales */
    typedef ResampleImageFilter< TFixedImageType, TFixedImageType > ResampleFilterType;

    /** Constructor to set default values */
    ScaleInvariantFeatureImageFilter();

    /** Writes an image to disk */
    void writeImage(FixedImagePointer fixedImage, const char *filename);

    /** Create a filter that resamples the image (scale up or down)  */
    typename ResampleFilterType::Pointer getScaleResampleFilter ( typename TFixedImageType::Pointer fixedImage, float scale );

    /** Set Parameters */
    void SetDoubling (bool tmp);
    void SetNumScales ( unsigned int tmp);
    void SetMatchRatio ( float tmp);
    void SetInitialSigma(float InitialSigma);
    void SetDescriptorDimension (float descriptor_dim);
    void SetContrast (float contrast);
    void SetCurvature (float curvature);

    /** Generate and return the scale invariant keypoints and features */
    PointSetTypePointer getSiftFeatures(FixedImagePointer fixedImageInput, bool flag_curve, bool normalization, const char *filename_phy_max, const char *filename_phy_min, const char *filename_im_max, const char *filename_im_min,const char *filename_rej_contrast,const char *filename_rej_curvature);

    /* Save the keypoints to a file */
    void save_pointset (const char* filename);

    /** Match keypoints based on similarity of features. Matches are printed
     *  to standard out.  Supply the inverse transform to get performance 
     *  measures.*/
    static void MatchKeypointsFeatures(
        PointSetTypePointer keypoints1, PointSetTypePointer keypoints2,
        const char *filename_phy_match1, const char *filename_phy_match2,
        float max_feature_distance_ratio);

private:

    /** Filter to obtain gradient directions and magnitudes */
    typedef itk::GradientImageFilter<TFixedImageType> GradientFilterType;
    typedef typename GradientFilterType::OutputImageType GradientImageType;
    typedef itk::GradientMagnitudeImageFilter<TFixedImageType, TFixedImageType> GradientMagFilterType;

    /** Shorthand for an identity transform */
    typedef IdentityTransform< double, VDimension >  IdentityTransformType;
    typename IdentityTransformType::Pointer m_IdentityTransform;

    /** Scale up the original image by a factor of 2 before processing */
    bool m_DoubleOriginalImage;

    /** Number of image pyramid levels (octaves) to test */
    unsigned int    m_ImageScalesTestedNumber;

    /** Factor by which the images are scaled between pyramid levels */
    float  m_ScalingFactor;

    /** The range of Gaussian sigma that will be tested */
    float m_GaussianSigma; 

    /** Number of Gaussian Images that will be used to sample the range of 
     * sigma values ... */
    unsigned int    m_GaussianImagesNumber;

    /** ... determining the number of difference of gaussian images ... */
    unsigned int    m_DifferenceOfGaussianImagesNumber;

    /** ... determining the number of sets of images we can test for 
     * feature points */
    unsigned int m_DifferenceOfGaussianTestsNumber;

    /** Threshold on image contrast. Minimum voxel intensity for a feature point */
    float m_MinKeypointValue;

    /** Threshold on image curvature */
    float m_ThresholdPrincipalCurve;

    /** When looking for difference of Gaussian extrema in the images, 
     * consider a voxel to be extremal even if there are voxels 
     * that are this much more extreme.*/
    float m_ErrorThreshold;

    /** When matching points, reject a match if the ratio: 
     * \frac{\textrm{distance to best match}}{\textrm{distance to second-best match}}
     * is greater than this value*/
    float m_MaxFeatureDistanceRatio;

    PointSetTypePointer m_KeypointSet;
    long m_PointsCount;

    /** Distance from the centre to the edge of the hypercube
     * summarised by the nSIFT feature*/
    unsigned int m_SIFTHalfWidth;

    /** The hypercube edge length (in voxels) of the subregion summarised
     * by each multidimensional histogram of the nSIFT feature*/
    unsigned int m_SIFTSubfeatureWidth;

    /** Number of bin used to summarise the hyperspherical coordinates
     * of the voxels in each subregion*/
    unsigned int m_SIFTSubfeatureBins;
    
    /** The sigma for the Gaussian filtering for the j-th image on a level
     * of the image pyramid*/
    double GetGaussianScale( int j ); 

    /** Returns an image where all gradients are converted to hyperspherical
     * coordinates*/
    typename GradientImageType::Pointer GetHypersphericalCoordinates(typename GradientImageType::Pointer inputImg);

    /** Returns the value of the principal curvature - thresholding on image curvature */
    typedef itk::DiscreteHessianGaussianImageFunction<  TFixedImageType, PixelType > HessianGaussianImageFunctionType;
    typedef itk::HessianRecursiveGaussianImageFilter< TFixedImageType >  myFilterType;
    typedef typename myFilterType::OutputImageType myHessianImageType;
    unsigned int GetHessian(typename myHessianImageType::Pointer ImgInput, IndexType pixelIndex, bool isMax, bool isMin, int i, int j);

    /** Size of the nSIFT feature */
    unsigned int SiftFeatureSize();

    /** Used when generating nSIFT features to figure out what portion
     * of the nSIFT feature vector the current summary histogram starts at*/ 
    unsigned int DeltaToSiftIndex (int delta[]);

    /** Generate nSIFT feature for pixel at pixelIndex */
    FeatureType GetSiftKey(typename GradientImageType::Pointer inputImg,
        FixedImagePointer multImg,
        IndexType pixelIndex);

    /** Determine whether voxel at pixelIndex with intensity
     * pixelvalue is the local max/min*/
    void CheckLocalExtrema(FixedImagePointer image, IndexType pixelIndex,
        PixelType pixelValue,
        bool &isMax, bool &isMin,
        bool checkCentre);

    /** Generate the nSIFT feature at a point*/
    FeatureType GetFeatures( FixedImagePointer fixedImage, 
        typename GradientImageType::Pointer hgradImage,
        PointType &point, float currScale);
};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkScaleInvariantFeatureImageFilter.hxx"
#endif

#endif /* SIFTKEY_H */
