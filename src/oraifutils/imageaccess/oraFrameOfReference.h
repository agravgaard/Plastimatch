
#ifndef ORAFRAMEOFREFERENCE_HXX_
#define ORAFRAMEOFREFERENCE_HXX_

#include <map>

#include <itkObject.h>
#include <itkScalableAffineTransform.h>
#include <itkVector.h>
#include <itkMatrix.h>

#include <vtkSmartPointer.h>
#include <vtkTransform.h>


namespace ora 
{


/**
 * Represents a frame of reference (FOR) whereon image coordinate systems can
 * be defined. This concept is in principle comparable to the DICOM frame of
 * references which typically relates to one or more series. It describes the
 * relative spatial relationship between a set of images.
 *
 * The frame of reference can directly be represented as affine transformation
 * (ITK-/VTK-compatible).
 *
 * @author phil 
 * @version 1.2
 */
class FrameOfReference
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef FrameOfReference Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  typedef itk::Transform<double, 3, 3> GenericTransformType;
  typedef itk::ScalableAffineTransform<double, 3> TransformType;
  typedef TransformType::Pointer TransformPointer;
  typedef itk::Vector<double, 3> VectorType;
  typedef itk::Matrix<double, 3, 3> MatrixType;
  typedef itk::Point<double, 3> PointType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(FrameOfReference, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Load the frame of reference from a typical open radART frame of reference
   * entry (Frames.inf).
   * @param oraString the open radART frame of reference entry
   * @return true if successful
   */
  bool LoadFromORAString(std::string oraString);

  /**
   * Get a pointer to the internal transformation describing the FOR
   * (generic transform type).
   **/
  GenericTransformType::ConstPointer GetTransform()
  {
    return itk::SmartPointer<const GenericTransformType>(
        static_cast<const GenericTransformType *>(m_Transform.GetPointer()));
  }

  /**
   * Get a pointer to the internal transformation describing the FOR (native
   * affine transform type).
   **/
  TransformType::ConstPointer GetAffineTransform()
  {
    return itk::SmartPointer<const TransformType>(
        static_cast<const TransformType *>(m_Transform.GetPointer()));
  }

  /**
   * Get internal affine transformation (translation, rotation, scale, origin);
   * contains the transformation of THIS FOR for VTK visualization purposes!
   */
  vtkSmartPointer<vtkTransform> GetVTKTransform()
  {
    return m_VTKTransform;
  }

  /** Get the FOR Descriptor (patient position). **/
  std::string GetDescriptor()
  {
    return m_Descriptor;
  }
  /**
   * Set the FOR Descriptor (patient position).
   * @param descriptor the patient position: <br>
   * HFS, HFP, HFDR, HFDL, FFS, FFP, FFDR, FFDL <br>
   * <b>NOTE:</b> setting this prop causes to transformations to be updated!
   **/
  void SetDescriptor(std::string descriptor)
  {
    m_Descriptor = descriptor;
    UpdateInternalTransformations();
  }

  /** Get translational transformation vector. **/
  VectorType GetTranslation()
  {
    return m_Translation;
  }
  /**
   * Set translational transformation vector.
   * <b>NOTE:</b> setting this prop causes to transformations to be updated!
   **/
  void SetTranslation(VectorType translation)
  {
    m_Translation = translation;
    UpdateInternalTransformations();
  }

  /** Get rotational matrix coefficients of FOR transformation. **/
  MatrixType GetRotation()
  {
    return m_Rotation;
  }
  /**
   * Set rotational matrix coefficients of FOR transformation.
   * <b>NOTE:</b> setting this prop causes to transformations to be updated!
   **/
  void SetRotation(MatrixType rotation)
  {
    m_Rotation = rotation;
    UpdateInternalTransformations();
  }

  /** Get FOR origin of FOR transformation. **/
  PointType GetOrigin()
  {
    return m_Origin;
  }
  /**
   * Set FOR origin of FOR transformation.
   * <b>NOTE:</b> setting this prop causes to transformations to be updated!
   **/
  void SetOrigin(PointType origin)
  {
    m_Origin = origin;
    UpdateInternalTransformations();
  }

  /** Get scaling factors of FOR transformation. **/
  VectorType GetScaling()
  {
    return m_Scaling;
  }
  /**
   * Set scaling factors of FOR transformation.
   * <b>NOTE:</b> setting this prop causes to transformations to be updated!
   **/
  void SetScaling(VectorType scaling)
  {
    m_Scaling = scaling;
    UpdateInternalTransformations();
  }

  /** Set (optional in case of old FORs) FOR ID (usually an integer number) **/
  void SetFORID(std::string forID)
  {
    m_FORID = forID;
  }
  /** Get (optional in case of old FORs) FOR ID (usually an integer number) **/
  std::string GetFORID()
  {
    return m_FORID;
  }

  /** Get the FOR UID. **/
  std::string GetUID()
  {
    return m_UID;
  }
  /** Set the FOR UID. **/
  void SetUID(std::string UID)
  {
    m_UID = UID;
  }

protected:
  /**
   * internal affine transformation (translation, rotation, scale, origin);
   * just contains the transformation of THIS FOR
   **/
  TransformPointer m_Transform;
  /**
   * internal affine transformation (translation, rotation, scale, origin);
   * contains the transformation of THIS FOR for VTK visualization purposes!
   */
  vtkSmartPointer<vtkTransform> m_VTKTransform;

  /** FOR unique identifier (typically a UID originating from DICOM) **/
  std::string m_UID;
  /** textual descriptor of the FOR **/
  std::string m_Descriptor;
  /** translational transformation vector **/
  VectorType m_Translation;
  /** rotational matrix coefficients of transformation **/
  MatrixType m_Rotation;
  /** FOR origin **/
  PointType m_Origin;
  /** scaling factors of transformation **/
  VectorType m_Scaling;
  /** (optional in case of old FORs) FOR ID (usually an integer number) **/
  std::string m_FORID;

  /** Default constructor. **/
  FrameOfReference();
  /** Default destructor. **/
  ~FrameOfReference();

  /**
   * Update the internal transformations w.r.t. the set translation,
   * rotation and scaling.
   */
  void UpdateInternalTransformations();

private:
  /** purposely not implemented **/
  FrameOfReference(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


/**
 * Builds up a collection of available FORs.
 *
 * @author phil 
 * @version 1.0
 */
class FrameOfReferenceCollection
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef FrameOfReferenceCollection Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(FrameOfReferenceCollection, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Create a FOR collection from a typical open radART frame of reference
   * file.
   * @param framesFile the open radART frame of reference file
   * @return the created FOR collection if successful, NULL otherwise
   * @see ora::FrameOfReference
   */
  static Pointer CreateFromFile(std::string framesFile);

  /**
   * Get class-wide static default FOR ('unity') which can be retrieved without
   * instantiation; SINGLETON.
   **/
  static FrameOfReference::Pointer GetDefaultFOR();

  /**
   * Find a specified FOR in the collection.
   * @param UID identifier of the FOR
   * @return the FOR if found, NULL otherwise
   **/
  FrameOfReference::Pointer FindFOR(const std::string UID);

  /**
   * Add a FOR to the collection.
   * @param FOR the FOR to be added
   * @return TRUE if the FOR could be added
   */
  bool AddFOR(FrameOfReference::Pointer FOR);

  /**
   * Remove a FOR from the collection.
   * @param FOR the FOR to be removed
   * @return TRUE if the FOR could be removed
   */
  bool RemoveFOR(FrameOfReference::Pointer FOR);

  /**
   * Remove a FOR from the collection.
   * @param UID the UID of the FOR to be removed
   * @return TRUE if the FOR could be removed
   */
  bool RemoveFOR(std::string UID);

  /**
   * Clear the complete collection.
   */
  void Clear();

  /**
   * @return the UIDs of the contained FORs.
   */
  std::vector<std::string> GetFORUIDs();

  itkGetMacro(PatientUID, std::string);
  itkSetMacro(PatientUID, std::string);

  itkGetMacro(PatientName, std::string);
  itkSetMacro(PatientName, std::string);

  /** @return the number of FORs contained in this FOR collection **/
  unsigned int GetNumberOfFORs();

protected:
  /** FOR map type **/
  typedef std::map<std::string, FrameOfReference::Pointer> FORMapType;

  /**
   * class-wide static default FOR ('unity') which can be retrieved without
   * instantiation; SINGLETON
   **/
  static FrameOfReference::Pointer m_DefaultFOR;

  /**
   * internal FOR map holding the references and UIDs; the first component is
   * the FOR UID and the second component is a smart pointer to the FOR
   * information in object-representation
   **/
  FORMapType m_FORMap;

  /** the according patient's ORA UID **/
  std::string m_PatientUID;
  /** the according patient's name (no specified formatting) **/
  std::string m_PatientName;

  /** Default constructor. **/
  FrameOfReferenceCollection();
  /** Default destructor. **/
  ~FrameOfReferenceCollection();

private:
  /** purposely not implemented **/
  FrameOfReferenceCollection(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


}


#endif /* ORAFRAMEOFREFERENCE_HXX_ */
