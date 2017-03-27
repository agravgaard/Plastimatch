//
#ifndef ORASPARSEAUTOREGISTRATIONTRANSFORMTASK_H_
#define ORASPARSEAUTOREGISTRATIONTRANSFORMTASK_H_

#include "oraAutoRegistrationTransformTask.h"

namespace ora
{

/** FIXME:
 *
 * @see ora::AutoRegistrationTransformTask
 *
 * @author phil 
 * @author Markus 
 * @version 1.1
 */
class SparseAutoRegistrationTransformTask : public AutoRegistrationTransformTask
{
  Q_OBJECT

  /*
   TRANSLATOR ora::SparseAutoRegistrationTransformTask

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Enumeration describing the type of sparse auto-registration. **/
  typedef enum
  {
    RT_UNKNOWN = 0,
    RT_CROSS_CORRELATION = 1
  } RegistrationType;

  /** Default constructor. **/
  SparseAutoRegistrationTransformTask();
  /** Destructor. **/
  virtual ~SparseAutoRegistrationTransformTask();

  /** @see AbstractTransformTask#GetLogDescription() **/
  virtual std::string GetLogDescription();

  /** @see AbstractTransformTask#GetShortDescription() **/
  virtual std::string GetShortDescription();

  /** Set the registration type. **/
  virtual void SetRegistrationType(RegistrationType regType);
  /** Get the registration type. **/
  virtual RegistrationType GetRegistrationType();

protected:
  /** Registration type. @see RegistrationType **/
  RegistrationType m_RegistrationType;

};

}


#endif /* ORASPARSEAUTOREGISTRATIONTRANSFORMTASK_H_ */
