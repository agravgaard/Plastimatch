/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pqt_data_source_dialog_h_
#define _pqt_data_source_dialog_h_

#include "plmqt_config.h"
#include "ui_pqt_data_source_dialog.h"
#include "pqt_data_source_list_model.h"

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

class Pqt_data_source_dialog : public QDialog, private Ui::pqtDataSourceDialog {
    Q_OBJECT
    ;

public:
    Pqt_data_source_dialog ();
    ~Pqt_data_source_dialog ();

    Pqt_data_source_list_model *m_data_source_list_model;

public slots:
    void pushbutton_new_released (void);
    void pushbutton_save_released (void);
    void pushbutton_delete_released (void);
    void listview_data_source_activated (QModelIndex model_index);

public:
    int m_active_index;
    void refresh_from_database (void);
    void update_fields (void);
};
#endif
