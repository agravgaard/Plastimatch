/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _iqt_main_window_h_
#define _iqt_main_window_h_

//#include "plm_config.h"
//#include "iqt_data_source_dialog.h"
//#include "iqt_findscu.h"
//#include "iqt_patient_list_model.h"
#include "ui_iqt_main_window.h"

//QT_BEGIN_NAMESPACE
// class QAction;
// class QDialogButtonBox;
// class QGroupBox;
// class QLabel;
// class QLineEdit;
// class QMenu;
// class QMenuBar;
// class QPushButton;
// class QTextEdit;
//QT_END_NAMESPACE

class Iqt_main_window : public QMainWindow, private Ui::iqtMainWindow {
    Q_OBJECT
    ;

public:
    Iqt_main_window ();
    ~Iqt_main_window ();

   // void render_sphere ();

    //Iqt_data_source_dialog *m_data_source_dialog;

    //Iqt_patient_list_model *m_patient_list_model;

    //Iqt_findscu m_findscu;

    QTimer *m_qtimer;


public slots:
    void slot_load (void);
    void slot_timer (void);
};
#endif
