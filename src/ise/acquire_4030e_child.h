/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _acquire_4030e_child_h_
#define _acquire_4030e_child_h_

#include <QApplication>
#include <QString>
#include <QCloseEvent>
#include <QLocalSocket>

#include <fstream>
#include "acquire_4030e_define.h"
#include "YKOptionSetting.h"


class Dips_panel;
class QTimer;
class Varian_4030e;
class Acquire_4030e_DlgControl;

class YK16GrayImage;
class QProgressDialog;

union UQueryProgInfo;
struct SModeInfo;

class Acquire_4030e_child : public QApplication
{
    Q_OBJECT
    ;	
   
signals:
	void ready();

public slots:			
	void TimerMainLoop_event(); //subtitute of large while loop in run func.
	void SOCKET_ConnectToServer(QString& strServerName);	
	void SOCKET_ReadMessageFromParent(); //when msg comes to client
	void SOCKET_PrintError(QLocalSocket::LocalSocketError e);

public:
    Acquire_4030e_child (int argc, char* argv[]);	
    ~Acquire_4030e_child ();
public:
	bool init(int panelIdx);
    int open_receptor (const char* path); //return value = result
    bool close_receptor ();
  

public:   
    PSTAT m_enPanelStatus;

    int idx;
    Dips_panel *dp;
    Varian_4030e *vp;

    bool m_bAcquisitionOK; //determine "while" loop go or stop.    
    int m_iProcNum;
    QString m_strReceptorPath;
   
    QString m_strFromParent;

    void InterpretAndFollow();    

    bool SWSingleAcquisition(UQueryProgInfo& crntStatus);    
    bool PerformDarkFieldCalibration(UQueryProgInfo& crntStatus, int avgFrameCnt);   

	YK16GrayImage* m_pCurrImage;
	YK16GrayImage* m_pDarkImage;
	YK16GrayImage* m_pGainImage;

	YK16GrayImage* m_pCurrImageRaw;
	YK16GrayImage* m_pCurrImageDarkCorrected;

    void ReleaseMemory();
   
    bool AuditImage(int sizeX, int sizeY, USHORT* pImg);

	bool LoadDarkImage(QString& filePath);
	bool LoadGainImage(QString& filePath);
	
	QTimer * m_TimerMainLoop;	

	UQueryProgInfo* m_pCrntStatus; //currentStatus    
	SModeInfo* m_pModeInfo;

	int m_timeOutCnt;

	///* Panel control functions //		03/06/2013
	bool PC_ReOpenPanel();
	bool PC_ActivatePanel();
	bool PC_WaitForPanelReady();
	bool PC_WaitForPulse();
	bool PC_GetImageHardware();
	bool PC_WaitForComplete();	
	bool PC_CallForStanby(); //also can be used for SW acquisition for reloop
	bool PC_WaitForStanby();

	bool PC_SoftwareAcquisition_SingleShot(); //should be done at READY_FOR_PULSE
	bool PC_DarkFieldAcquisition_SingleShot(int avgFrames); //should be done at READY_FOR_PULSE

	bool GetGainImageFromCurrent();	

	int m_iNumOfFramesRequested;// = 1 default
	Acquire_4030e_DlgControl *m_dlgControl;

	void ReDraw(int lowerWinVal, int upperWinVal);//current image only
	int m_iCurWinMidVal, m_iCurWinWidthVal; //window level of current image
	
	QProgressDialog* m_dlgProgBar;		

	QLocalSocket* m_pClient;
	YKOptionSetting m_OptionSettingChild; 

	bool m_bLockInPanelStandby;
	bool m_bCancelAcqRequest;//Skip ReadyForPulse when the other panel is selected
	void ChangePanelStatus(PSTAT enStatus);


	std::vector<BADPIXELMAP> m_vBadPixelMap;
	bool LoadBadPixelMap(const char* filePath); //fill m_vBadPixelMap with loaded data(mapping)

	bool m_bFirstBoot_OpenSuccess; // send aqprintf("PANEL_OPEN_SUCCESS\n"); to parent for Sequencial Booting of the panels

	//void DeleteOldImageFiles(QString& folderName, int iDaysAgo); //returns number of deleted files
	void DeleteOldImageFiles();

	bool Debug_GetImageFromFile(QString& filePath);
};

#endif