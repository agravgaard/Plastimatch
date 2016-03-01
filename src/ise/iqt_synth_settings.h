/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _iqt_synth_settings_h_
#define _iqt_synth_settings_h_

//#include "plm_config.h"
//#include "iqt_data_source_dialog.h"
//#include "iqt_findscu.h"
//#include "iqt_patient_list_model.h"
#include "ui_iqt_synth_settings.h"

class Iqt_main_window;

class Iqt_synth_settings : public QDialog, private Ui::iqtSynthSettings {
    Q_OBJECT
    ;

public:
    Iqt_synth_settings (QWidget *parent);
    ~Iqt_synth_settings ();

//    void render_sphere ();

    //Iqt_data_source_dialog *m_data_source_dialog;

    //Iqt_patient_list_model *m_patient_list_model;

    //Iqt_findscu m_findscu;
    int rows;
    int cols;
    double ampl;
    int mark;
    int fps;
    Iqt_main_window *mw;

public slots:
    void slot_default (void);
    void slot_proceed (void);
    void slot_cancel (void);
    void slot_attenuate (void);
    void slot_max_amplitude (int height);
    //void slot_settings (void);
};
#endif
