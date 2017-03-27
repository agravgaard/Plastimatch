//
#ifndef ORAINTENSITYTRANSFERFUNCTION_H_
#define ORAINTENSITYTRANSFERFUNCTION_H_

//ITK
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <vector>

namespace ora
{

/** Struct for easy use of supporting points. **/
struct SupportingPointStruct{
	  double inValue;
	  double outValue;
};

/** \class IntensityTransferFunction
 * \brief Defines a general intensity transfer function (ITF) suitable for DRR generation.
 *
 * This class implements the basic definition of an intensity transfer function
 * (ITF) as proposed in "plastimatch digitally reconstructed radiographs
 * (DRR) application programming interface (API)" (design document).
 *
 * Please,
 * refer to this design document in order to retrieve more information on the
 * assumptions and restrictions regarding ITF definition!
 *
 * <b>Tests</b>:<br>
 * TestIntensityTransferFunction.cxx <br>
 *
 * @author phil
 * @author jeanluc
 * @version 1.0
 */
class IntensityTransferFunction : public itk::Object
{
public:

  /** Standard class typedefs. **/
  typedef IntensityTransferFunction Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  typedef std::vector<SupportingPointStruct> Vector;

  /** Method for creation through the object factory. **/
  itkNewMacro(Self);

  /** Run-time type information (and related methods). **/
  itkTypeMacro(IntensityTransferFunction, itk::Object);

  // Manipulation methods for adding/removing/retrieving supporting points
  /** Add new supporting point **/
  void AddSupportingPoint(double in, double out);
  
  /** Return the current number of supporting points **/
  unsigned int GetNumberOfSupportingPoints() const;
  
  /** Remove the supporting point number i, i: 0..n-1**/
  void RemoveSupportingPoint(int i); // i is 0-based supporting point index
  
  /** Clear all supporting points**/
  void RemoveAllSupportingPoints();
  
  /** Return all input intensity thresholds **/
  const Vector *GetSupportingPoints();
  
// Mapping methods which map in values to out values according to the supporting
// points and computing values between the supporting points using 
// linear interpolation
  template<typename TPixel> inline TPixel MapInValue(TPixel in);
  template<typename TPixel> inline void MapInValue(TPixel in, TPixel &out);
  template<typename TPixel> inline void MapInValues(int count, TPixel *ins, TPixel *outs);
  
protected:

  /** Supporting point vector. **/
  Vector m_SupportingPoints;
  /** Update helper. **/
  mutable unsigned long int m_LastSortTimeStamp;
  /** Default constructor. **/
  IntensityTransferFunction();
  /** Default destructor. **/
  virtual ~IntensityTransferFunction();
  /** Standard object output. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;
  /** Calculate output intensity for given input intensity. **/
  double GetOutputIntensity(double in);
  /** Implements a smaller than implementation for Supporting point structure. **/
  static bool SupportingPointsSmallerThan(const SupportingPointStruct &sp1,
		 const SupportingPointStruct &sp2);
};

}

#include "oraIntensityTransferFunction.txx"

#endif /* ORAINTENSITYTRANSFERFUNCTION_H_ */
