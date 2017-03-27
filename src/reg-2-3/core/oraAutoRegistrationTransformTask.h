//
#ifndef ORAAUTOREGISTRATIONTRANSFORMTASK_H_
#define ORAAUTOREGISTRATIONTRANSFORMTASK_H_

#include "oraAbstractTransformTask.h"

namespace ora
{

/** FIXME:
 *
 * @author phil 
 * @author Markus 
 * @version 1.1
 */
class AutoRegistrationTransformTask : public AbstractTransformTask
{
  Q_OBJECT

  /*
   TRANSLATOR ora::AutoRegistrationTransformTask

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  AutoRegistrationTransformTask();
  /** Destructor. **/
  virtual ~AutoRegistrationTransformTask();

  /** @see AbstractTransformTask#GetLogDescription() **/
  virtual std::string GetLogDescription();

  /** @see AbstractTransformTask#GetShortDescription() **/
  virtual std::string GetShortDescription();

  /** Get registration time in seconds **/
  double GetRegistrationTime();
  /** Set registration time in seconds **/
  void SetRegistrationTime(double regTime);

  /** Set registration canceled by user flag **/
  void SetUserCancel(bool cancel);
  /** Get registration canceled by user flag **/
  bool GetUserCancel();

  /** Set number of optimization iterations **/
  void SetNumberOfIterations(int num);
  /** Get number of optimization iterations **/
  int GetNumberOfIterations();

protected:
  /** Registration time in seconds **/
  double m_RegistrationTime;
  /** Registration canceled by user flag **/
  bool m_UserCancel;
  /** Number of optimization iterations **/
  int m_NumberOfIterations;

};

}


#endif /* ORAAUTOREGISTRATIONTRANSFORMTASK_H_ */
