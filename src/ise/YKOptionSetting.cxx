#include "YKOptionSetting.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <fstream>
#include "aqprintf.h"
#include "acquire_4030e_define.h"

using namespace std;


YKOptionSetting::YKOptionSetting(void)
{
	m_crntDir = QDir::current(); //folder where current exe file exists.
	QString crntPathStr = m_crntDir.absolutePath();

	//printf("print current path %s\n",crntPathStr.toLocal8Bit().constData());
	
	m_defaultParentOptionPath = crntPathStr + "\\ProgramData" + "\\ParentOption.cfg";
	m_defaultChildOptionPath[0] = crntPathStr + "\\ProgramData" + "\\PANEL_0" + "\\ChildOption_0.cfg"; //this file contains all3 mode's option
	m_defaultChildOptionPath[1]= crntPathStr + "\\ProgramData" + "\\PANEL_1" + "\\ChildOption_1.cfg";	 

	for (int idx = 0 ; idx<2 ; idx++)
	{
		m_strAcqFileSavingFolder[idx] = crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\IMAGE_DATA";
		m_strDarkImageSavingFolder[idx]= crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\DARK"; //not explicitly editable
		m_strGainImageSavingFolder[idx]= crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\GAIN"; //not explicitly editable
		m_strDefectMapSavingFolder[idx]= crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\DEFECT"; //not explicitly editable;
	}	

	
}


YKOptionSetting::~YKOptionSetting(void)
{
}

bool YKOptionSetting::GenDefaultFolders()
{	
	QString crntPathStr = m_crntDir.absolutePath();

	//QString strDir0 = crntPathStr.append("\\").append(m_dirLevel0);
	// instead of dirName, crntPathStr.append("\\").append(m_dirLevel0) itself can be used

	QStringList listDir;

	QString strDir0 = crntPathStr + "\\" + "ProgramData";
	//Default folder is hard-coded
	listDir << strDir0;

	QString strDir1_0 = strDir0 + "\\" + "LOG";
	listDir << strDir1_0;
	QString strDir1_1 = strDir0 + "\\" + "PANEL_0";
	listDir << strDir1_1;
	QString strDir1_2 = strDir0 + "\\" + "PANEL_1";
	listDir << strDir1_2;
	
	//PANEL 0
	QString strDir1_1_0 = strDir1_1+"\\"+"01_RAD_2304_3200";
	QString strDir1_1_1 = strDir1_1+"\\"+"02_FLU_2304_3200";
	QString strDir1_1_2 = strDir1_1+"\\"+"03_FLU_1152_1600";

	listDir << strDir1_1_0 << strDir1_1_1 << strDir1_1_2;

	QString strDir1_1_0_0 = strDir1_1_0+"\\"+"DARK";
	QString strDir1_1_0_1 = strDir1_1_0+"\\"+"DEFECT";
	QString strDir1_1_0_2 = strDir1_1_0+"\\"+"GAIN";
	QString strDir1_1_0_3 = strDir1_1_0+"\\"+"IMAGE_DATA";

	listDir << strDir1_1_0_0 << strDir1_1_0_1 << strDir1_1_0_2 << strDir1_1_0_3;

	QString strDir1_1_1_0 = strDir1_1_1+"\\"+"DARK";
	QString strDir1_1_1_1 = strDir1_1_1+"\\"+"DEFECT";
	QString strDir1_1_1_2 = strDir1_1_1+"\\"+"GAIN";
	QString strDir1_1_1_3 = strDir1_1_1+"\\"+"IMAGE_DATA";

	listDir << strDir1_1_1_0 << strDir1_1_1_1 << strDir1_1_1_2 << strDir1_1_1_3;



	QString strDir1_1_2_0 = strDir1_1_2+"\\"+"DARK";
	QString strDir1_1_2_1 = strDir1_1_2+"\\"+"DEFECT";
	QString strDir1_1_2_2 = strDir1_1_2+"\\"+"GAIN";
	QString strDir1_1_2_3 = strDir1_1_2+"\\"+"IMAGE_DATA";

	listDir << strDir1_1_2_0 << strDir1_1_2_1 << strDir1_1_2_2 << strDir1_1_2_3;

	//PANEL 1
	QString strDir1_2_0 = strDir1_2+"\\"+"01_RAD_2304_3200";
	QString strDir1_2_1 = strDir1_2+"\\"+"02_FLU_2304_3200";
	QString strDir1_2_2 = strDir1_2+"\\"+"03_FLU_1152_1600";

	listDir << strDir1_2_0 << strDir1_2_1 << strDir1_2_2;

	QString strDir1_2_0_0 = strDir1_2_0+"\\"+"DARK";
	QString strDir1_2_0_1 = strDir1_2_0+"\\"+"DEFECT";
	QString strDir1_2_0_2 = strDir1_2_0+"\\"+"GAIN";
	QString strDir1_2_0_3 = strDir1_2_0+"\\"+"IMAGE_DATA";

	listDir << strDir1_2_0_0 << strDir1_2_0_1 << strDir1_2_0_2 << strDir1_2_0_3;

	QString strDir1_2_1_0 = strDir1_2_1+"\\"+"DARK";
	QString strDir1_2_1_1 = strDir1_2_1+"\\"+"DEFECT";
	QString strDir1_2_1_2 = strDir1_2_1+"\\"+"GAIN";
	QString strDir1_2_1_3 = strDir1_2_1+"\\"+"IMAGE_DATA";

	listDir << strDir1_2_1_0 << strDir1_2_1_1 << strDir1_2_1_2 << strDir1_2_1_3;

	QString strDir1_2_2_0 = strDir1_2_2+"\\"+"DARK";
	QString strDir1_2_2_1 = strDir1_2_2+"\\"+"DEFECT";
	QString strDir1_2_2_2 = strDir1_2_2+"\\"+"GAIN";
	QString strDir1_2_2_3 = strDir1_2_2+"\\"+"IMAGE_DATA";

	listDir << strDir1_2_2_0 << strDir1_2_2_1 << strDir1_2_2_2 << strDir1_2_2_3;

	for (int i = 0 ; i<listDir.size() ; i++)
	{
		CheckAndMakeDir((QString&)listDir.at(i));
	}

	return true;
}

bool YKOptionSetting::CheckAndLoadOptions_Parent()
{
	//QMessageBox msg;
	//Check option file is exists
	QFileInfo parentFileInfo(m_defaultParentOptionPath);	

	//QFileInfo childFileInfo1(m_defaultChildOptionPath[0]);

	bool bRecreationNeeded = false;

	if (parentFileInfo.exists())
	{
		if (!LoadParentOption(m_defaultParentOptionPath))
		{
			//msg.setText("Error on parent option file. File will be overwritten with default setting.");
			//msg.exec();
			printf("Error on parent option file. File will be overwritten with default setting.\n");
			bRecreationNeeded = true;
		}
	}

	if (!parentFileInfo.exists() || bRecreationNeeded)
	{
		LoadParentOptionDefault(); //fill parent option variables with default values.
		ExportParentOption(m_defaultParentOptionPath); //export current parent options to a specified file.
	}

	return true;
}


bool YKOptionSetting::CheckAndLoadOptions_Child(int idx)
{
	//aqprintf("test1\n");
	//aqprintf("idx = %d\n", idx);

	QFileInfo childtFileInfo[2];
	childtFileInfo[0] = QFileInfo(m_defaultChildOptionPath[0]);
	childtFileInfo[1] = QFileInfo(m_defaultChildOptionPath[1]);


	QMessageBox msg;	

	//for (int i = 0 ; i<2 ; i++)
	//{
	bool bRecreationNeeded = false;

	if (childtFileInfo[idx].exists())
	{
		//printf("after fileinfo exist\n");
		if (!LoadChildOption(m_defaultChildOptionPath[idx], idx))
		{
			msg.setText("Error on child option file. File will be overwritten with default setting.");
			msg.exec();
			aqprintf("Error on child option file. File will be overwritten with default setting.");
			bRecreationNeeded = true;
		}
	}

	//aqprintf("test2\n");

	if (!childtFileInfo[idx].exists() || bRecreationNeeded)
	{
		//aqprintf("test3 bfore LoadChildOptionDefault\n");

		if (!LoadChildOptionDefault(idx)) //fill parent option variables with default values.
			return false;
		else
			ExportChildOption(m_defaultChildOptionPath[idx], idx);
	}
	//}
	return true;
}


void YKOptionSetting::CheckAndMakeDir(QString& dirPath)
{
	QDir tmpDir = QDir(dirPath);

	if (!tmpDir.exists())
		tmpDir.mkpath(dirPath);
}

bool YKOptionSetting::LoadParentOption( QString& parentOptionPath )
{
	aqprintf("Loading options for parent process\n");

	ifstream fin;
	fin.open(parentOptionPath.toLocal8Bit().constData()); //find it in same folder

	if (fin.fail())	
	{
		aqprintf("Option file is not found\n");
		return false;
	}
	char str[MAX_LINE_LENGTH];

	QStringList strList;
	while(!fin.eof())
	{
		memset (str, 0, MAX_LINE_LENGTH);
		fin.getline(str, MAX_LINE_LENGTH); // for header

		strList << str;
	}
	fin.close();


	if (strList.size() < 1)
		return false;
	//first should have some keywords
	if (!strList.at(0).contains("ACQUIRE4030E_OPTION_PARAMETER_PARENT"))
		return false;


	QString tmpReadString;
	QStringList TokenList;

	QString tokenFirst;
	QString tokenSecond;

	for (int i = 0 ; i<strList.size() ; i++)
	{
		tmpReadString = strList.at(i);
		TokenList.clear();

		TokenList = tmpReadString.split("\t");

		if (TokenList.size() > 1)
		{
			tokenFirst = TokenList.at(0);
			tokenSecond= TokenList.at(1);
		}
		else
			continue; //pass this line

		if (tokenFirst.contains("PRIMARY_LOG_PATH"))
		{
			m_strPrimaryLogPath = tokenSecond;			
		}
		else if (tokenFirst.contains("ALTERNATIVE_LOG_PATH"))
		{
			m_strAlternateLogPath = tokenSecond;
		}		
	}	
	return true;
}

bool YKOptionSetting::LoadChildOption( QString& childOptionPath, int idx )
{
	aqprintf("Loading options for child process\n");

	ifstream fin;
	fin.open(childOptionPath.toLocal8Bit().constData()); //find it in same folder

	if (fin.fail())	
	{
		aqprintf("Option file is not found\n");
		return false;
	}
	char str[MAX_LINE_LENGTH];

	QStringList strList;

	while(!fin.eof())
	{
		memset (str, 0, MAX_LINE_LENGTH);
		fin.getline(str, MAX_LINE_LENGTH); // for header

		strList << str;
	}
	fin.close();


	if (strList.size() < 1)
		return false;
	//first should have some keywords
	if (!strList.at(0).contains("ACQUIRE4030E_OPTION_PARAMETER_CHILD"))
		return false;


	QString tmpReadString;
	QStringList TokenList;

	QString tokenFirst;
	QString tokenSecond;

	for (int i = 0 ; i<strList.size() ; i++)
	{
		tmpReadString = strList.at(i);

		TokenList.clear();
		TokenList = tmpReadString.split("\t");

		if (TokenList.size() > 1)
		{
			tokenFirst = TokenList.at(0);
			tokenSecond= TokenList.at(1);
		}
		else
			continue; //pass this line


		if (tokenFirst.contains("DRIVER_FOLDER"))
		{		
			m_strDriverFolder[idx] = tokenSecond;			
			QDir tmpDir = QDir(m_strDriverFolder[idx]);
			if (!tmpDir.exists())
			{
				aqprintf("Driver directory couldn't be found\n");
				return false;
			}
		}
		else if (tokenFirst.contains("WIN_LEVEL_UPPER"))
		{
			m_iWinLevelUpper[idx] = tokenSecond.toInt();
			
		}
		else if (tokenFirst.contains("WIN_LEVEL_LOWER"))
		{
			m_iWinLevelLower[idx] = tokenSecond.toInt();
		}


		else if (tokenFirst.contains("ENABLE_SAVE_TO_FILE"))
		{
			m_bSaveToFileAfterAcq[idx] = (bool)tokenSecond.toInt();

		}
		else if (tokenFirst.contains("ENABLE_SEND_TO_DIPS"))
		{
			m_bSendImageToDipsAfterAcq[idx] = (bool)tokenSecond.toInt();

		}
		else if (tokenFirst.contains("ENABLE_SAVE_DARK"))
		{
			m_bSaveDarkCorrectedImage[idx] = (bool)tokenSecond.toInt();

		}
		else if (tokenFirst.contains("ENABLE_SAVE_RAW"))
		{
			m_bSaveRawImage[idx] = (bool)tokenSecond.toInt();
		}
		else if (tokenFirst.contains("SAVING_FOLDER_PATH"))
		{
			m_strAcqFileSavingFolder[idx] = tokenSecond;
			QDir tmpDir = QDir(m_strAcqFileSavingFolder[idx]);
			if (!tmpDir.exists())
			{
				m_strAcqFileSavingFolder[idx] = "C:\\";
			}			
		}

		else if (tokenFirst.contains("ENABLE_SOFTWARE_HANDSHAKING"))
		{
			m_bSoftwareHandshakingEnabled[idx] = (bool)tokenSecond.toInt();
		}


		else if (tokenFirst.contains("ENABLE_DARK_CORRECTION_APPLY"))
		{
			m_bDarkCorrectionOn[idx] =  (bool)tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_FRAME_NUM"))
		{
			m_iDarkFrameNum[idx] = tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_IMAGE_PATH"))
		{
			m_strDarkImagePath[idx] = tokenSecond;

			QFileInfo tmpInfo = QFileInfo(m_strDarkImagePath[idx]);			

			if (!tmpInfo.exists())
			{
				m_strDarkImagePath[idx] = "";
			}
		}

		else if (tokenFirst.contains("ENABLE_DARK_TIMER"))
		{
			m_bTimerAcquisitionEnabled[idx] =  (bool)tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_TIMER_INTERVAL_MIN"))
		{
			m_iTimerAcquisitionMinInterval[idx] =  tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_CUTOFF_UPPER_MEAN"))
		{
			m_fDarkCufoffUpperMean[idx] =  tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_CUTOFF_LOWER_MEAN"))
		{
			m_fDarkCufoffLowerMean[idx] =  tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_CUTOFF_UPPER_SD"))
		{
			m_fDarkCufoffUpperSD[idx] =  tokenSecond.toInt();

		}
		else if (tokenFirst.contains("DARK_CUTOFF_LOWER_SD"))
		{
			m_fDarkCufoffLowerSD[idx] =  tokenSecond.toInt();

		}

		else if (tokenFirst.contains("ENABLE_GAIN_CORRECTION_APPLY"))
		{
			m_bGainCorrectionOn[idx] =  (bool)tokenSecond.toInt();

		}
		else if (tokenFirst.contains("ENABLE_MULTI_LEVEL_GAIN"))
		{
			m_bMultiLevelGainEnabled[idx] =  (bool)tokenSecond.toInt();
		}
		else if (tokenFirst.contains("GAIN_SINGLE_PATH"))
		{
			m_strSingleGainPath[idx] = tokenSecond;

			QFileInfo tmpInfo = QFileInfo(m_strSingleGainPath[idx]);			

			if (!tmpInfo.exists())
			{
				m_strSingleGainPath[idx] = "";
			}
		}
		else if (tokenFirst.contains("GAIN_CALIB_FACTOR"))
		{
			m_fSingleGainCalibFactor[idx] =  tokenSecond.toDouble();
		}		


		else if (tokenFirst.contains("BAD_PIXEL_MAP_PATH"))
		{
			m_strDefectMapPath[idx] = tokenSecond;			

			QFileInfo tmpInfo = QFileInfo(m_strDefectMapPath[idx]);			

			if (!tmpInfo.exists())
			{
				m_strDefectMapPath[idx] = "";
			}
		}
		else if (tokenFirst.contains("ENABLE_DEFECT_CORRECTION_APPLY"))
		{
			m_bDefectMapApply[idx] = (bool)tokenSecond.toInt();
		}	

		else if (tokenFirst.contains("ENABLE_FORCED_THRESHOLDING"))
		{
			m_bEnableForcedThresholding[idx] = (bool)tokenSecond.toInt();
		}	
		else if (tokenFirst.contains("THRESHOLDING_VALUE"))
		{
			m_iThresholdVal[idx] = tokenSecond.toInt();
		}	
		else if (tokenFirst.contains("ENABLE_OLD_IMG_DELETION"))
		{
			m_bDeleteOldImg[idx] = (bool)tokenSecond.toInt();
		}	
		else if (tokenFirst.contains("OLD_IMG_DELETION_PERIOD"))
		{
			m_iAfterDays[idx] = tokenSecond.toInt();
		}	
	}
	return true;
}



bool YKOptionSetting::LoadParentOptionDefault()
{
	QString crntPathStr = m_crntDir.absolutePath();	

	m_strPrimaryLogPath = crntPathStr + "\\" + "ProgramData" + "\\" + "LOG";
	m_strAlternateLogPath = "\\\\Dfa15\\ro_publc$\\Proton_Fluoro\\4030E_LOG\\STAR";
	//will be filled later, if in case.
	crntPathStr = m_crntDir.absolutePath();		

	//if (m_strDriverFolder[idx].length() < 3)
	//	return false;


	return true;
}

bool YKOptionSetting::LoadChildOptionDefault( int idx ) //should be called after all the default directories are completely generated
{
	QString crntPathStr = m_crntDir.absolutePath();	

	//Image Panel directory should be set
	if (idx == 0)
		m_strDriverFolder[idx] = QFileDialog::getExistingDirectory(0, "Open Driver Directory For Panel_0 (axial). Proper folder selection is mandatory!","C:\\IMAGERs",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);	
	else
		m_strDriverFolder[idx] = QFileDialog::getExistingDirectory(0, "Open Driver Directory For Panel_1 (g90). Proper folder selection is mandatory!","C:\\IMAGERs",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
	
	if (m_strDriverFolder[idx].length() < 3)
		return false;

	m_iWinLevelUpper[idx] = 15000;
	m_iWinLevelLower[idx] = 0;	
	
	/* File Save */
	m_bSaveToFileAfterAcq[idx] = true;
	m_bSendImageToDipsAfterAcq[idx] = true;
	m_bSaveDarkCorrectedImage[idx] = true;
	m_bSaveRawImage[idx] = true;
	m_strAcqFileSavingFolder[idx] = crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\IMAGE_DATA";

	/* Panel Control */
	m_bSoftwareHandshakingEnabled[idx] = false; //false = hardware handshaking

	/*Dark Image Correction */
	m_bDarkCorrectionOn[idx] = true;
	m_iDarkFrameNum[idx] =  4;
	m_strDarkImageSavingFolder[idx] = crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\DARK";
	m_strDarkImagePath[idx] = ""; //should be set later
	m_bTimerAcquisitionEnabled[idx] = false; //not implemented yet
	m_iTimerAcquisitionMinInterval[idx] = 60;//not implemented yet

	m_fDarkCufoffUpperMean[idx] = 2500;
	m_fDarkCufoffLowerMean[idx] = 500;

	m_fDarkCufoffUpperSD[idx] = 200;
	m_fDarkCufoffLowerSD[idx] = 10;	

	/*Gain Image Correction */
	m_bGainCorrectionOn[idx] = true;
	m_bMultiLevelGainEnabled[idx] = false; // false = single gain correction	
	m_strGainImageSavingFolder[idx] = crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\GAIN";
	m_strSingleGainPath[idx] = ""; //should be set later
	m_fSingleGainCalibFactor[idx] = 1.000;

	m_strDefectMapPath[idx] = crntPathStr + "\\ProgramData" + QString("\\PANEL_%1").arg(idx) + "\\01_RAD_2304_3200" + "\\DEFECT";
	m_strDefectMapPath[idx] = "";//should be set later	
	m_bDefectMapApply[idx] = false;


//	m_bEnbleCustomThre[idx] = false; //apply after Gain Correction
//	m_iCustomThreshold[idx] = 13000; //apply after Gain Correction		

	m_bDeleteOldImg[idx] = true;
	m_iAfterDays[idx] = 15;	

	return true;
}


bool YKOptionSetting::ExportParentOption( QString& filePath)
{
	ofstream fout;
	fout.open(filePath.toLocal8Bit().constData());

	if (fout.fail())
		return false;

	fout << "% ACQUIRE4030E_OPTION_PARAMETER_PARENT" << endl;
	fout << "% Symbols:(%%: comment, #: Parameter data, $: special character for list)" << endl;
	fout << "% It is not recommended to edit this file manually." << endl;

	fout << endl;

	fout << "$BEGIN_OF_OPTION" << endl;

	fout << "#PRIMARY_LOG_PATH" << "	" <<  m_strPrimaryLogPath.toLocal8Bit().constData()<< endl;	
	fout << "%ALTERNATIVE_LOG_PATH is alternative log file path in addition to default log path (\\LOG). Usually it can be set network directory." << endl;
	fout << "#ALTERNATIVE_LOG_PATH" << "	" << m_strAlternateLogPath.toLocal8Bit().constData() << endl;	

	fout << "$END_OF_OPTION" << endl;
	fout.close();

	return true;
}
bool YKOptionSetting::ExportChildOption( QString& filePath, int idx)
{
	ofstream fout;
	fout.open(filePath.toLocal8Bit().constData());

	if (fout.fail())
		return false;

	/**********************************************/
	fout << "% ACQUIRE4030E_OPTION_PARAMETER_CHILD" << endl;
	fout << "% Symbols:(%: comment, #: Parameter data, $: special character for list)" << endl;
	fout << "% It is not recommended to edit this file manually." << endl;

	fout << endl;

	/**********************************************/
	fout << "$BEGIN_OF_PANELINFO" << endl;

	fout << "#DRIVER_FOLDER" << "	" << m_strDriverFolder[idx].toLocal8Bit().constData() << endl;
	fout << "#WIN_LEVEL_UPPER" << "	" << m_iWinLevelUpper[idx] << endl;
	fout << "#WIN_LEVEL_LOWER" << "	" << m_iWinLevelLower[idx] << endl;

	
	fout << "$END_OF_PANELINFO" << endl;
	fout << endl;


	/**********************************************/
	fout << "$BEGIN_OF_FILESAVE" << endl;

	fout << "#ENABLE_SAVE_TO_FILE" << "	" << m_bSaveToFileAfterAcq[idx] << endl;
	fout << "#ENABLE_SEND_TO_DIPS" << "	" << m_bSendImageToDipsAfterAcq[idx] << endl;
	fout << "#ENABLE_SAVE_DARK" << "	" << m_bSaveDarkCorrectedImage[idx] << endl;
	fout << "#ENABLE_SAVE_RAW" << "	" << m_bSaveRawImage[idx] << endl;
	fout << "#SAVING_FOLDER_PATH" << "	" << m_strAcqFileSavingFolder[idx].toLocal8Bit().constData() << endl;




	fout << "$END_OF_FILESAVE" << endl;
	fout << endl;

	/**********************************************/
	fout << "$BEGIN_OF_PANELCONTROL" << endl;
	fout << "#ENABLE_SOFTWARE_HANDSHAKING" << "	" << m_bSoftwareHandshakingEnabled[idx] << endl;



	fout << "$END_OF_PANELCONTROL" << endl;
	fout << endl;

	/**********************************************/
	
	fout << "$BEGIN_OF_DARKCORRECTION" << endl;	
	fout << "#ENABLE_DARK_CORRECTION_APPLY" << "	" << m_bDarkCorrectionOn[idx] << endl;
	fout << "#DARK_FRAME_NUM" << "	" << m_iDarkFrameNum[idx] << endl;
	//fout << "#DARK_IMAGE_SAVING_FOLDER" << "	" << m_strDarkImageSavingFolder << endl;
	fout << "#DARK_IMAGE_PATH" << "	" << m_strDarkImagePath[idx].toLocal8Bit().constData() << endl;
	fout << "#ENABLE_DARK_TIMER" << "	" << m_bTimerAcquisitionEnabled[idx] << endl;
	fout << "#DARK_TIMER_INTERVAL_MIN" << "	" << m_iTimerAcquisitionMinInterval[idx] << endl;
	fout << "#DARK_CUTOFF_UPPER_MEAN" << "	" << m_fDarkCufoffUpperMean[idx] << endl;
	fout << "#DARK_CUTOFF_LOWER_MEAN" << "	" << m_fDarkCufoffLowerMean[idx] << endl;
	fout << "#DARK_CUTOFF_UPPER_SD" << "	" << m_fDarkCufoffUpperSD[idx] << endl;
	fout << "#DARK_CUTOFF_LOWER_SD" << "	" << m_fDarkCufoffLowerSD[idx] << endl;

	fout << "$END_OF_DARKCORRECTION" << endl;
	fout << endl;
	/**********************************************/

	fout << "$BEGIN_OF_GAINCORRECTION" << endl;	
	fout << "#ENABLE_GAIN_CORRECTION_APPLY" << "	" << m_bGainCorrectionOn[idx] << endl;
	fout << "#ENABLE_MULTI_LEVEL_GAIN" << "	" << m_bMultiLevelGainEnabled[idx] << endl;
	//fout << "#DARK_IMAGE_SAVING_FOLDER" << "	" << m_strDarkImageSavingFolder << endl;
	fout << "#GAIN_SINGLE_PATH" << "	" << m_strSingleGainPath[idx].toLocal8Bit().constData() << endl;
	fout << "#GAIN_CALIB_FACTOR" << "	" << m_fSingleGainCalibFactor[idx] << endl;

	fout << "$END_OF_GAINCORRECTION" << endl;	
	/**********************************************/

	fout << "$BEGIN_OF_DEFECT_CORRECTION" << endl;	
	fout << "#ENABLE_DEFECT_CORRECTION_APPLY" << "	" << m_bDefectMapApply[idx] << endl;	
	fout << "#BAD_PIXEL_MAP_PATH" << "	" << m_strDefectMapPath[idx].toLocal8Bit().constData() << endl;	
	fout << "$END_OF_DEFECT_CORRECTION" << endl;	
	/**********************************************/

	//Miscelleneous//
	fout << "$BEGIN_OF_MISCELLENEOUS" << endl;	
	fout << "#ENABLE_FORCED_THRESHOLDING" << "	" << m_bEnableForcedThresholding[idx] << endl;	
	fout << "#THRESHOLDING_VALUE" << "	" << m_iThresholdVal[idx] << endl;	
	fout << "#ENABLE_OLD_IMG_DELETION_" << "	" << m_bDeleteOldImg[idx] << endl;	
	fout << "#OLD_IMG_DELETION_PERIOD" << "	" << m_iAfterDays[idx] << endl;	
	fout << "$END_OF_MISCELLENEOUS" << endl;	

	fout.close();

	return true;
}


//Copy all of the options from src to target except for panel dependent information
//
//bool YKOptionSetting::CopyTrivialChildOptions(int idxS, int idxT) //not used
//{
//	if (idxS != 0 && idxS != 1)
//		return false;
//	if (idxT != 0 && idxT != 1)
//		return false;
//	if (idxS == idxT)
//		return false;
//
//	//m_strDriverFolder[idxT] = m_strDriverFolder[idxS]; // should not be copied
//	m_iWinLevelUpper[idxT] = m_iWinLevelUpper[idxS];
//	m_iWinLevelLower[idxT] = m_iWinLevelLower[idxS];	
//
//	/* File Save */
//	m_bSaveToFileAfterAcq[idxT] = m_bSaveToFileAfterAcq[idxS];
//	m_bSendImageToDipsAfterAcq[idxT] = m_bSendImageToDipsAfterAcq[idxS];
//	m_bSaveDarkCorrectedImage[idxT] = m_bSaveDarkCorrectedImage[idxS];
//	m_bSaveRawImage[idxT] = m_bSaveRawImage[idxS];
//	//m_strAcqFileSavingFolder[idxT] = m_strAcqFileSavingFolder[idxS];
//
//	/* Panel Control */
//	m_bSoftwareHandshakingEnabled[idxT] = m_bSoftwareHandshakingEnabled[idxS]; //false = hardware handshaking
//
//	/*Dark Image Correction */
//	m_bDarkCorrectionOn[idxT] = m_bDarkCorrectionOn[idxS];
//	m_iDarkFrameNum[idxT] =  m_iDarkFrameNum[idxS];
//	//m_strDarkImageSavingFolder[idxT] = m_strDarkImageSavingFolder[idxS];
//	//m_strDarkImagePath[idxT] = m_strDarkImagePath[idxS]; //should be set later
//	m_bTimerAcquisitionEnabled[idxT] = m_bTimerAcquisitionEnabled[idxS]; //not implemented yet
//	m_iTimerAcquisitionMinInterval[idxT] = m_iTimerAcquisitionMinInterval[idxS];//not implemented yet
//
//	m_fDarkCufoffUpperMean[idxT] = m_fDarkCufoffUpperMean[idxS];
//	m_fDarkCufoffLowerMean[idxT] = m_fDarkCufoffLowerMean[idxS];
//
//	m_fDarkCufoffUpperSD[idxT] = m_fDarkCufoffUpperSD[idxS];
//	m_fDarkCufoffLowerSD[idxT] = m_fDarkCufoffLowerSD[idxS];	
//
//	/*Gain Image Correction */
//	m_bGainCorrectionOn[idxT] = m_bGainCorrectionOn[idxS];
//	m_bMultiLevelGainEnabled[idxT] = m_bMultiLevelGainEnabled[idxS]; // false = single gain correction	
//	//m_strGainImageSavingFolder[idxT] = m_strGainImageSavingFolder[idxS];
//	//m_strSingleGainPath[idxT] = m_strSingleGainPath[idxS];
//	m_fSingleGainCalibFactor[idxT] = m_fSingleGainCalibFactor[idxS];
//
//	return true;
//}
