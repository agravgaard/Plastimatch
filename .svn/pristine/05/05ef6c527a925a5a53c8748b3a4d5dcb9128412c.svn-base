

#ifndef ORAIMAGELIST_H_
#define ORAIMAGELIST_H_

#include "oraFrameOfReference.h"

#include <itkObject.h>
#include <itkPoint.h>
#include <itkMatrix.h>
#include <itkVector.h>


namespace ora 
{


/**
 * Represents an image list entry which encapsulates a set of image-related
 * meta information.
 * @author phil 
 * @version 1.0
 */
class ImageListEntry
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef ImageListEntry Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** important typedefs **/
  typedef FrameOfReference FORType;
  typedef FORType::Pointer FORPointer;
  typedef FrameOfReferenceCollection FORCollType;
  typedef FORCollType::Pointer FORCollPointer;
  typedef itk::Point<double, 3> PointType;
  typedef itk::Matrix<double, 3, 3> MatrixType;
  typedef itk::Vector<double, 3> VectorType;

  /** Image geometry type **/
  typedef enum
  {
    /** undefined image geometry **/
    UNDEFINED = -1,
    /** perspective image (e.g. DRR, simulator image) **/
    PERSPECTIVE,
    /** orthographic image (e.g. scintigraphy) **/
    ORTHOGRAPHIC,
    /** topogram image (1-dimensional divergence) **/
    TOPOGRAM,
    /** slice image (all cut views) **/
    SLICE
  } GeometryType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageListEntry, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Load the image list entry from a typical open radART image list
   * entry (ImageList.inf).
   * @param oraString the open radART image list entry
   * @param forColl loaded Frame of Reference Collection
   * @return true if successful
   */
  bool LoadFromORAString(std::string oraString,
      FORCollPointer forColl);

  /** Get image file name. **/
  std::string GetImageFileName()
  {
    return m_ImageFileName;
  }
  /** Set image file name. **/
  void SetImageFileName(std::string fileName)
  {
    m_ImageFileName = fileName;
  }

  /** Get acquisition type. **/
  std::string GetAcquisitionType()
  {
    return m_AcquisitionType;
  }
  /** Set acquisition type. **/
  void SetImageAcquisitionType(std::string acquisitionType)
  {
    m_AcquisitionType = acquisitionType;
  }

  /** Get acquisition date. **/
  std::string GetAcquisitionDate()
  {
    return m_AcquisitionDate;
  }
  /** Set acquisition date. **/
  void SetImageAcquisitionDate(std::string acquisitionDate)
  {
    m_AcquisitionDate = acquisitionDate;
  }

  /** Get patient position. **/
  std::string GetPatientPosition()
  {
    return m_PatientPosition;
  }
  /** Set patient position. **/
  void SetPatientPosition(std::string patientPosition)
  {
    m_PatientPosition = patientPosition;
  }

  /** Get referenced FOR. **/
  FORPointer GetReferencedFOR()
  {
    return m_ReferencedFOR;
  }
  /** Set referenced FOR. **/
  void SetReferencedFOR(FORPointer referencedFOR)
  {
    m_ReferencedFOR = referencedFOR;
  }

  /** Get Image position (in patient coordinate system). **/
  PointType GetImagePosition()
  {
    return m_ImagePosition;
  }
  /** Set Image position (in patient coordinate system). **/
  void SetImagePosition(PointType imagePosition)
  {
    m_ImagePosition = imagePosition;
  }

  /** Get Image orientation (in patient coordinate system). **/
  MatrixType GetImageOrientation()
  {
    return m_ImageOrientation;
  }
  /** Set Image orientation (in patient coordinate system). **/
  void SetImageOrientation(MatrixType imageOrientation)
  {
    m_ImageOrientation = imageOrientation;
  }

  /** Get Image geometry. **/
  GeometryType GetGeometry()
  {
    return m_Geometry;
  }
  /** Set Image geometry. **/
  void SetGeometry(GeometryType geometry)
  {
    m_Geometry = geometry;
  }

  /** Get Projection focal point. **/
  PointType GetFocalPoint()
  {
    return m_FocalPoint;
  }
  /** Set Projection focal point. **/
  void SetFocalPoint(PointType focalPoint)
  {
    m_FocalPoint = focalPoint;
  }

  /** Get Projection focal point. **/
  VectorType GetEyeLine()
  {
    return m_EyeLine;
  }
  /** Set Projection focal point. **/
  void SetEyeLine(VectorType eyeLine)
  {
    m_EyeLine = eyeLine;
  }

protected:
  /** Image file name **/
  std::string m_ImageFileName;
  /** Acquisition type **/
  std::string m_AcquisitionType;
  /** Acquisition date as string **/
  std::string m_AcquisitionDate;
  /** Typical DICOM patient position **/
  std::string m_PatientPosition;
  /** Referenced Frame of Reference **/
  FORPointer m_ReferencedFOR;
  /** Image position (in patient coordinate system) **/
  PointType m_ImagePosition;
  /** Image orientation (in patient coordinate system) **/
  MatrixType m_ImageOrientation;
  /** Image geometry **/
  GeometryType m_Geometry;
  /** Projection focal point **/
  PointType m_FocalPoint;
  /** Eye line **/
  VectorType m_EyeLine;

  /** Default constructor. **/
  ImageListEntry();
  /** Default destructor. **/
  ~ImageListEntry();

private:
  /** purposely not implemented **/
  ImageListEntry(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


/**
 * Builds up a list of available images.
 * @author phil 
 * @version 1.0
 */
class ImageList
  : public itk::Object
{
public:
  /** some relevant typedefs **/
  typedef ImageList Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  /** Image list map type **/
  typedef std::map<std::string, ImageListEntry::Pointer> ImageListMapType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageList, itk::Object);

  /** object information streaming **/
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

  /**
   * Create an image list from a typical open radART image list file.
   * @param imageListFile the open radART image list file
   * @param forColl FOR collection which is needed for Frame of Reference
   * detection
   * @return the created image list if successful, NULL otherwise
   * @see ora::ImageListEntry
   */
  static Pointer CreateFromFile(std::string imageListFile,
      FrameOfReferenceCollection::Pointer forColl);

  /**
   * Find a specified image list entry in the list.
   * @param fileName of the image list entry
   * @return the image list entry if found, NULL otherwise
   **/
  ImageListEntry::Pointer FindEntry(const std::string fileName);

  /**
   * Add an image list entry to the list.
   * @param imageListEntry the image list entry to be added
   * @return TRUE if the image list entry could be added
   */
  bool AddEntry(ImageListEntry::Pointer imageListEntry);

  /**
   * Remove an image list entry from the list.
   * @param imageListEntry the image list entry to be removed
   * @return TRUE if the entry could be removed
   */
  bool RemoveEntry(ImageListEntry::Pointer imageListEntry);

  /**
   * Remove an image list entry from the list.
   * @param fileName the file name of the entry to be removed
   * @return TRUE if the entry could be removed
   */
  bool RemoveEntry(std::string fileName);

  /**
   * Clear the complete list.
   */
  void Clear();

  /**
   * @return the file names of the contained image list entries.
   */
  std::vector<std::string> GetImageListEntries();

  /**
   * @return the direct pointer to the internal image list entry map
   */
  ImageListMapType *GetDirectImageListMap()
  {
    return &m_ImageListMap;
  }

protected:
  /**
   * internal FOR map holding the references and UIDs; the first component is
   * the image file name and the second component is a smart pointer to the
   * according image list entry in object-representation
   **/
  ImageListMapType m_ImageListMap;

  /** Default constructor. **/
  ImageList();
  /** Default destructor. **/
  ~ImageList();

private:
  /** purposely not implemented **/
  ImageList(const Self&);

  /** purposely not implemented **/
  void operator=(const Self&);

};


}


#endif /* ORAIMAGELIST_H_ */

