/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
/* This program can merge vector fields based on the binary masks of 
   non-overlapping regions (e.g., moving and non-moving tissues). It 
   should be called with 5 input parameters. Besides the vf1, vf2, 
   and output vf names, the additional 2 files are mask1 and mask2. 
   The program will compose the vector fields according to the masks 
   and leave zero vectors for the area that is outside both masks.
*/
#include "plm_config.h"
#include <fstream>
#include <string>

#include "itkImageFileReader.h" 
#include "itkImageFileWriter.h" 
#include "itkAffineTransform.h"
#include "itkImageRegionIterator.h"
#include "itkImageToImageFilter.h"
#include "itkInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"

const unsigned int Dimension = 3;

typedef unsigned char MaskPixelType;
typedef itk::Image < unsigned char, 3 > UCharImageType;
typedef itk::Image < MaskPixelType, Dimension > MaskImageType;
typedef itk::ImageFileReader < UCharImageType > MhaUCharReaderType;
typedef itk::ImageRegionIterator< UCharImageType > UCharIteratorType;

template<class RdrT>
void
load_mha_rdr(RdrT reader, char *fn)
{
    reader->SetFileName(fn);
    try {
	reader->Update();
    }
    catch(itk::ExceptionObject & ex) {
	printf ("Exception reading mha file: %s!\n",fn);
	std::cout << ex << std::endl;
	getchar();
	exit(1);
    }
}

int main( int argc, char *argv[] )
{
  if (argc!=6)  {
    std::cerr << "Wrong Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " vector_field1 vector_field2";
    std::cerr << " outputVectorFieldFile";
    std::cerr << " vector_field_mask1 vector_field_mask2 " << std::endl;
    return 1;
  }

  //load vector2 field file
  typedef itk::Vector< float, Dimension >  VectorType;
  typedef itk::Image< VectorType, Dimension >  DeformationFieldType;
  typedef itk::ImageFileReader< DeformationFieldType >  FieldReaderType;

  typedef itk::Image< float, Dimension >  FloatImageType;

  typedef itk::InterpolateImageFunction<FloatImageType, float> InterpolatorType;

  FieldReaderType::Pointer fieldReader1 = FieldReaderType::New();
  fieldReader1->SetFileName( argv[1] );
  
  try 
  {
	  fieldReader1->Update();
  }
  catch (itk::ExceptionObject& excp) 
  {
	  std::cerr << "Exception thrown " << std::endl;
	  std::cerr << excp << std::endl;
    return 0;
  }
  DeformationFieldType::Pointer deform_field1 = fieldReader1->GetOutput();

  //load vector2 field file
  FieldReaderType::Pointer fieldReader = FieldReaderType::New();
  fieldReader->SetFileName( argv[2] );
  
  try 
  {
	  fieldReader->Update();
  }
  catch (itk::ExceptionObject& excp) 
  {
	  std::cerr << "Exception thrown " << std::endl;
	  std::cerr << excp << std::endl;
    return 0;
  }
  DeformationFieldType::Pointer deform_field2 = fieldReader->GetOutput();

  printf("loaded two vf\n" );

  DeformationFieldType::Pointer field_out = DeformationFieldType::New();

  MaskImageType::Pointer mask1 = MaskImageType::New();
  MaskImageType::Pointer mask2 = MaskImageType::New();
  MhaUCharReaderType::Pointer mask_reader1 = MhaUCharReaderType::New();
  MhaUCharReaderType::Pointer mask_reader2 = MhaUCharReaderType::New();

  load_mha_rdr(mask_reader1, argv[4]);
  mask1 = mask_reader1->GetOutput();
  load_mha_rdr(mask_reader2, argv[5]);
  mask2 = mask_reader2->GetOutput();

  UCharIteratorType it1(mask1, mask1->GetBufferedRegion());
  UCharIteratorType it2(mask2, mask1->GetBufferedRegion());

  printf("masks loaded\n" );

  field_out->SetRegions (deform_field2->GetBufferedRegion());
  field_out->SetOrigin  (deform_field2->GetOrigin());
  field_out->SetSpacing (deform_field2->GetSpacing());
  field_out->Allocate();
  
  printf("allocated out vector field\n" );
	  
  typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
  FieldIterator d_f1 (deform_field1, deform_field1->GetBufferedRegion());
  FieldIterator d_f2 (deform_field2, deform_field2->GetBufferedRegion());
  FieldIterator f_out (field_out, deform_field2->GetBufferedRegion());

  // populate the output field
  f_out.GoToBegin();
  d_f1.GoToBegin();
  d_f2.GoToBegin();
  it1.GoToBegin();
  it2.GoToBegin();
  while (!f_out.IsAtEnd()) {
      unsigned char m1 = it1.Get();
      unsigned char m2 = it2.Get();

      VectorType vf;
      if (m1>0) {
	  vf = d_f1.Get();
      } else if (m2>0) {
	  vf = d_f2.Get();
      } else {
	  vf[0] = 0; vf[1] = 0; vf[2] = 0;
      }
      
      f_out.Set(vf);
      ++f_out; ++d_f1; ++d_f2; ++it1; ++it2;
  }

  printf("produced out vector field\n" );
  typedef itk::ImageFileWriter< DeformationFieldType >  FieldWriterType;
  FieldWriterType::Pointer fieldWriter = FieldWriterType::New();
  fieldWriter->SetInput (field_out);
  fieldWriter->SetFileName (argv[3]);
  try 
  {
    fieldWriter->Update();
  }
  catch (itk::ExceptionObject& excp) 
  {
    std::cerr << "Exception thrown " << std::endl;
    std::cerr << excp << std::endl;
  }
  return 0;
}

