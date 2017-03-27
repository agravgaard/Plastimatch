#ifndef BEAMDATA_GEN_GUI_H
#define BEAMDATA_GEN_GUI_H

#include <QtGui/QMainWindow>
#include "ui_beamdata_gen_gui.h"
#include <QStringList>
#include <vector>
#include "yk_config.h"

#include "itk_image_type.h"

#include "BeamDataRFA.h"

using namespace std;

class DlgGammaView;
class YK16GrayImage;
class qyklabel;
class QStandardItemModel;

class beamdata_gen_gui : public QMainWindow
{
    Q_OBJECT

public:
    beamdata_gen_gui(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~beamdata_gen_gui(); 

    void UpdateProbePos(qyklabel* qlabel, enPLANE enPlane);
    void UpdateTable(vector<QPointF>& vData1, float fNorm, float fMag);
    void MouseWheelUpdateCommon(enPLANE curPlane);    

    void WhenMousePressedRight(qyklabel* pWnd);
    void UpdatePanCommon(qyklabel* qWnd);

    void RenameFileByDCMInfo(QStringList& filenameList);    
    //QString ReplaceUpperDirOnly(QString& strOriginalPath, QString& strCurrDirPath, QString& strDelim);       

    void SaveDoseIBAGenericTXTFromItk(QString strFilePath, FloatImage2DType::Pointer& spFloatDose);


    void UpdateBeamDataList();

    bool ExportBeamDataRFA300(QString& strPathOut, vector<CBeamDataRFA>& vBeamDataRFA);

    public slots:
        void SLTM_ImportRDFiles();
        void SLTM_RenameRDFilesByDICOMInfo();  


        void SLT_UpdateProbePosFromAxial();
        void SLT_UpdateProbePosFromSagittal();
        void SLT_UpdateProbePosFromFrontal();

        void SLT_MouseWheelUpdateAxial();
        void SLT_MouseWheelUpdateSagittal();
        void SLT_MouseWheelUpdateFrontal();


        void SLT_RestoreZoomPan();      

        
        void SLT_UpdateComboContents();        
        void SLT_WhenSelectCombo();        
        void SLT_WhenChangePlane();//Radio Button, to be implemented

        void SLT_DrawAll();                
        void SLT_DrawGraph(bool bInitMinMax = false);               
        void SLT_DrawGraphRFA300(bool bInitMinMax = false);

        void SLT_CopyTableToClipboard();

        //void SLT_DrawGraph(); 

        void SLT_GoCenterPos();        
        
        void SLT_MousePressedRightAxial();        
        void SLT_MousePressedRightSagittal();
        void SLT_MousePressedRightFrontal();


        void SLT_MouseReleasedRightAxial();
        void SLT_MouseReleasedRightSagittal();
        void SLT_MouseReleasedRightFrontal();

        void SLT_UpdatePanSettingAxial();                       
        void SLT_UpdatePanSettingSagittal();
        void SLT_UpdatePanSettingFrontal();

        void SLT_TableEdit_Invert();
        void SLT_TableEdit_SetOrigin();
        void SLT_TableEdit_TrimXMin();
        void SLT_TableEdit_TrimXMax();
        void SLT_TableEdit_Restore();        

        void SLT_AddBeamDataToRFA300List();
        void SLT_ClearBeamDataRFA300();
        void SLT_ExportBeamDataRFA300();
        void SLT_ResampleAll();

public:    
    QString m_strPathInputDir;//this is for input DCM. initialized when Load Ref files or Load Comp files

    QStringList m_strlistPath; //RD files, before the conversio
    QStringList m_strlistFileBaseName;
    
    vector<FloatImageType::Pointer> m_vDoseImages;    
    vector<float> m_vRefDose; // for normalization
    
    FloatImage2DType::Pointer m_spCur2DAxial;    //extracted 2D from 3D
    FloatImage2DType::Pointer m_spCur2DSagittal;    //extracted 2D from 3D
    FloatImage2DType::Pointer m_spCur2DFrontal;    //extracted 2D from 3D

    YK16GrayImage* m_pCurImageAxial; //for display YK image
    YK16GrayImage* m_pCurImageSagittal; //for display YK image
    YK16GrayImage* m_pCurImageFrontal; //for display YK image


    QStandardItemModel *m_pTableModel; //for dose profile, col: 2
    vector<VEC3D> m_vColormapDose;
    
    bool m_bMousePressedRightAxial;    
    bool m_bMousePressedRightSagittal;
    bool m_bMousePressedRightFrontal;

    QPoint m_ptPanStart;
    QPoint m_ptOriginalDataOffset;        

    int m_iLastLoadedIndex;

    vector<CBeamDataRFA> m_vBeamDataRFA;

private:
    Ui::beamdata_gen_guiClass ui;
};

#endif // beamdata_gen_gui_H
