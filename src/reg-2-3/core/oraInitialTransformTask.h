//
#ifndef ORAINITIALTRANSFORMTASK_H_
#define ORAINITIALTRANSFORMTASK_H_

#include "oraAbstractTransformTask.h"

namespace ora
{

/** FIXME:
 * CANNOT BE UNDONE!
 *
 * @author phil 
 * @author Markus 
 * @version 1.1
 */
class InitialTransformTask : public AbstractTransformTask
{
  Q_OBJECT

  /*
   TRANSLATOR ora::InitialTransformTask

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  InitialTransformTask();
  /** Destructor. **/
  virtual ~InitialTransformTask();

  /** @see AbstractTransformTask#GetLogDescription() **/
  virtual std::string GetLogDescription();

  /** @see AbstractTransformTask#GetShortDescription() **/
  virtual std::string GetShortDescription();

};

}


#endif /* ORAINITIALTRANSFORMTASK_H_ */
