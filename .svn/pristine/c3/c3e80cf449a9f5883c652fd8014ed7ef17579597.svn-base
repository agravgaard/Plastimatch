/*
 TRANSLATOR ora::TaskPresentationWidget

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraTaskPresentationWidget.h"

#include <QPalette>

namespace ora 
{

TaskPresentationWidget::TaskPresentationWidget(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);
  SetBackgroundColor(QColor(0xf8, 0xf3, 0xcc));
  ui.MessageLabel->setText("");
  ui.ProgressBar->setEnabled(false);
  ui.ProgressBar->setTextVisible(false);
  ui.ProgressBar->setValue(0);
  ui.CancelButton->setEnabled(false);
  this->connect(ui.CancelButton, SIGNAL(pressed()), this,
                SLOT(OnCancelButtonPressed()));
}

TaskPresentationWidget::~TaskPresentationWidget()
{
  this->disconnect(ui.CancelButton, SIGNAL(pressed()), this,
                   SLOT(OnCancelButtonPressed()));
}

void TaskPresentationWidget::SetBackgroundColor(QColor col)
{
  m_BackgroundColor = col;
  QPalette pal = this->palette();
  pal.setColor(QPalette::Background, m_BackgroundColor);
  this->setPalette(pal);
}

void TaskPresentationWidget::ShowMessage(QString msg)
{
  ui.MessageLabel->setText(msg);
}

void TaskPresentationWidget::ActivateCancelButton(bool active)
{
  ui.CancelButton->setEnabled(active);
}

void TaskPresentationWidget::ActivateProgressBar(bool active)
{
  ui.ProgressBar->setEnabled(active);
  ui.ProgressBar->setTextVisible(active);
}

void TaskPresentationWidget::SetProgress(double p)
{
  if (p < 0)
    p = 0;
  if (p > 100)
    p = 100;
  ui.ProgressBar->setValue((int)p);
}

void TaskPresentationWidget::OnCancelButtonPressed()
{
  emit UserCancelRequest();
}

}
