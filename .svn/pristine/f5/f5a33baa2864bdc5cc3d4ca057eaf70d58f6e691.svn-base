//
#ifndef ORAIMAGECONSUMER_H_
#define ORAIMAGECONSUMER_H_

#include "oraITKVTKImage.h"

namespace ora
{

/** \class ImageConsumer
 * \brief A simple wrapper around an ITK/VTK image which offers a standard interface.
 *
 * A simple wrapper around an ITK/VTK image which offers a standard consumer
 * interface. In fact, this class simply holds a pointer to an ITK / VTK image.
 *
 * Classes benefiting from this class could replace image member variables by
 * ImageConsumer instances.
 *
 * @author phil 
 * @version 1.0
 */
class ImageConsumer
{
public:
  /** Image type **/
  typedef ITKVTKImage ImageType;

  /** Default constructor **/
  ImageConsumer();
  /**
   * Extended constructor.
   * @param deleteImageAtEnd delete-image-at-end flag
   * @see m_DeleteImageAtEnd
   **/
  ImageConsumer(bool deleteImageAtEnd);
  /** Destructor **/
  ~ImageConsumer();

  /** Set image to be encapsulated. **/
  void ConsumeImage(ImageType *image);
  /**
   * Unset encapsulated image (NULL). NOTE: if DeleteImageAtEnd is set to TRUE,
   * the image will be destroyed!
   **/
  void UnconsumeImage();
  /** Get encapsulated image reference. **/
  ImageType *ProduceImage();

  /**
   * Set flag indicating that the associated image should be destroyed at object
   * destruction.
   **/
  void SetDeleteImageAtEnd(bool flag)
  {
    m_DeleteImageAtEnd = flag;
  }
  /**
   * Get flag indicating that the associated image should be destroyed at object
   * destruction.
   **/
  bool GetDeleteImageAtEnd()
  {
    return m_DeleteImageAtEnd;
  }

protected:
  /**
   * Wrapped ITK/VTK image, FIXME: remove pixel type later, implement forward
   * declaration
   */
  ImageType *m_Image;
  /**
   * Flag indicating that the associated image should be destroyed at object
   * destruction.
   **/
  bool m_DeleteImageAtEnd;

};

}


#endif /* ORAIMAGECONSUMER_H_ */
