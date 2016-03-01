
#ifndef ORAOBSERVER_H_
#define ORAOBSERVER_H_


namespace ora 
{


/**
 * A simple class defining the necessary structure for the observer design
 * pattern.
 * @author phil 
 * @version 1.0
 */
class Observer
{

public:
  /** Default constructor **/
  Observer()
  {
    ;
  }

  /** Default constructor **/
  virtual ~Observer()
  {
    ;
  }

  /**
   * Update the content of the observer. This method must be overwritten in
   * concrete subclasses.
   * @param id unique identifier of content of interest which needs to be
   * considered
   */
  virtual void Update(int id) = 0;

};


}


#endif /* ORAOBSERVER_H_ */
