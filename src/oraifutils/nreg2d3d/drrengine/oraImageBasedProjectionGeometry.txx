//
#ifndef ORAIMAGEBASEDPROJECTIONGEOMETRY_TXX_
#define ORAIMAGEBASEDPROJECTIONGEOMETRY_TXX_

#include "oraImageBasedProjectionGeometry.h"

namespace ora
{

template<class TPixelType>
ImageBasedProjectionGeometry<TPixelType>::ImageBasedProjectionGeometry()
	: Superclass()
{
  ;
}

template<class TPixelType>
ImageBasedProjectionGeometry<TPixelType>::~ImageBasedProjectionGeometry()
{
  ;
}

template<class TPixelType>
void ImageBasedProjectionGeometry<TPixelType>::PrintSelf(std::ostream& os,
		itk::Indent indent) const
{
	Superclass::PrintSelf(os, indent);
	indent = indent.GetNextIndent();
	// nothing to declare ...
}

template<class TPixelType>
bool ImageBasedProjectionGeometry<TPixelType>::ExtractDetectorGeometryFromImageAndRegion(
		ImagePointer image, const RegionType &region)
{
	if (!image)
		return false;
	if (region.GetSize()[0] <= 0 ||
			region.GetSize()[1] <= 0 ||
			region.GetSize()[2] != 1 ||
			region.GetIndex()[2] != 0)
		return false;

	typedef typename ImageType::SpacingType SpacingType;
	typedef typename ImageType::SizeType SizeType;
	typedef typename ImageType::PointType PointType;
	typedef typename ImageType::DirectionType DirectionType;

	// - detector spacing
	SpacingType s = image->GetSpacing();
	double ds[2];
	ds[0] = s[0];
	ds[1] = s[1];
	this->SetDetectorPixelSpacing(ds);
	// - detector size
  SizeType sz = region.GetSize();
  int isz[2];
  isz[0] = sz[0];
  isz[1] = sz[1];
  this->SetDetectorSize(isz);
  // - detector orientation
  DirectionType dir = image->GetDirection();
  double v1[3];
  v1[0] = dir[0][0]; // column-based row-direction
  v1[1] = dir[1][0];
  v1[2] = dir[2][0];
  this->SetDetectorRowOrientation(v1);
  double v2[3];
  v2[0] = dir[0][1]; // column-based column-direction
  v2[1] = dir[1][1];
  v2[2] = dir[2][1];
  this->SetDetectorColumnOrientation(v2);
  // - detector origin
  PointType orig = image->GetOrigin();
  double dor[3];
  for (int d = 0; d < 3; d++)
  	dor[d] = orig[d] + static_cast<double>(region.GetIndex(0)) * ds[0] * v1[d] +
  		static_cast<double>(region.GetIndex(1)) * ds[1] * v2[d];
  this->SetDetectorOrigin(dor);

	return true;
}

template<class TPixelType>
bool ImageBasedProjectionGeometry<TPixelType>::ExtractDetectorGeometryFromImage(
		ImagePointer image)
{
	if (!image)
		return false;

	return ExtractDetectorGeometryFromImageAndRegion(image,
			image->GetLargestPossibleRegion());
}

template<class TPixelType>
bool ImageBasedProjectionGeometry<TPixelType>::ExtractDetectorGeometryFromImageAndAssumeSourcePosition(
		ImagePointer image, double cx, double cy, double SFD, bool inverse)
{
	return ExtractDetectorGeometryFromImageAndRegionAndAssumeSourcePosition(image,
			image->GetLargestPossibleRegion(), cx, cy, SFD, inverse);
}

template<class TPixelType>
bool ImageBasedProjectionGeometry<TPixelType>::ExtractDetectorGeometryFromImageAndRegionAndAssumeSourcePosition(
		ImagePointer image, const RegionType &region, double cx, double cy,
		double SFD, bool inverse)
{
	bool b = ExtractDetectorGeometryFromImageAndRegion(image, region);
	if (b)
	{
	  double d = 1.0;
	  if (inverse)
	    d = -1;
	  const double *r = this->GetDetectorRowOrientation(); // normalized
	  const double *c = this->GetDetectorColumnOrientation(); // normalized
		double n[3]; // normalized
	  n[0] = (r[1] * c[2] - r[2] * c[1]) * d;
	  n[1] = (r[2] * c[0] - r[0] * c[2]) * d;
	  n[2] = (r[0] * c[1] - r[1] * c[0]) * d;
	  double S[3]; // compute source position
	  typename ImageType::PointType O = image->GetOrigin();
	  S[0] = O[0] + r[0] * cx + c[0] * cy + n[0] * SFD;
	  S[1] = O[1] + r[1] * cx + c[1] * cy + n[1] * SFD;
	  S[2] = O[2] + r[2] * cx + c[2] * cy + n[2] * SFD;
	  this->SetSourcePosition(S);
	  b = this->IsGeometryValid(); // validate as well
	}
	return b;
}

}

#endif /* ORAIMAGEBASEDPROJECTIONGEOMETRY_TXX_ */
