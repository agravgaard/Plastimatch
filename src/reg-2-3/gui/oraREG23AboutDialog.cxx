/*
TRANSLATOR ora::REG23AboutDialog
*/

#include "oraREG23AboutDialog.h"

#include <QTimer>
#include <QMessageBox>

#include <sstream>

#include "reg23info.h"

namespace ora
{


REG23AboutDialog::REG23AboutDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	m_AnimationTimer = NULL;
	m_CurrentDirection = 0;
	m_CurrentPercentage = 0;
	Initialize();
}

REG23AboutDialog::~REG23AboutDialog()
{
  if (m_AnimationTimer)
    delete m_AnimationTimer;
  m_AnimationTimer = NULL;
}

void REG23AboutDialog::Initialize()
{
  m_AnimationTimer = new QTimer(this);
  m_AnimationTimer->setInterval(40);
  m_AnimationTimer->start();

  this->connect(m_AnimationTimer, SIGNAL(timeout()),
      this, SLOT(OnAnimationTimerTimeout()));

  this->setWindowTitle(REG23AboutDialog::tr("About ..."));

  QString text = "";
  text += "<h2><b>";
  text += QString::fromStdString(ora::REG23GlobalInformation::GetCombinedApplicationName());
  text += "</b></h2><h3>";
  text += QString::fromStdString(ora::REG23GlobalInformation::GetCombinedVersionString());
  text += "</h3><br><br>";
  std::vector<ora::REG23GlobalInformation::AuthorInformation> authors =
      ora::REG23GlobalInformation::GetAuthorsInformation();

  text += "<br><b>" + REG23AboutDialog::tr("Authors:") + "</b><br><ul>";
  for (std::size_t i = 0; i < authors.size(); i++)
  {
    text += "<li><b>" + QString::fromStdString(authors[i].Name) + "<br></b>";
    text += "(" + QString::fromStdString(authors[i].Contribution) + ")<br>";
  }
  text += "</ul>";
  ui.ContentLabel->setText(text);

  std::ostringstream os;
  os << "Copyright (c) 2010-2011, Philipp Steininger, Markus Neuner, Heinz Deutschmann" << std::endl;
  os << "All rights reserved." << std::endl;
  os << "" << std::endl;
  os << "Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:" << std::endl;
  os << "  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer." << std::endl;
  os << "  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution." << std::endl;
  os << "  * Neither the name of Institute for Research and Development on Advanced Radiation Technologies (radART), Paracelsus Medical University (PMU), Austria, nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission." << std::endl;
  os << "" << std::endl;
  os << "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." << std::endl;
  os << "" << std::endl;
  os << "The views and conclusions contained in the software and documentation are those of the authors and should not be interpreted as representing official policies, either expressed or implied, of Philipp Steininger, Markus Neuner, Heinz Deutschmann." << std::endl;
  ui.TextBrowser->setText(QString::fromStdString(os.str()));

  REG23TaskPresentationWidget *trw = ui.AnimationWidget;
  trw->ActivateProgressBar(true);
  trw->SetCancelButtonVisibility(false);
  trw->SetStartButtonVisibility(false);
  trw->SetProgressLabelVisibility(false);
  trw->SetProgressBarVisibility(false);

  this->connect(ui.CloseButton, SIGNAL(pressed()), this, SLOT(accept()));
}

void REG23AboutDialog::OnAnimationTimerTimeout()
{
  if (m_CurrentDirection == 1)
  {
    m_CurrentPercentage += 1;
    if (m_CurrentPercentage > 100)
    {
      m_CurrentPercentage = 100;
      m_CurrentDirection = -1;
    }
  }
  else if (m_CurrentDirection == -1)
  {
    m_CurrentPercentage -= 1;
    if (m_CurrentPercentage < 0)
    {
      m_CurrentPercentage = 0;
      m_CurrentDirection = 1;
    }
  }
  else // init
  {
    m_CurrentPercentage = 0;
    m_CurrentDirection = 1;
  }

  REG23TaskPresentationWidget *trw = ui.AnimationWidget;
  trw->SetProgress(m_CurrentPercentage);
  trw->repaint();
  trw->update();
  this->repaint();
  this->update();
}

}
