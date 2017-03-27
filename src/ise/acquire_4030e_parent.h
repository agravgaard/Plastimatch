//Whenever There is change, minimum change should be given to this file!
//changed 0604 0605 0627 0627_2(only CMakeLists)

/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _acquire_4030e_parent_h_
#define _acquire_4030e_parent_h_

#include "ise_config.h"
#include <QApplication>
#include <QProcess>
#include <fstream>
#include "acquire_4030e_define.h"
#include "acquire_4030e_window.h"
#include "YKOptionSetting.h"

#define SVN_VERSION "$Rev$"
#define SVN_DATE "$Date$"
#define SVN_AUTHOUR "$Author$"

class Advantech;
class QTimer;
class QLocalServer;
class QLocalSocket;

using namespace std;

class Acquire_4030e_parent : public QApplication
{
    Q_OBJECT
    ;
public:
    Acquire_4030e_parent (int argc, char* argv[]);
    ~Acquire_4030e_parent ();

public:    	
	bool initialize (QString& strEXE_Path);
    void kill_rogue_processes ();
    void log_output (const QString& log);
public slots:
    void timer_event ();    
    void poll_child_messages ();
    void about_to_quit ();		
	void SOCKET_ConnectClient();

public:
    Acquire_4030e_window *window;
    
	QLocalServer* m_pServer[2];
	QLocalSocket* m_pClientConnect[2];

    int num_process;
    QProcess process[2];
    QTimer *timer;
	
    Advantech *advantech;    
    Generator_state generator_state;
    bool panel_select;    

    PSTAT m_enPanelStatus[2]; //	init (ready), acquiring (yellow), ready (green)

    QString m_program[2];
    QStringList m_arguments[2];

    //Destructor will not be called
    //In destructor: standby -> close link -> memory release
    bool RestartChildProcess(int idx);   

    void UpdateLableStatus();

    bool m_bPleoraErrorHasBeenOccurredFlag[2];    

    void SendCommandToChild(int idx, CommandToChild enCommand); //Msg box    
    
    QString m_strLogFilePath;

    ofstream m_logFout;

	bool m_bChildReadyToQuit[2]; //flag for child process
	bool m_bPanelOpeningSuccess[2]; //for sequencial starting

	bool m_bParentBusy;

public:
	bool SOCKET_StartServer(int iPanelIdx);
	//void SOCKET_ConnectClient(int iPanelIdx);
	bool SOCKET_SendMessage(int idx, QString& msg);

	YKOptionSetting m_OptionSettingParent;

	QString m_strEXEPath;
	void Acquire_4030e_parent::Start_Process (int procIdx);

	void BackupLogFiles ();
	QString m_strReceptorDriverFolder[2]; //only used for Debug file setting //this variable can be filled when child is initialized	

	int m_ArrAdvantechIDI[8]; //Advantech Isolated Diginal Input Value (not Relay)
	void UpdateAdvantechIDIVal (); //IDI = Advantech Isolated Digital Input
};
#endif
