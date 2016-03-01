//
#ifndef ORAREG23TASKPRESENTATIONWIDGET_H
#define ORAREG23TASKPRESENTATIONWIDGET_H

#include <QtGui/QWidget>
#include <QVector>
#include "ui_oraREG23TaskPresentationWidget.h"

namespace ora 
{

class REG23TaskPresentationWidget : public QWidget
{
  Q_OBJECT

  /*
   TRANSLATOR ora::REG23TaskPresentationWidget

   lupdate: Qt-based translation with NAMESPACE-support!
   */

public:
  /** Default constructor. **/
  REG23TaskPresentationWidget(QWidget *parent = 0);
  /** Destructor **/
  ~REG23TaskPresentationWidget();

  /** Set widget's background color **/
  void SetBackgroundColor(QColor col);
  /** Get widget's background color **/
  QColor GetBackgroundColor()
  {
    return m_BackgroundColor;
  }

  /** Display the specified message on the message label. **/
  void ShowMessage(QString msg);
  /** Activate/deactivate (enable/disable) the cancel button. **/
  void ActivateCancelButton(bool active);
  /** @return whether the cancel button is currently enabled **/
  bool IsCancelButtonActivated();
  /** @return whether the start button is currently enabled **/
  bool IsStartButtonActivated();
  /** Activate/deactivate (enable/disable) the start button. **/
  void ActivateStartButton(bool active);
  /** Set the cancel button visible/invisible. **/
  void SetCancelButtonVisibility(bool visible);
  /** Set the start button visible/invisible. **/
  void SetStartButtonVisibility(bool visible);
  /** Set the progress label visible/invisible. **/
  void SetProgressLabelVisibility(bool visible);
  /** Set the progress bar visible/invisible. **/
  void SetProgressBarVisibility(bool visible);
  /** Activate/deactivate the progress bar. **/
  void ActivateProgressBar(bool active);
  /** Set the progress as percentage [0;100] **/
  void SetProgress(double p);

  /** Set tool tip text for cancel button. **/
  void SetCancelButtonToolTip(QString tip);
  /** Set tool tip text for start button. **/
  void SetStartButtonToolTip(QString tip);

  /** Imitate a click on the cancel button. **/
  void ClickCancelButton();
  /** Imitate a click on the start button. **/
  void ClickStartButton();

signals:
  /** Emitted on user cancel request (stop button). **/
  void UserCancelRequest();
  /** Emitted on user start request (play button). **/
  void UserStartRequest();

protected:
  /** Internal background color of widget **/
  QColor m_BackgroundColor;
  /** List of icons that replace the progress bar **/
  QVector<QPixmap *> m_Icons;

protected slots:
  void OnCancelButtonPressed();
  void OnPlayButtonPressed();

private:
  Ui::REG23TaskPresentationWidgetClass ui;
};

}

#endif // ORAREG23TASKPRESENTATIONWIDGET_H
