//
#ifndef ORAREG23ABOUTDIALOG_H
#define ORAREG23ABOUTDIALOG_H

#include <QtGui/QDialog>
#include "ui_oraREG23AboutDialog.h"

class QTimer;

namespace ora
{

class REG23AboutDialog : public QDialog
{
Q_OBJECT

/*
TRANSLATOR ora::REG23AboutDialog
*/

public:
  /** Default constructor **/
  REG23AboutDialog(QWidget *parent = 0);
  /** Destructor **/
  ~REG23AboutDialog();

protected:
  /** Timer for controlling the hand-animation. **/
  QTimer *m_AnimationTimer;
  /** Percentage for animation [0;100] **/
  double m_CurrentPercentage;
  /** Current direction of animation **/
  int m_CurrentDirection;

  /** Initialize the GUI components (content) **/
  void Initialize();

protected slots:
  void OnAnimationTimerTimeout();

private:
    Ui::REG23AboutDialogClass ui;
};

}

#endif // ORAREG23ABOUTDIALOG_H
