

#ifndef ORAQVTKPROGRESSEVENTADAPTOR_H_
#define ORAQVTKPROGRESSEVENTADAPTOR_H_

// ORAIFTools
#include "SimpleDebugger.h"

#include <QObject>

// Forward declarations
class vtkCommand;
class vtkCallbackCommand;
class vtkObject;


namespace ora
{


/**
 * Connects a VTK progress event (vtkCommand::ProgressEvent) with Qt signal/
 * slot mechanism.
 * This is achieved by providing a Qt Progress(double) signal that is emitted
 * when the according VTK progress event is received. The progress value is
 * expected in the VTK event's call data pointer.
 *
 * @author phil 
 * @version 1.0
 */
class QVTKProgressEventAdaptor
  : public QObject, public SimpleDebugger
{

  Q_OBJECT

public:
  /** Default constructor **/
  QVTKProgressEventAdaptor();
  /** Destructor **/
  ~QVTKProgressEventAdaptor();

  /**
   * Attach this connector to a specified VTK object and observe its progress
   * events.
   * @param object the VTK object to be observed
   * @return TRUE if successful
   **/
  bool Register(vtkObject *object);

  /** Detach this connector from current observed VTK object. **/
  void Unregister();

  /**
   * Another method for observation: prepare an internal callback command and
   * return the reference to it. This is an alternative to the Register()/
   * Unregister() methods.
   * @return pointer to the prepared callback command
   **/
  vtkCommand *GetPreparedCommand();

signals:
  /**
   * Progress update signal.
   * @param progress current progress value (usually in the range 0.0 .. 1.0)
   **/
  void Progress(double progress);

protected:
  /** Internal callback for receiving VTK events **/
  vtkCallbackCommand *m_CallbackCommand;
  /** Current observed VTK object **/
  vtkObject *m_CurrentVTKObject;
  /** Current observer tag for VTK object **/
  unsigned long m_CurrentObserverTag;

  /** Internal processor for VTK progress events. **/
  static void ProcessProgressEvents(vtkObject *object, unsigned long event,
    void *clientdata, void *calldata);

  /**
   * Trigger emission of the Qt progress signal.
   * @param progress the progress value (usually 0.0 .. 1.0)
   **/
  virtual void EmitProgressSignal(double progress);

};


}


#endif /* ORAQVTKPROGRESSEVENTADAPTOR_H_ */
