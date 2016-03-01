/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pqt_database_h_
#define _pqt_database_h_

#include "plmqt_config.h"
#include <QString>
#include <QSqlQuery>

void
pqt_database_start (QString db_path);
void
pqt_database_stop (void);

QSqlQuery
pqt_database_query_data_source_label (void);
void
pqt_database_insert_data_source (QString label, QString host, 
    QString port, QString aet);
void
pqt_database_delete_data_source (QString label, QString host, 
    QString port, QString aet);

#endif
