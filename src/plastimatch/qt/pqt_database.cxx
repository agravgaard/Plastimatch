/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmqt_config.h"
#include <stdio.h>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QVariant>

/* Use global variable for database handle */
static QSqlDatabase global_db;

static void
print_database_error (QSqlError sql_error)
{
    QMessageBox::information (0, QString ("Database error"),
	QString ("Database error: %1, %2, %3")
	.arg(sql_error.type())
	.arg(sql_error.number())
	.arg(sql_error.text()));
}

void
pqt_database_start (QString db_path)
{
    /* Make parent directory for database file if it doesn't exist */
    QDir().mkpath(QFileInfo(db_path).absolutePath());

    /* Open the sqlite database file. */
    global_db = QSqlDatabase::addDatabase ("QSQLITE");
    global_db.setDatabaseName (db_path);
    bool ok = global_db.open ();
    if (!ok) {
	print_database_error (global_db.lastError());
	return;
    }

    QSqlQuery query;
    QString sql;

    /* Check database for version upgrade */
    sql = 
	"CREATE TABLE IF NOT EXISTS "
	"pqt_application_version ( "
	"  version TEXT "
	");";
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }
    sql = 
	"SELECT version FROM pqt_application_version;";
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }
    if (query.next ()) {
	QString version_string = query.value(0).toString();
#if defined (commentout)
	QMessageBox::information (0, QString ("Version string"),
	    QString ("PQT database version = %1").arg(version_string));
#endif
    } else {
	/* New database.  Add version string. */
	sql = 
	    "INSERT INTO pqt_application_version values ('Experimental');";
	if (!query.exec (sql)) {
	    print_database_error (query.lastError());
	    return;
	}
    }

    /* Create tables if they don't exist */
    sql = 
	"CREATE TABLE IF NOT EXISTS "
	"data_source ( "
	"  oi INTEGER PRIMARY KEY, "
	"  label TEXT, "
	"  type TEXT, "
	"  host TEXT, "
	"  port TEXT, "
	"  aet TEXT "
	");";
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }

    sql = 
	"CREATE TABLE IF NOT EXISTS "
	"data_source_dicom ( "
	"  oi INTEGER PRIMARY KEY, "
	"  host TEXT, "
	"  port TEXT, "
	"  aet TEXT "
	");";
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }

    sql = 
	"CREATE TABLE IF NOT EXISTS "
	"data_source_directory ( "
	"  oi INTEGER PRIMARY KEY, "
	"  directory TEXT "
	");";
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }
}

void
pqt_database_stop (void)
{
    printf ("Closing databse\n");
    global_db.close ();
    printf ("Done closing databse\n");
}

QSqlQuery
pqt_database_query_data_source_label (void)
{
    QString sql = 
	"SELECT label,host,port,aet FROM data_source ORDER BY label;";
    QSqlQuery query = QSqlQuery (sql);

#if defined (commentout)
    while (query.next ()) {
	QString label_string = query.value(0).toString();
	QTextStream(stdout) << QString ("label = %1\n").arg(label_string);
    }
    query.seek (-1);
#endif

    return query;
}

void
pqt_database_insert_data_source (QString label, QString host, 
    QString port, QString aet)
{
    QString sql = QString (
	"INSERT INTO data_source "
	"(label, type, host, port, aet) "
	"values ('%1', '', '%2', '%3', '%4');")
	.arg (label)
	.arg (host)
	.arg (port)
	.arg (aet);

    QSqlQuery query;
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }
}

void
pqt_database_delete_data_source (QString label, QString host, 
    QString port, QString aet)
{
    QString sql = QString (
	"DELETE FROM data_source WHERE "
	"label = \"%1\" AND host = \"%2\" AND port = \"%3\" AND aet = \"%4\";")
	.arg (label)
	.arg (host)
	.arg (port)
	.arg (aet);
    
    QTextStream(stdout) << sql << "\n";

    QSqlQuery query;
    if (!query.exec (sql)) {
	print_database_error (query.lastError());
	return;
    }
}
