
#ifndef ORAVIEWCONTROLLER_H_
#define ORAVIEWCONTROLLER_H_


#include "oraObserver.h"

// ORAIFTools
#include "SimpleDebugger.h"

// Forward declarations
namespace ora
{
class Model;
}

namespace ora 
{


/**
 * Defines the structure for a combined view and controller. E.g. for simple
 * Qt integration together with a graphical UI designer.
 * @see Observer
 * @see SimpleDebugger
 * @author phil 
 * @version 1.0
 */
class ViewController
  : public Observer, public SimpleDebugger
{
public:

  /** Default constructor **/
  ViewController();

  /** Default destructor **/
  virtual ~ViewController();

  /** @see Observer#Update(int) **/
  virtual void Update(int id);

  /** Get the model reference. **/
  Model *GetModel()
  {
    return m_Model;
  }
  /** Set the model reference. **/
  void SetModel(Model *model)
  {
    m_Model = model;
  }

protected:
  /** Reference to the model. **/
  Model *m_Model;

};


}


#endif /* ORAVIEWCONTROLLER_H_ */
