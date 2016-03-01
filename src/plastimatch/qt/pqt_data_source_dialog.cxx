/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmqt_config.h"
#include <QtGui>
#include "pqt_data_source_dialog.h"
#include "pqt_database.h"

Pqt_data_source_dialog::Pqt_data_source_dialog ()
{
    setupUi (this); // this sets up the GUI

    /* Hide status */
    this->label_status->hide();

    /* Attach model to QT listView */
    this->m_data_source_list_model = new Pqt_data_source_list_model;
    this->listView_data_source_list->setModel (this->m_data_source_list_model);

    /* Which data source is active in dialog box */
    this->m_active_index = -1;
}

Pqt_data_source_dialog::~Pqt_data_source_dialog ()
{
    
}

void
Pqt_data_source_dialog::pushbutton_new_released (void)
{
    /* Update dialog box */
    /* GCS FIX: This works correctly, but emits a Qt warning on the console */
    this->m_active_index = -1;
    this->m_data_source_list_model->set_active_row (-1);
    update_fields ();
}

void
Pqt_data_source_dialog::pushbutton_save_released (void)
{
    /* Validate input */
    if (this->lineEdit_data_source_name->text().isEmpty()) {
	QMessageBox::information (0, QString ("Info"), 
	    QString ("Please fill in the data source name."));
	return;
    }
    if (this->lineEdit_host->text().isEmpty()) {
	QMessageBox::information (0, QString ("Info"), 
	    QString ("Please fill in the hostname."));
	return;
    }
    if (this->lineEdit_port->text().isEmpty()) {
	QMessageBox::information (0, QString ("Info"), 
	    QString ("Please fill in the port."));
	return;
    }
    if (this->lineEdit_aet->text().isEmpty()) {
	QMessageBox::information (0, QString ("Info"), 
	    QString ("Please fill in AET."));
	return;
    }

    /* Insert into database */
    pqt_database_insert_data_source (
	this->lineEdit_data_source_name->text(),
	this->lineEdit_host->text(),
	this->lineEdit_port->text(),
	this->lineEdit_aet->text());

    /* Refresh list model and dialog box fields */
    this->refresh_from_database ();
}

void
Pqt_data_source_dialog::pushbutton_delete_released (void)
{
    /* Do nothing if no data source is highlighted */
    if (this->m_active_index == -1) {
	return;
    }

    /* Delete the row from the database */
    this->m_data_source_list_model->set_active_row (this->m_active_index);
    pqt_database_delete_data_source (
	this->m_data_source_list_model->get_label (),
	this->m_data_source_list_model->get_host (),
	this->m_data_source_list_model->get_port (),
	this->m_data_source_list_model->get_aet ());

    /* Refresh list model and dialog box fields */
    this->refresh_from_database ();
}

void
Pqt_data_source_dialog::listview_data_source_activated (
    QModelIndex model_index
)
{
    if (model_index.row() == this->m_active_index) {
	return;
    }

    this->m_active_index = model_index.row();
    update_fields ();
}

void
Pqt_data_source_dialog::update_fields (void)
{
    QString label = this->m_data_source_list_model->get_label ();
    this->lineEdit_data_source_name->setText (label);
    QString host = this->m_data_source_list_model->get_host ();
    this->lineEdit_host->setText (host);
    QString port = this->m_data_source_list_model->get_port ();
    this->lineEdit_port->setText (port);
    QString aet = this->m_data_source_list_model->get_aet ();
    this->lineEdit_aet->setText (aet);
}

void
Pqt_data_source_dialog::refresh_from_database (void)
{
    /* Refresh model - this also sets query index to -1 */
    this->m_data_source_list_model->load_query ();

    /* Update dialog box */
    /* GCS FIX: This works correctly, but emits a Qt warning on the console */
    this->m_active_index = -1;
    update_fields ();
}
