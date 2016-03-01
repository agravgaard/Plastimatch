/* -----------------------------------------------------------------------
See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
----------------------------------------------------------------------- */
#include "ise_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>  
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTime>
#include <windows.h>
#include <QFileDialog>
#include <QLocalServer>
#include <QLocalSocket>

#include "acquire_4030e_parent.h"
#include "advantech.h"
#include "kill.h"

using namespace std;

Acquire_4030e_parent::Acquire_4030e_parent (int argc, char* argv[]) 
: QApplication (argc, argv)
{
	if (argc != 1)
	{
		printf("not proper arguments\n");
		exit(-1);
	}

	printf ("Welcome to acquire_4030e\n"); 	

	m_bChildReadyToQuit[0] = true; //exit function can be called in initial status
	m_bChildReadyToQuit[1] = true;

	m_pServer[0] = NULL;
	m_pServer[1] = NULL;

	m_pClientConnect[0] = NULL;
	m_pClientConnect[1] = NULL;	
	
	//QString exePath = argv[0];

	m_bPanelOpeningSuccess[0] = false; //for sequencial starting
	m_bPanelOpeningSuccess[1] = false; //for sequencial starting
	
//	if (!initialize (exePath))
//		close();	

	for (int i = 0 ; i < 8 ; i++)
	{
		m_ArrAdvantechIDI[i] = 0;
	}

}

Acquire_4030e_parent::~Acquire_4030e_parent () //not called!!
{
	/* Timer is deleted automatically */

	/* Detatch from advantech */
	this->advantech->relay_open (0);
	this->advantech->relay_open (3);
	this->advantech->relay_open (4);
	delete this->advantech;

	/* Destroy window */
	delete this->window;	
}


bool Acquire_4030e_parent::initialize (QString& strEXE_Path)
{	
	//Generatate default folders	
	m_OptionSettingParent.GenDefaultFolders();		
	printf("Generating default folder if it is not exsisting\n");
	m_OptionSettingParent.CheckAndLoadOptions_Parent();

	/* Set up event handler for cleanup */
	connect (this, SIGNAL(aboutToQuit()), this, SLOT(about_to_quit())); //whenever quit signal --> run quit


	m_strEXEPath = strEXE_Path;
	num_process = 2; //fixed!

	//init log file
	m_strLogFilePath = m_OptionSettingParent.m_strPrimaryLogPath;

	m_strLogFilePath.append("\\acquire4030e_log_");

	QDate crntDate = QDate::currentDate ();
	QString dateFormat = "_yyyy_MM_dd";
	QString strDate = crntDate.toString(dateFormat);

	QTime crntTime = QTime::currentTime ();
	QString timeFormat = "_hh_mm_ss";
	QString strTime = crntTime.toString(timeFormat);

	m_strLogFilePath.append(strDate);
	m_strLogFilePath.append(strTime);
	m_strLogFilePath.append(".txt");

	m_logFout.open(m_strLogFilePath.toLocal8Bit().constData());		
	if (m_logFout.fail())
	{
		printf("Cannot open log output file");
	}

	/* Start up main window */	
	this->window = new Acquire_4030e_window();  //GUI-linked window	

	QString strSVNDate = QString(SVN_DATE);
	QString strSVNVer = QString(SVN_VERSION);

	strSVNDate.remove(QChar('$'));

	QStringList dateList = strSVNDate.split(" ");//space bar
	QString newStrDate = dateList.at(1);

	strSVNVer.remove(QChar('$'));
	QStringList verList = strSVNVer.split(" ");//space bar
	QString newStrVer = verList.at(1);

	this->window->setWindowTitle(QString("Acquire 4030e v%1 (%2)").arg(newStrVer).arg(newStrDate));	

	this->window->UpdateLabel(0, NOT_OPENNED);
	this->window->UpdateLabel(1, NOT_OPENNED);
	
	this->window->show ();

		/* Look for advantech device, spawn advantech thread */
	this->advantech = new Advantech;
	int advantechResult = this->advantech->Init();

	if (advantechResult != ADVANTECH_NO_ERROR)
	{
		QString strMsg = QString("Advantech Error. Error code = %1. Check USB connection. Program is exiting.").arg(advantechResult);

		log_output(strMsg);

		//about_to_quit();
		m_logFout.close();
		BackupLogFiles();

		/*QMessageBox msgBox = QMessageBox(this,"Warning", strMsg);
		msgBox.exec();		*/
		QMessageBox::warning(window, "Warning",strMsg );		
		return false;
	}

	this->generator_state = WAITING;
	this->panel_select = false;
	this->advantech->relay_open (0);
	this->advantech->relay_open (3);
	this->advantech->relay_open (4);


	/* Kill any leftover rogue processes */
	kill_rogue_processes (); //delete same process

	/* Check for system tray to store the UI */
	if (QSystemTrayIcon::isSystemTrayAvailable()) {
		printf ("System tray found.\n");
	}	
	else {
		printf ("System tray not found.\n");
	}

	if (!SOCKET_StartServer(0))
	{
		log_output("[p] Starting server 0 failed. Exit program");
		return false;
	}
	else
		log_output("[p] Starting server 0 success.");

	if (!SOCKET_StartServer(1))
	{
		log_output("[p] Starting server 1 failed.Exit Program");	
		return false;
	}
	else
		log_output("[p] Starting server 1 success.");


	m_enPanelStatus[0] = NOT_OPENNED;
	m_enPanelStatus[1] = NOT_OPENNED;

	m_bPleoraErrorHasBeenOccurredFlag[0] = false;
	m_bPleoraErrorHasBeenOccurredFlag[1] = false;	

	UpdateAdvantechIDIVal ();
	
	///* Start child processes */
	//printf ("Creating First child process.\n");

	///*for (int i = 0; i < this->num_process; i++)
	//{*/
	//	m_program[0] = strEXE_Path; //exe file path
	//	m_arguments[0].clear();
	//	m_arguments[0] << "--child" << QString("%1").arg(0).toUtf8();
	//	connect (&this->process[0], SIGNAL(readyReadStandardOutput()),this, SLOT(poll_child_messages()));
	//	this->process[0].start(m_program[0], m_arguments[0]);
	////}

	this->window->set_icon(0,NOT_OPENNED);// Tray should be shown after Label is updated.
	this->window->set_icon(1,NOT_OPENNED);

	Start_Process (0);

	/* Spawn the timer for polling devices */
	this->timer = new QTimer(this);
	connect (timer, SIGNAL(timeout()), this, SLOT(timer_event()));
	m_bParentBusy = false;
	timer->start (100);

	return true;
}

void 
Acquire_4030e_parent::UpdateAdvantechIDIVal () //IDI = Advantech Isolated Digital Input
{
	int tmpCurArr[8];
	for (int i = 0 ; i<8 ; i++)
	{
		tmpCurArr[i] = advantech->read_bit (i);
	}
	/*int res0 = advantech->read_bit (0);
	int res1 = advantech->read_bit (1);
	int res2 = advantech->read_bit (2);
	int res3 = advantech->read_bit (3);
	int res4 = advantech->read_bit (4);*/
	
	//Update IDI info on GUI
	QString strIDIVal = QString("IDI_INFO [0]Slct:%1 [1]Prep:%2 [2]Beam:%3 [3]Rdy0:%4 [4]Rdy1:%5 [567]%6%7%8")
		.arg(tmpCurArr[0])
		.arg(tmpCurArr[1])
		.arg(tmpCurArr[2])
		.arg(tmpCurArr[3])
		.arg(tmpCurArr[4])
		.arg(tmpCurArr[5])
		.arg(tmpCurArr[6])
		.arg(tmpCurArr[7]);
	window->ShowROIVal(strIDIVal);

	bool bChanged = false;

	for (int i = 0 ; i<8 ; i++)
	{
		if (tmpCurArr[i] != m_ArrAdvantechIDI[i])
		{
			bChanged = true;		
			m_ArrAdvantechIDI[i] = tmpCurArr[i];
		}
	}	
	//Log out only when there is change in values

	if (bChanged)
	{
		log_output(QString(strIDIVal).prepend("[p] "));
	}
}

void 
Acquire_4030e_parent::kill_rogue_processes ()
{
	/* Kill child processes (either ours, or from previous instances) */	
	kill_process ("acquire_4030e.exe");
}

void Acquire_4030e_parent::Start_Process (int procIdx)
{
	/* Start child processes */
	printf ("Creating child process %d.\n",procIdx);
	
	m_program[procIdx] = m_strEXEPath; //exe file path
	m_arguments[procIdx].clear();
	m_arguments[procIdx] << "--child" << QString("%1").arg(procIdx).toUtf8();
	connect (&this->process[procIdx], SIGNAL(readyReadStandardOutput()),this, SLOT(poll_child_messages()));
	this->process[procIdx].start(m_program[procIdx], m_arguments[procIdx]);	

}

void 
Acquire_4030e_parent::about_to_quit () //called from window->FinalQuit() as well
{	

	timer->stop();

	if (advantech != NULL)
	{
		this->advantech->relay_open (0);
		this->advantech->relay_open (3);
		this->advantech->relay_open (4);	

		/* Destroy window */
		//yk: WHEN KILL COMMAND OCCURRED			
		delete advantech;	
		advantech = NULL;
	}	

	//disconnect
	disconnect (&this->process[0], SIGNAL(readyReadStandardOutput()), this, SLOT(poll_child_messages()));
	disconnect (&this->process[1], SIGNAL(readyReadStandardOutput()), this, SLOT(poll_child_messages()));

	//log_output("parent_before ill rogue process");
	if (window != NULL)
	{
		window->tray_icon1->hide ();
		window->tray_icon2->hide ();
		delete window;
		window = NULL;
	}
	m_logFout.close();	
	

	/*Panel Debug File Backup*/

	//Log file: 
	//Target Path = 	

	/* Kill children before we die */
	kill_rogue_processes (); //not safe quit (for child)

	BackupLogFiles();

	Sleep(1000);	
}

void Acquire_4030e_parent::BackupLogFiles ()
{
	/*Log file backup in Network drive */
	QFileInfo tmpInfo(m_strLogFilePath);
	QString acquireLogFileName = tmpInfo.fileName();
	QString newFilePath = m_OptionSettingParent.m_strAlternateLogPath;
	newFilePath.append(QString("\\%1").arg(acquireLogFileName));
	QFile::copy(m_strLogFilePath, newFilePath);

	/*Debug file backup in Network drive */
	//m_OptionSettingParent.m_strDriverFolder[]; //this info is saved in child-Option class
	QString DebugFileOldPath[2];
	QString DebugFileNewPath[2];

	DebugFileOldPath[0] = m_strReceptorDriverFolder[0];
	DebugFileOldPath[1] = m_strReceptorDriverFolder[1];

	DebugFileNewPath[0] = m_strReceptorDriverFolder[0];
	DebugFileNewPath[1] = m_strReceptorDriverFolder[1];

	DebugFileOldPath[0].append("\\HcpDebug.txt");
	DebugFileOldPath[1].append("\\HcpDebug.txt");	

	DebugFileNewPath[0].append(QString("\\HcpDebug_0_%1").arg(acquireLogFileName));
	DebugFileNewPath[1].append(QString("\\HcpDebug_1_%1").arg(acquireLogFileName));

	QFile::rename(DebugFileOldPath[0], DebugFileNewPath[0]); //no prob even if file doesn't exist
	QFile::rename(DebugFileOldPath[1], DebugFileNewPath[1]);

	QFileInfo DebugFileInfo[2];
	DebugFileInfo[0] = QFileInfo(DebugFileNewPath[0]);
	DebugFileInfo[1] = QFileInfo(DebugFileNewPath[1]);

	QString debugFileName[2];
	debugFileName[0] = DebugFileInfo[0].fileName();
	debugFileName[1] = DebugFileInfo[1].fileName();

	QString newDebugFilePath_0 = m_OptionSettingParent.m_strAlternateLogPath;
	QString newDebugFilePath_1 = m_OptionSettingParent.m_strAlternateLogPath;

	newDebugFilePath_0.append(QString("\\%1").arg(debugFileName[0]));
	newDebugFilePath_1.append(QString("\\%1").arg(debugFileName[1]));

	QFile::copy(DebugFileNewPath[0], newDebugFilePath_0);
	QFile::copy(DebugFileNewPath[1], newDebugFilePath_1);
	
	QFile::remove(DebugFileNewPath[0]);
	QFile::remove(DebugFileNewPath[1]);

	/*QString newFilePathForDebug = m_OptionSettingParent.m_strAlternateLogPath;

	newFilePath.append(QString("\\%1").arg(acquireLogFileName));

	QFileInfo tmpInfoDebug(m_strLogFilePath);
	QString acquireLogFileName = tmpInfo.fileName();*/

	//  -->//"HcpDebug.txt" should be renamed --> copied to network folder
}

void 
Acquire_4030e_parent::log_output (const QString& log)
{
	/* Dump to window log */
	window->log_output (log);

	if (!m_logFout.fail())
		m_logFout << log.toLocal8Bit().constData() << std::endl;

	/* Dump to stdout */
	QByteArray ba = log.toAscii ();
	printf ("%s\n", (const char*) ba);
}

void 
Acquire_4030e_parent::poll_child_messages () //often called even window is closed
{	
	for (int i = 0; i < this->num_process; i++)
	{
		QByteArray result = process[i].readAllStandardOutput();
		QStringList lines = QString(result).split("\n");
		QString originalLine;

		foreach (QString line, lines)
		{
			line = line.trimmed();
			if (line.isEmpty()) {
				continue;
			}
			if (window == NULL)
				break; //do nothing

			QTime time = QTime::currentTime();    
			QString strTime = time.toString("\t@hh:mm:ss.zzz");

			originalLine = line;

			line = QString("[%1] %2").arg(i).arg(line);
			line.append(strTime);
			this->log_output (line);

			//Assumming that process number = panel number
			if (line.contains("PRESPP"))
			{	
				if (line.contains("ACTIVATE")) //PRESPP means aleady close the link
				{					
					m_bPleoraErrorHasBeenOccurredFlag[i] = false;
				}			
				else if (line.contains("KILL")) //PRESPP means aleady close the link
				{			
					process[i].close(); //call destructor 				
					m_bPleoraErrorHasBeenOccurredFlag[i] = false;
					m_bChildReadyToQuit[i] = true;

					if (m_bChildReadyToQuit[0] &&  m_bChildReadyToQuit[1])
						window->FinalQuit ();
				}
				else if (line.contains("RESTART"))
				{				
					m_bPleoraErrorHasBeenOccurredFlag[i] = false;
					RestartChildProcess(i);
				}							
			}			
			else if (line.contains("PSTAT0"))
			{				
				m_enPanelStatus[i] = NOT_OPENNED;				
			}
			else if (line.contains("PSTAT1"))
			{								
				m_enPanelStatus[i] = OPENNED;
				m_bPleoraErrorHasBeenOccurredFlag[i] = false;
			}	 
			else if (line.contains("PSTAT2"))
			{				
				m_enPanelStatus[i] = PANEL_ACTIVE;
				m_bPleoraErrorHasBeenOccurredFlag[i] = false;
			}	
			else if (line.contains("PSTAT3")) //Ready for pulse
			{							
				m_enPanelStatus[i] = READY_FOR_PULSE;
				m_bPleoraErrorHasBeenOccurredFlag[i] = false;

				m_bChildReadyToQuit[0] = false; //cannot exit without quit the child process safely
				m_bChildReadyToQuit[1] = false;
			}	
			else if (line.contains("READY FOR X-RAYS - EXPOSE AT ANY TIME"))
			{
				m_enPanelStatus[i] = READY_FOR_PULSE;
				m_bPleoraErrorHasBeenOccurredFlag[i] = false;

				m_bChildReadyToQuit[0] = false; //cannot exit without quit the child process safely
				m_bChildReadyToQuit[1] = false;
			}

			else if (line.contains("PSTAT4"))
			{
				//log_output("[p]PTAT4_Chaned");
				m_enPanelStatus[i] = PULSE_CHANGE_DETECTED;				
			}
			else if (line.contains("PSTAT5"))
			{
				m_enPanelStatus[i] = COMPLETE_SIGNAL_DETECTED;				
			}
			else if (line.contains("PSTAT6"))
			{
				m_enPanelStatus[i] = IMAGE_ACQUSITION_DONE;				
			}
			else if (line.contains("PSTAT7"))
			{
				m_enPanelStatus[i] = STANDBY_CALLED;				
			}	
			else if (line.contains("PSTAT8"))
			{
				m_enPanelStatus[i] = STANDBY_SIGNAL_DETECTED;				
			}
			else if (line.contains("PSTAT9"))
			{
				m_enPanelStatus[i] = ACQUIRING_DARK_IMAGE;				
			}	
			else if (line.contains("PANEL_OPEN_SUCCESS")) //called only once
			{					
				m_bPanelOpeningSuccess[i] = true; //for sequencial starting
				
				if (m_bPanelOpeningSuccess[0] && !m_bPanelOpeningSuccess[1])
					Start_Process(1);				
			}	
			else if (line.contains("DRIVER_PATH_OF_PANEL")) //called only once
			{	
				QStringList tmpStrList = originalLine.split(" "); //space
				m_strReceptorDriverFolder[i] = tmpStrList.at(1); //
				//log_output(m_strReceptorDriverFolder[i]);
			}				
			else if (line.contains("RESTART_PROCESS")) //
			{		
				m_enPanelStatus[i] = NOT_OPENNED;		
				m_bPleoraErrorHasBeenOccurredFlag[i] = false;
				RestartChildProcess(i);
			}	    
			else if(line.contains("open_receptor_link returns error")) //error during start
			{
				this->m_enPanelStatus[i] = NOT_OPENNED; //go to init status->red

				//messageBox
				QMessageBox msgBox;				
				QString strTitle = QString("Panel %1 Error").arg(i);
				msgBox.setWindowTitle(strTitle);

				QString str = QString("Panel #%1: Cable connection error during the start. Ethernet cable or power cable may have been unplugged.\n [1] Check cable connection\n [2] Re-plug in\n [3] Wait for at least 10 s then push [Retry] button.\n # if you click [Close] then exit from the program.\n").arg(i);
				msgBox.setText(str);
				msgBox.setStandardButtons(QMessageBox::Retry|QMessageBox::Close);
				msgBox.setDefaultButton(QMessageBox::Retry);
				int result = msgBox.exec();
				if (result == QMessageBox::Retry)
				{
					RestartChildProcess(i);
				}
				else if (result == QMessageBox::Close)
				{
					quit();
				}
			}
			else if(line.contains("Pleora Error")) //error during running 1)ethernet, 2) power
			{
				this->m_enPanelStatus[i] = NOT_OPENNED; //go to init status->red
				m_bPleoraErrorHasBeenOccurredFlag[i] = true;
				
				QMessageBox msgBox;
				QString strTitle = QString("Panel %1 Error").arg(i);
				msgBox.setWindowTitle(strTitle);

				QString str = QString("Panel #%1: Cable connection error. Ethernet cable or power cable may have been unplugged.\n [1] Check cable connection\n [2] Re-plug in\n [3] Wait for at least 10 s then push [Retry] button.\n # if you click [Close] then exit from the program.\n").arg(i);		 

				msgBox.setText(str);
				msgBox.setStandardButtons(QMessageBox::Retry|QMessageBox::Close);
				msgBox.setDefaultButton(QMessageBox::Retry);
				int result = msgBox.exec();
				if (result == QMessageBox::Retry)
				{
					RestartChildProcess(i);
				}
				else if (result == QMessageBox::Close)
				{
					quit();					
				}		
			}
			else if(line.contains("State Error")) //Occassionally, when power was re-plugged in
			{
				if (m_bPleoraErrorHasBeenOccurredFlag[i])// only when PleoraError has been occurred just before --> restart is required
				{
					this->m_enPanelStatus[i] = NOT_OPENNED; //go to init status->red

					QMessageBox msgBox;
					QString strTitle = QString("Panel %1 Error").arg(i);
					msgBox.setWindowTitle(strTitle);

					QString str = QString("Panel #%1: State error. This will occur when power cable replugged in.\n Push [retry] button to restart the child process\n # if you click [Close] then exit from the program.\n").arg(i);		 
					msgBox.setText(str);
					msgBox.setStandardButtons(QMessageBox::Retry|QMessageBox::Close);
					msgBox.setDefaultButton(QMessageBox::Retry);
					int result = msgBox.exec();
					if (result == QMessageBox::Retry)
					{
						RestartChildProcess(i);
					}
					else if (result == QMessageBox::Close)
					{						
						quit();					
					}
				}		
			}
			/***********************************************************/
		}
	}
}

bool Acquire_4030e_parent::RestartChildProcess(int idx)
{
	if (idx >=2)
		return false;

	process[idx].close();
	
	m_enPanelStatus[idx] = NOT_OPENNED;
	connect (&this->process[idx], SIGNAL(readyReadStandardOutput()), this, SLOT(poll_child_messages()));

	this->process[idx].start(m_program[idx], m_arguments[idx]); //temporary deleted //Restart the main

	m_bChildReadyToQuit[idx] = false;
	return true;
}
/* On STAR, panel 0 is axial, and panel 1 is g90 */
void Acquire_4030e_parent::timer_event () //will be runned from the first time.
{
	if (m_bParentBusy)
		return;

	m_bParentBusy = true;
	
	UpdateLableStatus(); //based-on m_bPanelReady status; //every 50 ms
	
	/* On STAR, there is no distinction between prep & expose, i.e. there 
	is only prep signal. */    		

	UpdateAdvantechIDIVal ();

	int res0 = m_ArrAdvantechIDI[0];
	int res1 = m_ArrAdvantechIDI[1];
	int res2 = m_ArrAdvantechIDI[2];
	int res3 = m_ArrAdvantechIDI[3];
	int res4 = m_ArrAdvantechIDI[4];

	if (res0 == Advantech::STATE_ERROR ||
		res1 == Advantech::STATE_ERROR ||
		res2 == Advantech::STATE_ERROR ||
		res3 == Advantech::STATE_ERROR ||
		res4 == Advantech::STATE_ERROR)
		return;

	bool gen_panel_select = (bool)res0; //0:false -> panel_0, 1:true ->panel_1	
	bool gen_prep_request = (bool)res1; /* Ignored on STAR */	
	bool gen_expose_request = (bool)res2;;  //beam-on signal to advantech	
	bool panel_0_ready = (bool)res3; //signal from panel	
	bool panel_1_ready = (bool)res4; //signal from panel    		
	
	//if panel is selected and that panel is now Standby, then go to proceed. But if the other panel is not standby mode, please wait.

	if (gen_panel_select == 0 && m_enPanelStatus[0] == STANDBY_SIGNAL_DETECTED && m_enPanelStatus[1] == STANDBY_SIGNAL_DETECTED)
	{
		SendCommandToChild(0, PCOMMAND_UNLOCKFORPREPARE); //changes "go further to activate panel". after one cycle has been done, it will be automatically locked in standby mode (stuck in standby)
	}	
	else if (gen_panel_select == 1 && m_enPanelStatus[0] == STANDBY_SIGNAL_DETECTED && m_enPanelStatus[1] == STANDBY_SIGNAL_DETECTED)
	{
		SendCommandToChild(1, PCOMMAND_UNLOCKFORPREPARE); //changes "go further to activate panel". after one cycle has been done, it will be automatically locked in standby mode (stuck in standby)
	}
	
	/* Write a debug message */

	if (gen_expose_request || gen_prep_request) {
		{
			this->log_output (
				QString("[p] Generator status: %1 %2 %3 %4 %5")
				.arg(gen_panel_select).arg(gen_prep_request)
				.arg(gen_expose_request).arg(panel_0_ready).arg(panel_1_ready));	
		}
	}

	/* Check for new prep/expose request from generator */
	if (gen_expose_request && this->generator_state == WAITING) // if panel is not ready but only gen state and expose request should not trigger the relay.//at least do nothing with panel when it is not ready
	{
		/* Save state about which generator is active */
		this->panel_select = gen_panel_select;

		/* Close relay, asking panel to begin integration */
		if (gen_panel_select == 0)
		{
			if (m_enPanelStatus[0] == READY_FOR_PULSE )
			{
				this->log_output (
					QString("[p] Closing relay to panel: axial"));
				this->advantech->relay_close (3); //close = connected					
				m_enPanelStatus[0] = PULSE_CHANGE_DETECTED;
				this->generator_state = EXPOSE_REQUEST;
			}
			else
			{
				this->log_output (
					QString("[p] Waiting for the panel to be activated: axial"));
			}			
		}	
		else if (gen_panel_select == 1)
		{			
			if (m_enPanelStatus[1] == READY_FOR_PULSE)
			{
				
				this->log_output (
					QString("[p] Closing relay to panel: g90"));
				this->advantech->relay_close (4); //close = connected								
				m_enPanelStatus[1] = PULSE_CHANGE_DETECTED;
				this->generator_state = EXPOSE_REQUEST;
			}
			else
			{
				this->log_output (
					QString("[p] Waiting for the panel to be activated: g90"));
			}
		}			
	}
	/* Check if panel is ready */
	if (gen_expose_request && this->generator_state == EXPOSE_REQUEST) {
		/* When panel is ready, close relay on generator */
		if (this->panel_select == 0 && panel_0_ready) {
			this->log_output (
				QString("[p] Closing relay to generator"));
			this->advantech->relay_close (0); //beam on signal to gen.
			this->generator_state = EXPOSING;
		}
		else if (this->panel_select == 1 && panel_1_ready) {
			this->log_output (
				QString("[p] Closing relay to generator"));
			this->advantech->relay_close (0);
			this->generator_state = EXPOSING;
		}
		
		if (panel_0_ready && this->panel_select == true
			|| panel_1_ready && this->panel_select == false)
		{
			this->log_output (
				QString("[p] Warning, panel %1 was unexpectedly ready")
				.arg(panel_0_ready ? 0 : 1));
		}		
	}   
	/* Check if generator prep request complete */	
	if (!gen_expose_request && generator_state != WAITING)
	{ //when exposure is over then relay_open: allow panel to read data		

		log_output("[p] Opening relay to generator");		
		this->advantech->relay_open (0); //Beam on (N0, COM0)

		if (panel_select == 0)
		{			
			log_output("[p] Opening relay to panel: axial");		
			this->advantech->relay_open (3); //Expose request for panel 0- release
		}
		if (panel_select == 1)
		{		
			log_output("[p] Opening relay to panel: g90");		
			this->advantech->relay_open (4); //Expose request for panel 0- release
		}
		this->log_output (
			QString("[p] Reset generator state to WAITING."));		
		this->generator_state = WAITING;
	}

	if (gen_panel_select == 0 && m_enPanelStatus[1] == READY_FOR_PULSE) //Abnormal case: jump to standby before acquisition.
		SendCommandToChild(1, PCOMMAND_CANCELACQ);
	else if (gen_panel_select == 1 && m_enPanelStatus[0] == READY_FOR_PULSE) //Abnormal case: jump to standby before acquisition.
		SendCommandToChild(0, PCOMMAND_CANCELACQ);

	m_bParentBusy = false;
}

void 
Acquire_4030e_parent::UpdateLableStatus()
{
	for (int i = 0 ; i<MAX_PANEL_NUM_YK ; i++)
	{
		this->window->UpdateLabel(i,this->m_enPanelStatus[i]);
		this->window->set_icon((1-i),this->m_enPanelStatus[i]);
	} 
}

void Acquire_4030e_parent::SendCommandToChild(int idx, CommandToChild enCommand) //Msg box
{    
	// if process is not ready, nothing.
	//RESTART only when the process is killed
	
	if (!process[idx].isOpen() && enCommand == PCOMMAND_RESTART) //if even the process has not been started,
	{
		RestartChildProcess(idx);
		return;
	}
	else if (!process[idx].isOpen() && enCommand != PCOMMAND_RESTART)
	{
		return;
	}   

	QString msg;

	switch (enCommand)
	{	
	case PCOMMAND_KILL:		

		msg = "PCOMMAND_KILL";
		if (!SOCKET_SendMessage(idx, msg))
			log_output("[p] Failed to send a message. Client is not connected");
		break;
	
	case PCOMMAND_RESTART:		
		msg = "PCOMMAND_RESTART";		
		if (!SOCKET_SendMessage(idx, msg))
			log_output("[p] Failed to send a message. Client is not connected");
		break;	
		
	case PCOMMAND_SHOWDLG:		
		msg = "PCOMMAND_SHOWDLG";	
		if (!SOCKET_SendMessage(idx, msg))
			log_output("[p] Failed to send a message. Client is not connected");	

		break;

	case PCOMMAND_UNLOCKFORPREPARE:		
		msg = "PCOMMAND_UNLOCKFORPREPARE";		
		if (!SOCKET_SendMessage(idx, msg))
			log_output("[p] Failed to send a message. Client is not connected");		

		break;


	case PCOMMAND_CANCELACQ:		

		msg = "PCOMMAND_CANCELACQ";
		if (!SOCKET_SendMessage(idx, msg))
			log_output("[p] Failed to send a message. Client is not connected");

		break;
	}    

	return;
}

bool Acquire_4030e_parent::SOCKET_SendMessage(int idx, QString& msg)
{
	if (m_pClientConnect[idx] == NULL)
		return false;

	if (m_pClientConnect[idx]->waitForConnected(1000))
	{
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_0);
		out << msg;
		out.device()->seek(0);
		m_pClientConnect[idx]->write(block);
		m_pClientConnect[idx]->flush();
	}
	return true;
}


bool Acquire_4030e_parent::SOCKET_StartServer(int iPanelIdx)
{
	if (m_pServer[iPanelIdx] != NULL)
	{
		delete m_pServer[iPanelIdx];
		m_pServer[iPanelIdx] = NULL;
	}	

	m_pServer[iPanelIdx] = new QLocalServer(this);	

	QString strServerName = QString("SOCKET_MSG_TO_CHILD_%1").arg(iPanelIdx);
	
	connect(m_pServer[iPanelIdx], SIGNAL(newConnection()), this, SLOT(SOCKET_ConnectClient()));

	if (!m_pServer[iPanelIdx]->listen(strServerName))	
		return false;	
	/*else
	{
		log_output("Server is listening");
	}*/

	return true;
}

//
//void Acquire_4030e_parent::SOCKET_ConnectClient(int iPanelIdx)
//{
//	m_pClientConnect[iPanelIdx] = m_pServer[iPanelIdx]->nextPendingConnection();
//	connect(m_pClientConnect[iPanelIdx], SIGNAL(disconnected()), m_pClientConnect[iPanelIdx], SLOT(deleteLater()));
//
//	if (m_pClientConnect[iPanelIdx] != NULL)
//		log_output(QString("[p] Client For child %1 is approved to be connected").arg(iPanelIdx));
//	else
//	{
//		log_output(QString("[p] Client For child %1 is not connected. Check the server name.").arg(iPanelIdx));
//		kill_rogue_processes();
//	}
//}


void Acquire_4030e_parent::SOCKET_ConnectClient()
{
	if (m_pServer[0]->hasPendingConnections())
	{
		m_pClientConnect[0] = m_pServer[0]->nextPendingConnection();

		connect(m_pClientConnect[0], SIGNAL(disconnected()), m_pClientConnect[0], SLOT(deleteLater()));
		if (m_pClientConnect[0] != NULL)
			log_output(QString("[p] Client For child %1 is approved to be connected").arg(0));
		else
		{
			log_output(QString("[p] Client For child %1 is not connected. Check the server name.").arg(0));
		}

	}
	if (m_pServer[1]->hasPendingConnections())
	{
		m_pClientConnect[1] = m_pServer[1]->nextPendingConnection();

		connect(m_pClientConnect[1], SIGNAL(disconnected()), m_pClientConnect[1], SLOT(deleteLater()));
		if (m_pClientConnect[1] != NULL)
			log_output(QString("[p] Client For child %1 is approved to be connected").arg(1));
		else
		{
			log_output(QString("[p] Client For child %1 is not connected. Check the server name.").arg(1));
		}
	}	
}