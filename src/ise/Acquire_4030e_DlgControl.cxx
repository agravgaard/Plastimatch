#include "Acquire_4030e_DlgControl.h"
#include "Acquire_4030e_child.h"
#include <QCheckBox>
#include <QFileDialog>
#include <windows.h>
#include <QMessageBox>
#include "aqprintf.h"
#include "varian_4030e.h"
//#include "YKOptionSetting.h"

Acquire_4030e_DlgControl::Acquire_4030e_DlgControl(): QDialog ()
{
    /* Sets up the GUI */
    setupUi (this);

	this->SpinDarkAvgFrames->setValue(4);

	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;	
	QString str= QString("%1").arg(qMyApp->idx);
	
}

Acquire_4030e_DlgControl::~Acquire_4030e_DlgControl()
{
}

void Acquire_4030e_DlgControl::CloseLink()
{    
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	if (!qMyApp->close_receptor())
	{
		aqprintf("PCMSG_CLOSING_RECEPTOR_ERROR\n");		
	}
	else
	{
		qMyApp->m_enPanelStatus = NOT_OPENNED;
		qMyApp->m_bAcquisitionOK = false; //Timer is still on but dummy loop
		aqprintf("PCMSG_RECEPTOR_CLOSED\n");
		//or timer off
	}
	return;
}
void Acquire_4030e_DlgControl::OpenLink()
{ 
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	qMyApp->m_enPanelStatus = NOT_OPENNED;
	qMyApp->m_bAcquisitionOK = true; //or Timer start
	aqprintf("PCMSG_RECEPTOR_OPENNED\n");
	
	return;
}
void Acquire_4030e_DlgControl::Run()
{    
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	qMyApp->m_bAcquisitionOK = true; 
	aqprintf("PCMSG_LOOP_RESUMED\n");
}
void Acquire_4030e_DlgControl::Pause()
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	qMyApp->m_bAcquisitionOK = false; 
	aqprintf("PCMSG_LOOP_STOPPED\n");    
}
void Acquire_4030e_DlgControl::EndProc() //KILL
{ 
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	CloseLink();
	qMyApp->quit();
}
void Acquire_4030e_DlgControl::RestartProc()
{
	CloseLink();
	aqprintf("PRESPP_RESTART\n");//parent will do it    
}
void Acquire_4030e_DlgControl::HardHandshakingOn()
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	if (qMyApp->m_enPanelStatus != READY_FOR_PULSE && qMyApp->m_enPanelStatus != STANDBY_SIGNAL_DETECTED) //should be done in standby status
	{
		aqprintf("PCMSG_pannel is not ready\n");
		RadioHardHandshakingEnable->setChecked(false);
		RadioSoftHandshakingEnable->setChecked(true);		
		return;
	}		
	int result = vip_io_enable (HS_ACTIVE);
	Sleep(1000);
	
	qMyApp->ChangePanelStatus(IMAGE_ACQUSITION_DONE); //re loop //Standby	
	aqprintf("PCMSG_HARDHANDSHAKING_ON\n");
}



void Acquire_4030e_DlgControl::SoftHandshakingOn()
{	
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;	

	if (qMyApp->m_enPanelStatus != READY_FOR_PULSE && qMyApp->m_enPanelStatus != STANDBY_SIGNAL_DETECTED) //should be done in standby status
	{
		aqprintf("Pannel is not ready\n");		
		RadioHardHandshakingEnable->setChecked(true);
		RadioSoftHandshakingEnable->setChecked(false);
		return;
	}
	qMyApp->m_bAcquisitionOK = false;
	aqprintf("PCMSG_SOFTHANDSHAKING_ON\n");
}

void Acquire_4030e_DlgControl::SoftBeamOn()
{    
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	if (qMyApp->m_dlgControl->RadioSoftHandshakingEnable->isChecked())
	{
		aqprintf("PCMSG_SOFTHANDSHAKING_ACQUSITION_START\n");
		qMyApp->PC_SoftwareAcquisition_SingleShot();
		aqprintf("PCMSG_SOFTHANDSHAKING_ACQUSITION_COMPLETE\n");
	}
	else
	{
		aqprintf ("First, change the handshaking status\n");
	}		
}

void Acquire_4030e_DlgControl::GetPanelInfo()
{
	//Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	//qMyApp->m_enPanelStatus = COMPLETE_SIGNAL_DETECTED; //Standby once again
}

void Acquire_4030e_DlgControl::GetDarkFieldImage()
{  
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	if (qMyApp->m_enPanelStatus != READY_FOR_PULSE)
	{
		aqprintf("Dark field acquisition only can be performed in READY FOR PULSE\n");
		return;
	}		
	qMyApp->ChangePanelStatus(ACQUIRING_DARK_IMAGE);	
}

void Acquire_4030e_DlgControl::closeEvent(QCloseEvent *event)
{
	hide();	
	event->ignore();
}

void Acquire_4030e_DlgControl::OpenDarkImage() //Btn
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	int idx =qMyApp->m_iProcNum;

	QString defaultSearchFolder = qMyApp->m_OptionSettingChild.m_strDarkImageSavingFolder[idx];

	aqprintf("defaultSerchFolderForDark. ProcNum = %d, =  %s\n",idx,defaultSearchFolder.toLocal8Bit().constData());

	QString filePath;
	if (defaultSearchFolder.length() < 2)
	{
		
		filePath = QFileDialog::getOpenFileName(this, "Open Image","", "Image Files (*.raw)",0,0);
	}
	else
		filePath = QFileDialog::getOpenFileName(this, "Open Image",defaultSearchFolder, "Image Files (*.raw)",0,0);

	if (filePath.length() < 3)
		return;

	lineEditDarkPath->setText(filePath);
	//Fill Dark Image Array with this file

	QMessageBox dlgMsgBox;
	
	if (!qMyApp->LoadDarkImage(filePath))
	{
		dlgMsgBox.setText(QString("Error on loading dark image! path = %1").arg(filePath));
		dlgMsgBox.exec();
		this->lineEditDarkPath->setText("");
	}		
}

void Acquire_4030e_DlgControl::OpenGainImage() //Btn
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	//QString filePath;
	int idx =qMyApp->m_iProcNum;

	QString defaultSearchFolder = qMyApp->m_OptionSettingChild.m_strGainImageSavingFolder[idx];

	aqprintf("defaultSerchFolderForGain =  %s\n",defaultSearchFolder.toLocal8Bit().constData());

	QString filePath;
	if (defaultSearchFolder.length() < 2)
	{
		filePath = QFileDialog::getOpenFileName(this, "Open Gain Image","", "Image Files (*.raw)",0,0);
	}
	else
	{
		filePath = QFileDialog::getOpenFileName(this, "Open Gain Image",defaultSearchFolder, "Image Files (*.raw)",0,0);
	}

	if (filePath.length() < 3)
		return;

	lineEditGainPath->setText(filePath);	

	QMessageBox dlgMsgBox;

	if (!qMyApp->LoadGainImage(filePath))
	{
		dlgMsgBox.setText(QString("Error on loading gain image!%1").arg(filePath));
		dlgMsgBox.exec();

		this->lineEditGainPath->setText("");
	}		
}


void Acquire_4030e_DlgControl::OpenCurImgFolder()
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	
	
	int idx = qMyApp->m_iProcNum;
	QString startFolder = qMyApp->m_OptionSettingChild.m_strAcqFileSavingFolder[idx];
	if (startFolder.length() < 3)
	{
		QDir dir = QDir::current(); //changeable?
		startFolder = dir.path();
	}	

	QString strFolderPath = QFileDialog::getExistingDirectory(this, "Open Directory",startFolder,
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (strFolderPath.length() < 2)
		return;


	this->lineEditCurImageSaveFolder->setText(strFolderPath);
}

void Acquire_4030e_DlgControl::ReDrawImg()
{
	int lowerVal = spinLowerVal->value();
	int upperVal = spinUpperVal->value();

	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	qMyApp->ReDraw(lowerVal, upperVal);//current image only
}

void Acquire_4030e_DlgControl::CopyCurrentImageForGainImage()
{
	QMessageBox msgBox;	
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;	

	if (!qMyApp->GetGainImageFromCurrent())
	{
		aqprintf("Error on Getting Gain Image from current image. \n");
		msgBox.setText("Error on Getting Gain Image from current image!");
		msgBox.exec();
		return;
	}	
}


//Save this setting as a default setting when used during software starts.
void Acquire_4030e_DlgControl::SaveSettingAsDefault_Child()
{
	//1. Update setting data
	//2. Export as a file
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	YKOptionSetting* pOptionSetting = &(((Acquire_4030e_child*)qApp)->m_OptionSettingChild);

	int idx = qMyApp->m_iProcNum;
	
	//Image Panel directory should be set	
	pOptionSetting->m_strDriverFolder[idx] = this->EditDriverPath->text();

	QDir tmpDir = QDir(pOptionSetting->m_strDriverFolder[idx]);
	if (!tmpDir.exists())
	{
		aqprintf("Cannot save this setting.Panel driver should be found!\n");
		return;
	}	

	pOptionSetting->m_iWinLevelUpper[idx] = this->spinUpperVal->value();
	pOptionSetting->m_iWinLevelLower[idx] = this->spinLowerVal->value();;	

	/* File Save */
 	pOptionSetting->m_bSaveToFileAfterAcq[idx] = this->ChkAutoSave->isChecked();
	pOptionSetting->m_bSendImageToDipsAfterAcq[idx] = this->ChkAutoSendToDIPS->isChecked();
	pOptionSetting->m_bSaveDarkCorrectedImage[idx] = this->ChkDarkCorrectedSave->isChecked();
	pOptionSetting->m_bSaveRawImage[idx] = this->ChkRawSave->isChecked();
	pOptionSetting->m_strAcqFileSavingFolder[idx] = this->lineEditCurImageSaveFolder->text();

	/* Panel Control */
	pOptionSetting->m_bSoftwareHandshakingEnabled[idx] = this->RadioSoftHandshakingEnable->isChecked(); //false = hardware handshaking

	/*Dark Image Correction */
	pOptionSetting->m_bDarkCorrectionOn[idx] = this->ChkDarkFieldApply->isChecked();
	pOptionSetting->m_iDarkFrameNum[idx] =  this->SpinDarkAvgFrames->value();
	//m_strDarkImageSavingFolder[idx] = this->;
	pOptionSetting->m_strDarkImagePath[idx] = this->lineEditDarkPath->text(); //should be set later
	pOptionSetting->m_bTimerAcquisitionEnabled[idx] = this->ChkAutoDarkOn->isChecked(); //not implemented yet
	pOptionSetting->m_iTimerAcquisitionMinInterval[idx] = this->SpinTimeInterval->value();//not implemented yet

	//YK TEMP: not implemented yet. leave it as default value	

	/*Gain Image Correction */
	pOptionSetting->m_bGainCorrectionOn[idx] = this->ChkGainCorrectionApply->isChecked();
	pOptionSetting->m_bMultiLevelGainEnabled[idx] = this->RadioMultiGain->isChecked(); // false = single gain correction	
	//m_strGainImageSavingFolder[idx] = leave it as default
	pOptionSetting->m_strSingleGainPath[idx] = this->lineEditGainPath->text();
	pOptionSetting->m_fSingleGainCalibFactor[idx] = (this->lineEditSingleCalibFactor->text()).toDouble();

	pOptionSetting->m_bDefectMapApply[idx] = this->ChkBadPixelCorrApply->isChecked();
	pOptionSetting->m_strDefectMapPath[idx] = this->lineEditBadPixelMapPath->text();



	pOptionSetting->m_bEnableForcedThresholding[idx] = this->ChkForcedThresholding->isChecked();
	pOptionSetting->m_iThresholdVal[idx] = this->lineEditForcedThresholdVal->text().toInt();
	pOptionSetting->m_bDeleteOldImg[idx] = this->ChkDeleteImgAfter->isChecked();
	pOptionSetting->m_iAfterDays[idx] = this->lineEditDeleteAfterDays->text().toInt();
	



	pOptionSetting->ExportChildOption(pOptionSetting->m_defaultChildOptionPath[idx], idx);

}


//should be called when start
void Acquire_4030e_DlgControl::UpdateGUIFromSetting_Child() // it can be used as go_back to default setting
{
	//aqprintf("test1\n");
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;
	YKOptionSetting* pOptionSetting = &(((Acquire_4030e_child*)qApp)->m_OptionSettingChild);
	int idx = qMyApp->m_iProcNum;

	EditDriverPath->setText(pOptionSetting->m_strDriverFolder[idx]);
			
	spinUpperVal->setValue(pOptionSetting->m_iWinLevelUpper[idx]);
	spinLowerVal->setValue(pOptionSetting->m_iWinLevelLower[idx]);	

	///* File Save */
	ChkAutoSave->setChecked(pOptionSetting->m_bSaveToFileAfterAcq[idx]);
	ChkAutoSendToDIPS->setChecked(pOptionSetting->m_bSendImageToDipsAfterAcq[idx]);
	ChkDarkCorrectedSave->setChecked(pOptionSetting->m_bSaveDarkCorrectedImage[idx]);
	ChkRawSave->setChecked(pOptionSetting->m_bSaveRawImage[idx]);		
	lineEditCurImageSaveFolder->setText(pOptionSetting->m_strAcqFileSavingFolder[idx]);

	RadioSoftHandshakingEnable->setChecked(pOptionSetting->m_bSoftwareHandshakingEnabled[idx]);	
	RadioHardHandshakingEnable->setChecked(!pOptionSetting->m_bSoftwareHandshakingEnabled[idx]);	
	
	this->ChkDarkFieldApply->setChecked(pOptionSetting->m_bDarkCorrectionOn[idx]);

	///*Dark Image Correction */	
	this->SpinDarkAvgFrames->setValue(pOptionSetting->m_iDarkFrameNum[idx]);	
	
	lineEditDarkPath->setText(pOptionSetting->m_strDarkImagePath[idx]);

	this->ChkAutoDarkOn->setChecked(pOptionSetting->m_bTimerAcquisitionEnabled[idx]);
	this->SpinTimeInterval->setValue(pOptionSetting->m_iTimerAcquisitionMinInterval[idx]);	

	///*Gain Image Correction */	
	this->ChkGainCorrectionApply->setChecked(pOptionSetting->m_bGainCorrectionOn[idx]);
	
	this->RadioMultiGain->setChecked(pOptionSetting->m_bMultiLevelGainEnabled[idx]);
	this->RadioSingleGain->setChecked(!pOptionSetting->m_bMultiLevelGainEnabled[idx]);
	
	this->lineEditGainPath->setText(pOptionSetting->m_strSingleGainPath[idx]);
	
	QString tmpStr = QString("%1").arg(pOptionSetting->m_fSingleGainCalibFactor[idx]);
	this->lineEditSingleCalibFactor->setText(tmpStr);

	tmpStr = QString("%1").arg(pOptionSetting->m_strDefectMapPath[idx]);
	this->lineEditBadPixelMapPath->setText(tmpStr);
	this->ChkBadPixelCorrApply->setChecked(pOptionSetting->m_bDefectMapApply[idx]);

	
	this->ChkForcedThresholding->setChecked(pOptionSetting->m_bEnableForcedThresholding[idx]);
	this->lineEditForcedThresholdVal->setText(QString("%1").arg(pOptionSetting->m_iThresholdVal[idx]));	

	this->ChkDeleteImgAfter->setChecked(pOptionSetting->m_bDeleteOldImg[idx]);
	this->lineEditDeleteAfterDays->setText(QString("%1").arg(pOptionSetting->m_iAfterDays[idx]));
	
	ReLoadCalibImages();
}

void Acquire_4030e_DlgControl::SingleGainOn()
{
	this->RadioSingleGain->setChecked(true);
	this->RadioMultiGain->setChecked(false);
}
void Acquire_4030e_DlgControl::MultiGainOn()
{
	this->RadioSingleGain->setChecked(false);
	this->RadioMultiGain->setChecked(true);
}

void Acquire_4030e_DlgControl::ReLoadCalibImages()
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	qMyApp->LoadBadPixelMap(this->lineEditBadPixelMapPath->text().toLocal8Bit().constData());
	qMyApp->LoadDarkImage(this->lineEditDarkPath->text());
	qMyApp->LoadGainImage(this->lineEditGainPath->text());
}


void Acquire_4030e_DlgControl::OpenDefectMapFile() //Btn
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	//QString filePath;
	int idx =qMyApp->m_iProcNum;	

	QString defaultSearchFolder = qMyApp->m_OptionSettingChild.m_strDefectMapSavingFolder[idx];

	aqprintf("defaultSerchFolderForDefectMap =  %s\n",defaultSearchFolder.toLocal8Bit().constData());

	QString filePath;
	if (defaultSearchFolder.length() < 2)
	{
		filePath = QFileDialog::getOpenFileName(this, "Open pixel mapping file","", "Pixel Mapping Files (*.pmf)",0,0);
	}
	else
	{
		filePath = QFileDialog::getOpenFileName(this, "Open pixel mapping file",defaultSearchFolder, "Pixel Mapping Files (*.pmf)",0,0);
	}

	if (filePath.length() < 3)
		return;

	lineEditBadPixelMapPath->setText(filePath);	

	QMessageBox dlgMsgBox;

	if (!qMyApp->LoadBadPixelMap(filePath.toLocal8Bit().constData()))
	{
		dlgMsgBox.setText(QString("Error on loading pixel mapping file!_%1").arg(filePath));
		dlgMsgBox.exec();

		this->lineEditBadPixelMapPath->setText("");
	}		
}

void Acquire_4030e_DlgControl::SeeSavingFolder() //Window Explorer for saving folder
{
	QString strCurFolder = this->lineEditCurImageSaveFolder->text();
	strCurFolder.replace('/','\\');
	QString strCommand = QString("explorer %1").arg(strCurFolder);
	::system(strCommand.toLocal8Bit().constData());
	
}

void Acquire_4030e_DlgControl::LoadTestRawImage()
{
	Acquire_4030e_child* qMyApp = (Acquire_4030e_child*)qApp;

	QString filePath;

	filePath = QFileDialog::getOpenFileName(this, "Open Image","", "Image Files (*.raw)",0,0);	
	
	if (filePath.length() < 3)
		return;	

	QMessageBox dlgMsgBox;

	qMyApp->Debug_GetImageFromFile(filePath);
}