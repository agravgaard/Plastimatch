/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _pqt_findscu_h_
#define _pqt_findscu_h_

#include "plmqt_config.h"
#include <QList>
#include <QString>

class Pqt_findscu_entry
{
public:
    QString m_patient_id;
    QString m_patient_name;
};

class Pqt_findscu
{
public:
    ~Pqt_findscu (void);

public:
    QList<Pqt_findscu_entry*> m_patient_list;

public:
    void query (QString host, QString port, QString aet);
    void debug (void);
};

#endif
