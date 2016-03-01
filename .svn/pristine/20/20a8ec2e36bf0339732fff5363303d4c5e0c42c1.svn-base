/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <QApplication>
#include <QCloseEvent>
#include <QProcess>
#include <QMessageBox>
#include <QTimer>
#include <windows.h>

#include "acquire_4030e_window.h"
#include "acquire_4030e_parent.h"
#include "advantech.h"

Acquire_4030e_window::Acquire_4030e_window ()
    : QMainWindow ()
{
    /* Sets up the GUI */
    setupUi (this);

    /* Set up the icon for the system tray */
    create_actions ();
    create_tray_icon ();    

    /* Chuck some text into the text box for testing */
    log_viewer->appendPlainText ("Welcome to acquire_4030e.exe.");
}

void Acquire_4030e_window::set_icon (int idx, PSTAT pstat)  // set_icon + tray icon
{
	Label_style matchingLabelStyle;

	switch (pstat)
	{
	case NOT_OPENNED:
		matchingLabelStyle = LABEL_NOT_READY;
		break;
	case OPENNED:
		matchingLabelStyle = LABEL_PREPARING;
		break;
	case PANEL_ACTIVE:
		matchingLabelStyle = LABEL_PREPARING;
		break;
	case READY_FOR_PULSE:
		matchingLabelStyle = LABEL_READY;
		break;
	case PULSE_CHANGE_DETECTED:
		matchingLabelStyle = LABEL_ACQUIRING;
		break;
	case COMPLETE_SIGNAL_DETECTED:
		matchingLabelStyle = LABEL_ACQUIRING;
		break;
	case IMAGE_ACQUSITION_DONE:
		matchingLabelStyle = LABEL_PREPARING;

		break;
	case STANDBY_CALLED:
		matchingLabelStyle = LABEL_PREPARING;
		break;
	case STANDBY_SIGNAL_DETECTED:
		matchingLabelStyle = LABEL_STANDBY;
		break;	
	case ACQUIRING_DARK_IMAGE:
		matchingLabelStyle = LABEL_ACQUIRING;
		break;
	}

	if (idx == 0)
	{
		switch (matchingLabelStyle) {
	case LABEL_NOT_READY:	    
		tray_icon1->setIcon (QIcon(":/red_ball.png"));		
		tray_icon1->setToolTip (tr("Acquire 4030e_Panel_1"));	    
		break;

	case LABEL_ACQUIRING:	    		
		tray_icon1->setIcon (QIcon(":/yellow_ball.png"));
		tray_icon1->setToolTip (tr("Acquire 4030e_Panel_1"));	    
		break;

	case LABEL_PREPARING:	    		
		tray_icon1->setIcon (QIcon(":/orange_ball.png"));
		tray_icon1->setToolTip (tr("Acquire 4030e_Panel_1"));	    
		break;

	case LABEL_STANDBY:	    		
		tray_icon1->setIcon (QIcon(":/blue_ball.png"));
		tray_icon1->setToolTip (tr("Acquire 4030e_Panel_1"));	    
		break;

	case LABEL_READY:	    		
		tray_icon1->setIcon (QIcon(":/green_ball.png"));
		tray_icon1->setToolTip (tr("AAcquire 4030e_Panel_1"));
		break;
		}
		tray_icon1->show ();
	}    
	else if (idx ==1)
	{
		switch (matchingLabelStyle) {
	case LABEL_NOT_READY:	    
		tray_icon2->setIcon (QIcon(":/red_ball.png"));	    	    		
		tray_icon2->setToolTip (tr("Acquire 4030e_Panel_0"));	
		break;
	case LABEL_ACQUIRING:	    		
		tray_icon2->setIcon (QIcon(":/yellow_ball.png"));
		tray_icon2->setToolTip (tr("Acquire 4030e_Panel_0"));	    
		break;

	case LABEL_PREPARING:	    
		tray_icon2->setIcon (QIcon(":/orange_ball.png"));
		tray_icon2->setToolTip (tr("Acquire 4030e_Panel_0"));	    
		break;
	case LABEL_STANDBY:	    		
		tray_icon2->setIcon (QIcon(":/blue_ball.png"));
		tray_icon2->setToolTip (tr("Acquire 4030e_Panel_0"));	    
		break;
	case LABEL_READY:	    
		tray_icon2->setIcon (QIcon(":/green_ball.png"));
		tray_icon2->setToolTip (tr("AAcquire 4030e_Panel_0"));
		break;
		}
		tray_icon2->show ();
	}    
}



void 
Acquire_4030e_window::create_actions()
{
    show_action = new QAction(tr("&Show"), this);
    connect(show_action, SIGNAL(triggered()), this, SLOT(showNormal()));

    quit_action = new QAction(tr("&Quit"), this);
    connect(quit_action, SIGNAL(triggered()), this, SLOT(request_quit()));
}

void 
Acquire_4030e_window::create_tray_icon ()
{
    tray_icon_menu = new QMenu(this);
    tray_icon_menu->addAction (show_action);
    tray_icon_menu->addSeparator ();
    tray_icon_menu->addAction (quit_action);

    tray_icon2 = new QSystemTrayIcon (this); //YK
    tray_icon1 = new QSystemTrayIcon (this);    

    tray_icon2->setContextMenu (tray_icon_menu);
    tray_icon1->setContextMenu (tray_icon_menu);    

    connect (tray_icon2, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(systray_activated(QSystemTrayIcon::ActivationReason)));
    connect (tray_icon1, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(systray_activated(QSystemTrayIcon::ActivationReason)));
    
}

void 
Acquire_4030e_window::log_output (const QString& log)
{
    log_viewer->appendPlainText (log);
}


void Acquire_4030e_window::ShowROIVal(const QString& strIDIVal)
{
	this->lineEditAdvantechIDI->setText(strIDIVal);
}

void Acquire_4030e_window::set_label_style (int panel_no, Label_style style) //change Label_style color
{
    QString style_sheet;
    switch (style) {
	case LABEL_NOT_READY:
	    style_sheet = "QLabel { background-color : rgba(255,0,0,200); color : white; font-weight: bold; font-size: 15px}";
	    break;
	case LABEL_ACQUIRING:
	    style_sheet = "QLabel { background-color : rgba(230,230,0,200); color : black; font-weight: bold;font-size: 15px}";
	    break;
	case LABEL_PREPARING:
	    style_sheet = "QLabel { background-color : rgba(255,153,0,200); color : black; font-weight: bold;font-size: 15px}";
	    break;
	case LABEL_STANDBY:
		style_sheet = "QLabel { background-color : rgba(0,0,200,200); color : white; font-weight: bold;font-size: 15px}";
		break;
	case LABEL_READY:
	    style_sheet = "QLabel { background-color : rgba(0,255,0,200); color : black; font-weight: bold;font-size: 15px}";
	    break;
    }
    if (panel_no == 0) {
	panel_1_status->setStyleSheet(style_sheet); //panel_1_status: QLabel - HTML type input (str)
    }
    else {
	panel_2_status->setStyleSheet(style_sheet);
    }
}


void 
Acquire_4030e_window::set_label (int panel_no, const QString& log)
{
    if (panel_no == 0) {
	panel_1_status->setText(log);
    }
    else {
	panel_2_status->setText(log);
    }
}

void Acquire_4030e_window::request_quit ()
{
	//m_bSeqKillReady = true;
	
	((Acquire_4030e_parent*)qApp)->SendCommandToChild(0, PCOMMAND_KILL);
	((Acquire_4030e_parent*)qApp)->SendCommandToChild(1, PCOMMAND_KILL);	
	
	QTimer::singleShot(5000,this, SLOT(FinalQuit()));	
}

void Acquire_4030e_window::FinalQuit ()
{		
	tray_icon1->hide ();
	tray_icon2->hide ();
	
	qApp->quit(); //qApp = Parent	 -->will call About_to_Quit in parent
}

void Acquire_4030e_window::systray_activated (
    QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::MiddleClick:
        this->show ();
        break;
    default:
        ;
    }
}


void Acquire_4030e_window::UpdateLabel(int iPanelIdx, PSTAT enStyle) // 0 based panel ID //called from child proc except the first time
{
	switch (enStyle)
	{    
	case NOT_OPENNED:
		this->set_label_style (iPanelIdx, LABEL_NOT_READY);
		this->set_label (iPanelIdx, "  Initializing");
		break;	

	case OPENNED:
		this->set_label_style (iPanelIdx, LABEL_PREPARING);
		this->set_label (iPanelIdx, "  Preparing");
		break;	

	case PANEL_ACTIVE:
		this->set_label_style (iPanelIdx, LABEL_PREPARING);
		this->set_label (iPanelIdx, "  Preparing");
		break;	

	case READY_FOR_PULSE:
		this->set_label_style (iPanelIdx, LABEL_READY);
		this->set_label (iPanelIdx, "  Ready");
		break;	

	case PULSE_CHANGE_DETECTED:
		this->set_label_style (iPanelIdx, LABEL_ACQUIRING);
		this->set_label (iPanelIdx, "  Acquiring");
		break;	

	case COMPLETE_SIGNAL_DETECTED:
		this->set_label_style (iPanelIdx, LABEL_ACQUIRING);
		this->set_label (iPanelIdx, "  Acquiring");
		break;	

	case IMAGE_ACQUSITION_DONE:
		this->set_label_style (iPanelIdx, LABEL_PREPARING);
		this->set_label (iPanelIdx, "  Preparing");
		break;

	case STANDBY_SIGNAL_DETECTED:
		this->set_label_style (iPanelIdx, LABEL_STANDBY);
		this->set_label (iPanelIdx, "  Stand-by");
		break;    

	case ACQUIRING_DARK_IMAGE:
		this->set_label_style (iPanelIdx, LABEL_ACQUIRING);
		this->set_label (iPanelIdx, "  Acquiring dark images");
		break;    

	default:
		break;
	}
}

void 
Acquire_4030e_window::closeEvent(QCloseEvent *event)
{
    if (tray_icon1->isVisible()) {
        hide();        
    }
    if (tray_icon2->isVisible()) {
        hide();        
    }
    event->ignore();
}

void 
Acquire_4030e_window::RestartPanel_0 ()
{      
    ((Acquire_4030e_parent*)qApp)->RestartChildProcess(0);
}

void 
Acquire_4030e_window::RestartPanel_1 ()
{    
    ((Acquire_4030e_parent*)qApp)->RestartChildProcess(1);
}

void Acquire_4030e_window::ShowPanelControl_0 ()
{        
	((Acquire_4030e_parent*)qApp)->SendCommandToChild(0, PCOMMAND_SHOWDLG);
}

void Acquire_4030e_window::ShowPanelControl_1 ()
{ 
	((Acquire_4030e_parent*)qApp)->SendCommandToChild(1, PCOMMAND_SHOWDLG);
}
//
//void Acquire_4030e_window::RunRelay_Panel0()
//{
//	((Acquire_4030e_parent*)qApp)->advantech->relay_close(3);
//
//	Sleep(2000);
//
//	((Acquire_4030e_parent*)qApp)->advantech->relay_open (3);
//
//}
//
//void Acquire_4030e_window::RunRelay_Panel1()
//{
//	((Acquire_4030e_parent*)qApp)->advantech->relay_close(4);
//
//	Sleep(2000);
//
//	((Acquire_4030e_parent*)qApp)->advantech->relay_open (4);
//}

void Acquire_4030e_window::SaveSettingAsDefault_Parent()
{
	YKOptionSetting* pOptionSetting = &(((Acquire_4030e_parent*)qApp)->m_OptionSettingParent);		

}