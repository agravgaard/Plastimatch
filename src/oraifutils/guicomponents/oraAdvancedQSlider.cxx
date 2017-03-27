//
#include "oraAdvancedQSlider.h"

#include <QMouseEvent>
#include <QToolTip>

namespace ora
{

AdvancedQSlider::AdvancedQSlider(QWidget *parent)
  : QSlider(parent)
{
  m_LastGlobalPos.setX(0);
  m_LastGlobalPos.setY(0);
  this->connect(this, SIGNAL(sliderMoved(int)), this, SLOT(OnSliderMoved(int)));
}

AdvancedQSlider::~AdvancedQSlider()
{
  this->disconnect(this, SIGNAL(sliderMoved(int)), this, SLOT(OnSliderMoved(int)));
}

void AdvancedQSlider::mouseDoubleClickEvent(QMouseEvent *me)
{
  emit DoubleClick();
}

void AdvancedQSlider::mousePressEvent(QMouseEvent *me)
{
  if (me)
  {
    m_LastGlobalPos.setX(me->globalPos().x());
    m_LastGlobalPos.setY(me->globalPos().y());
  }
  this->QSlider::mousePressEvent(me); // forward
  ShowInformationToolTip(this->value());
}

void AdvancedQSlider::mouseMoveEvent(QMouseEvent *me)
{
  if (me)
  {
    m_LastGlobalPos.setX(me->globalPos().x());
    m_LastGlobalPos.setY(me->globalPos().y());
  }
  this->QSlider::mouseMoveEvent(me); // forward
}

void AdvancedQSlider::ShowInformationToolTip(int currentValue)
{
  QString s = "";
  emit RequestSliderInformationToolTipText(s, currentValue); // request text
  if (s.length() > 0) // OK, filled
    QToolTip::showText(m_LastGlobalPos, s, this);
}

void AdvancedQSlider::OnSliderMoved(int value)
{
  ShowInformationToolTip(value);
}

}
