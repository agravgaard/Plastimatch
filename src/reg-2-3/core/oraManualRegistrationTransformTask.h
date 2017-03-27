//
#ifndef ORAMANUALREGISTRATIONTRANSFORMTASK_H_
#define ORAMANUALREGISTRATIONTRANSFORMTASK_H_

#include "oraAbstractTransformTask.h"

namespace ora
{

/** FIXME:
 *
 * @author phil 
 * @author Markus 
 * @version 1.1
 */
class ManualRegistrationTransformTask : public AbstractTransformTask
{
  Q_OBJECT

  /*
   TRANSLATOR ora::ManualRegistrationTransformTask

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  ManualRegistrationTransformTask();
  /** Destructor. **/
  virtual ~ManualRegistrationTransformTask();

  /** @see AbstractTransformTask#GetLogDescription() **/
  virtual std::string GetLogDescription();

  /** @see AbstractTransformTask#GetShortDescription() **/
  virtual std::string GetShortDescription();

  /** Get registration transform type: rotation or translation **/
  bool GetIsRotation();
  /** Set registration transform type: rotation or translation **/
  void SetIsRotation(bool rotation);

protected:
  /** Registration transform type: rotation or translation **/
  bool m_IsRotation;

};

}


#endif /* ORAMANUALREGISTRATIONTRANSFORMTASK_H_ */
