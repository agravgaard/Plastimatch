//
#ifndef ORAREFERENCETRANSFORMTASK_H_
#define ORAREFERENCETRANSFORMTASK_H_

#include "oraAbstractTransformTask.h"

namespace ora
{

/** FIXME:
 *
 * @author phil 
 * @author Markus 
 * @version 1.1
 */
class ReferenceTransformTask : public AbstractTransformTask
{
  Q_OBJECT

  /*
   TRANSLATOR ora::ReferenceTransformTask

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  ReferenceTransformTask();
  /** Destructor. **/
  virtual ~ReferenceTransformTask();

  /** @see AbstractTransformTask#GetLogDescription() **/
  virtual std::string GetLogDescription();

  /** @see AbstractTransformTask#GetShortDescription() **/
  virtual std::string GetShortDescription();

};

}

#endif /* ORAREFERENCETRANSFORMTASK_H_ */
