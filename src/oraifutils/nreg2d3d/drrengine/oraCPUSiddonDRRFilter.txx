//
#ifndef ORACPUSIDDONDRRFILTER_TXX_
#define ORACPUSIDDONDRRFILTER_TXX_

#include "oraCPUSiddonDRRFilter.h"
//STL
#include <math.h>
//internal
#include "oraProjectionGeometry.h"
//ITK
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

namespace ora
{

template<class TInputPixelType, class TOutputPixelType>
const double CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::EPSILON =
		0.00001;

template<class TInputPixelType, class TOutputPixelType>
CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::CPUSiddonDRRFilter()
	: Superclass()
{
	this->m_LastCPUPreProcessingTime = -1;
	this->m_LastCPUPostProcessingTime = -1;
	this->m_LastCPUPureProcessingTime = -1;
	this->m_PreProcessingClock = ClockType::New();
	this->m_PostProcessingClock = ClockType::New();
	this->m_PureProcessingClock = ClockType::New();
	this->m_GeometryTransform = TransformType::New();
	this->m_CurrentGeometry = GeometryType::New();
	this->m_MappedInput = NULL;
	this->m_LastITFCalculationTimeStamp = 0;
	this->m_OffTheFlyITFMapping = false;
	this->m_LastInputVolumeTimeStamp = 0;
}

template<class TInputPixelType, class TOutputPixelType>
CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::~CPUSiddonDRRFilter()
{
	this->SetInput(NULL);
	this->m_PreProcessingClock = NULL;
	this->m_PostProcessingClock = NULL;
	this->m_PureProcessingClock = NULL;
	this->m_CurrentGeometry = NULL;
	this->m_MappedInput = NULL;
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::GenerateOutputInformation()
{
	if (this->m_NumberOfIndependentOutputs <= 0)
		return;
	this->UpdateCurrentImagingGeometry(); // update current geometry
	 // fix image size and further props with real (not temp.) projection
	this->EnsureCurrentImageIsCorrect(
	    this->GetProjectionGeometry(this->m_CurrentDRROutputIndex));
	// needs to be checked to be sure that currentGeomentry is valid
	// if there is no geometry for the current output, the currentGeometry is not valid
	if (!this->DRRCanBeComputed())
	{
	  itkExceptionMacro(<< "ERROR: " << this->GetNameOfClass()
              << " (DRR cannot be computed. Verify that a valid input is set and a ProjectionGeometry"
		 " for the current output exists.)")
	}
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::SetInput(
		InputImagePointer input)
{
	this->Superclass::SetInput(input);
	this->m_LastInputVolumeTimeStamp = this->GetMTime();
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::ThreadedGenerateData(
		const OutputImageRegionType& outputRegionForThread, int threadId)
{
	// NOTE: We use index-based point computation here in order to increase
	// performance; therefore, we assume the volume to the correctly ordered!
	// Index-based point computation is simply faster than using ITK iterators!

	// current output image and thread region:
	OutputImagePointer output = this->GetOutput(this->m_CurrentDRROutputIndex);
	OutputImagePixelType *drr = output->GetBufferPointer();
	typename OutputImageType::IndexType start = outputRegionForThread.GetIndex();
	typename OutputImageType::SizeType size = outputRegionForThread.GetSize();
	unsigned int xmax = start[0] + size[0];
	unsigned int ymax = start[1] + size[1];

	//set the right input
	MappedVolumePixelType *mappedVolume = NULL;
	if (this->m_OffTheFlyITFMapping && this->m_ITF)
		mappedVolume = this->m_MappedInput->GetBufferPointer();

	// current image mask
	MaskPixelType *mask = NULL;
	MaskImagePointer maskImage = this->GetDRRMask(
			(unsigned int) this->m_CurrentDRROutputIndex);
	if (maskImage)
		mask = maskImage->GetBufferPointer();

	//current intensity transfer function
	ITFPointer itf = this->m_ITF;

	// detector size (2D):
	const int *detectorSize = this->m_CurrentGeometry->GetDetectorSize();
	// detector spacing (2D):
	const double *ds = this->m_CurrentGeometry->GetDetectorPixelSpacing();

	// go through all DRR pixels of current thread output region:
	int idxy, idx; // linearized DRR pixel indices
	int dlinepitch = static_cast<int> (detectorSize[0]);

	PointType R;
	R[0] = this->m_CurrentGeometry->GetDetectorOrigin()[0];
	R[1] = this->m_CurrentGeometry->GetDetectorOrigin()[1];
	R[2] = this->m_CurrentGeometry->GetDetectorOrigin()[2];

	VectorType v1;
	v1[0] = this->m_CurrentGeometry->GetDetectorRowOrientation()[0];
	v1[1] = this->m_CurrentGeometry->GetDetectorRowOrientation()[1];
	v1[2] = this->m_CurrentGeometry->GetDetectorRowOrientation()[2];

	VectorType v2;
	v2[0] = this->m_CurrentGeometry->GetDetectorColumnOrientation()[0];
	v2[1] = this->m_CurrentGeometry->GetDetectorColumnOrientation()[1];
	v2[2] = this->m_CurrentGeometry->GetDetectorColumnOrientation()[2];

	PointType S;
	S[0] = this->m_CurrentGeometry->GetSourcePosition()[0];
	S[1] = this->m_CurrentGeometry->GetSourcePosition()[1];
	S[2] = this->m_CurrentGeometry->GetSourcePosition()[2];

	PointType Xh; // output pixel position (WCS) due to y index
	PointType X; // overall output pixel position (WCS)
	VectorType v_SX; // direction from S to X
	double d_conv; // effective ray length
	OutputImagePixelType izero = static_cast<OutputImagePixelType> (0); // 0-intensity
	typename InputImageType::SizeType N =
			this->m_Input->GetLargestPossibleRegion().GetSize();
	int vlinepitch = static_cast<int> (N[0]);
	int vslicepitch = vlinepitch * static_cast<int> (N[1]);

	typename InputImageType::SpacingType s = this->m_Input->GetSpacing();
	PointType O = this->m_Input->GetOrigin();

	// NOTE: For the computations below, we need the position of the corner of
	// the first voxel, not its center! However, GetOrigin() normally returns
	// the center position of the first voxel, therefore, we have to correct it.
	typename InputImageType::DirectionType vdir = this->m_Input->GetDirection();
	VectorType vv;
	vv[0] = vdir[0][0]; // volume row direction
	vv[1] = vdir[1][0];
	vv[2] = vdir[2][0];
	O = O - vv * s[0] / 2.;
	vv[0] = vdir[0][1]; // volume column direction
	vv[1] = vdir[1][1];
	vv[2] = vdir[2][1];
	O = O - vv * s[1] / 2.;
	vv[0] = vdir[0][2]; // volume slicing direction
	vv[1] = vdir[1][2];
	vv[2] = vdir[2][2];
	O = O - vv * s[2] / 2.;
	InputImagePixelType *volume = this->m_Input->GetBufferPointer();
	double alpha_x_0, alpha_y_0, alpha_z_0; // parametric plane crossings
	double alpha_x_Nx, alpha_y_Ny, alpha_z_Nz;
	double alpha_x_min, alpha_y_min, alpha_z_min;
	double alpha_x_max, alpha_y_max, alpha_z_max;
	double alpha_min, alpha_max; // parametric ray-volume entry / exit coordinates
	double d12 = 0.; // effective intensity sum along the casted ray
	int i_0, j_0, k_0; // basic indices along x-/y-/z-directions
	double alpha_c; // helper variables for main loop
	int i, j, k, i_u, j_u, k_u;
	double alpha_x_u, alpha_y_u, alpha_z_u;
	double l_i_j_k;
	double alpha_x, alpha_y, alpha_z;
	int vidx;
	double ivol;
	for (unsigned int y = start[1]; y < ymax; y++)
	{
		idxy = y * dlinepitch; // base offset
		Xh = R + v2 * static_cast<double> (y) * ds[1];
		for (unsigned int x = start[0]; x < xmax; x++)
		{
			idx = idxy + x; // final linearized pixel index

			// check whether this pixel is masked
			if (mask && !mask[idx])
			{
				drr[idx] = izero; // apply 0-attenuation
				continue;
			}

			X = Xh + v1 * static_cast<double> (x) * ds[0]; // curr. DRR pixel position (WCS)

			v_SX = X - S; // ray direction (unnormalized)
			d_conv = sqrt(v_SX[0] * v_SX[0] + v_SX[1] * v_SX[1] + v_SX[2] * v_SX[2]); // ray length

			// compute the parametric entry (alpha_min) and exit (alpha_max) point
			// "coordinates" of the ray S-X through the virtually axis-aligned volume:
			if (d_conv < EPSILON) // invalid length
			{
				drr[idx] = izero;
				continue;
			}
			// - x-component
			if (fabs(v_SX[0]) > EPSILON)
			{
				alpha_x_0 = (O[0] - S[0]) / v_SX[0]; // 1st x-plane
				alpha_x_Nx = (O[0] + static_cast<double> (N[0]) * s[0] - S[0])
						/ v_SX[0]; // last x-plane
				alpha_x_min = std::min(alpha_x_0, alpha_x_Nx);
				alpha_x_max = std::max(alpha_x_0, alpha_x_Nx);
			}
			else // no change along x
			{
				alpha_x_min = -5.; // impossible values
				alpha_x_max = 5.;
				// - be sure that ray intersects with volume
				if (S[0] <= O[0] || S[0] >= (O[0] + static_cast<double> (N[0]) * s[0]))
				{
					drr[idx] = izero;
					continue;
				}
			}
			// - y-component
			if (fabs(v_SX[1]) > EPSILON)
			{
				alpha_y_0 = (O[1] - S[1]) / v_SX[1]; // 1st y-plane
				alpha_y_Ny = (O[1] + static_cast<double> (N[1]) * s[1] - S[1])
						/ v_SX[1]; // last y-plane
				alpha_y_min = std::min(alpha_y_0, alpha_y_Ny);
				alpha_y_max = std::max(alpha_y_0, alpha_y_Ny);
			}
			else // no change along y
			{
				alpha_y_min = -5.; // impossible values
				alpha_y_max = 5.;
				// - be sure that ray intersects with volume
				if (S[1] <= O[1] || S[1] >= (O[1] + static_cast<double> (N[1]) * s[1]))
				{
					drr[idx] = izero;
					continue;
				}
			}
			// - z-component
			if (fabs(v_SX[2]) > EPSILON)
			{
				alpha_z_0 = (O[2] - S[2]) / v_SX[2]; // 1st y-plane
				alpha_z_Nz = (O[2] + static_cast<double> (N[2]) * s[2] - S[2])
						/ v_SX[2]; // last y-plane
				alpha_z_min = std::min(alpha_z_0, alpha_z_Nz);
				alpha_z_max = std::max(alpha_z_0, alpha_z_Nz);
			}
			else // no change along z
			{
				alpha_z_min = -5.; // impossible values
				alpha_z_max = 5.;
				// - be sure that ray intersects with volume
				if (S[2] <= O[2] || S[2] >= (O[2] + static_cast<double> (N[2]) * s[2]))
				{
					drr[idx] = izero;
					continue;
				}
			}
			// - overall min/max
			alpha_min = std::max(alpha_x_min, std::max(alpha_y_min, alpha_z_min));
			alpha_max = std::min(alpha_x_max, std::min(alpha_y_max, alpha_z_max));

			d12 = 0.;
			if (alpha_min < alpha_max) // real intersection
			{
				// compute index of first intersected voxel: i_0, j_0, k_0
				i_0 = (int) floor((S[0] + alpha_min * v_SX[0] - O[0]) / s[0]);
				j_0 = (int) floor((S[1] + alpha_min * v_SX[1] - O[1]) / s[1]);
				k_0 = (int) floor((S[2] + alpha_min * v_SX[2] - O[2]) / s[2]);
				// now "correct" the indices (there are special cases):
				if (alpha_min == alpha_y_min) // j is special
				{
					if (alpha_y_min == alpha_y_0)
						j_0 = 0;
					else
						// alpha_y_Ny
						j_0 = N[1] - 1;
				}
				else if (alpha_min == alpha_x_min) // i is special
				{
					if (alpha_x_min == alpha_x_0)
						i_0 = 0;
					else
						// alpha_x_Nx
						i_0 = N[0] - 1;
				}
				else if (alpha_min == alpha_z_min) // k is special
				{
					if (alpha_z_min == alpha_z_0)
						k_0 = 0;
					else
						// alpha_z_Nz
						k_0 = N[2] - 1;
				}

				// initialize main variables for incremental Siddon-like approach:
				alpha_c = alpha_min;
				i = i_0;
				j = j_0;
				k = k_0;
				alpha_x_u = s[0] / fabs(v_SX[0]);
				alpha_y_u = s[1] / fabs(v_SX[1]);
				alpha_z_u = s[2] / fabs(v_SX[2]);
				i_u = (S[0] < X[0]) ? 1 : -1;
				j_u = (S[1] < X[1]) ? 1 : -1;
				k_u = (S[2] < X[2]) ? 1 : -1;
				// compute alphas of 1st intersection after volume-entry:
				if (fabs(v_SX[0]) > EPSILON)
				{
					if (S[0] < X[0])
						alpha_x = (O[0] + (static_cast<double> (i) + 1.) * s[0] - S[0])
								/ v_SX[0];
					else
						alpha_x = (O[0] + static_cast<double> (i) * s[0] - S[0]) / v_SX[0];
				}
				else // no change along x
				{
					alpha_x = alpha_max; // irrelevant
				}
				if (fabs(v_SX[1]) > EPSILON)
				{
					if (S[1] < X[1])
						alpha_y = (O[1] + (static_cast<double> (j) + 1.) * s[1] - S[1])
								/ v_SX[1];
					else
						alpha_y = (O[1] + static_cast<double> (j) * s[1] - S[1]) / v_SX[1];
				}
				else // no change along y
				{
					alpha_y = alpha_max; // irrelevant
				}
				if (fabs(v_SX[2]) > EPSILON)
				{
					if (S[2] < X[2])
						alpha_z = (O[2] + (static_cast<double> (k) + 1.) * s[2] - S[2])
								/ v_SX[2];
					else
						alpha_z = (O[2] + static_cast<double> (k) * s[2] - S[2]) / v_SX[2];
				}
				else // no change along z
				{
					alpha_z = alpha_max; // irrelevant
				}

				// main loop: go step by step along ray
				while ((alpha_c + EPSILON) < alpha_max) // account for limited precision
				{
					// NOTE: the index is already computed here (index is leading)
					vidx = i + j * vlinepitch + k * vslicepitch;

					// check whether intensity transfer function is defined
					// and whether the mapped value were calculated before
					if (mappedVolume)
					{
						ivol = static_cast<double> (mappedVolume[vidx]);
					}
					else
					{
						ivol = static_cast<double> (volume[vidx]);
						if (itf) // NOTE: We also support no explicit ITF!
							ivol = itf->MapInValue(ivol); // replace value by mapped value
					}

					if (alpha_x <= alpha_y && alpha_x <= alpha_z)
					{
						l_i_j_k = alpha_x - alpha_c;
						i += i_u;
						alpha_c = alpha_x;
						alpha_x += alpha_x_u;
					}
					else if (alpha_y <= alpha_x && alpha_y <= alpha_z)
					{
						l_i_j_k = alpha_y - alpha_c;
						j += j_u;
						alpha_c = alpha_y;
						alpha_y += alpha_y_u;
					}
					else // if (alpha_z <= alpha_x && alpha_z <= alpha_y)
					{
						l_i_j_k = alpha_z - alpha_c;
						k += k_u;
						alpha_c = alpha_z;
						alpha_z += alpha_z_u;
					}
					d12 += (l_i_j_k * ivol);
				}
				d12 *= d_conv; // finally scale with ray length
			}

			// finally write the integrated and weighted intensity
			drr[idx] = static_cast<OutputImagePixelType> (d12);
		}
	}
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::SetOffTheFlyITFMapping(
		bool flag)
{
	if (flag != this->m_OffTheFlyITFMapping)
	{
		this->m_OffTheFlyITFMapping = flag;
		if (!this->m_OffTheFlyITFMapping)
			this->m_MappedInput = NULL; // save on memory
		this->Modified();
	}
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::BeforeThreadedGenerateData()
{
	this->m_LastCPUPreProcessingTime = this->m_PreProcessingClock->GetTimeStamp();

	// FIXME: to be multi-threaded (write an own MT-imageToImagefilter which applies a
	// specified ITF)!!

	// NOTE: If the off-the-fly ITF mode is active, we rather map all the
	// intensities of the volume (in a temporary internal volume) instead of
	// mapping the intensities on the fly during DRR computation. The internal
	// precision is FLOAT in order to save on memory. This may induce minor
	// inaccuracies in the output intensities which can be observed between
	// on-the-fly and off-the-fly modes.

	//check whether temporary volume must be remapped with itf
	if (this->m_OffTheFlyITFMapping && this->m_ITF && (this->m_ITF->GetMTime()
			> m_LastITFCalculationTimeStamp || m_LastInputVolumeTimeStamp
			> m_LastITFCalculationTimeStamp))
	{
		//check if internal volume needs to be updated
		if (this->m_LastInputVolumeTimeStamp > this->m_LastITFCalculationTimeStamp)
		{
			this->m_MappedInput = NULL;
			this->m_MappedInput = MappedVolumeImageType::New();
			this->m_MappedInput->SetSpacing(this->m_Input->GetSpacing());
			this->m_MappedInput->SetOrigin(this->m_Input->GetOrigin());
			this->m_MappedInput->SetDirection(this->m_Input->GetDirection());
			this->m_MappedInput->SetRegions(this->m_Input->GetLargestPossibleRegion());
			this->m_MappedInput->Allocate();
		}
		//reset timestamp
		if (this->m_LastInputVolumeTimeStamp > this->GetITF()->GetMTime())
			this->m_LastITFCalculationTimeStamp = this->m_LastInputVolumeTimeStamp;
		else
			this->m_LastITFCalculationTimeStamp = this->GetITF()->GetMTime();
		//map values
		typedef itk::ImageRegionConstIterator<InputImageType>
				InputConstIteratorType;
		InputConstIteratorType itInput(this->m_Input,
				this->m_Input->GetLargestPossibleRegion());
		typedef itk::ImageRegionIterator<MappedVolumeImageType> OutputIteratorType;
		OutputIteratorType itMapped(this->m_MappedInput,
				this->m_MappedInput->GetLargestPossibleRegion());
		itInput.GoToBegin();
		itMapped.GoToBegin();
		do
		{
			itMapped.Set(
					this->m_ITF->MapInValue(
							static_cast<MappedVolumePixelType> (itInput.Get())));
			++itMapped;
			++itInput;
		} while (!itInput.IsAtEnd() && !itMapped.IsAtEnd());
	}

	if (!this->m_OffTheFlyITFMapping)
		this->m_MappedInput = NULL; // save on memory!

	this->m_LastCPUPreProcessingTime
			= (this->m_PreProcessingClock->GetTimeStamp()
					- this->m_LastCPUPreProcessingTime) * 1000.;
	this->m_LastCPUPureProcessingTime
			= this->m_PureProcessingClock->GetTimeStamp();
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::AfterThreadedGenerateData()
{
	this->m_LastCPUPureProcessingTime
			= (this->m_PureProcessingClock->GetTimeStamp()
					- this->m_LastCPUPureProcessingTime) * 1000.;
	this->m_LastCPUPostProcessingTime
			= this->m_PostProcessingClock->GetTimeStamp();

	// CPU post-processing code here ...

	this->m_LastCPUPostProcessingTime
			= (this->m_PostProcessingClock->GetTimeStamp()
					- this->m_LastCPUPostProcessingTime) * 1000.;
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::DRRCanBeComputed() const
{
	if (!this->m_Input) // need input
		return false;
	typename InputImageType::RegionType reg =
			this->m_Input->GetLargestPossibleRegion();
	if (reg.GetNumberOfPixels() < 1) // input is requested to have valid size
		return false;
	//check if Geometry is set for this output
	if (!this->GetProjectionGeometry(this->m_CurrentDRROutputIndex))
	{
		return false;
	}
	//check if Geometry is valid
	return this->GetProjectionGeometry(this->m_CurrentDRROutputIndex)-> IsGeometryValid();
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsGPUBased() const
{
	return false; // non-GPU-based
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsCPUBased() const
{
	return true; // CPU-based (even multi-threaded)
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsCPUMultiThreaded() const
{
	return true; // CPU-based and multi-threaded
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingDRRMasks() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingITFOnTheFly() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingITFOffTheFly() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingRigidTransformation() const
{
	return true;
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingAffineTransformation() const
{
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
bool CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::IsSupportingElasticTransformation() const
{
	return false;
}

template<class TInputPixelType, class TOutputPixelType>
void CPUSiddonDRRFilter<TInputPixelType, TOutputPixelType>::UpdateCurrentImagingGeometry()
{
	// current desired geometry:
	GeometryPointer geom = this->GetProjectionGeometry(this->m_CurrentDRROutputIndex);
	if (!geom)
		return;

	// NOTE: Here, we assume some matrix-offset-transform-based transform type.
	// More precisely, even a rigid transform. In order to get GetInverse() safely
	// to work, we reinterpret the transform as matrix-offset-transform.
	if (!this->m_Transform)
		return;
	TransformType *transform =
			dynamic_cast<TransformType*> (this->m_Transform.GetPointer());
	if (!transform)
		return;

	this->m_GeometryTransform->SetIdentity();
	// correct the spatial transform (if set) by applying it inversely:
	if (transform)
		transform->GetInverse(this->m_GeometryTransform);

	// correct the input volume orientation by applying it inversely:
	// (NOTE: We have to rotate about the volume's corner, not around the
	// first voxel's center - this was a bug before!)
	typename InputImageType::PointType O = this->m_Input->GetOrigin();
	typename InputImageType::DirectionType vdir = this->m_Input->GetDirection();
	typename InputImageType::SpacingType s = this->m_Input->GetSpacing();
	VectorType vv;
	vv[0] = vdir[0][0]; // volume row direction
	vv[1] = vdir[1][0];
	vv[2] = vdir[2][0];
	O = O - vv * s[0] / 2.;
	vv[0] = vdir[0][1]; // volume column direction
	vv[1] = vdir[1][1];
	vv[2] = vdir[2][1];
	O = O - vv * s[1] / 2.;
	vv[0] = vdir[0][2]; // volume slicing direction
	vv[1] = vdir[1][2];
	vv[2] = vdir[2][2];
	O = O - vv * s[2] / 2.;
	TransformPointer orientTransform = TransformType::New();
	typedef typename TransformType::MatrixType MatrixType;
	MatrixType Rv = MatrixType(this->m_Input->GetDirection().GetInverse());
	orientTransform->SetCenter(O);
	orientTransform->SetMatrix(Rv);
	// compose
	this->m_GeometryTransform->Compose(orientTransform, false);

	PointType sourcePosition;
	PointType DRROrigin;
	VectorType DRRRowDirection;
	VectorType DRRColumnDirection;
	for (int i = 0; i < 3; i++)
	{
		sourcePosition[i] = geom->GetSourcePosition()[i];
		DRROrigin[i] = geom->GetDetectorOrigin()[i];
		DRRRowDirection[i] = geom->GetDetectorRowOrientation()[i];
		DRRColumnDirection[i] = geom->GetDetectorColumnOrientation()[i];
	}

	// inverse transform to account for rigid transform:
	sourcePosition = this->m_GeometryTransform->TransformPoint(sourcePosition);
	DRROrigin = this->m_GeometryTransform->TransformPoint(DRROrigin);
	DRRRowDirection = this->m_GeometryTransform->TransformVector(DRRRowDirection);
	DRRColumnDirection = this->m_GeometryTransform->TransformVector(
			DRRColumnDirection);

	// save back into currentGeometry
	double srcTemp[3];
	double drrOriginTemp[3];
	double drrRowDirTemp[3];
	double drrColumnDirTemp[3];
	for (int i = 0; i < 3; i++)
	{
		srcTemp[i] = sourcePosition[i];
		drrOriginTemp[i] = DRROrigin[i];
		drrRowDirTemp[i] = DRRRowDirection[i];
		drrColumnDirTemp[i] = DRRColumnDirection[i];
	}
	this->m_CurrentGeometry->SetDetectorOrigin(drrOriginTemp);
	this->m_CurrentGeometry->SetSourcePosition(srcTemp);
	this->m_CurrentGeometry->SetDetectorRowOrientation(drrRowDirTemp);
	this->m_CurrentGeometry->SetDetectorColumnOrientation(drrColumnDirTemp);

	// these props are not affected, just copy them:
	this->m_CurrentGeometry->SetDetectorPixelSpacing(
			geom->GetDetectorPixelSpacing());
	this->m_CurrentGeometry->SetDetectorSize(geom->GetDetectorSize());
}

}

#endif /* ORAITKDRRFILTER_TXX_ */
