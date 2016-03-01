/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmqt_config.h"
#include <QtGui>
#include "pqt_patient_list_model.h"

int 
Pqt_patient_list_model::rowCount (
    const QModelIndex& parent
) const
{
    return 2;
}

int 
Pqt_patient_list_model::columnCount (
    const QModelIndex& parent
) const
{
    return 3;
}

QVariant 
Pqt_patient_list_model::data (const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
	return QVariant();
    }
    if (index.row() >= 2) {
	return QVariant();
    }

    if (role == Qt::DisplayRole) {
	return QString ("String %1").arg(index.row());
    } else {
	return QVariant();
    }
}

QVariant 
Pqt_patient_list_model::headerData (
    int section, 
    Qt::Orientation orientation,
    int role
) const
{
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal)
         return QString("Column %1").arg(section);
     else
         return QString("Row %1").arg(section);
}
