

#ifndef ORAMODEL_H_
#define ORAMODEL_H_


#include <vector>

// Forward declarations
namespace ora
{
class Observer;
}

namespace ora 
{


/**
 * Defines the structure for a typical MVC model component. It is expected to
 * interact with view/controller components.
 * @author phil 
 * @version 1.0
 */
class Model
{
public:
  /** Basic component types **/
  typedef std::vector<Observer *> ObserverVectorType;
  typedef ObserverVectorType * ObserverVectorPointer;

  /**
   * Construct new model instance (default).
   */
  Model();

  /**
    * Destroy current model instance.
    */
  virtual ~Model();

  /**
   * Register an observer. This observer is notified each time the model
   * changes (PUSH-model).
   * @param observer observer to be registered
   */
  virtual void Register(Observer *observer);

  /**
   * Unregister an observer. This observer is no longer notified at model-
   * changes.
   * @param observer observer to be unregistered
   */
  virtual void Unregister(Observer *observer);


protected:
  /**
   * List of objects implementing the Observer design pattern
   * (PUSH-model)
   */
  ObserverVectorPointer m_Observers;

  /**
   * Notify all registered observers (PUSH-model).
   * @param id identifier of content of interest which needs to be considered
   * by observers during their update-processes
   */
  virtual void Notify(int id);

};


}


#endif /* ORAMODEL_H_ */

