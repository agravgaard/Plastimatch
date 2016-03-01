/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmqt_config.h"
#include <QtGui>
#include <QSqlQuery>
#include "pqt_database.h"
#include "pqt_data_source_list_model.h"

int 
Pqt_data_source_list_model::rowCount (
    const QModelIndex& parent
) const
{
    return this->m_num_rows;
}

QVariant 
Pqt_data_source_list_model::data (const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
	return QVariant();
    }
    if (index.row() >= this->m_num_rows) {
	return QVariant();
    }

    if (role != Qt::DisplayRole) {
	return QVariant();
    }

    this->m_query.seek (index.row());
    return this->m_query.value(0).toString();
}

void
Pqt_data_source_list_model::set_active_row (int index)
{
    this->m_query.seek (index);
}

QString
Pqt_data_source_list_model::get_label (void)
{
    return this->m_query.value(0).toString();
}

QString
Pqt_data_source_list_model::get_host (void)
{
    return this->m_query.value(1).toString();
}

QString
Pqt_data_source_list_model::get_port (void)
{
    return this->m_query.value(2).toString();
}

QString
Pqt_data_source_list_model::get_aet (void)
{
    return this->m_query.value(3).toString();
}

void
Pqt_data_source_list_model::load_query (void)
{
    /* Load data sources from database */
    this->m_query = pqt_database_query_data_source_label ();
    this->m_num_rows = 0;

    /* QSqlQuery::size returns -1 if not supported (sqlite).  
       So we manually count the rows. */
    while (this->m_query.next()) {
	this->m_num_rows ++;
    }

    /* Reset query back to first row */
    this->m_query.seek (-1);

    /* Refresh widget by resetting model (I think this is how it is 
       supposed to be done, can't find in documentation) */
    this->reset ();
}
