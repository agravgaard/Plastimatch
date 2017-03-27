#ifndef GAMMA_GUI_H
#define GAMMA_GUI_H

#include <QtGui/QMainWindow>
#include "ui_gamma_gui.h"
#include <QStringList>
#include <vector>
#include "yk_config.h"

//#include "itkImageFileReader.h"
//#include "itkImageFileWriter.h"
#include "itk_image_type.h"

//#include "YK16GrayImage.h"
//#include <fstream>

using namespace std;

class Gamma_parms;

class DlgGammaView;
class YK16GrayImage;
class qyklabel;
class QStandardItemModel;


class gamma_gui : public QMainWindow
{
    Q_OBJECT

public:
    gamma_gui(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~gamma_gui();     
    
    QString GammaMain(Gamma_parms* parms, float& refDoseGy, const QString& strPathBkupRef = QString(""), const QString& strPathBkupComp = QString(""));

    void Load_FilesToMem();//all ref, comp, gamma map should be prepared    
    void UpdateProbePos(qyklabel* qlabel);    

    void UpdateTable(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3,
    float fNorm1, float fNorm2, float fNorm3, float fMag1, float fMag2, float fMag3);    

    void WhenMousePressedRight(qyklabel* pWnd);
    void UpdatePanCommon(qyklabel* qWnd);

    void RenameFileByDCMInfo(QStringList& filenameList);

    void SaveCurrentGammaWorkSpace(QString& strPathGammaWorkSpace);

    bool LoadGammaWorkSpace(QString& strPathGammaWorkSpace);
    bool ReadProtonDoseSet(QString& strPathProtonDoseSet, ProtonSetFileMGH& protonSetInfo);

    void SaveBatchGamma3DSimpleReport(QString& strFilePath);

    void SetWorkDir(const QString& strPath);

    QString ReplaceUpperDirOnly(QString& strOriginalPath, QString& strCurrDirPath, QString& strDelim);

    QString ConvertMGHProtonDoseToMha(QString& strPathBinnary, VEC3D& fDim, VEC3D& fOrigin, VEC3D& fSpacing);

	bool ConvertOPG2FloatMHA(QString& strFilePathOPG, QString& strFilePathMHA);

    public slots:        
        void SLT_Load_RD_Ref();
        void SLT_Load_RD_Comp();        
        void SLT_LoadResults();

        void SLT_RunBatchGamma();
        void SLT_ProfileView();

        void SLT_DrawDoseImages();// refer to probe positions, selected 3D file (spPointer), plane direction
        void SLT_DrawGammaMap3D();
        void SLT_DrawGammaMap2D();
        void SLT_UpdateComboContents();        

        void SLT_WhenSelectCombo();
        void SLT_DrawAll();
        void SLT_DrawTable();
        void SLT_DrawChart();

        void SLT_WhenChangePlane();//restore zoom and pan then draw all

        void SLT_UpdateReportTxt();

        void SLT_UpdateProbePosRef();
        void SLT_UpdateProbePosComp();
        void SLT_UpdateProbePosGamma2D();
        void SLT_UpdateProbePosGamma3D();
        void SLT_CopyTableToClipboard();
        //void SLT_DrawGraph(); 
        void SLT_DrawGraph(bool bInitMinMax = false);
        void SLT_RunGamma2D();

        void SLT_GoCenterPosRef();
        void SLT_GoCenterPosComp();
        void SLT_NormCompFromRefNorm();        

        void SaveDoseIBAGenericTXTFromItk(QString strFilePath, FloatImage2DType::Pointer& spFloatDose);

        void SLT_MouseWheelUpdateRef();
        void SLT_MouseWheelUpdateComp();
        void SLT_MouseWheelUpdateGamma2D();
        void SLT_MouseWheelUpdateGamma3D();

        void SLT_RestoreZoomPan();

        void SLT_MouseMoveUpdateRef();
        void SLT_MouseMoveUpdateComp();
        void SLT_MouseMoveUpdateGamma2D();
        void SLT_MouseMoveUpdateGamma3D();

        void SLT_MousePressedRightRef();
        void SLT_MousePressedRightComp();
        void SLT_MousePressedRightGamma3D();
        void SLT_MousePressedRightGamma2D();     

        void SLT_MouseReleasedRightRef();
        void SLT_MouseReleasedRightComp();
        void SLT_MouseReleasedRightGamma3D();
        void SLT_MouseReleasedRightGamma2D();

        void SLT_UpdatePanSettingRef();
        void SLT_UpdatePanSettingComp();
        void SLT_UpdatePanSettingGamma3D();
        void SLT_UpdatePanSettingGamma2D();

        void SLTM_RenameRDFilesByDICOMInfo();

        void SLTM_LoadSavedWorkSpace();
        void SLTM_SaveBatchModeSimpleReport();

        void SLT_SetWorkDir();

        void SLTM_ExportBatchReport();

        void SLTM_LoadProtonDoseSetFile();       
		void SLTM_ConvertIBAOPG_Files();

public:    
    QStringList m_strlistPath_RD_Original_Ref; //RD files, before the conversion
    QStringList m_strlistPath_RD_Original_Comp;

    QStringList m_strlistPath_RD_Read_Ref; //mha files, after the conversion
    QStringList m_strlistPath_RD_Read_Comp;

    QStringList m_strlistFileBaseName_Ref;
    QStringList m_strlistFileBaseName_Comp;
    
    QStringList m_strlistBatchReport;
    
    QStringList m_strlistPath_Output_Gammamap;
    QStringList m_strlistPath_Output_Failure;
    QStringList m_strlistPath_Output_Report;

    vector<float> m_vRefDose;

    //DlgGammaView* m_pView;    

    vector<FloatImageType::Pointer> m_vRefDoseImages;
    vector<FloatImageType::Pointer> m_vCompDoseImages;
    vector<FloatImageType::Pointer> m_vGammaMapImages;

    //checkBox_low_mem
    FloatImageType::Pointer m_spDummyLowMemImg;

    FloatImage2DType::Pointer m_spCurRef2D;
    FloatImage2DType::Pointer m_spCurComp2D;
    FloatImage2DType::Pointer m_spGamma2DResult; //Read from output of 2D gamma

    /*FloatImageType::Pointer m_spRefDoseImages;
    FloatImageType::Pointer m_spCompDoseImages;
    FloatImageType::Pointer m_spGammaMapImages;*/

    YK16GrayImage* m_pCurImageRef;
    YK16GrayImage* m_pCurImageComp;
    YK16GrayImage* m_pCurImageGamma3D;
    YK16GrayImage* m_pCurImageGamma2D;
    QStandardItemModel *m_pTableModel;

    vector<VEC3D> m_vColormapDose;
    vector<VEC3D> m_vColormapGamma;

    bool m_bGamma2DIsDone;

    bool m_bMousePressedRightRef;
    bool m_bMousePressedRightComp;
    bool m_bMousePressedRightGamma3D;
    bool m_bMousePressedRightGamma2D;

    QPoint m_ptPanStart;
    QPoint m_ptOriginalDataOffset;

    QString m_strPathDirWorkDir;//this is for output

    QString m_strPathInputDir;//this is for input DCM. initialized when Load Ref files or Load Comp files

    int m_iLastLoadedIndex;

private:
    Ui::gamma_guiClass ui;
};

#endif // gamma_gui_H
