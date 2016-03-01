/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pqt_main_window_h_
#define _pqt_main_window_h_

#include "plmqt_config.h"
#include "pqt_data_source_dialog.h"
#include "pqt_findscu.h"
#include "pqt_patient_list_model.h"
#include "ui_pqt_main_window.h"

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

class Pqt_main_window : public QMainWindow, private Ui::pqtMainWindow {
    Q_OBJECT
    ;

public:
    Pqt_main_window ();
    ~Pqt_main_window ();

    Pqt_data_source_dialog *m_data_source_dialog;

    Pqt_patient_list_model *m_patient_list_model;

    Pqt_findscu m_findscu;

public slots:
    void new_data_source (void);
};
#endif
