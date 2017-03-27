//
#ifndef ORAADVANCEDQSLIDER_H_
#define ORAADVANCEDQSLIDER_H_

#include <QSlider>
#include <QPoint>

namespace ora
{

/**
 * Extension of the QSlider widget.
 * In addition, this class supports:<br>
 * - a double-click signal,<br>
 * - an information tool tip which is displayed during sliding (the
 * RequestSliderInformationToolTipText()-signal must be listened for, and the
 * text-argument must be filled adequately with the desired display text)
 *
 * @author phil 
 * @version 1.0
 */
class AdvancedQSlider : public QSlider
{
  Q_OBJECT

public:
  /** Default constructor **/
  AdvancedQSlider(QWidget *parent = 0);
  /** Destructor **/
  virtual ~AdvancedQSlider();

signals:
  /** A double-click occurred on the slider. **/
  void DoubleClick();
  /** The information tool tip is about to be displayed. If the text argument is
   * filled (non-empty), the filled text will be displayed as information tool
   * tip. In addition the current value is provided. **/
  void RequestSliderInformationToolTipText(QString &text, int value);

protected slots:
  /** Listen for sliderMoved()-events of this. **/
  void OnSliderMoved(int value);

protected:
  /** Stores last global position of cursor. **/
  QPoint m_LastGlobalPos;

  /** Internal double-click event. **/
  virtual void mouseDoubleClickEvent(QMouseEvent *me);
  /** Internal mouse-press event. **/
  virtual void mousePressEvent(QMouseEvent *me);
  /** Internal mouse-move event. **/
  virtual void mouseMoveEvent(QMouseEvent *me);

  /** Shows the customizable "information" tool tip during slider change. **/
  virtual void ShowInformationToolTip(int currentValue);

};


}


#endif /* ORAADVANCEDQSLIDER_H_ */
