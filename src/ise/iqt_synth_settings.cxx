/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <QtGui>
#include <QTimer>
#include <QFileDialog>
#include <QDir>
#include "iqt_video_widget.h"
#include "iqt_application.h"
#include "iqt_main_window.h"
#include "iqt_synth_settings.h"

Iqt_synth_settings::Iqt_synth_settings (QWidget *parent)
    : QDialog (parent)
{
    /* Set up GUI */
    setupUi (this);
    this->mw = (Iqt_main_window*) parent;
}

Iqt_synth_settings::~Iqt_synth_settings ()
{
       
}

void
Iqt_synth_settings::slot_cancel ()
{
    this->close();
}

void
Iqt_synth_settings::slot_proceed ()
{
    rows = rowBox->value();
    cols = colBox->value();
    ampl = ampBox->value();
    mark = markBox->value();
    fps = noiseBox->value();

 //   mw->synth = true; //bool in main window so slot_stop checks if synth is running  //deleted by YK
    ise_app->set_synthetic_source (mw, rows, cols, ampl, mark, fps);
    this->close();
}

void
Iqt_synth_settings::slot_default ()
{
    rowBox->setValue(768);
    colBox->setValue(1280);
    attenuate->setChecked(false);
    ampBox->setValue(1);
    markBox->setValue(0);
    noiseBox->setValue(20);
}

void
Iqt_synth_settings::slot_attenuate ()
{   

    double amp = ampBox->value();
    int height = rowBox->value();
    if (attenuate->isChecked())
    {
        ampBox->setRange(0.1, 1.0);
        ampBox->setSingleStep(0.1);
        ampBox->setDecimals(1);
        ampBox->setValue(amp/10.);
    } else {
        ampBox->setRange(1., (double)height/100);
        ampBox->setSingleStep(1.);
        ampBox->setDecimals(0);
        ampBox->setValue(amp*10.);
    }
}

void
Iqt_synth_settings::slot_max_amplitude (int height)
{
    ampBox->setMaximum((double)height/100);
}
