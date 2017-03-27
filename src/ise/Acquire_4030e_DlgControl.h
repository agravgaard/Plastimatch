#pragma once
#include <QDialog>
#include "ui_acquire_4030e_panel.h"
#include "acquire_4030e_define.h"

class QString;

class Acquire_4030e_DlgControl : public QDialog,
    public Ui::DlgPanelControl
{
    Q_OBJECT
    ;

public slots:
    void CloseLink();
    void OpenLink();
    void Run();
    void Pause();
    void EndProc();
    void RestartProc();    
    void HardHandshakingOn();    
    void SoftHandshakingOn();
    void SoftBeamOn();    
    void GetDarkFieldImage();    
    void GetPanelInfo();    
	void OpenDarkImage();
	void OpenGainImage(); //Btn
	void OpenCurImgFolder();
	void ReDrawImg();
	void CopyCurrentImageForGainImage();
	void SaveSettingAsDefault_Child();
	void UpdateGUIFromSetting_Child();
	void SingleGainOn();
	void MultiGainOn();
	void ReLoadCalibImages();
	void OpenDefectMapFile(); //Btn
	void SeeSavingFolder();
	void LoadTestRawImage();

public:
    Acquire_4030e_DlgControl();
    ~Acquire_4030e_DlgControl();

public:
    int m_iPanelIdx;
	void closeEvent(QCloseEvent *event);	
	
};
