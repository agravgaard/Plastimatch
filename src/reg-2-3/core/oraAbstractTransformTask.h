//
#ifndef ORAABSTRACTTRANSFORMTASK_H_
#define ORAABSTRACTTRANSFORMTASK_H_

#include <QObject>

#include <itkArray.h>
#include <itkPoint.h>

namespace ora
{

/** FIXME:
 *
 * @author phil 
 * @version 1.0
 */
class AbstractTransformTask : public QObject
{
  Q_OBJECT

  /*
   TRANSLATOR ora::AbstractTransformTask

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Transformation parameters type. **/
  typedef itk::Array<double> ParametersType;
  /** 3D point type **/
  typedef itk::Point<double, 3> Point3DType;

  /** Default constructor. **/
  AbstractTransformTask();
  /** Destructor. **/
  virtual ~AbstractTransformTask();

  /** Set transformation parameters applied by this task. **/
  void SetParameters(ParametersType pars);
  /** Get transformation parameters applied by this task. **/
  ParametersType GetParameters();
  /** Get the realtive transformation parameters emerging from stack position. **/
  ParametersType GetRelativeParameters();
  /** @return true if the current relative parameters imply a notable
   * transformation w.r.t. to the specified EPSILON **/
  bool ImpliesRelativeTransformation(const double EPSILON = 1e-3);

  /** Information string that is logged in manager's log list whenever this
   * task is reported, undone or redone (description should contain the
   * parameters and time stamp and further information).
   * @see ConvertParametersToString() Must be implemented in subclasses!
   * @see GetTimeStamp() **/
  virtual std::string GetLogDescription() = 0;

  /** Short information string that is used for GUI visualization. It should
   * be clear and short (userfriendly and readable). Must be implemented in
   * subclasses! **/
  virtual std::string GetShortDescription() = 0;

  /** Convert task parameters into string representation. **/
  virtual std::string ConvertParametersToString();

  /** Get time stamp of task creation (usually when the real task finished). **/
  std::string GetTimeStamp();

  /** Compute the relative transform parameters compared to the specified task
   * and store it in m_RelativeParameters.
   * @return true if the relative parameters could be computed **/
  bool ComputeRelativeTransform(AbstractTransformTask *task);

protected:
  /** Transformation parameters applied by this task. **/
  ParametersType m_Parameters;
  /** Relative transformation parameters compared to previous task. **/
  ParametersType m_RelativeParameters;
  /** Time stamp of task creation (usually when the real task finished). **/
  std::string m_TimeStamp;

};

}

#endif /* ORAABSTRACTTRANSFORMTASK_H_ */
