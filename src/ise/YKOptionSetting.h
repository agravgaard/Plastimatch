#pragma once

#include <QDir>
#include <QString>

class YKOptionSetting //class for acquire4030e
{
public:
	YKOptionSetting(void);
	~YKOptionSetting(void);

public:	
	QDir m_crntDir;	
	bool GenDefaultFolders();
	void CheckAndMakeDir(QString& dirPath);
	bool CheckAndLoadOptions_Parent();
	bool CheckAndLoadOptions_Child(int idx);

	bool LoadParentOption(QString& parentOptionPath);
	bool LoadChildOption(QString& childOptionPath, int idx); //index = 0 or 1

	bool LoadParentOptionDefault();	
	bool LoadChildOptionDefault(int idx);
	
	bool ExportParentOption( QString& filePath);
	bool ExportChildOption( QString& filePath, int idx);

	QString m_defaultParentOptionPath;
	QString m_defaultChildOptionPath[2];

	

// Default folder name variables
public:
	/*QString m_dirLevel0;

	QString m_dirLevel1_0;
	QString m_dirLevel1_1;
	QString m_dirLevel1_2;

	QString m_dirLevel2_0;
	QString m_dirLevel2_1;
	QString m_dirLevel2_2;

	QString m_dirLevel3_0;
	QString m_dirLevel3_1;
	QString m_dirLevel3_2;

	QString m_dirLevel3_2_0;
	QString m_dirLevel3_2_1;*/
public:
	/*OptionsForParent*/
	QString m_strPrimaryLogPath;
	QString m_strAlternateLogPath; //alternative log is saved by copying edit box's contents. It may be "P:\"
	
	//Primary log is being saved in LOG sub-folder

	/*OptionsForChild_0, 1*/
	//int m_iMode; //current = 0 (Rad). if FLU mode, different GUI(DlgPanel) should be developed

		/* Panel Info */
	QString m_strDriverFolder[2]; //MostImportant
	int m_iWinLevelUpper[2];
	int m_iWinLevelLower[2];

		/* File Save */
	bool m_bSaveToFileAfterAcq[2];
	bool m_bSendImageToDipsAfterAcq[2];
	bool m_bSaveDarkCorrectedImage[2];
	bool m_bSaveRawImage[2];
	QString m_strAcqFileSavingFolder[2];

		/* Panel Control */
	bool m_bSoftwareHandshakingEnabled[2]; //true = soft-on, hard-off
	
		/*Dark Image Correction */
	bool m_bDarkCorrectionOn[2];
	int m_iDarkFrameNum[2];
	QString m_strDarkImageSavingFolder[2]; //not explicitly editable
	QString m_strDarkImagePath[2];
	bool m_bTimerAcquisitionEnabled[2]; //not implemented yet
	int m_iTimerAcquisitionMinInterval[2];//not implemented yet

	double m_fDarkCufoffUpperMean[2];
	double m_fDarkCufoffLowerMean[2];

	double m_fDarkCufoffUpperSD[2];
	double m_fDarkCufoffLowerSD[2];	

		/*Gain Image Correction */
	bool m_bGainCorrectionOn[2];
	bool m_bMultiLevelGainEnabled[2]; // false = single gain correction	
	QString m_strGainImageSavingFolder[2]; //not explicitly editable
	QString m_strSingleGainPath[2];
	double m_fSingleGainCalibFactor[2];

	QString m_strDefectMapSavingFolder[2]; //default
	QString m_strDefectMapPath[2];
	bool m_bDefectMapApply[2];

	
	bool m_bEnableForcedThresholding[2];
	int m_iThresholdVal[2];

	bool m_bDeleteOldImg[2];
	int m_iAfterDays[2];
	
	//bool CopyTrivialChildOptions(int idxS, int idxT);
};