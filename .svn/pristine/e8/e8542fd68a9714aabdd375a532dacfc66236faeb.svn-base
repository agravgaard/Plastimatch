

/**
 *
 * Some simple macros for easy working.
 * @author phil 
 * @author Markus 
 * @version 1.3
 */


/**
 * Macro for standard setters of classes.
 */
#define SimpleSetter(member, _type) \
  /**
   * Set member of type _type.
   * @see m_##member for a more detailed description
   */ \
  virtual void Set##member(_type _arg_##member) \
  { \
    m_##member = _arg_##member;\
  }

/**
 * Macro for standard getters of classes.
 */
#define SimpleGetter(member, _type) \
  /**
   * Get member of type _type.
   * @see m_##member for a more detailed description
   */ \
  virtual _type Get##member() \
  { \
    return m_##member;\
  }


/**
 * Macro for standard setters of VTK-based-classes.
 */
#define SimpleVTKSetter(member, _type) \
  /**
   * Set member of type _type.
   * @see member for a more detailed description
   */ \
  virtual void Set##member(_type _arg_##member) \
  { \
    member = _arg_##member;\
  }

/**
 * Macro for standard getters of VTK-based-classes.
 */
#define SimpleVTKGetter(member, _type) \
  /**
   * Get member of type _type.
   * @see member for a more detailed description
   */ \
  virtual _type Get##member() \
  { \
    return member;\
  }

/**
 * Create a new VTK object and store a VTK smart pointer for a specified
 * VTK-type.
 * (Macro requires a class to include vtkSmartPointer.h)
 */
#define VSPNEW(instance, type) \
  vtkSmartPointer<type> instance = vtkSmartPointer<type>::New();

/**
 * Do a VTK smart pointer wrap for a specified object.
 * (Macro requires a class to include vtkSmartPointer.h)
 */
#define VSP(smartpointer, type) \
vtkSmartPointer<type> smartpointer

/** \def TEMPLATE_CALL_COMP(componentType, function, ...)
 * Calls a templated \function function based on the value of component
 * type \componentType componentType of the image.
 * See ora::ITKVTKImage::ITKComponentType for possible types.
 * A variable number of \... parameters can be passed to the function.
 * Return values can be catched by setting \function function to e.g.
 * 'retval = templateFunction' without quotes.
 */
#define TEMPLATE_CALL_COMP(componentType, function, ...) \
{\
    switch ( componentType ) \
    { \
      case itk::ImageIOBase::UCHAR: \
        function <unsigned char>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::CHAR: \
        function <char>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::USHORT: \
        function <unsigned short>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::SHORT: \
        function <short>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::UINT: \
        function <unsigned int>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::INT: \
        function <int>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::ULONG: \
        function <unsigned long>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::LONG: \
        function <long>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::FLOAT: \
        function <float>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::DOUBLE: \
        function <double>( __VA_ARGS__ ); \
        break; \
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE: \
      default: \
        std::cerr << "unknown component type" << std::endl; \
        break; \
    } \
}

/** \def ITKVTKIMAGE_DOWNCAST(componentType, baseImage, castImage)
 * Established a local down-cast from ora::ITKVTKImage::ITKImagePointer
 * to its original itk::Image type. This situation is usually found within
 * template functions that take the ITK component type as template parameter.
 * It is just a convenience macro for lazy developers.
 * NOTE: This method automatically defines 'OriginalImageType' and
 * 'OriginalImagePointer' which reprent the original ITK image type.
 */
#define ITKVTKIMAGE_DOWNCAST(componentType, baseImage, castImage) \
  typedef itk::Image<componentType, ITKVTKImage::Dimensions> OriginalImageType; \
  typedef typename OriginalImageType::Pointer OriginalImagePointer; \
  typename OriginalImageType::Pointer castImage = static_cast<OriginalImageType * >(baseImage.GetPointer());
