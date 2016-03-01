/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef __itkAndConstantToImageFilter_h
#define __itkAndConstantToImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

namespace itk
{
  
/* Thanks for broilerplate code:
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 * which is based on filters from the Insight Journal paper:
 * http://hdl.handle.net/1926/510
 */
namespace Functor {  
  
template< class TInput, class TConstant, class TOutput>
class AndConstantTo
{
public:
    AndConstantTo() : m_Constant(NumericTraits<TConstant>::One) {};
    ~AndConstantTo() {};
    bool operator!=( const AndConstantTo & other ) const
    {
	return !(*this == other);
    }
    bool operator==( const AndConstantTo & other ) const
    {
	return other.m_Constant == m_Constant;
    }
    inline TOutput operator()( const TInput & A ) const
    {
	// Because the user has to specify the constant we don't
	// check if the cte is not 0;
	return static_cast<TOutput>( ((A & m_Constant) > 0) );
    }
    void SetConstant(TConstant ct) {this->m_Constant = ct; }
    const TConstant & GetConstant() const { return m_Constant; }
  
    TConstant m_Constant;
};
}

template <class TInputImage, class TConstant, class TOutputImage>
class ITK_EXPORT AndConstantToImageFilter :
	public
UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                        Functor::AndConstantTo< 
			    typename TInputImage::PixelType, TConstant,
			    typename TOutputImage::PixelType> >
{
  public:
    /** Standard class typedefs. */
    typedef AndConstantToImageFilter                 Self;
    typedef UnaryFunctorImageFilter<
	TInputImage,TOutputImage, 
	Functor::AndConstantTo< 
	    typename TInputImage::PixelType, TConstant,
	    typename TOutputImage::PixelType>   >             Superclass;

    typedef SmartPointer<Self>                            Pointer;
    typedef SmartPointer<const Self>                      ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(AndConstantToImageFilter, UnaryFunctorImageFilter);

  
    /** Set the constant that will be used to multiply all the image
     * pixels */
    void SetConstant(TConstant ct)
    {
	if( ct != this->GetFunctor().GetConstant() )
	{
	    this->GetFunctor().SetConstant(ct);
	    this->Modified();
	}
    }
    const TConstant & GetConstant() const
    {
	return this->GetFunctor().GetConstant();
    }

#ifdef ITK_USE_CONCEPT_CHECKING
    /** Begin concept checking */
    itkConceptMacro(InputConvertibleToOutputCheck,
	(Concept::Convertible<typename TInputImage::PixelType,
			      typename TOutputImage::PixelType>));
    itkConceptMacro(Input1Input2OutputAndOperatorCheck,
	(Concept::AdditiveOperators<typename TInputImage::PixelType,
				    TConstant,
				    typename TOutputImage::PixelType>));
    /** End concept checking */
#endif

  protected:
    AndConstantToImageFilter() {};
    virtual ~AndConstantToImageFilter() {};
   
    void PrintSelf(std::ostream &os, Indent indent) const
    {
	Superclass::PrintSelf(os, indent);
	os << indent << "Constant: " 
	    << static_cast<typename NumericTraits<TConstant>::PrintType>(this->GetConstant())
	    << std::endl;
    }

  private:
    AndConstantToImageFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

};


} // end namespace itk

#endif
