/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmqt_config.h"
#include <stdio.h>
#include <QProcess>
#include <QRegExp>
#include <QTextStream>
#include "pqt_findscu.h"

Pqt_findscu::~Pqt_findscu (void)
{
    while (!this->m_patient_list.isEmpty())
	delete this->m_patient_list.takeFirst();
}

void
Pqt_findscu::query (QString host, QString port, QString aet)
{
    QTextStream(stdout) 
	<< QString("Performing query: %1 %2 %3\n")
	.arg (host)
	.arg (port)
	.arg (aet);

    /* Execute findscu as a synchronous external process */
    QString program = "findscu";
    QStringList arguments;
    arguments << "-P" 
	      << "-k" << "0010,0010" 
	      << "-k" << "0010,0020" 
	      << "-k" << "0008,0052=PATIENT" 
	      << "-aec" << "READWRITE" << "localhost" << "5678";
    QProcess *process = new QProcess;
    process->start (program, arguments);
    if (!process->waitForStarted())
	return;
    if (!process->waitForFinished())
	return;

    /* Parse stdout from findscu, look for patient id, patient name, and 
       add to the list of patients */
    QByteArray result;
    char buf[4096];
    qint64 rc;
    QRegExp rx("\\(([^)]*)[^[]*\\[([^]]*)");
    Pqt_findscu_entry *fe = 0;
    while ((rc = process->readLine (buf, sizeof(buf))) != -1) {
	
	int match = rx.indexIn (buf);
	if (match == -1) {
	    continue;
	}
	/* Got dicom tag, value pair */
	QString tag = rx.cap(1);
	QString value = rx.cap(2);

	/* Check for patient name */
	if (!QString::compare (tag, "0010,0010")) {
	    fe = new Pqt_findscu_entry;
	    fe->m_patient_name = value;
	    continue;
	}
	if (!fe) continue;

	/* Check for patient id */
	if (!QString::compare (tag, "0010,0020")) {
	    fe->m_patient_id = value;
	    /* Last tag/value.  Add to list. */
	    this->m_patient_list << fe;
	    fe = 0;
	}
    }
    if (fe) delete fe;
}

void
Pqt_findscu::debug (void)
{
    for (int i = 0; i < this->m_patient_list.size(); ++i) {
	Pqt_findscu_entry *fe = this->m_patient_list.at(i);
	QTextStream(stdout) << QString("[%1] [%2] [%3]\n")
	    .arg (i)
	    .arg (fe->m_patient_id)
	    .arg (fe->m_patient_name);
    }
}
