/*
 TRANSLATOR ora::REG23TaskPresentationWidget

 lupdate: Qt-based translation with NAMESPACE-support!
 */
#include "oraREG23TaskPresentationWidget.h"

#include <QPalette>

#include <itkMath.h>

#include <math.h>

namespace ora 
{

REG23TaskPresentationWidget::REG23TaskPresentationWidget(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);
  SetBackgroundColor(QColor(0xf8, 0xf3, 0xcc));
  ui.MessageLabel->setText("");

  // load pixmaps from resource:
  QString basefn = ":u23tpw2/img/hand-64x64-";
  QString ext = ".png";
  QString num = "";
  for (int i = 0; i < 30; i++)
  {
    num = QString::number(i, 10);
    if (num.length() < 2)
      num = "0" + num;
    QPixmap *pm = new QPixmap(basefn + num + ext);
    m_Icons.push_back(pm);
  }
  ui.IconLabel->setEnabled(false);

  ui.ProgressBar->setEnabled(false);

  SetProgress(0);

  ui.CancelButton->setEnabled(false);
  ui.PlayButton->setEnabled(false);
  this->connect(ui.CancelButton, SIGNAL(pressed()), this,
                SLOT(OnCancelButtonPressed()));
  this->connect(ui.PlayButton, SIGNAL(pressed()), this,
                SLOT(OnPlayButtonPressed()));
}

REG23TaskPresentationWidget::~REG23TaskPresentationWidget()
{
  this->disconnect(ui.CancelButton, SIGNAL(pressed()), this,
                   SLOT(OnCancelButtonPressed()));
  this->disconnect(ui.PlayButton, SIGNAL(pressed()), this,
                   SLOT(OnPlayButtonPressed()));
  for (int i = 0; i < m_Icons.size(); i++)
    delete m_Icons[i];
  m_Icons.clear();
}

void REG23TaskPresentationWidget::SetBackgroundColor(QColor col)
{
  m_BackgroundColor = col;
  QPalette pal = this->palette();
  pal.setColor(QPalette::Background, m_BackgroundColor);
  this->setPalette(pal);
}

void REG23TaskPresentationWidget::ShowMessage(QString msg)
{
  ui.MessageLabel->setText(msg);
}

void REG23TaskPresentationWidget::ActivateCancelButton(bool active)
{
  ui.CancelButton->setEnabled(active);
}

void REG23TaskPresentationWidget::ActivateStartButton(bool active)
{
  ui.PlayButton->setEnabled(active);
}

void REG23TaskPresentationWidget::SetCancelButtonVisibility(bool visible)
{
  ui.CancelButton->setVisible(visible);
}

void REG23TaskPresentationWidget::SetStartButtonVisibility(bool visible)
{
  ui.PlayButton->setVisible(visible);
}

void REG23TaskPresentationWidget::SetProgressLabelVisibility(bool visible)
{
  ui.ProgressBar->setEnabled(visible);
}

void REG23TaskPresentationWidget::SetProgressBarVisibility(bool visible)
{
  ui.ProgressBar->setVisible(visible);
}

void REG23TaskPresentationWidget::ActivateProgressBar(bool active)
{
  ui.IconLabel->setEnabled(active);
  ui.ProgressBar->setEnabled(active);
}

void REG23TaskPresentationWidget::SetProgress(double p)
{
  if (p < 0)
    p = 0;
  if (p > 100)
    p = 100;
  ui.ProgressBar->setValue(p);

  int i = itk::Math::Round<int, double>(p / 100. * (double)(m_Icons.size() - 1));
  ui.IconLabel->setPixmap(*m_Icons[i]);
}

void REG23TaskPresentationWidget::OnCancelButtonPressed()
{
  emit UserCancelRequest();
}

void REG23TaskPresentationWidget::OnPlayButtonPressed()
{
  emit UserStartRequest();
}

void REG23TaskPresentationWidget::ClickCancelButton()
{
  if (ui.CancelButton->isEnabled())
    ui.CancelButton->click();
}

void REG23TaskPresentationWidget::ClickStartButton()
{
  if (ui.PlayButton->isEnabled())
    ui.PlayButton->click();
}


void REG23TaskPresentationWidget::SetCancelButtonToolTip(QString tip)
{
  ui.CancelButton->setToolTip(tip);
}

void REG23TaskPresentationWidget::SetStartButtonToolTip(QString tip)
{
  ui.PlayButton->setToolTip(tip);
}

bool REG23TaskPresentationWidget::IsCancelButtonActivated()
{
  return ui.CancelButton->isEnabled();
}

bool REG23TaskPresentationWidget::IsStartButtonActivated()
{
  return ui.PlayButton->isEnabled();
}

}
