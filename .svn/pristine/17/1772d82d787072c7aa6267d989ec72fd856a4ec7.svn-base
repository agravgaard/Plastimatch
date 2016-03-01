/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pqt_data_source_list_model_h_
#define _pqt_data_source_list_model_h_

#include "plmqt_config.h"
#include <QSqlQuery>
#include "ui_pqt_main_window.h"

class Pqt_data_source_list_model : public QAbstractListModel {
    Q_OBJECT
    ;

public:
    Pqt_data_source_list_model (QObject *parent = 0)
	: QAbstractListModel (parent) { load_query (); }
    ~Pqt_data_source_list_model () {}

    /* Overrides from base class */
    int rowCount (const QModelIndex& parent = QModelIndex()) const;
    QVariant data (const QModelIndex& index, int role) const;

    /* Other methods */
    void load_query (void);
    void set_active_row (int index);
    QString get_label (void);
    QString get_host (void);
    QString get_port (void);
    QString get_aet (void);

public:
    mutable QSqlQuery m_query;
    int m_num_rows;
};

#endif
