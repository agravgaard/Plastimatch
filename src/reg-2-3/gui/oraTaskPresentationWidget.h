//
#ifndef ORATASKPRESENTATIONWIDGET_H
#define ORATASKPRESENTATIONWIDGET_H

#include <QtGui/QWidget>
#include <QColor>
#include "ui_oraTaskPresentationWidget.h"

namespace ora 
{

class TaskPresentationWidget : public QWidget
{
  Q_OBJECT
  /*
   TRANSLATOR ora::TaskPresentationWidget

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  TaskPresentationWidget(QWidget *parent = 0);
  /** Destructor **/
  ~TaskPresentationWidget();

  /** Set widget's background color **/
  void SetBackgroundColor(QColor col);
  /** Get widget's background color **/
  QColor GetBackgroundColor()
  {
    return m_BackgroundColor;
  }

  /** Display the specified message on the message label. **/
  void ShowMessage(QString msg);
  /** Activate/deactivate the cancel button. **/
  void ActivateCancelButton(bool active);
  /** Activate/deactivate the progress bar. **/
  void ActivateProgressBar(bool active);
  /** Set the progress as percentage [0;100] **/
  void SetProgress(double p);

signals:
  /** Emitted on user cancel requests. **/
  void UserCancelRequest();

protected:
  /** Internal background color of widget **/
  QColor m_BackgroundColor;

protected slots:
  void OnCancelButtonPressed();

private:
  Ui::TaskPresentationWidgetClass ui;
};

}

#endif // ORATASKPRESENTATIONWIDGET_H
