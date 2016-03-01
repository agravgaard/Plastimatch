//

#ifndef ORAITKVTKLINEARTRANSFORMCONNECTOR_H_
#define ORAITKVTKLINEARTRANSFORMCONNECTOR_H_

#include <itkObjectFactory.h>
#include <itkMatrixOffsetTransformBase.h>
#include <itkCommand.h>

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkCommand.h>

namespace ora
{

class ITKVTKLinearTransformConnector; // see below


/** \class VTKTransformModifiedObserver
 * \brief Observer for a linear transformation's modified event (VTK).
 *
 * Simple observer for VTK modified event. Dedicated to transformation
 * observation and mutual ITK / VTK transformation update.
 *
 * @see ITKVTKLinearTransformConnector
 *
 * @author phil 
 * @version 1.0
 *
 * \ingroup Transforms
 */
class VTKTransformModifiedObserver:
    public vtkCommand
{
public:
  /** accessibility typedefs **/
  typedef ITKVTKLinearTransformConnector HostType;
  typedef HostType* HostPointer;

  /** Standard construction method. **/
  static VTKTransformModifiedObserver *New();

  /**
   * Main entry point of the command.
   * @param caller the command caller object
   * @param event type of event (must be supported)
   * @param callData pointer to custom data structures
   **/
  void Execute(vtkObject *caller, unsigned long event, void *callData);

  /**
   * Set Host which is capable of transformation conversion: this host must have
   * access to the source transformations and must provide sufficient
   * reentrancy protection.
   **/
  void SetHost(HostPointer host);
  /** Get Host which is capable of transformation conversion. **/
  HostPointer GetHost();

protected:
  /**
   * Host which is capable of transformation conversion: this host must have
   * access to the source transformations and must provide sufficient
   * reentrancy protection.
   **/
  HostPointer m_Host;

  /** Internal constructor **/
  VTKTransformModifiedObserver();

  /** Internal destructor **/
  ~VTKTransformModifiedObserver();

private:
  // purposely not implemented
  VTKTransformModifiedObserver(const VTKTransformModifiedObserver &);
  // purposely not implemented
  void operator=(const VTKTransformModifiedObserver &);

};

/** \class ITKTransformModifiedObserver
 * \brief Observer for a linear transformation's modified event (ITK).
 *
 * Simple observer for ITK modified event. Dedicated to transformation
 * observation and mutual ITK / VTK transformation update.
 *
 * @see ITKVTKLinearTransformConnector
 *
 * @author phil 
 * @version 1.0
 */
class ITKTransformModifiedObserver:
    public itk::Command
{
public:
  /** Standard class typedefs. */
  typedef ITKTransformModifiedObserver Self;
  typedef itk::Command Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** accessibility typedefs **/
  typedef ITKVTKLinearTransformConnector HostType;
  typedef HostType* HostPointer;

  /** Standard construction method. **/
  itkNewMacro(Self)
  ;

  /** purposely not implemented. **/
  void Execute(itk::Object *caller, const itk::EventObject &event);

  /**
   * Main-method for observer.
   * @param object reference to the host
   * @param event specifies the event-type
   */
  void Execute(const itk::Object *object, const itk::EventObject &event);

  /**
   * Set Host which is capable of transformation conversion: this host must have
   * access to the source transformations and must provide sufficient
   * reentrancy protection.
   **/
  void SetHost(HostPointer host);
  /** Get Host which is capable of transformation conversion. **/
  HostPointer GetHost();

protected:
  /**
   * Host which is capable of transformation conversion: this host must have
   * access to the source transformations and must provide sufficient
   * reentrancy protection.
   **/
  HostPointer m_Host;

  /** Optimizer iteration observer constructor. **/
  ITKTransformModifiedObserver();

  /** Optimizer iteration observer destructor. **/
  ~ITKTransformModifiedObserver();

private:
  // purposely not implemented
  ITKTransformModifiedObserver(const Self &);
  // purposely not implemented
  void operator=(const Self &);

};

/** \class ITKVTKLinearTransformConnector
 * \brief Mutually connects a pair of ITK and VTK linear 3D transformations.
 *
 * Mutually connects a pair of ITK and VTK linear 3D transformations by
 * observing the transformations' modified events, and updating the opposite
 * transformation adequately.
 *
 * This may for example be useful for visualizing (using VTK) transformations
 * that emerge from ITK algorithms (e.g. image registration).
 *
 * NOTE: Updating the VTK transform triggered by an ITK transform's Modified()
 * event will set a NEW matrix in order to avoid legacy code warning messages.
 * Therefore any application using this connector must take care to renew the
 * VTK matrix pointer each time the VTK transform is changed! The legacy
 * workaround is the main reason for the vtkTransform-base instead of a
 * vtkLinearTransform-base.
 *
 * In addition relative transforms that are additionally post-multiplied to the
 * conversion results can be specified for both directions. This enables more
 * advanced transform mapping (e.g. additional frame of reference information
 * can be integrated).
 *
 * \ingroup Transforms
 *
 * @author phil 
 * @version 1.3
 */
class ITKVTKLinearTransformConnector:
    public itk::Object
{
public:
  /** Standard class typedefs. */
  typedef ITKVTKLinearTransformConnector Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** accessibility types **/
  typedef vtkSmartPointer<vtkTransform> VTKTransformPointer;
  typedef itk::MatrixOffsetTransformBase<double, 3, 3> ITKTransformType;
  typedef ITKTransformType::Pointer ITKTransformPointer;
  typedef vtkSmartPointer<vtkMatrix4x4> RelativeMatrixPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ITKVTKLinearTransformConnector, itk::Object)
  ;

  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  ;

  /**
   * Set an optional relative transformation (by matrix) that maps the ITK
   * linear transformation to an equivalent VTK linear transformation. Could
   * be useful for implicit frame of reference implementation. NOTE: this
   * transform is simply post-multiplied after basic transform conversion. This
   * matrix's modified event is not observed!
   **/
  virtual void SetRelativeITKVTKMatrix(RelativeMatrixPointer matrix);
  /**
   * Get relative transformation (by matrix) that maps the ITK
   * linear transformation to an equivalent VTK linear transformation.
   **/
  itkGetMacro(RelativeITKVTKMatrix, RelativeMatrixPointer)

  /**
   * Set an optional relative transformation (by matrix) that maps the VTK
   * linear transformation to an equivalent ITK linear transformation. Could
   * be useful for implicit frame of reference implementation. NOTE: this
   * transform is simply post-multiplied after basic transform conversion. This
   * matrix's modified event is not observed!
   **/
  virtual void SetRelativeVTKITKMatrix(RelativeMatrixPointer matrix);
  /**
   * Get relative transformation (by matrix) that maps the VTK
   * linear transformation to an equivalent ITK linear transformation.
   **/
  itkGetMacro(RelativeVTKITKMatrix, RelativeMatrixPointer)

  /** Set source transform: ITK transform **/
  virtual void SetITKTransform(ITKTransformType *itkTransf);
  /** Get source transform: ITK transform **/
  itkGetObjectMacro(ITKTransform, ITKTransformType)

  /** Set source transform: VTK transform **/
  virtual void SetVTKTransform(VTKTransformPointer vtkTransf);
  /** Get source transform: VTK transform **/
  itkGetMacro(VTKTransform, VTKTransformPointer)

protected:
  /** some good friends ;) **/
  friend class VTKTransformModifiedObserver;
  friend class ITKTransformModifiedObserver;

  /** relative transformation matrix from ITK to VTK conversion **/
  RelativeMatrixPointer m_RelativeITKVTKMatrix;
  /** relative transformation matrix from VTK to ITK conversion **/
  RelativeMatrixPointer m_RelativeVTKITKMatrix;
  /** source: VTK transform **/
  VTKTransformPointer m_VTKTransform;
  /** source: ITK transform **/
  ITKTransformPointer m_ITKTransform;
  /** reentrancy protection flag **/
  bool m_IsUpdating;
  /** observer for VTK transform **/
  vtkSmartPointer<VTKTransformModifiedObserver> m_VTKObserver;
  /** VTK observer tag **/
  unsigned int long m_VTKObserverTag;
  /** observer for ITK transform **/
  ITKTransformModifiedObserver::Pointer m_ITKObserver;
  /** ITK observer tag **/
  unsigned int long m_ITKObserverTag;

  /** Default constructor. **/
  ITKVTKLinearTransformConnector();
  /** Default destructor. **/
  virtual ~ITKVTKLinearTransformConnector();

  /** Print description of this object. **/
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Mutually update the transformations from each other.
   * @param itkToVTK if TRUE the VTK transform is updated from the ITK
   * transform; if FALSE the ITK transform is updated from the VTK transform
   */
  virtual void UpdateTransform(bool itkToVTK);

private:
  /** Purposely not implemented. **/
  ITKVTKLinearTransformConnector(const Self&);
  /** Purposely not implemented. **/
  void operator=(const Self&);

};

}

#endif /* ORAITKVTKLINEARTRANSFORMCONNECTOR_H_ */
