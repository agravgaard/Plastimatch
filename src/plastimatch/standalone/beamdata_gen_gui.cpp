#include "beamdata_gen_gui.h"
#include <QString>
#include <QFileDialog>
#include <QListView>
#include <QMessageBox>
#include <fstream>
#include "YK16GrayImage.h"

#include "plm_image.h"
#include "rt_study_metadata.h"
#include "gamma_dose_comparison.h"

#include <QFileInfo>
#include "logfile.h"
#include "pcmd_gamma.h"
#include "print_and_exit.h"
#include "plm_file_format.h"
#include "rt_study.h"
#include "qt_util.h"
#include <QStandardItemModel>
#include <QClipboard>

#include "dcmtk_rt_study.h"
#include "dcmtk_rt_study_p.h"
#include "dcmtk_series.h"

#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dctagkey.h"
#include "dcmtk/dcmdata/dcsequen.h"
#include "dcmtk/dcmdata/dcitem.h"
#include "dcmtk/dcmdata/dcdeftag.h"

#include <QDataStream>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkFlipImageFilter.h"

#include "itkMinimumMaximumImageCalculator.h"

#include <algorithm>

#include "dcmtk_rt_study.h"
#include "rtplan.h"
#include "rtplan_beam.h"
#include "rtplan_control_pt.h"

beamdata_gen_gui::beamdata_gen_gui(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
    ui.setupUi(this);

    m_pCurImageAxial = new YK16GrayImage();
    m_pCurImageSagittal = new YK16GrayImage();
    m_pCurImageFrontal = new YK16GrayImage();

    //QUTIL::LoadColorTableFromFile("colormap_jet.txt", m_vColormapDose);
    QUTIL::LoadColorTableInternal(m_vColormapDose, COL_TABLE_JET);

    if (m_vColormapDose.size() < 1)
    {
        cout << "Fatal error!: colormap is not ready. colormap_table should be checked in QtUtil." << endl;
    }
    else
    {
        cout << "Colormap_Jet is successfully loaded" << endl;
    }

    m_pCurImageAxial->SetColorTable(m_vColormapDose);
    m_pCurImageSagittal->SetColorTable(m_vColormapDose);
    m_pCurImageFrontal->SetColorTable(m_vColormapDose);

    connect(ui.labelDoseImgAxial, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosFromAxial())); //added
    connect(ui.labelDoseImgSagittal, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosFromSagittal())); //added    
    connect(ui.labelDoseImgFrontal, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosFromFrontal())); //added


    connect(ui.labelDoseImgAxial, SIGNAL(Mouse_Left_DoubleClick()), this, SLOT(SLT_GoCenterPos())); //added   
    connect(ui.labelDoseImgSagittal, SIGNAL(Mouse_Left_DoubleClick()), this, SLOT(SLT_GoCenterPos())); //added   
    connect(ui.labelDoseImgFrontal, SIGNAL(Mouse_Left_DoubleClick()), this, SLOT(SLT_GoCenterPos())); //added   


    connect(ui.labelDoseImgAxial, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateAxial())); //added
    connect(ui.labelDoseImgSagittal, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateSagittal())); //added
    connect(ui.labelDoseImgFrontal, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateFrontal())); //added


    connect(ui.labelDoseImgAxial, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingAxial())); //added
    connect(ui.labelDoseImgSagittal, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingSagittal())); //added
    connect(ui.labelDoseImgFrontal, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingFrontal())); //added
    
    connect(ui.labelDoseImgAxial, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightAxial())); //added
    connect(ui.labelDoseImgSagittal, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightSagittal())); //added
    connect(ui.labelDoseImgFrontal, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightFrontal())); //added
    

    connect(ui.labelDoseImgAxial, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightAxial())); //added
    connect(ui.labelDoseImgSagittal, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightSagittal())); //added
    connect(ui.labelDoseImgFrontal, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightFrontal())); //added    
  

    m_pTableModel = NULL;    
    m_bMousePressedRightAxial = false;
    m_bMousePressedRightSagittal = false;
    m_bMousePressedRightFrontal = false;    

    //Test code for dicom rt plan loading

    Dcmtk_rt_study* pRTstudyRP = new Dcmtk_rt_study();
    QString strTest = "D:/dcmrt_plan.dcm";
    Plm_file_format file_type_dcm_plan = plm_file_format_deduce(strTest.toLocal8Bit().constData());

    if (file_type_dcm_plan == PLM_FILE_FMT_DICOM_RTPLAN)
    {
        pRTstudyRP->load(strTest.toLocal8Bit().constData());
    }


    Rtplan::Pointer rtplan = pRTstudyRP->get_rtplan();
    if (!rtplan)
    {
        cout << "Error! no dcm plan is loaded" << endl;        
        return;
    }

    int iCntBeam = rtplan->beamlist.size();
    if (iCntBeam < 1)
    {
        cout << "Error! no beam is found" << endl;
        return;
    }

    float* final_iso_pos = NULL;
    for (int i = 0; i < iCntBeam; i++)
    {
        Rtplan_beam *curBeam = rtplan->beamlist[i];

        int iCntCP = curBeam->cplist.size();
        for (int j = 0; j < iCntCP; j++)
        {
            float* cur_iso_pos = curBeam->cplist[j]->get_isocenter();
            cout << "Beam ID: " << j
                << ", Control point ID: " << j
                << ", Isocenter pos : " << cur_iso_pos[0] << "/" << cur_iso_pos[1] << "/" << cur_iso_pos[2] << endl;

            if (i == 0 && j == 0) //choose first beam's isocenter
                final_iso_pos = curBeam->cplist[j]->get_isocenter();
        }
    }
    if (final_iso_pos == NULL)
    {
        cout << "Error!  No isocenter position was found. " << endl;
        return;
    }

    cout << final_iso_pos[0] << " " << final_iso_pos[1] << " " << final_iso_pos[1] << endl;
    delete pRTstudyRP;    
}

beamdata_gen_gui::~beamdata_gen_gui()
{
    //delete m_pImgOffset;
    //delete m_pImgGain;

    //m_vPixelReplMap.clear(); //not necessary
    //delete m_pView;
    m_vDoseImages.clear();         


    delete m_pCurImageAxial;
    delete m_pCurImageSagittal;
    delete m_pCurImageFrontal;

    if (m_pTableModel != NULL)
    {
        delete m_pTableModel;
        m_pTableModel = NULL;
    }

    if (m_vBeamDataRFA.size() > 0)
    {
        cout << m_vBeamDataRFA.size() << " beam data were successfully deleted." << endl;
        m_vBeamDataRFA.clear(); //cascade deleting, otherwise pointer vector is needed.  
    }        
}

void beamdata_gen_gui::SLTM_ImportRDFiles()
{
    QStringList tmpList = QFileDialog::getOpenFileNames(this, "Select one or more files to open", m_strPathInputDir, "3D dose file (*.dcm *.mha)");

    int iFileCnt = tmpList.size();

    if (iFileCnt < 1)
        return;

    m_strlistPath.clear();
    m_strlistFileBaseName.clear();
    m_vDoseImages.clear();
    m_vRefDose.clear();

    m_strlistPath = tmpList;

    for (int i = 0; i < iFileCnt; i++)
    {        
        QFileInfo tmpInfo = QFileInfo(m_strlistPath.at(i));
        m_strlistFileBaseName.push_back(tmpInfo.completeBaseName());
    }

    QFileInfo finfo(m_strlistPath.at(0));
    QDir crntDir = finfo.absoluteDir();
    m_strPathInputDir = crntDir.absolutePath();
    
    //path to m_spImage    
    int iCnt = m_strlistPath.count();
    QString crntPath;

    typedef itk::ImageFileReader<FloatImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();

    for (int i = 0; i < iCnt; i++)
    {
        crntPath = m_strlistPath.at(i);
        QFileInfo fInfo(crntPath);

        Plm_file_format cur_file_type;
        Rt_study cur_rt_study;

        cur_file_type = plm_file_format_deduce(crntPath.toLocal8Bit().constData());

        FloatImageType::Pointer spCurFloat;
        
        if (cur_file_type == PLM_FILE_FMT_DICOM_DOSE)
        {
            cur_rt_study.load(crntPath.toLocal8Bit().constData(), cur_file_type);

            if (cur_rt_study.has_dose())
            {
                //Plm_image::Pointer plm_img = cur_rt_study.get_dose();
                Plm_image::Pointer plm_img = cur_rt_study.get_dose()->clone();
                spCurFloat = plm_img->itk_float();
            }
            else
            {
                cout << "Error! File= " << crntPath.toLocal8Bit().constData() << " DICOM DOSE but no dose info contained" << endl;
            }
        }
        else if (fInfo.suffix() == "mha" || fInfo.suffix() == "MHA")
        {
            reader->SetFileName(crntPath.toLocal8Bit().constData());
            reader->Update();            
            spCurFloat = reader->GetOutput();
        }

        if (spCurFloat)
        {
            m_vDoseImages.push_back(spCurFloat);

            //Calculate dose max
            typedef itk::MinimumMaximumImageCalculator<FloatImageType> MinimumMaximumImageCalculatorType;
            MinimumMaximumImageCalculatorType::Pointer minimumMaximumImageCalculatorFilter = MinimumMaximumImageCalculatorType::New();
            minimumMaximumImageCalculatorFilter->SetImage(spCurFloat);
            minimumMaximumImageCalculatorFilter->Compute();
            float maxVal = minimumMaximumImageCalculatorFilter->GetMaximum();
            m_vRefDose.push_back(maxVal);
        }         
    }
    cout << m_vDoseImages.size() << " files were successfully loaded." << endl;



    disconnect(ui.comboBoxFileName, SIGNAL(currentIndexChanged(int)), this, SLOT(SLT_WhenSelectCombo()));
    SLT_UpdateComboContents();
    connect(ui.comboBoxFileName, SIGNAL(currentIndexChanged(int)), this, SLOT(SLT_WhenSelectCombo()));

    SLT_WhenSelectCombo(); //Draw all included
    SLT_GoCenterPos(); //Draw all included
}

void beamdata_gen_gui::SLT_UpdateComboContents() //compare image based..
{    
    QComboBox* crntCombo = ui.comboBoxFileName;
    crntCombo->clear();    

    int cntComp = m_strlistFileBaseName.count();

    for (int i = 0; i < cntComp; i++)
    {
        //SLT_WHenComboSelect should be disconnected here
        crntCombo->addItem(m_strlistFileBaseName.at(i));
    }    
}

void beamdata_gen_gui::SLT_DrawAll()
{
    //Get combo box selection
    QComboBox* crntCombo = ui.comboBoxFileName;
    //QString curStr = crntCombo->currentText(); //this should be basename
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    int iCnt = crntCombo->count();

    if (iCnt < 1)
        return;

    if ((int)(m_vDoseImages.size()) != iCnt)
    {
        cout << "Error! iCnt not matching in DrawAll" << endl;
        return;
    }        
    FloatImageType::Pointer spCurImg3D = m_vDoseImages.at(curIdx);         
      
    //DICOM
    float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
    float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
    float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();

    /*FloatImage2DType::Pointer spCurGamma2DFrom3D; */
    double finalPos1, finalPos2, finalPos3;

    //enPLANE curPlane = PLANE_AXIAL;
    float fixedPos = 0.0;
    float probePos2D_X =0.0;
    float probePos2D_Y = 0.0;

    bool bYFlip = false;

    vector<QPointF> vProfileAxial, vProfileSagittal, vProfileFrontal;
    enPROFILE_DIRECTON enDirection = PRIFLE_HOR;
    float fixedPosProfileAxial, fixedPosProfileSagittal, fixedPosProfileFrontal;

    if (ui.radioButtonHor->isChecked())
        enDirection = PRIFLE_HOR;    
    else if (ui.radioButtonVert->isChecked())
        enDirection = PRIFLE_VER;

    enPLANE curPlane = PLANE_AXIAL;
    if (curPlane == PLANE_AXIAL)
    {        
        fixedPos = probePosZ;
        probePos2D_X = probePosX;
        probePos2D_Y = probePosY;
        bYFlip = false;

        QUTIL::Get2DFrom3DByPosition(spCurImg3D, m_spCur2DAxial, PLANE_AXIAL, fixedPos, finalPos1);


        //YKImage receives 2D float
        m_pCurImageAxial->UpdateFromItkImageFloat(m_spCur2DAxial, GY2YKIMG_MAG, NON_NEG_SHIFT, bYFlip); //flip Y for display only        

        m_pCurImageAxial->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);

        if (enDirection == PRIFLE_HOR)
            fixedPosProfileAxial = probePosY;
        else
            fixedPosProfileAxial = probePosX;        
       
        ui.lineEdit_ProbePosZ->setText(QString::number(finalPos1, 'f', 1));        
    }

    curPlane = PLANE_SAGITTAL;
    //Actually, frontal and sagittal image should be flipped for display purpose (in axial, Y is small to large, SAG and FRONTAL, Large to Small (head to toe direction)
    //Let's not change original data itself due to massing up the origin. Only change the display image
    //Point probe and profiles, other things works fine.

    if (curPlane == PLANE_SAGITTAL)
    {        
        fixedPos = probePosX;

        probePos2D_X = probePosY;
        probePos2D_Y = probePosZ;  //YKDebug: may be reversed     
        bYFlip = true;

        QUTIL::Get2DFrom3DByPosition(spCurImg3D, m_spCur2DSagittal, PLANE_SAGITTAL, fixedPos, finalPos2);

        //YKImage receives 2D float
        m_pCurImageSagittal->UpdateFromItkImageFloat(m_spCur2DSagittal, GY2YKIMG_MAG, NON_NEG_SHIFT, bYFlip); //flip Y for display only    

        m_pCurImageSagittal->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);

        if (enDirection == PRIFLE_HOR)
            fixedPosProfileSagittal = probePosZ;
        else
            fixedPosProfileSagittal = probePosY;

        ui.lineEdit_ProbePosX->setText(QString::number(finalPos2, 'f', 1));
    }

    curPlane = PLANE_FRONTAL;

    if (curPlane == PLANE_FRONTAL)
    {
        fixedPos = probePosY;        

        probePos2D_X = probePosX;
        probePos2D_Y = probePosZ;//YKDebug: may be reversed   
        bYFlip = true;

        QUTIL::Get2DFrom3DByPosition(spCurImg3D, m_spCur2DFrontal, PLANE_FRONTAL, fixedPos, finalPos3);

        m_pCurImageFrontal->UpdateFromItkImageFloat(m_spCur2DFrontal, GY2YKIMG_MAG, NON_NEG_SHIFT, bYFlip); //flip Y for display only    

        m_pCurImageFrontal->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);

        if (enDirection == PRIFLE_HOR)
            fixedPosProfileFrontal = probePosZ;
        else
            fixedPosProfileFrontal = probePosX;

        ui.lineEdit_ProbePosY->setText(QString::number(finalPos3, 'f', 1));
    }

    float doseGyNorm = 0.01 * (ui.sliderNormDose->value());    //cGy --> Gy    

    m_pCurImageAxial->SetNormValueOriginal(doseGyNorm);    
    m_pCurImageSagittal->SetNormValueOriginal(doseGyNorm);
    m_pCurImageFrontal->SetNormValueOriginal(doseGyNorm);

    m_pCurImageAxial->FillPixMapDose();            
    m_pCurImageSagittal->FillPixMapDose();
    m_pCurImageFrontal->FillPixMapDose();   
    
    m_pCurImageAxial->m_bDrawCrosshair = true;
    m_pCurImageSagittal->m_bDrawCrosshair = true;
    m_pCurImageFrontal->m_bDrawCrosshair = true;    

    m_pCurImageAxial->m_bDrawOverlayText = true;
    m_pCurImageSagittal->m_bDrawOverlayText = true;
    m_pCurImageFrontal->m_bDrawOverlayText = true;

    QString strValAxial = QString::number(m_pCurImageAxial->GetCrosshairOriginalData() * 100, 'f', 1) + " cGy";
    QString strValSagittal = QString::number(m_pCurImageSagittal->GetCrosshairOriginalData() * 100, 'f', 1) + " cGy";
    QString strValFrontal = QString::number(m_pCurImageFrontal->GetCrosshairOriginalData() * 100, 'f', 1) + " cGy";       

    float fPercAxial = m_pCurImageAxial->GetCrosshairPercData();
    float fPercSagittal = m_pCurImageSagittal->GetCrosshairPercData();
    float fPercFrontal = m_pCurImageFrontal->GetCrosshairPercData();
    
    QString strPercAxial = QString::number(fPercAxial, 'f', 1) + "%";
    QString strPercSagittal = QString::number(fPercSagittal, 'f', 1) + "%";
    QString strPercFrontal = QString::number(fPercFrontal, 'f', 1) + "%";    

    m_pCurImageAxial->m_strOverlayText = strValAxial + " [" + strPercAxial + "]";
    m_pCurImageSagittal->m_strOverlayText = strValSagittal + " [" + strPercSagittal + "]";
    m_pCurImageFrontal->m_strOverlayText = strValFrontal + " [" + strPercFrontal + "]";    

    ui.labelDoseImgAxial->SetBaseImage(m_pCurImageAxial);
    ui.labelDoseImgSagittal->SetBaseImage(m_pCurImageSagittal);
    ui.labelDoseImgFrontal->SetBaseImage(m_pCurImageFrontal);    
    
    ui.labelDoseImgAxial->update();
    ui.labelDoseImgSagittal->update();
    ui.labelDoseImgFrontal->update();    

    //Update Table and Chart

    //1) prepare vector float
   
    QUTIL::GetProfile1DByPosition(m_spCur2DAxial, vProfileAxial, fixedPosProfileAxial, enDirection);
    QUTIL::GetProfile1DByPosition(m_spCur2DSagittal, vProfileSagittal, fixedPosProfileSagittal, enDirection);
    QUTIL::GetProfile1DByPosition(m_spCur2DFrontal, vProfileFrontal, fixedPosProfileFrontal, enDirection);        
    
    //fNorm: Gy
    //float doseGyNormRef = 0.01 * (ui.sliderNormRef->value());
    //float doseGyNormComp = 0.01 *(ui.sliderNormComp->value());

    if (ui.radioButtonAxial->isChecked())
        UpdateTable(vProfileAxial, doseGyNorm, 100.0);
    else if (ui.radioButtonSagittal->isChecked())
        UpdateTable(vProfileSagittal, doseGyNorm, 100.0);
    else if (ui.radioButtonFrontal->isChecked())
        UpdateTable(vProfileFrontal, doseGyNorm, 100.0);

    SLT_DrawGraph(ui.checkBoxAutoAdjust->isChecked());

    

    if (ui.checkBox_FixedAuto->isChecked())
    {
        if (ui.radioButtonAxial->isChecked())
            curPlane = PLANE_AXIAL;
        else if (ui.radioButtonSagittal->isChecked())
            curPlane = PLANE_SAGITTAL;
        else if (ui.radioButtonFrontal->isChecked())
            curPlane = PLANE_FRONTAL;

        //Set RFA value according to the plane selected. //can be later changed by user
        if (curPlane == PLANE_AXIAL && enDirection == PRIFLE_HOR)
        {
            ui.radio_RFA300_Profile_Cr->setChecked(true);
        }
        else if (curPlane == PLANE_SAGITTAL && enDirection == PRIFLE_HOR)
        {
            ui.radio_RFA300_PDD->setChecked(true);
        }
        else if (curPlane == PLANE_FRONTAL && enDirection == PRIFLE_HOR)
        {
            ui.radio_RFA300_Profile_Cr->setChecked(true);
        }
        else if (curPlane == PLANE_AXIAL && enDirection == PRIFLE_VER)
        {
            ui.radio_RFA300_PDD->setChecked(true);
        }
        else if (curPlane == PLANE_SAGITTAL && enDirection == PRIFLE_VER)
        {
            ui.radio_RFA300_Profile_In->setChecked(true);
        }
        else if (curPlane == PLANE_FRONTAL &&enDirection == PRIFLE_VER)
        {
            ui.radio_RFA300_Profile_In->setChecked(true);
        }

        //Device Rot is not accounted yet. just follow DICOM convention
        ui.lineEdit_RFA300_FixedCR->setText(ui.lineEdit_ProbePosX->text());
        ui.lineEdit_RFA300_FixedIN->setText(ui.lineEdit_ProbePosZ->text()); //assume Sup: +
        //ui.lineEdit_RFA300_FixedDepth->setText(ui.lineEdit_ProbePosY->text());

        //Get new origin value
        float curDCM_Y = ui.lineEdit_ProbePosY->text().toFloat(); //-129.5
        float curNewOrigin = ui.lineEditTableOrigin->text().toFloat(); //-149.5
        float depth_mm = curDCM_Y - curNewOrigin;

        ui.lineEdit_RFA300_FixedDepth->setText(QString::number(depth_mm, 'f', 1));
    }    
}

void beamdata_gen_gui::SLT_UpdateProbePosFromAxial()//by Mouse Left click
{
    ui.radioButtonAxial->setChecked(true);
    UpdateProbePos(ui.labelDoseImgAxial, PLANE_AXIAL);    
}

void beamdata_gen_gui::SLT_UpdateProbePosFromSagittal()
{
    ui.radioButtonSagittal->setChecked(true);
    UpdateProbePos(ui.labelDoseImgSagittal, PLANE_SAGITTAL);
}

void beamdata_gen_gui::SLT_UpdateProbePosFromFrontal()
{
    ui.radioButtonFrontal->setChecked(true);
    UpdateProbePos(ui.labelDoseImgFrontal, PLANE_FRONTAL);
}

void beamdata_gen_gui::UpdateProbePos(qyklabel* qlabel, enPLANE enPlane)
{
    YK16GrayImage* pYKImg = qlabel->m_pYK16Image;

    if (pYKImg == NULL)
        return;

    QPoint viewPt = QPoint(qlabel->x, qlabel->y);
    QPoint crntDataPt = qlabel->GetDataPtFromViewPt(viewPt.x(), viewPt.y());

    float originX = pYKImg->m_fOriginX;
    float originY = pYKImg->m_fOriginY;

    float spacingX = pYKImg->m_fSpacingX;
    float spacingY = pYKImg->m_fSpacingY;

    //float iWidth = pYKImg->m_iWidth;
    float iHeight = pYKImg->m_iHeight;

    float physPosX = 0.0;
    float physPosY = 0.0;    

    if (enPlane == PLANE_AXIAL)
    {
        physPosX = crntDataPt.x()*spacingX + originX;
        physPosY = crntDataPt.y()*spacingY + originY;

        ui.lineEdit_ProbePosX->setText(QString::number(physPosX, 'f', 1));
        ui.lineEdit_ProbePosY->setText(QString::number(physPosY, 'f', 1));     
    }
    else if (enPlane == PLANE_SAGITTAL)
    {
        physPosX = crntDataPt.x()*spacingX + originX;
        physPosY = (iHeight - crntDataPt.y() - 1)*spacingY + originY;        

        ui.lineEdit_ProbePosY->setText(QString::number(physPosX, 'f', 1));
        ui.lineEdit_ProbePosZ->setText(QString::number(physPosY, 'f', 1));
    }
    else if (enPlane == PLANE_FRONTAL)
    {
        physPosX = crntDataPt.x()*spacingX + originX;
        physPosY = (iHeight - crntDataPt.y() - 1)*spacingY + originY;

        ui.lineEdit_ProbePosX->setText(QString::number(physPosX, 'f', 1));
        ui.lineEdit_ProbePosZ->setText(QString::number(physPosY, 'f', 1));
    }

    SLT_DrawAll();
}

void beamdata_gen_gui::UpdateTable(vector<QPointF>& vData, float fNorm, float fMag)
{
    
    if (m_pTableModel != NULL)
    {
        delete m_pTableModel;
        m_pTableModel = NULL;
    }    

    int columnSize = 3;
    int rowSize = vData.size();    

    if (fNorm <= 0)
        return;

    m_pTableModel = new QStandardItemModel(rowSize, columnSize, this); //2 Rows and 3 Columns
    m_pTableModel->setHorizontalHeaderItem(0, new QStandardItem(QString("mm")));
    m_pTableModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Dose[cGy]")));        
    m_pTableModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Dose[%]")));      

    

    for (int i = 0; i < rowSize; i++)
    {
        qreal tmpValX1 = vData.at(i).x();
        qreal tmpValY1 = vData.at(i).y()*fMag; //Gy --> cGy
        qreal tmpValY1_Perc = vData.at(i).y() / fNorm * 100.0;

        QString strValX, strValY,strValY_Perc;

        strValX = QString::number(tmpValX1, 'f', 1); //cGy
        strValY = QString::number(tmpValY1, 'f', 1); //cGy
        strValY_Perc = QString::number(tmpValY1_Perc, 'f', 1); //%

        m_pTableModel->setItem(i, 0, new QStandardItem(strValX));
        m_pTableModel->setItem(i, 1, new QStandardItem(strValY));        
        m_pTableModel->setItem(i, 2, new QStandardItem(strValY_Perc));
        
    }
    ui.tableViewProfile->setModel(m_pTableModel);
    ui.tableViewProfile->resizeColumnsToContents();

    ui.tableViewProfile->update();
    
}

void beamdata_gen_gui::SLT_CopyTableToClipboard()
{
    qApp->clipboard()->clear();

    QStringList list;

    int rowCnt = m_pTableModel->rowCount();
    int columnCnt = m_pTableModel->columnCount();

    //list << "\n";
    //for (int i = 0 ; i < columnCnt ; i++)		
    //{
    //QFileInfo tmpInfo = QFileInfo(ui.lineEdit_Cur3DFileName->text());
    //list << "Index";	
    //list << tmpInfo.baseName();
    list << "\n";

    /*  m_pTableModel->setHorizontalHeaderItem(0, new QStandardItem(QString("mm")));
      m_pTableModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Ref_cGy")));
      m_pTableModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Com_cGy")));
      m_pTableModel->setHorizontalHeaderItem(3, new QStandardItem(QString("Ref_%")));
      m_pTableModel->setHorizontalHeaderItem(4, new QStandardItem(QString("Com_%")));
      m_pTableModel->setHorizontalHeaderItem(5, new QStandardItem(QString("Gmma100")));
      */
    list << "mm";
    list << "Dose[cGy]";    
    list << "Ref[%]";    
    list << "\n";

    for (int j = 0; j < rowCnt; j++)
    {
        for (int i = 0; i < columnCnt; i++)
        {
            QStandardItem* item = m_pTableModel->item(j, i);
            list << item->text();
        }
        list << "\n";
    }

    qApp->clipboard()->setText(list.join("\t"));
}


void beamdata_gen_gui::SLT_DrawGraph(bool bInitMinMax)
{
    if (m_pTableModel == NULL)
        return;


    bool bNorm = ui.checkBoxNormalized->isChecked();// show percentage chart

    //Draw only horizontal, center

    QVector<double> vAxisX1; //can be rows or columns
    QVector<double> vAxisY1;

    //QVector<double> vAxisX2; //can be rows or columns
    //QVector<double> vAxisY2;

    //QVector<double> vAxisX3; //can be rows or columns
    //QVector<double> vAxisY3;

    //QStandardItemModel 	m_pTableModel.item()
    int dataLen = m_pTableModel->rowCount();
   // int columnLen = m_pTableModel->columnCount();

    if (dataLen < 1)
        return;

    ui.customPlotProfile->clearGraphs();

    double minX = 9999.0;
    double maxX = -1.0;

    double minY = 9999.0;
    double maxY = -1.0;

    for (int i = 0; i< dataLen; i++)
    {
        QStandardItem* tableItem_0 = m_pTableModel->item(i, 0);
        QStandardItem* tableItem_1 = m_pTableModel->item(i, 1);
        QStandardItem* tableItem_2 = m_pTableModel->item(i, 2);        

        double tableVal_0 = tableItem_0->text().toDouble();
        double tableVal_1 = tableItem_1->text().toDouble();
        double tableVal_2 = tableItem_2->text().toDouble();

        if (minX > tableVal_0)
            minX = tableVal_0;

        if (maxX < tableVal_0)
            maxX = tableVal_0;

        if (minY > tableVal_1)
            minY = tableVal_1;
        if (maxY < tableVal_1)
            maxY = tableVal_1;        

        if (bNorm) //%
        {            
            vAxisX1.push_back(tableVal_0);
            vAxisY1.push_back(tableVal_2);            
        }
        else
        {
            vAxisX1.push_back(tableVal_0);
            vAxisY1.push_back(tableVal_1);
        }        
    }

    ui.customPlotProfile->addGraph();
    ui.customPlotProfile->graph(0)->setData(vAxisX1, vAxisY1);
    ui.customPlotProfile->graph(0)->setPen(QPen(Qt::red));
    ui.customPlotProfile->graph(0)->setName("Dose");

    if (bInitMinMax)
    {        
        ui.lineEditXMin->setText(QString("%1").arg(minX));
        ui.lineEditXMax->setText(QString("%1").arg(maxX));        
    }    

    double tmpXMin = ui.lineEditXMin->text().toDouble();
    double tmpXMax = ui.lineEditXMax->text().toDouble();
    double tmpYMin = ui.lineEditYMin->text().toDouble();
    double tmpYMax = ui.lineEditYMax->text().toDouble();

    ui.customPlotProfile->xAxis->setRange(tmpXMin, tmpXMax);
    ui.customPlotProfile->yAxis->setRange(tmpYMin, tmpYMax);

    ui.customPlotProfile->xAxis->setLabel("mm");

    if (bNorm)
        ui.customPlotProfile->yAxis->setLabel("%");
    else
        ui.customPlotProfile->yAxis->setLabel("cGy");
    
    ui.customPlotProfile->setTitle("Dose profile");

    QFont titleFont = font();
    titleFont.setPointSize(10);

    ui.customPlotProfile->setTitleFont(titleFont);

    ui.customPlotProfile->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(8); // and make a bit smaller for legend
    ui.customPlotProfile->legend->setFont(legendFont);
    ui.customPlotProfile->legend->setPositionStyle(QCPLegend::psTopRight);
    ui.customPlotProfile->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
    ui.customPlotProfile->replot();
}


void beamdata_gen_gui::SLT_GoCenterPos() //double click
{
    QComboBox* crntCombo = ui.comboBoxFileName;    
    int curIdx = crntCombo->currentIndex(); //this should be basename       

    if (curIdx < 0)
        return;

    if (m_vDoseImages.empty())
        return;

    FloatImageType::Pointer spCurFloat3D = m_vDoseImages.at(curIdx);

    FloatImageType::PointType ptOrigin = spCurFloat3D->GetOrigin();
    FloatImageType::SizeType imgSize = spCurFloat3D->GetBufferedRegion().GetSize(); //dimension
    FloatImageType::SpacingType imgSpacing = spCurFloat3D->GetSpacing(); //dimension    

    VEC3D middlePtPos;
    middlePtPos.x = ptOrigin[0] + imgSize[0] * imgSpacing[0] / 2.0;
    middlePtPos.y = ptOrigin[1] + imgSize[1] * imgSpacing[1] / 2.0;
    middlePtPos.z = ptOrigin[2] + imgSize[2] * imgSpacing[2] / 2.0;

    ui.lineEdit_ProbePosX->setText(QString::number(middlePtPos.x, 'f', 1));
    ui.lineEdit_ProbePosY->setText(QString::number(middlePtPos.y, 'f', 1));
    ui.lineEdit_ProbePosZ->setText(QString::number(middlePtPos.z, 'f', 1));

    SLT_RestoreZoomPan();
    SLT_DrawAll(); //triggered when Go Position button
}

void beamdata_gen_gui::SaveDoseIBAGenericTXTFromItk(QString strFilePath, FloatImage2DType::Pointer& spFloatDose)
{
    if (!spFloatDose)
        return;

    //Set center point first (from MainDLg --> TIF) //this will update m_rXPos.a, b
    //SetRationalCenterPosFromDataCenter(dataCenterPoint);
    //POINT ptCenter = GetDataCenter(); //will get dataPt from m_rXPos    

    FloatImage2DType::PointType ptOrigin = spFloatDose->GetOrigin();
    FloatImage2DType::SizeType imgSize = spFloatDose->GetBufferedRegion().GetSize(); //dimension
    FloatImage2DType::SpacingType imgSpacing = spFloatDose->GetSpacing(); //dimension    

    int imgWidth = imgSize[0];
    int imgHeight = imgSize[1];

    float spacingX = imgSpacing[0];
    float spacingY = imgSpacing[1];

    float originX = ptOrigin[0];
    float originY = ptOrigin[1];

    ofstream fout;
    fout.open(strFilePath.toLocal8Bit().constData());

    fout << "<opimrtascii>" << endl;
    fout << endl;

    fout << "<asciiheader>" << endl;
    fout << "Separator:	[TAB]" << endl;
    fout << "Workspace Name:	n/a" << endl;
    fout << "File Name:	n/a" << endl;
    fout << "Image Name:	n/a" << endl;
    fout << "Radiation Type:	Photons" << endl;
    fout << "Energy:	0.0 MV" << endl;
    fout << "SSD:	10.0 cm" << endl;
    fout << "SID:	100.0 cm" << endl;
    fout << "Field Size Cr:	10.0 cm" << endl;
    fout << "Field Size In:	10.0 cm" << endl;
    fout << "Data Type:	Abs. Dose" << endl;
    fout << "Data Factor:	1.000" << endl;
    fout << "Data Unit:	mGy" << endl;
    fout << "Length Unit:	cm" << endl;
    fout << "Plane:	" << "XY" << endl;
    fout << "No. of Columns:	" << imgWidth << endl;
    fout << "No. of Rows:	" << imgHeight << endl;
    fout << "Number of Bodies:	" << 1 << endl;
    fout << "Operators Note:	made by ykp" << endl;
    fout << "</asciiheader>" << endl;
    fout << endl;

    fout << "<asciibody>" << endl;
    fout << "Plane Position:     0.00 cm" << endl;
    fout << endl;

    fout << "X[cm]" << "\t";

    QString strTempX;
    double fPosX = 0.0;

    for (int i = 0; i < imgWidth; i++)
    {
        fPosX = (originX + i*spacingX) * 0.1;//mm --> cm        
        fout << QString::number(fPosX, 'f', 3).toLocal8Bit().constData() << "\t";
    }
    fout << endl;

    fout << "Y[cm]" << endl;

    QString strTempY;
    double fPosY = 0.0; //cm

    int imgVal = 0; // mGy, integer


    itk::ImageRegionConstIterator<FloatImage2DType> it(spFloatDose, spFloatDose->GetRequestedRegion());
    it.GoToBegin();


    float* pImg;
    pImg = new float [imgWidth*imgHeight];
        
    int idx = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
        pImg[idx] = it.Get();
        idx++;
    }

    for (int i = imgHeight - 1; i >= 0; i--)
    {
        //spacing: mm/px --> to make a cm, * 0.1 
        fPosY = (originY + i*spacingY) * 0.1; //--> Image writing From bottom to Top   // Y value < 0  means Inferior in XY plane --> more make sense
        QString strYVal = QString::number(fPosY, 'f', 3);
        fout << strYVal.toLocal8Bit().constData() << "\t";

        for (int j = 0; j < imgWidth; j++)
        {
            imgVal = qRound(pImg[imgWidth*i + j] * 1000); // Gy --> mGy
            fout << imgVal << "\t";
        }
        fout << endl;
    }

    delete [] pImg;

    fout << "</asciibody>" << endl;
    fout << "</opimrtascii>" << endl;

    //  POINT GetDataCenter();//data center index, 0 based
    // double m_spacingX;	// mm/pixel
    //double m_spacingY;// mm/pixel

    fout.close();

    return;
}

void beamdata_gen_gui::SLT_WhenSelectCombo()
{
    QComboBox* crntCombo = ui.comboBoxFileName;    
    int curIdx = crntCombo->currentIndex(); //this should be basename    
    int iCnt = crntCombo->count();
    if (iCnt < 1)
        return;

    if ((int)m_vRefDose.size() != iCnt ||
        (int)m_strlistPath.size() != iCnt)
    {
        cout << "Error! SLT_WhenSelectCombo file count doesn't match!" << endl;
        cout << "crntComboCnt " << iCnt << endl;
        cout << "m_vRefDose " << m_vRefDose.size() << endl;
        cout << "m_strlistPath " << m_strlistPath.size() << endl;
        return;
    }
        
    if ((int)(m_vDoseImages.size()) != iCnt)
    {
        cout << "Error! ItkImage Pointer count doesn't match!" << endl;
        return;
    }    

    disconnect(ui.sliderNormDose, SIGNAL(valueChanged(int)), this, SLOT(SLT_DrawAll()));
    ui.sliderNormDose->setValue(qRound(m_vRefDose.at(curIdx) * 100)); //Gy to cGy
    connect(ui.sliderNormDose, SIGNAL(valueChanged(int)), this, SLOT(SLT_DrawAll()));

    //QString strPath_Cur = m_strlistPath.at(curIdx); //always readable mha format
    //QFileInfo info_ref = QFileInfo(strPath_Cur);   

    //SLT_WhenChangePlane(); //RestorePanZoom + DrawAll        

    SLT_RestoreZoomPan();
    SLT_DrawAll();
}

void beamdata_gen_gui::SLT_MouseWheelUpdateAxial()
{    
    MouseWheelUpdateCommon(PLANE_AXIAL);
}


void beamdata_gen_gui::SLT_MouseWheelUpdateSagittal()
{
    MouseWheelUpdateCommon(PLANE_SAGITTAL);
}

void beamdata_gen_gui::SLT_MouseWheelUpdateFrontal()
{
    MouseWheelUpdateCommon(PLANE_FRONTAL);
}

void beamdata_gen_gui::MouseWheelUpdateCommon(enPLANE curPlane)
{
    if (ui.labelDoseImgAxial->m_pYK16Image == NULL ||
        ui.labelDoseImgSagittal->m_pYK16Image == NULL ||
        ui.labelDoseImgFrontal->m_pYK16Image == NULL)
    {
        return;
    }
    QComboBox* crntCombo = ui.comboBoxFileName;
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    if (curIdx < 0)
        return;

    if (curIdx >= (int)(m_vDoseImages.size()))
        return;

    if (ui.checkBox_ScrollZoom->isChecked())
    {
        double oldZoom =1.0; 
        double fWeighting = 0.2;
        float vZoomVal = 1.0;

        if (curPlane == PLANE_AXIAL)
        {
            oldZoom = ui.labelDoseImgAxial->m_pYK16Image->m_fZoom;            
            vZoomVal = oldZoom + ui.labelDoseImgAxial->m_iMouseWheelDelta * fWeighting;
        }
        else if (curPlane == PLANE_SAGITTAL)
        {
            oldZoom = ui.labelDoseImgSagittal->m_pYK16Image->m_fZoom;
            vZoomVal = oldZoom + ui.labelDoseImgSagittal->m_iMouseWheelDelta * fWeighting;
        }
        else if (curPlane == PLANE_FRONTAL)
        {
            oldZoom = ui.labelDoseImgFrontal->m_pYK16Image->m_fZoom;
            vZoomVal = oldZoom + ui.labelDoseImgFrontal->m_iMouseWheelDelta * fWeighting;
        }

        ui.labelDoseImgAxial->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelDoseImgSagittal->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelDoseImgFrontal->m_pYK16Image->SetZoom(vZoomVal);
    }
    else
    {
        FloatImageType::Pointer spCurImg = m_vDoseImages.at(curIdx);

        VEC3D ptLimitStart = { 0.0, 0.0, 0.0 };
        VEC3D ptLimitEnd = { 0.0, 0.0, 0.0 };

        QUTIL::GetGeometricLimitFloatImg(spCurImg, ptLimitStart, ptLimitEnd);

        double fWeighting = 1.0;
        float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
        float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
        float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();

        if (curPlane == PLANE_AXIAL)
        {
            probePosZ = probePosZ + ui.labelDoseImgAxial->m_iMouseWheelDelta*fWeighting;

            if (probePosZ <= ptLimitStart.z)
                probePosZ = ptLimitStart.z;

            if (probePosZ >= ptLimitEnd.z)
                probePosZ = ptLimitEnd.z;

            ui.lineEdit_ProbePosZ->setText(QString::number(probePosZ, 'f', 1));
        }
        else if (curPlane == PLANE_SAGITTAL)
        {
            probePosX = probePosX + ui.labelDoseImgSagittal->m_iMouseWheelDelta*fWeighting;
            if (probePosX <= ptLimitStart.x)
                probePosX = ptLimitStart.x;

            if (probePosX >= ptLimitEnd.x)
                probePosX = ptLimitEnd.x;

            ui.lineEdit_ProbePosX->setText(QString::number(probePosX, 'f', 1));
        }
        else if (curPlane == PLANE_FRONTAL)
        {
            probePosY = probePosY + ui.labelDoseImgFrontal->m_iMouseWheelDelta*fWeighting;
            if (probePosY <= ptLimitStart.y)
                probePosY = ptLimitStart.y;
            if (probePosY >= ptLimitEnd.y)
                probePosY = ptLimitEnd.y;

            ui.lineEdit_ProbePosY->setText(QString::number(probePosY, 'f', 1));
        }
    }
    SLT_DrawAll();
}


void beamdata_gen_gui::SLT_RestoreZoomPan()
{
    if (ui.labelDoseImgAxial->m_pYK16Image == NULL ||
        ui.labelDoseImgSagittal->m_pYK16Image == NULL ||
        ui.labelDoseImgFrontal->m_pYK16Image == NULL)
    {
        return;
    }

    ui.labelDoseImgAxial->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelDoseImgAxial->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    ui.labelDoseImgSagittal->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelDoseImgSagittal->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    ui.labelDoseImgFrontal->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelDoseImgFrontal->setFixedHeight(DEFAULT_LABEL_HEIGHT);    

    ui.labelDoseImgAxial->m_pYK16Image->SetZoom(1.0);
    ui.labelDoseImgSagittal->m_pYK16Image->SetZoom(1.0);
    ui.labelDoseImgFrontal->m_pYK16Image->SetZoom(1.0);

    ui.labelDoseImgAxial->m_pYK16Image->SetOffset(0,0);
    ui.labelDoseImgSagittal->m_pYK16Image->SetOffset(0, 0);
    ui.labelDoseImgFrontal->m_pYK16Image->SetOffset(0, 0);
}

void beamdata_gen_gui::SLT_MousePressedRightAxial()
{    
    m_bMousePressedRightAxial = true;
    WhenMousePressedRight(ui.labelDoseImgAxial);
}

void beamdata_gen_gui::SLT_MousePressedRightSagittal()
{
    m_bMousePressedRightSagittal = true;
    WhenMousePressedRight(ui.labelDoseImgSagittal);
}

void beamdata_gen_gui::SLT_MousePressedRightFrontal()
{
    m_bMousePressedRightFrontal = true;
    WhenMousePressedRight(ui.labelDoseImgFrontal);
}

void beamdata_gen_gui::WhenMousePressedRight(qyklabel* pWnd)
{
    if (pWnd->m_pYK16Image == NULL)
        return;

    m_ptPanStart.setX(pWnd->x);
    m_ptPanStart.setY(pWnd->y);

    m_ptOriginalDataOffset.setX(pWnd->m_pYK16Image->m_iOffsetX);
    m_ptOriginalDataOffset.setY(pWnd->m_pYK16Image->m_iOffsetY);
}

void beamdata_gen_gui::SLT_MouseReleasedRightAxial()
{
    m_bMousePressedRightAxial = false;
}

void beamdata_gen_gui::SLT_MouseReleasedRightSagittal()
{
    m_bMousePressedRightSagittal = false;
}

void beamdata_gen_gui::SLT_MouseReleasedRightFrontal()
{
    m_bMousePressedRightFrontal = false;
}

void beamdata_gen_gui::UpdatePanCommon(qyklabel* qWnd)
{
    if (qWnd->m_pYK16Image == NULL)
        return;

    double dspWidth = qWnd->width();
    double dspHeight = qWnd->height();

    int dataWidth = qWnd->m_pYK16Image->m_iWidth;
    int dataHeight = qWnd->m_pYK16Image->m_iHeight;
    if (dataWidth*dataHeight == 0)
        return;

   // int dataX = qWnd->GetDataPtFromMousePos().x();
   // int dataY = qWnd->GetDataPtFromMousePos().y();

    ////Update offset information of dispImage
    //GetOriginalDataPos (PanStart)
    //offset should be 0.. only relative distance matters. offset is in realtime changing
    QPoint ptDataPanStartRel = qWnd->View2DataExt(m_ptPanStart, dspWidth,
        dspHeight, dataWidth, dataHeight, QPoint(0, 0), qWnd->m_pYK16Image->m_fZoom);

    QPoint ptDataPanEndRel = qWnd->View2DataExt(QPoint(qWnd->x, qWnd->y), dspWidth,
        dspHeight, dataWidth, dataHeight, QPoint(0, 0), qWnd->m_pYK16Image->m_fZoom);

    //int dspOffsetX = pOverlapWnd->x - m_ptPanStart.x();
    //int dspOffsetY = m_ptPanStart.y() - pOverlapWnd->y;

    /*QPoint ptDataStart= pOverlapWnd->GetDataPtFromViewPt(m_ptPanStart.x(),  m_ptPanStart.y());
    QPoint ptDataEnd= pOverlapWnd->GetDataPtFromViewPt(pOverlapWnd->x, pOverlapWnd->y);*/

    int curOffsetX = ptDataPanEndRel.x() - ptDataPanStartRel.x();
    int curOffsetY = ptDataPanEndRel.y() - ptDataPanStartRel.y();

    int prevOffsetX = m_ptOriginalDataOffset.x();
    int prevOffsetY = m_ptOriginalDataOffset.y();

    //double fZoom = qWnd->m_pYK16Image->m_fZoom;
    qWnd->m_pYK16Image->SetOffset(prevOffsetX - curOffsetX, prevOffsetY - curOffsetY);

    //SLT_DrawAll();
}

void beamdata_gen_gui::SLT_UpdatePanSettingAxial() //Mouse Move
{
    if (!m_bMousePressedRightAxial)
        return;

    if (ui.labelDoseImgAxial->m_pYK16Image == NULL || 
        ui.labelDoseImgSagittal->m_pYK16Image == NULL ||
        ui.labelDoseImgFrontal->m_pYK16Image == NULL)
        return;

    UpdatePanCommon(ui.labelDoseImgAxial);
    //Sync offset
    int offsetX = ui.labelDoseImgAxial->m_pYK16Image->m_iOffsetX;
    int offsetY = ui.labelDoseImgAxial->m_pYK16Image->m_iOffsetY;

    ui.labelDoseImgAxial->m_pYK16Image->SetOffset(offsetX, offsetY);

    //check the other plane
    /* int offsetX = ui.labelDoseImgAxial->m_pYK16Image->m_iOffsetX;
     int offsetY = ui.labelDoseImgAxial->m_pYK16Image->m_iOffsetY;
     ui.labelDoseImgFrontal->m_pYK16Image->SetOffset(offsetX, offsetY);
     */
    SLT_DrawAll();
}

void beamdata_gen_gui::SLT_UpdatePanSettingSagittal()
{
    if (!m_bMousePressedRightSagittal)
        return;

    if (ui.labelDoseImgAxial->m_pYK16Image == NULL ||
        ui.labelDoseImgSagittal->m_pYK16Image == NULL ||
        ui.labelDoseImgFrontal->m_pYK16Image == NULL)
        return;

    UpdatePanCommon(ui.labelDoseImgSagittal);
    //Sync offset
    int offsetX = ui.labelDoseImgSagittal->m_pYK16Image->m_iOffsetX;
    int offsetY = ui.labelDoseImgSagittal->m_pYK16Image->m_iOffsetY;

    ui.labelDoseImgSagittal->m_pYK16Image->SetOffset(offsetX, offsetY);

    //check the other plane

    SLT_DrawAll();
}

void beamdata_gen_gui::SLT_UpdatePanSettingFrontal()
{
    if (!m_bMousePressedRightFrontal)
        return;

    if (ui.labelDoseImgAxial->m_pYK16Image == NULL ||
        ui.labelDoseImgSagittal->m_pYK16Image == NULL ||
        ui.labelDoseImgFrontal->m_pYK16Image == NULL)
        return;

    UpdatePanCommon(ui.labelDoseImgFrontal);
    //Sync offset
    int offsetX = ui.labelDoseImgFrontal->m_pYK16Image->m_iOffsetX;
    int offsetY = ui.labelDoseImgFrontal->m_pYK16Image->m_iOffsetY;

    ui.labelDoseImgFrontal->m_pYK16Image->SetOffset(offsetX, offsetY);    

    //check the other plane

    SLT_DrawAll();
}

void beamdata_gen_gui::SLTM_RenameRDFilesByDICOMInfo()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open",
        m_strPathInputDir, "DICOM-RD files (*.dcm)");

    int cnt = files.size();
    if (cnt <= 0)
        return;

    QString strMsg = "Original file names will be gone. Backup is strongly recommended. Continue?";

    QMessageBox msgBox;
    msgBox.setText(strMsg);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    int res = msgBox.exec();

    if (res == QMessageBox::Yes)
    {

    }
        RenameFileByDCMInfo(files);
}


void beamdata_gen_gui::RenameFileByDCMInfo(QStringList& filenameList)
{
    int size = filenameList.size();

    QString crntFilePath;
    //Rt_study rt_study;

    
  
    for (int a = 0; a < size; a++)
    {
        crntFilePath = filenameList.at(a);

        //1) contructor
        Dcmtk_rt_study dss(crntFilePath.toLocal8Bit().constData());
        //2) parse directory: this will link dicome seriese to proper one (e.g. ds_dose)
        dss.parse_directory();       

        Dcmtk_series *pDcmSeries = dss.d_ptr->ds_rtdose; 

        if (pDcmSeries == NULL)
            continue;        
       
        //Pt name: 0010, 0010
        QString strPtId = QString(pDcmSeries->get_cstr(DcmTagKey(0x0010, 0x0020)));
        QString strRDType = QString(pDcmSeries->get_cstr(DcmTagKey(0x3004, 0x000A)));
        //QString strFractionGroup = QString(pDcmSeries->get_cstr(DcmTagKey(0x300C, 0x0022)));
        //QString strBeamNumberTmp = QString(pDcmSeries->get_cstr(DcmTagKey(0x300C, 0x0006)));

        //long int iFractionGroup = 0;
        //long int iBeamNumber = 0;

        DcmSequenceOfItems *seqRefPlan = 0;        

        bool rc = pDcmSeries->get_sequence(DcmTagKey(0x300c, 0x0002), seqRefPlan);
        //rc = pDcmSeries->get_sequence(DcmTagKey(0x300C, 0x0020), seqFractionGroup);

        long int iValBeamNumber = 0;
        long int iValFractionGroupNumber = 0;
        
        if (rc)
        {
            //DcmSequenceOfItems *seqFractionGroup = 0;
            int iNumOfRefPlanSeq = (int)(seqRefPlan->card());

            for (int i = 0; i < iNumOfRefPlanSeq; i++)
            {
                OFCondition orc;

               // const char *strVal = 0;

                DcmItem *itemRefPlan = seqRefPlan->getItem(i);

                //orc = item->findAndGetString(DcmTagKey(0x0008, 0x1150), strVal);//it works!
                /*orc = item->findAndGetLongInt(DcmTagKey(0x300C, 0x0022), iVal);*/
                /*if (!orc.good()){
                    continue;
                    }*/
                DcmSequenceOfItems *seqFractionGroup = 0;
                //rc = pDcmSeries->get_sequence(DcmTagKey(0x300c, 0x0020), seqFractionGroup);//ReferencedFractionGroupSequence                
                orc = itemRefPlan->findAndGetSequence(DCM_ReferencedFractionGroupSequence, seqFractionGroup);//ReferencedFractionGroupSequence                

                if (orc.good())
                {
                    int iNumOfFractionGroup = seqFractionGroup->card();

                    DcmItem *itemFractionGroup = 0;

                    for (int j = 0; j < iNumOfFractionGroup; j++)
                    {
                        itemFractionGroup = seqFractionGroup->getItem(j);
                        DcmSequenceOfItems *seqRefBeam = 0;

                        orc = itemFractionGroup->findAndGetLongInt(DCM_ReferencedFractionGroupNumber, iValFractionGroupNumber);

                        //cout << "Group Number changed = " << iValFractionGroupNumber << endl;

                        if (!orc.good())
                            cout << "error! refFraction group number is not found" << endl;

                        orc = itemFractionGroup->findAndGetSequence(DCM_ReferencedBeamSequence, seqRefBeam);//ReferencedFractionGroupSequence                                                              

                        if (!orc.good())
                            continue;

                        int iNumOfRefBeam = seqRefBeam->card();

                        for (int k = 0; k < iNumOfRefBeam; k++)
                        {
                            DcmItem *itemBeam = 0;
                            itemBeam = seqRefBeam->getItem(k);

                            //orc = itemBeam->findAndGetLongInt(DcmTagKey(0x300C, 0x0006), iValBeamNumber);
                            orc = itemBeam->findAndGetLongInt(DCM_ReferencedBeamNumber, iValBeamNumber);

                            //cout << "iValBeamNumber changed = " << iValBeamNumber << endl;
                        }
                    }
                }
            }                //iVal
        }


        //long int iFractionGroup = 0;
        //long int iBeamNumber = 0;

        //cout << "iFractionGroup " << iValFractionGroupNumber << endl;
        //cout << "iBeamNumber " << iValBeamNumber << endl;

        QString strFractionGroupNumber;
        strFractionGroupNumber = strFractionGroupNumber.sprintf("%02d", (int)iValFractionGroupNumber);

        QString strBeamNumber;
        strBeamNumber = strBeamNumber.sprintf("%03d", (int)iValBeamNumber);

        QFileInfo fileInfo = QFileInfo(crntFilePath);
        QDir dir = fileInfo.absoluteDir();

        QString newBaseName = strPtId + "_" + strRDType + "_" + strFractionGroupNumber + "_" + strBeamNumber;
        //QString extStr = fileInfo.completeSuffix();

        QString newFileName = newBaseName.append(".").append("dcm");
        QString newPath = dir.absolutePath() + "/" + newFileName;

        //cout << newPath.toLocal8Bit().constData() << endl;

        //extract former part
        QFile::rename(crntFilePath, newPath);
    }// end of for,
    
    cout << "In total "<< size << " files were successfully renamed" << endl;

}


void beamdata_gen_gui::SLT_WhenChangePlane()
{
    //Change the Graph
    SLT_DrawAll(); //is this enough to update the graph?
}

void beamdata_gen_gui::SLT_TableEdit_Invert()
{
    /*   cout << "RFA " << m_vBeamDataRFA.size() << endl;

       if (m_vBeamDataRFA.size() > 0)
       cout << "num of profile data " << m_vBeamDataRFA.at(0).m_vDoseProfile.size() << endl;

       return;*/

    vector<QPointF> vProfile1D;
    QUTIL::Get1DProfileFromTable(m_pTableModel, 0, 2, vProfile1D); //x: mm , y: %

    //Edit profile
    vector<QPointF> vProfile1D_new;
    vector<QPointF>::iterator it;

    QPointF fPt_old;
    QPointF fPt_new;
    for (it = vProfile1D.begin(); it != vProfile1D.end(); ++it)
    {
        fPt_old = (*it);        
        fPt_new.setX(-fPt_old.x());
        fPt_new.setY(fPt_old.y());

        vProfile1D_new.push_back(fPt_new);
    }

    //sort
    std::sort(vProfile1D_new.begin(), vProfile1D_new.end(), QUTIL::QPointF_Compare);

    ////Update Table
    //vProfile1D_new// CURRENTLY THIS IS DOSE
    //CURRENTLY DOSE VALUE = percent --> Norm=100, Mag=1.0
    UpdateTable(vProfile1D_new, 100.0, 1.0); //from now, dose cGy = %

    SLT_DrawGraph(ui.checkBoxAutoAdjust->isChecked());
}

void beamdata_gen_gui::SLT_TableEdit_SetOrigin()
{
    vector<QPointF> vProfile1D;
    QUTIL::Get1DProfileFromTable(m_pTableModel, 0, 2, vProfile1D); //x: mm , y: %

    //Get new origin value
    float newOriginX = ui.lineEditTableOrigin->text().toFloat();

    //Edit profile
    vector<QPointF> vProfile1D_new;
    vector<QPointF>::iterator it;

    QPointF fPt_old;
    QPointF fPt_new;
    for (it = vProfile1D.begin(); it != vProfile1D.end(); ++it)
    {
        fPt_old = (*it);
        fPt_new.setX(fPt_old.x() - newOriginX);
        fPt_new.setY(fPt_old.y());

        vProfile1D_new.push_back(fPt_new);
    }

    //sort
    std::sort(vProfile1D_new.begin(), vProfile1D_new.end(), QUTIL::QPointF_Compare);

    ////Update Table
    //vProfile1D_new// CURRENTLY THIS IS DOSE
    //CURRENTLY DOSE VALUE = percent --> Norm=100, Mag=1.0
    UpdateTable(vProfile1D_new, 100.0, 1.0); //from now, dose cGy = %

    SLT_DrawGraph(ui.checkBoxAutoAdjust->isChecked());

    //applies only depth mode
    if (ui.radio_RFA300_PDD->isChecked())
    {
        float curDCM_Y = ui.lineEdit_ProbePosY->text().toFloat(); //-129.5
        float curNewOrigin = ui.lineEditTableOrigin->text().toFloat(); //-149.5

        float depth_mm = curDCM_Y - curNewOrigin;
        ui.lineEdit_RFA300_FixedDepth->setText(QString::number(depth_mm, 'f', 1));
    }
}

void beamdata_gen_gui::SLT_TableEdit_TrimXMin()
{
    vector<QPointF> vProfile1D;
    QUTIL::Get1DProfileFromTable(m_pTableModel, 0, 2, vProfile1D); //x: mm , y: %

    //Get new origin value
    float newTrimX_Min = ui.lineEditTableMin->text().toFloat();
    float newTrimX_Max = ui.lineEditTableMax->text().toFloat();

    //Edit profile
    vector<QPointF> vProfile1D_new;
    vector<QPointF>::iterator it;

    QPointF fPt_old;
    QPointF fPt_new;
    for (it = vProfile1D.begin(); it != vProfile1D.end(); ++it)
    {
        fPt_old = (*it);
        if (fPt_old.x() >= newTrimX_Min)
        {
            fPt_new.setX(fPt_old.x());
            fPt_new.setY(fPt_old.y());
            vProfile1D_new.push_back(fPt_new);
        }        
    }

    //sort
    std::sort(vProfile1D_new.begin(), vProfile1D_new.end(), QUTIL::QPointF_Compare);

    ////Update Table
    //vProfile1D_new// CURRENTLY THIS IS DOSE
    //CURRENTLY DOSE VALUE = percent --> Norm=100, Mag=1.0
    UpdateTable(vProfile1D_new, 100.0, 1.0); //from now, dose cGy = %

    SLT_DrawGraph(ui.checkBoxAutoAdjust->isChecked());
}

void beamdata_gen_gui::SLT_TableEdit_TrimXMax()
{
    vector<QPointF> vProfile1D;
    QUTIL::Get1DProfileFromTable(m_pTableModel, 0, 2, vProfile1D); //x: mm , y: %

    //Get new origin value
    float newTrimX_Min = ui.lineEditTableMin->text().toFloat();
    float newTrimX_Max = ui.lineEditTableMax->text().toFloat();

    //Edit profile
    vector<QPointF> vProfile1D_new;
    vector<QPointF>::iterator it;

    QPointF fPt_old;
    QPointF fPt_new;
    for (it = vProfile1D.begin(); it != vProfile1D.end(); ++it)
    {
        fPt_old = (*it);
        if (fPt_old.x() <= newTrimX_Max)
        {
            fPt_new.setX(fPt_old.x());
            fPt_new.setY(fPt_old.y());
            vProfile1D_new.push_back(fPt_new);
        }
    }
    //sort
    std::sort(vProfile1D_new.begin(), vProfile1D_new.end(), QUTIL::QPointF_Compare);

    ////Update Table
    //vProfile1D_new// CURRENTLY THIS IS DOSE
    //CURRENTLY DOSE VALUE = percent --> Norm=100, Mag=1.0
    UpdateTable(vProfile1D_new, 100.0, 1.0); //from now, dose cGy = %

    SLT_DrawGraph(ui.checkBoxAutoAdjust->isChecked());
}

void beamdata_gen_gui::SLT_TableEdit_Restore()
{
    //DrawAll or goto button
    SLT_DrawAll();
}

void beamdata_gen_gui::SLT_AddBeamDataToRFA300List()
{
    if (m_pTableModel == NULL)
        return;

    vector<QPointF> vProfile1D;
    QUTIL::Get1DProfileFromTable(m_pTableModel, 0, 2, vProfile1D); //x: mm , y: %

    CBeamDataRFA beamDataRFA;
    beamDataRFA.m_vDoseProfile = vProfile1D; //deep copy?

    if (ui.radio_RFA300_PDD->isChecked())
    {
        beamDataRFA.m_ScanType = ST_PDD;
    }
    else if (ui.radio_RFA300_Profile_Cr->isChecked())
    {
        beamDataRFA.m_ScanType = ST_PROFILE_CR;
    }
    else if (ui.radio_RFA300_Profile_In->isChecked())
    {
        beamDataRFA.m_ScanType = ST_PROFILE_IN;
    }

    if (ui.radio_RFA300_BT_photon->isChecked())
    {
        beamDataRFA.m_BeamType = BT_PHOTON;
    }
    else if (ui.radio_RFA300_BT_electron->isChecked())
    {
        beamDataRFA.m_BeamType = BT_ELECTRON;
    }
    else if (ui.radio_RFA300_BT_proton->isChecked())
    {
        beamDataRFA.m_BeamType = BT_PROTON;
    }

    beamDataRFA.m_fBeamEnergy = ui.lineEdit_RFA300_Energy->text().toFloat();
    beamDataRFA.m_fFieldSizeX_cm = ui.lineEdit_RFA300_FS_Xcm->text().toFloat();
    beamDataRFA.m_fFieldSizeY_cm = ui.lineEdit_RFA300_FS_Ycm->text().toFloat();
    beamDataRFA.m_fSSD_cm = ui.lineEdit_RFA300_SSDcm->text().toFloat();

    beamDataRFA.m_fFixedPosX_mm = ui.lineEdit_RFA300_FixedCR->text().toFloat(); //no device rot applied yet. just dicom
    beamDataRFA.m_fFixedPosY_mm = ui.lineEdit_RFA300_FixedIN->text().toFloat();
    beamDataRFA.m_fFixedPosDepth_mm = ui.lineEdit_RFA300_FixedDepth->text().toFloat();

    m_vBeamDataRFA.push_back(beamDataRFA); //deep copy?   

    UpdateBeamDataList();
    SLT_DrawGraphRFA300(ui.checkBoxAutoAdjust_RFA300->isChecked());
}

void beamdata_gen_gui::UpdateBeamDataList() //Update List UI: listWidgetRFA300
{
    ui.listWidgetRFA300->clear();

    /*if (m_vBeamDataRFA.empty())
    {        
        return;
    }
*/
   // int iBeamDataCnt = m_vBeamDataRFA.size();

    vector<CBeamDataRFA>::iterator it;

    QString strBeamName;
    for (it = m_vBeamDataRFA.begin(); it != m_vBeamDataRFA.end(); ++it)
    {
        strBeamName = (*it).GetBeamName();
        ui.listWidgetRFA300->addItem(strBeamName);
    } 
}

void beamdata_gen_gui::SLT_ClearBeamDataRFA300()
{
    m_vBeamDataRFA.clear();

    UpdateBeamDataList();
    SLT_DrawGraphRFA300(ui.checkBoxAutoAdjust_RFA300->isChecked());
}

void beamdata_gen_gui::SLT_ExportBeamDataRFA300()
{
    if (m_vBeamDataRFA.size() < 1)
    {
        QUTIL::ShowErrorMessage("Error! no beam data to export");        
        return;
    }       

    QString strFilePath = QFileDialog::getSaveFileName(this, "Export Beam data as RFA300 format",m_strPathInputDir, "RFA300 (*.asc)", 0, 0);
    if (strFilePath.length() < 1)
        return;

    if (!ExportBeamDataRFA300(strFilePath, m_vBeamDataRFA))
    {
        QUTIL::ShowErrorMessage("Error! RFA300 export failed");
    }
    else
    {
        cout << "RFA300 file was successfully exported" << endl;
    }
}

bool beamdata_gen_gui::ExportBeamDataRFA300(QString& strPathOut, vector<CBeamDataRFA>& vBeamDataRFA)
{
    ofstream fout;
    fout.open(strPathOut.toLocal8Bit().constData());

    int iBeamCnt = vBeamDataRFA.size();

    if (iBeamCnt < 1)
        return false;

    //global header
    QString gStrHeader1, gStrHeader2;
    
    gStrHeader1.sprintf(":MSR \t%d\t # No. of measurement in file", iBeamCnt);
    gStrHeader2.sprintf(":SYS BDS 0 # Beam Data Scanner System");

    fout << gStrHeader1.toLocal8Bit().constData() << endl;
    fout << gStrHeader2.toLocal8Bit().constData() << endl;
    
    vector<CBeamDataRFA>::iterator it;

    QDateTime curDateTime = QDateTime::currentDateTime();
    QString strDate = curDateTime.toString("MM-dd-yyyy");
    QString strTime = curDateTime.toString("hh:mm:ss");

    int iBeamIdx = 0;
    VEC3D ptStart, ptEnd;

    ptStart.x = 0.0;
    ptStart.y = 0.0;
    ptStart.z = 0.0;
    
    ptEnd.x = 0.0;
    ptEnd.y = 0.0;
    ptEnd.z = 0.0;

    QString strSTS, strEDS;

    QString strBeamType;    
    QString strEnergy;

    QString strSSD;
    QString strBRD;
    QString strWEG;
    QString strFS;
    QString strSCN;
    QString strProfileDepth; //mm, seems to be no need

    for (it = vBeamDataRFA.begin(); it != vBeamDataRFA.end(); ++it)
    {
        CBeamDataRFA* pCurBeam = &(*it);

        int iCntDataPt = pCurBeam->m_vDoseProfile.size();

        if (iCntDataPt < 1)
        {
           // iBeamIdx++;
            continue; //skip, if there is no data
        }        

        float valX, valY;
        float valFixedX = 0.0; //mm
        float valFixedY = 0.0;
        float valFixedDepth = 0.0; //mm
        int iMeasType = -1;


        int iNumOfDataPt = pCurBeam->m_vDoseProfile.size();

        strSSD.sprintf("%4d", qRound(pCurBeam->m_fSSD_cm*10.0));
        strBRD.sprintf("%4d", qRound(pCurBeam->m_fSAD_cm*10.0)); //BeamReferenceDist 

        strWEG.sprintf("%d", pCurBeam->m_iWedgeType); //0

        //Inplane comes first!
        //strFS.sprintf("%d\t%d", qRound(pCurBeam->m_fFieldSizeX_cm*10.0), qRound(pCurBeam->m_fFieldSizeY_cm*10.0));
        strFS.sprintf("%d\t%d", qRound(pCurBeam->m_fFieldSizeY_cm*10.0), qRound(pCurBeam->m_fFieldSizeX_cm*10.0));

        switch (pCurBeam->m_BeamType)
        {
        case BT_PHOTON:
            strBeamType = "PHO\t";
            break;
        case BT_ELECTRON:
            strBeamType = "ELE\t";
            break;
        case BT_PROTON:
            strBeamType = "UDF\t";
            break;
        default:
            strBeamType = "UDF\t"; // unknown
            break;
        }        
        strEnergy.sprintf("% 7.1f", pCurBeam->m_fBeamEnergy);

        strBeamType = strBeamType + strEnergy;

        switch (pCurBeam->m_ScanType) // DPT, PRO, MTX, DIA, UDF
        {
        case ST_PDD:
            strSCN = "DPT";
            iMeasType = 1;
            break;
        case ST_PROFILE_CR:
            strSCN = "PRO";
            iMeasType = 2;
            break;
        case ST_PROFILE_IN:
            strSCN = "PRO";
            iMeasType = 2;
            break;
        default:
            strSCN = "UDF"; // unknown
            break;
        }
        

        //Header
        fout << "#" << endl;
        fout << "# RFA300 ASCII Measurement Dump ( BDS format )" << endl;
        fout << "#" << endl;
        fout << "# Measurement number " << "\t" << iBeamIdx+1 << endl;
        fout << "#" << endl;

        //Beam data header

        fout << "%VNR " << "1.0" << endl;
        fout << "%MOD \t" << "RAT" << endl;//ratio
        fout << "%TYP \t" << "SCN" << endl;
        fout << "%SCN \t" << strSCN.toLocal8Bit().constData() << endl;
        fout << "%FLD \t" << "ION" << endl;
        fout << "%DAT \t" << strDate.toLocal8Bit().constData() << endl;
        fout << "%TIM \t" << strTime.toLocal8Bit().constData() << endl;
        fout << "%FSZ \t" << strFS.toLocal8Bit().constData() << endl;
        fout << "%BMT \t" << strBeamType.toLocal8Bit().constData() << endl;
        fout << "%SSD \t" << strSSD.toLocal8Bit().constData() << endl;
        fout << "%BUP \t" << "0" << endl;
        fout << "%BRD \t" << strBRD.toLocal8Bit().constData() << endl;
        fout << "%FSH \t" << "-1" << endl;
        fout << "%ASC \t" << "0" << endl;
        fout << "%WEG \t" << strWEG.toLocal8Bit().constData() << endl; //open field
        fout << "%GPO \t" << "0" << endl;
        fout << "%CPO \t" << "0" << endl;
        //This MEA was originally 1 even in Profile. it seems to have caused the crash
        fout << "%MEA \t" << iMeasType << endl; //-1 undefined, 0: abs dose, 1: open depth, 2: open profile, 4:wedge, 5: wedge depth, 6: wedge profile
        fout << "%PRD \t" << "0.0" << endl; //ProfileDepth in 0.1 mm // Omnipro doesn't seem to care
        fout << "%PTS \t" << iNumOfDataPt << endl;
        
        if (pCurBeam->m_ScanType == ST_PDD)
        {       
            valFixedX = pCurBeam->m_fFixedPosX_mm;
            valFixedY = pCurBeam->m_fFixedPosY_mm;    

            ptStart.x = valFixedX;
            ptStart.y = valFixedY;
            ptStart.z = pCurBeam->m_vDoseProfile.at(0).x();

            ptEnd.x = valFixedX;
            ptEnd.y = valFixedY;            
            ptEnd.z = pCurBeam->m_vDoseProfile.at(iCntDataPt-1).x();

        }
        else if (pCurBeam->m_ScanType == ST_PROFILE_CR)
        {
            //This is for Device Rot 0
            valFixedY = pCurBeam->m_fFixedPosY_mm;
            valFixedDepth = pCurBeam->m_fFixedPosDepth_mm;

            ptStart.x = pCurBeam->m_vDoseProfile.at(0).x();
            ptStart.y = valFixedY;
            ptStart.z = valFixedDepth;

            ptEnd.x = pCurBeam->m_vDoseProfile.at(iCntDataPt-1).x();
            ptEnd.y = valFixedY;
            ptEnd.z = valFixedDepth;           
        }
        else if (pCurBeam->m_ScanType == ST_PROFILE_IN)
        {
               valFixedX = pCurBeam->m_fFixedPosX_mm;
               valFixedDepth = pCurBeam->m_fFixedPosDepth_mm;

               ptStart.x = valFixedX;
               ptStart.y = pCurBeam->m_vDoseProfile.at(0).x();
               ptStart.z = valFixedDepth;

               ptEnd.x = valFixedX;
               ptEnd.y = pCurBeam->m_vDoseProfile.at(iCntDataPt-1).x();
               ptEnd.z = valFixedDepth;         
        }        

        //Device Rotation should be considered! 
        //MGH: Device 90
        //CR : Put X value on Y column.
        //IN: Put Y value on X column. Inf: + (opposite direction)
        //Depth: same
      
        //Device0
        //strSTS.sprintf("% 7.1f\t% 7.1f\t% 7.1f ", ptStart.x, ptStart.y, ptStart.z); //allocated space: 7, precision: 1
        //strEDS.sprintf("% 7.1f\t% 7.1f\t% 7.1f ", ptEnd.x, ptEnd.y, ptEnd.z);

        //Device 90
        strSTS.sprintf("% 7.1f\t% 7.1f\t% 7.1f ", -ptStart.y, ptStart.x, ptStart.z); //allocated space: 7, precision: 1
        strEDS.sprintf("% 7.1f\t% 7.1f\t% 7.1f ", -ptEnd.y, ptEnd.x, ptEnd.z);

        fout << "%STS \t" << strSTS.toLocal8Bit().constData() << "# Start Scan values in mm ( X , Y , Z )" << endl;
        fout << "%EDS \t" << strEDS.toLocal8Bit().constData() << "# End Scan values in mm ( X , Y , Z )" << endl;

        fout << "#" << endl;
        fout << "#\t  X      Y      Z     Dose" << endl;
        fout << "#" << endl;

        vector<QPointF>::iterator itData;

        QString strDataPt;     

        if (pCurBeam->m_ScanType == ST_PDD)
        {
            for (itData = pCurBeam->m_vDoseProfile.begin(); itData != pCurBeam->m_vDoseProfile.end(); ++itData)
            {
                valX = (*itData).x();//mm
                valY = (*itData).y();//dose
                /*  valFixedX = pCurBeam->m_fFixedPosX_mm;
                  valFixedY = pCurBeam->m_fFixedPosY_mm;*/
            //Device 0
                //strDataPt.sprintf("= \t% 7.1f\t% 7.1f\t% 7.1f\t% 7.1f", valFixedX, valFixedY, valX, valY);
            //Device 90
                strDataPt.sprintf("= \t% 7.1f\t% 7.1f\t% 7.1f\t% 7.1f", -valFixedY, valFixedX, valX, valY);
                fout << strDataPt.toLocal8Bit().constData() << endl;
            }
        }
        else if (pCurBeam->m_ScanType == ST_PROFILE_CR)
        {
            for (itData = pCurBeam->m_vDoseProfile.begin(); itData != pCurBeam->m_vDoseProfile.end(); ++itData)
            {
                valX = (*itData).x();//mm
                valY = (*itData).y();//dose
                /*valFixedY = pCurBeam->m_fFixedPosY_mm;
                valFixedDepth = pCurBeam->m_fFixedPosDepth_mm;*/

                //strDataPt.sprintf("= \t% 7.1f\t% 7.1f\t% 7.1f\t% 7.1f", valX, valFixedY, valFixedDepth, valY);
                //DEVICE90 for MGH
                strDataPt.sprintf("= \t% 7.1f\t% 7.1f\t% 7.1f\t% 7.1f", -valFixedY, valX, valFixedDepth, valY);

                fout << strDataPt.toLocal8Bit().constData() << endl;
            }
        }
        else if (pCurBeam->m_ScanType == ST_PROFILE_IN)
        {
            for (itData = pCurBeam->m_vDoseProfile.begin(); itData != pCurBeam->m_vDoseProfile.end(); ++itData)
            {
                valX = (*itData).x();//mm, Sup:+, Inf:-
                valY = (*itData).y();//dose
                /*valFixedX = pCurBeam->m_fFixedPosX_mm;
                valFixedDepth = pCurBeam->m_fFixedPosDepth_mm;*/

                //strDataPt.sprintf("= \t% 7.1f\t% 7.1f\t% 7.1f\t% 7.1f", valFixedX, valX, valFixedDepth, valY);
                //DEVICE90 for MGH
                strDataPt.sprintf("= \t% 7.1f\t% 7.1f\t% 7.1f\t% 7.1f", -valX, valFixedX, valFixedDepth, valY);

                fout << strDataPt.toLocal8Bit().constData() << endl;
            }
        }
        fout << ":EOM  # End of Measurement" << endl;

      /*  %VNR 1.0
            %MOD 	RAT
            %TYP 	SCN
            %SCN 	DPT
            %FLD 	ION
            %DAT 	04 - 09 - 2015
            %TIM 	19:09 : 40
            %FSZ 	40	40
            %BMT 	ELE	    6.0
            %SSD 	1000
            %BUP 	0
            % BRD 	1000
            % FSH - 1
            % ASC 	0
            % WEG 	0
            % GPO 	0
            % CPO 	0
            % MEA 	1
            % PRD 	0
            % PTS 	259*/
        iBeamIdx++;
    }

    fout << ":EOF # End of File" << endl;

    fout.close();
    return true;
}

void beamdata_gen_gui::SLT_DrawGraphRFA300(bool bInitMinMax)
{
    //source: m_vBeamDataRFA    
    ui.customPlotProfile_RFA300->clearGraphs();

    /*  if (m_vBeamDataRFA.empty())
          return;*/

    vector<CBeamDataRFA>::iterator it;

    double minX = 9999.0;
    double maxX = -1.0;

    double minY = 9999.0;
    double maxY = -1.0;


    QVector<double> vAxisX; //can be rows or columns
    QVector<double> vAxisY;

    float ValX, ValY;

    int iIndex = 0;
    for (it = m_vBeamDataRFA.begin(); it != m_vBeamDataRFA.end(); ++it)
    {        
        vector<QPointF>::iterator itSub;

        vAxisX.clear();
        vAxisY.clear();

        for (itSub = (*it).m_vDoseProfile.begin(); itSub != (*it).m_vDoseProfile.end(); ++itSub)
        {        

            ValX = (*itSub).x();
            ValY = (*itSub).y();

            if (minX > ValX)
                minX = ValX;
            if (maxX < ValX)
                maxX = ValX;

            if (minY > ValY)
                minY = ValY;
            if (maxY < ValY)
                maxY = ValY;

            vAxisX.push_back(ValX);
            vAxisY.push_back(ValY);
        }       


        ui.customPlotProfile_RFA300->addGraph();
        ui.customPlotProfile_RFA300->graph(iIndex)->setData(vAxisX, vAxisY);

        //Get Color
        Qt::GlobalColor color;
        QString strScanName;

        if ((*it).m_ScanType == ST_PDD)
        {
            color = Qt::red;
            strScanName = "PDD";
        }            
        else if ((*it).m_ScanType == ST_PROFILE_CR)
        {
            color = Qt::blue;
            strScanName = "PROFILE_CR";
        }
        else if ((*it).m_ScanType == ST_PROFILE_IN)
        {
            color = Qt::green;
            strScanName = "PROFILE_IN";
        }           
        ui.customPlotProfile_RFA300->graph(iIndex)->setPen(QPen(color));
        ui.customPlotProfile_RFA300->graph(iIndex)->setName(strScanName);

        iIndex++;
    }

    if (bInitMinMax)
    {
        ui.lineEditXMin_RFA300->setText(QString("%1").arg(minX));
        ui.lineEditXMax_RFA300->setText(QString("%1").arg(maxX));
    }

    double tmpXMin = ui.lineEditXMin_RFA300->text().toDouble();
    double tmpXMax = ui.lineEditXMax_RFA300->text().toDouble();
    double tmpYMin = ui.lineEditYMin_RFA300->text().toDouble();
    double tmpYMax = ui.lineEditYMax_RFA300->text().toDouble();

    ui.customPlotProfile_RFA300->xAxis->setRange(tmpXMin, tmpXMax);
    ui.customPlotProfile_RFA300->yAxis->setRange(tmpYMin, tmpYMax);

    ui.customPlotProfile_RFA300->xAxis->setLabel("mm");
    ui.customPlotProfile_RFA300->yAxis->setLabel("%");    

    ui.customPlotProfile_RFA300->setTitle("Dose profiles");

    QFont titleFont = font();
    titleFont.setPointSize(10);

    ui.customPlotProfile_RFA300->setTitleFont(titleFont);

    //ui.customPlotProfile_RFA300->legend->setVisible(true);
    ui.customPlotProfile_RFA300->legend->setVisible(false);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(8); // and make a bit smaller for legend
    ui.customPlotProfile_RFA300->legend->setFont(legendFont);
    ui.customPlotProfile_RFA300->legend->setPositionStyle(QCPLegend::psTopRight);
    ui.customPlotProfile_RFA300->legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
    ui.customPlotProfile_RFA300->replot();
}

void beamdata_gen_gui::SLT_ResampleAll()
{
    if (m_vDoseImages.empty())
        return;    
    
    //Resample

    vector<FloatImageType::Pointer>::iterator it;

    vector<FloatImageType::Pointer> vDoseImagesResampled;

    for (it = m_vDoseImages.begin(); it != m_vDoseImages.end(); ++it)
    {
        FloatImageType::Pointer spCurImg = (*it);

        FloatImageType::Pointer spNewImg;
        VEC3D new_spacing;
        new_spacing.x = ui.lineEdit_NewSpacingDCM_X->text().toFloat();
        new_spacing.y = ui.lineEdit_NewSpacingDCM_Y->text().toFloat();
        new_spacing.z = ui.lineEdit_NewSpacingDCM_Z->text().toFloat();
        
        QUTIL::ResampleFloatImg(spCurImg, spNewImg, new_spacing);        
        vDoseImagesResampled.push_back(spNewImg);
    }

    m_vDoseImages.clear();//delete old
    m_vRefDose.clear();

    for (it = vDoseImagesResampled.begin(); it != vDoseImagesResampled.end(); ++it)
    {
        FloatImageType::Pointer spCurImg = (*it);        
        m_vDoseImages.push_back(spCurImg);

        typedef itk::MinimumMaximumImageCalculator<FloatImageType> MinimumMaximumImageCalculatorType;
        MinimumMaximumImageCalculatorType::Pointer minimumMaximumImageCalculatorFilter = MinimumMaximumImageCalculatorType::New();
        minimumMaximumImageCalculatorFilter->SetImage(spCurImg);
        minimumMaximumImageCalculatorFilter->Compute();
        float maxVal = minimumMaximumImageCalculatorFilter->GetMaximum();
        m_vRefDose.push_back(maxVal);
    }

    vDoseImagesResampled.clear();//may not be needed.

    cout << m_vDoseImages.size() << " files were successfully replaced." << endl;

    disconnect(ui.comboBoxFileName, SIGNAL(currentIndexChanged(int)), this, SLOT(SLT_WhenSelectCombo()));
    SLT_UpdateComboContents();
    connect(ui.comboBoxFileName, SIGNAL(currentIndexChanged(int)), this, SLOT(SLT_WhenSelectCombo()));

    SLT_WhenSelectCombo(); //Draw all included
    SLT_GoCenterPos(); //Draw all included
}



//void beamdata_gen_gui::SLT_RunBatchGamma()
//{
//    //number of batch should be matched
//    int cntRef = m_strlistPath_RD_Original_Ref.count();
//    int cntComp = m_strlistPath_RD_Original_Comp.count();
//
//    QDir tmpCheckDir(m_strPathDirWorkDir);
//
//
//    if (m_strPathDirWorkDir.isEmpty())
//    {
//        QUTIL::ShowErrorMessage("Error! No work space is specified. Set it first.");
//        return;
//    }
//
//    if (!tmpCheckDir.exists())
//    {
//        QUTIL::ShowErrorMessage("Error! Current work space doesn't exist. Set it again");
//        return;
//    }        
//
//    if (cntRef*cntComp == 0 || cntRef != cntComp)
//    {
//        cout << "ERROR! Invalid input file counts" << endl;
//        return;
//    }
//
//    //Check the working directory. Subfolders..This should have no relavant subdirectories    
//    
//    QString strParamSet;
//    strParamSet.sprintf("_%dmm_%dp", ui.lineEdit_dta_tol->text().toInt(), ui.lineEdit_dose_tol->text().toInt());
//
//    float fResmp = ui.lineEdit_inhereResample->text().toFloat();
//
//    if (fResmp > 0 && ui.checkBox_inhereResample->isChecked())
//        strParamSet = strParamSet + "_rsmp" + QString::number(fResmp, 'd', 0);
//
//    if (ui.checkBox_Interp_search->isChecked())
//        strParamSet = strParamSet + "_interp";    
//
//
//    QString timeStamp = QUTIL::GetTimeStampDirName();
//
//    QString strSubRef = "DoseRef_" + timeStamp;
//    QString strSubComp = "DoseComp_"+ timeStamp;
//    QString strSubAnalysis = "Analysis_" + timeStamp + strParamSet;
//
//    //Create Folders
//
//    QDir crntWorkDir(m_strPathDirWorkDir);
//    crntWorkDir.mkdir(strSubRef);
//    crntWorkDir.mkdir(strSubComp);
//    crntWorkDir.mkdir(strSubAnalysis);
//
//    QString strPathDirReadRef = m_strPathDirWorkDir + "/" + strSubRef;
//    QString strPathDirReadComp = m_strPathDirWorkDir + "/" + strSubComp;
//    QString strPathDirAnalysis = m_strPathDirWorkDir + "/" + strSubAnalysis;
//
//    m_strlistPath_RD_Read_Ref.clear();
//    m_strlistPath_RD_Read_Comp.clear();
//
//    m_strlistBatchReport.clear();
//    m_strlistPath_Output_Gammamap.clear();
//    m_strlistPath_Output_Failure.clear();
//    m_strlistPath_Output_Report.clear();
//    m_vRefDose.clear();
//
//    QString dirPathFirstFileDir; //for saving workspace
//    QString dirPathFirstFileBase; //for saving workspace
//
//    for (int i = 0; i < cntRef; i++)
//    {
//        QString strPathRef = m_strlistPath_RD_Original_Ref.at(i);
//        QString strPathComp = m_strlistPath_RD_Original_Comp.at(i);
//
//        QFileInfo fInfoRef = QFileInfo(strPathRef);
//        QFileInfo fInfoComp = QFileInfo(strPathComp);        
//
//        QString baseNameRef = fInfoRef.completeBaseName();
//        QString baseNameComp = fInfoComp.completeBaseName();
//
//        if (i == 0) //first image location
//        {
//            //dirPathFirstFileDir = dirPath;
//            dirPathFirstFileBase = baseNameComp;
//        }
//        QString strPathBkupRef = strPathDirReadRef + "/" + baseNameRef + ".mha";
//        QString strPathBkupComp = strPathDirReadComp + "/" + baseNameComp + ".mha";
//
//
//        if (strPathRef.length() < 2 || strPathComp.length() < 2)
//            continue;//skip this pair
//
//        Gamma_parms parms;
//        //Gamma param: should come from the UI
//        parms.b_ref_only_threshold = false;
//        parms.mask_image_fn = "";
//        //parms->reference_dose;
//        parms.gamma_max = 2.0;
//        parms.b_compute_full_region = false;
//        parms.b_resample_nn = false; //default: false
//
//        //From File List
//        parms.ref_image_fn = strPathRef.toLocal8Bit().constData();
//        parms.cmp_image_fn = strPathComp.toLocal8Bit().constData();
//
//        //From GUI
//        if (ui.checkBox_inhereResample->isChecked())
//            parms.f_inherent_resample_mm = ui.lineEdit_inhereResample->text().toDouble();
//        else
//            parms.f_inherent_resample_mm = -1.0;
//        
//        parms.b_interp_search = ui.checkBox_Interp_search->isChecked();
//        
//        if (ui.radioButton_localGamma->isChecked())
//        {
//            parms.b_local_gamma = true;
//        }
//        else
//        {
//            parms.b_local_gamma = false;
//
//        }
//
//        float inputRefDose = ui.lineEdit_refDoseInGy->text().toFloat();
//
//        if (inputRefDose <= 0) //blank
//        {
//            parms.have_reference_dose = false;
//            parms.reference_dose = 0.0;
//        }
//        else
//        {
//            parms.have_reference_dose = true;
//            parms.reference_dose = inputRefDose;
//        }
//
//        parms.dta_tolerance = ui.lineEdit_dta_tol->text().toDouble();
//        parms.dose_tolerance = ui.lineEdit_dose_tol->text().toDouble() / 100.0;//gui input: 3% --> param: 0.03
//        parms.f_analysis_threshold = ui.lineEdit_cutoff_dose->text().toDouble() / 100.0;
//        
//        //Saving folder: comp folder. FileName Should Include dta, dose, local/global      
//
//        QString strLocGlob;
//
//        if (parms.b_local_gamma)
//            strLocGlob = "loc";        
//        else
//            strLocGlob = "glb";
//
//        QString strSettingAbs = QString::number(parms.dta_tolerance, 'f', 0) + "mm_" + ""
//            + QString::number(parms.dose_tolerance*100.0, 'f', 0) + "%_" + strLocGlob;
//
//
//        QString outputPath = strPathDirAnalysis + "/" + baseNameComp + "_gammamap" + ".mha";
//        parms.out_image_fn = outputPath.toLocal8Bit().constData();
//        m_strlistPath_Output_Gammamap.push_back(outputPath);
//      
//
//        //if (ui.checkBox_failuremap_output->isChecked())
//        //{
//            //QString outputPath = dirPath + "/" + baseName + "_failmap_" + strSettingAbs + ".mha";
//        outputPath = strPathDirAnalysis + "/" + baseNameComp + "_failmap" + ".mha";
//        parms.out_failmap_fn = outputPath.toLocal8Bit().constData();
//        m_strlistPath_Output_Failure.push_back(outputPath);
//        //}           
//
//        //QString outputPath = dirPath + "/" + baseName + "_report_" + strSettingAbs + ".txt";
//        outputPath = strPathDirAnalysis + "/" + baseNameComp + "_report" + ".txt";
//        parms.out_report_fn = outputPath.toLocal8Bit().constData();
//        m_strlistPath_Output_Report.push_back(outputPath);
//
//        float refDoseGy;
//        QString overallReport = GammaMain(&parms, refDoseGy, strPathBkupRef, strPathBkupComp);        
//        m_strlistBatchReport.push_back(overallReport);
//
//        m_strlistPath_RD_Read_Ref.push_back(strPathBkupRef);
//        m_strlistPath_RD_Read_Comp.push_back(strPathBkupComp);
//
//        m_vRefDose.push_back(refDoseGy);        
//    }
//    //Save WorkSpace File for future loading
//
//    
//    //QString strPathGammaWorkSpace = m_strPathDirWorkDir + "/" + dirPathFirstFileBase + "_" + strParamSet + "_" + QString("%1").arg(cntRef) + "cases.gws"; //gamma work space
//
//    QString strPathGammaWorkSpace = m_strPathDirWorkDir + "/" + strSubAnalysis + ".gws"; //gamma work space
//    QString strFilePathReport = m_strPathDirWorkDir + "/" + strSubAnalysis + "BatchReport.txt"; //gamma work space
//
//    SaveCurrentGammaWorkSpace(strPathGammaWorkSpace);
//
//    cout << cntRef << " analysis were successfully done!" << endl;
//
//    SLT_LoadResults();
//
//    //After the batch mode analysis, export the simpe report.    
//    //Only when the number of files is > 1
////    if (cntRef == 1)
//  //      return;    
//
//    SaveBatchGamma3DSimpleReport(strFilePathReport);
//        
//        /*QString fileName = QFileDialog::getSaveFileName(this, "Save batch report file", "", "report (*.txt)", 0, 0);
//
//        if (fileName.length() < 1)
//        return;
//
//        ofstream fout;
//        fout.open(fileName.toLocal8Bit().constData());
//        fout << "Reference_File\t"
//        << "Compare_File\t"
//        << "dta_tolerance[mm]\t"
//        << "dose_tolerance[%]\t"
//        << "doseCutoff[%]\t"
//        << "Local/Global\t"
//        << "Ref_dose[Gy]\t"
//        << "VoxNumAnalyzed\t"
//        << "VoxNumPassed\t"
//        << "GammaPassRate[%]" << endl;
//
//        for (int i = 0; i < cntRef; i++)
//        {
//        fout << m_strlistBatchReport.at(i).toLocal8Bit().constData() << endl;
//        }
//
//        fout.close();*/
//}
