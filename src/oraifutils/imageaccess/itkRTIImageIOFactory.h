

#ifndef ITKRTIIMAGEIOFACTORY_H
#define ITKRTIIMAGEIOFACTORY_H

#include <cstddef> /* Workaround bug in ITK 3.20 */

#include <itkObjectFactoryBase.h>
#include <itkImageIOBase.h>


namespace itk
{


/**
  * ITK-compliant Image IO Factory for reading open radART *.rti images.
  * @author phil 
  * @version 1.0
  */
class RTIImageIOFactory : public ObjectFactoryBase
{

public:
  /** Standard class typedefs. */
  typedef RTIImageIOFactory Self;
  typedef ObjectFactoryBase Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);
  static RTIImageIOFactory *FactoryNew()
  {
    return new RTIImageIOFactory;
  }
  /** Run-time type information (and related methods). */
  itkTypeMacro(RTIImageIOFactory, ObjectFactoryBase);

  /** Register one factory of this type  */
  static void RegisterOneFactory(void)
  {
    RTIImageIOFactory::Pointer rtiFactory = RTIImageIOFactory::New();
    ObjectFactoryBase::RegisterFactory(rtiFactory);
  }

protected:
  /** Constructor **/
  RTIImageIOFactory();

  /** Destructor **/
  ~RTIImageIOFactory();

private:
  RTIImageIOFactory(const Self&); // purposely not implemented

  void operator=(const Self&); // purposely not implemented

};


}

#endif /* ITKRTIIMAGEIOFACTORY_H */
