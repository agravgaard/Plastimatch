#include "gamma_gui.h"
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


gamma_gui::gamma_gui(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
    ui.setupUi(this);

    m_pCurImageRef = new YK16GrayImage();
    m_pCurImageComp = new YK16GrayImage();
    m_pCurImageGamma3D = new YK16GrayImage();
    m_pCurImageGamma2D = new YK16GrayImage();

    //QUTIL::LoadColorTableFromFile("colormap_jet.txt", m_vColormapDose);
    //QUTIL::LoadColorTableFromFile("colormap_customgamma.txt", m_vColormapGamma);

    QUTIL::LoadColorTableInternal(m_vColormapDose, COL_TABLE_JET);
    QUTIL::LoadColorTableInternal(m_vColormapGamma, COL_TABLE_GAMMA);
    

    m_pCurImageRef->SetColorTable(m_vColormapDose);
    m_pCurImageComp->SetColorTable(m_vColormapDose);

    m_pCurImageGamma3D->SetColorTable(m_vColormapGamma);
    //m_pCurImageGamma3D->SetColorTable(m_vColormapGammaLow);

    m_pCurImageGamma2D->SetColorTable(m_vColormapGamma);
    //m_pCurImageGamma2D->SetColorTableGammaHigh(m_vColormapGammaLow);


    connect(ui.labelReferDose, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosRef())); //added
    connect(ui.labelCompDose, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosComp())); //added
    connect(ui.labelGammaMap2D, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosGamma2D())); //added
    connect(ui.labelGammaMap3D, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosGamma3D())); //added

    connect(ui.labelReferDose, SIGNAL(Mouse_Left_DoubleClick()), this, SLOT(SLT_GoCenterPosRef())); //added
    connect(ui.labelCompDose, SIGNAL(Mouse_Left_DoubleClick()), this, SLOT(SLT_GoCenterPosComp())); //added


    connect(ui.labelReferDose, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateRef())); //added
    connect(ui.labelCompDose, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateComp())); //added
    connect(ui.labelGammaMap2D, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateGamma2D())); //added
    connect(ui.labelGammaMap3D, SIGNAL(Mouse_Wheel()), this, SLOT(SLT_MouseWheelUpdateGamma3D())); //added


    connect(ui.labelReferDose, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingRef())); //added
    connect(ui.labelCompDose, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingComp())); //added
    connect(ui.labelGammaMap3D, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingGamma3D())); //added
    connect(ui.labelGammaMap2D, SIGNAL(Mouse_Move()), this, SLOT(SLT_UpdatePanSettingGamma2D())); //added

    connect(ui.labelReferDose, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightRef())); //added
    connect(ui.labelCompDose, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightComp())); //added
    connect(ui.labelGammaMap3D, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightGamma3D())); //added
    connect(ui.labelGammaMap2D, SIGNAL(Mouse_Pressed_Right()), this, SLOT(SLT_MousePressedRightGamma2D())); //added

    connect(ui.labelReferDose, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightRef())); //added
    connect(ui.labelCompDose, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightComp())); //added
    connect(ui.labelGammaMap3D, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightGamma3D())); //added
    connect(ui.labelGammaMap2D, SIGNAL(Mouse_Released_Right()), this, SLOT(SLT_MouseReleasedRightGamma2D())); //added    


    if (m_vColormapDose.size() < 1 || m_vColormapGamma.size() < 1)
    {
        cout << "Fatal error!: colormap is not ready. Colormap table should be checked." << endl;
    }

    m_pTableModel = NULL;

    m_bGamma2DIsDone = false;

    m_bMousePressedRightRef = false;
    m_bMousePressedRightComp = false;
    m_bMousePressedRightGamma3D = false;
    m_bMousePressedRightGamma2D = false;

    QUTIL::CreateItkDummyImg(m_spDummyLowMemImg, 10, 10, 10, 1.0);   

    m_iLastLoadedIndex = -1;
}

gamma_gui::~gamma_gui()
{
    //delete m_pImgOffset;
    //delete m_pImgGain;

    //m_vPixelReplMap.clear(); //not necessary
    //delete m_pView;
     m_vRefDoseImages.clear();
     m_vCompDoseImages.clear();
     m_vGammaMapImages.clear();
     
    delete m_pCurImageRef;
    delete m_pCurImageComp;
    delete m_pCurImageGamma3D;
    delete m_pCurImageGamma2D;

    if (m_pTableModel != NULL)
    {
        delete m_pTableModel;
        m_pTableModel = NULL;
    }

}

void gamma_gui::SLT_Load_RD_Ref()
{
    QStringList tmpList = QFileDialog::getOpenFileNames(this, "Select one or more files to open", m_strPathInputDir, "3D dose file (*.dcm *.mha)");

    int iFileCnt = tmpList.size();

    if (iFileCnt < 1)
        return;

    ui.plainTextEdit_RD_Ref->clear();
    m_strlistPath_RD_Original_Ref.clear();
    m_strlistFileBaseName_Ref.clear();

    m_strlistPath_RD_Original_Ref = tmpList;

    for (int i = 0; i < iFileCnt; i++)
    {
        ui.plainTextEdit_RD_Ref->appendPlainText(m_strlistPath_RD_Original_Ref.at(i)); //just for display
        QFileInfo tmpInfo = QFileInfo(m_strlistPath_RD_Original_Ref.at(i));
        m_strlistFileBaseName_Ref.push_back(tmpInfo.completeBaseName());
    }

    QFileInfo finfo(m_strlistPath_RD_Original_Ref.at(0));
    QDir crntDir = finfo.absoluteDir();
    m_strPathInputDir = crntDir.absolutePath();
}

void gamma_gui::SLT_Load_RD_Comp()
{
    QStringList tmpList = QFileDialog::getOpenFileNames(this, "Select one or more files to open", m_strPathInputDir, "3D dose file (*.dcm *.mha)");

    int iFileCnt = tmpList.size();

    if (iFileCnt < 1)
        return;
    
    ui.plainTextEdit_RD_Comp->clear();
    m_strlistPath_RD_Original_Comp.clear();
    m_strlistFileBaseName_Comp.clear();

    m_strlistPath_RD_Original_Comp = tmpList;

    for (int i = 0; i < iFileCnt; i++)
    {
        ui.plainTextEdit_RD_Comp->appendPlainText(m_strlistPath_RD_Original_Comp.at(i)); //just for display

        QFileInfo tmpInfo = QFileInfo(m_strlistPath_RD_Original_Comp.at(i));

        m_strlistFileBaseName_Comp.push_back(tmpInfo.completeBaseName());
    }

    QFileInfo finfo(m_strlistPath_RD_Original_Comp.at(0));
    QDir crntDir = finfo.absoluteDir();
    m_strPathInputDir = crntDir.absolutePath();
}

void gamma_gui::SLT_RunBatchGamma()
{
    //number of batch should be matched
    int cntRef = m_strlistPath_RD_Original_Ref.count();
    int cntComp = m_strlistPath_RD_Original_Comp.count();

    QDir tmpCheckDir(m_strPathDirWorkDir);


    if (m_strPathDirWorkDir.isEmpty())
    {
        QUTIL::ShowErrorMessage("Error! No work space is specified. Set it first.");
        return;
    }

    if (!tmpCheckDir.exists())
    {
        QUTIL::ShowErrorMessage("Error! Current work space doesn't exist. Set it again");
        return;
    }        

    if (cntRef*cntComp == 0 || cntRef != cntComp)
    {
        cout << "ERROR! Invalid input file counts" << endl;
        return;
    }

    //Check the working directory. Subfolders..This should have no relavant subdirectories    
    
    QString strParamSet;
    strParamSet.sprintf("_%dmm_%dp", ui.lineEdit_dta_tol->text().toInt(), ui.lineEdit_dose_tol->text().toInt());

    float fResmp = ui.lineEdit_inhereResample->text().toFloat();

    if (fResmp > 0 && ui.checkBox_inhereResample->isChecked())
        strParamSet = strParamSet + "_rsmp" + QString::number(fResmp, 'd', 0);

    if (ui.checkBox_Interp_search->isChecked())
        strParamSet = strParamSet + "_interp";    


    QString timeStamp = QUTIL::GetTimeStampDirName();

    QString strSubRef = "DoseRef_" + timeStamp;
    QString strSubComp = "DoseComp_"+ timeStamp;
    QString strSubAnalysis = "Analysis_" + timeStamp + strParamSet;

    //Create Folders

    QDir crntWorkDir(m_strPathDirWorkDir);
    crntWorkDir.mkdir(strSubRef);
    crntWorkDir.mkdir(strSubComp);
    crntWorkDir.mkdir(strSubAnalysis);

    QString strPathDirReadRef = m_strPathDirWorkDir + "/" + strSubRef;
    QString strPathDirReadComp = m_strPathDirWorkDir + "/" + strSubComp;
    QString strPathDirAnalysis = m_strPathDirWorkDir + "/" + strSubAnalysis;

    m_strlistPath_RD_Read_Ref.clear();
    m_strlistPath_RD_Read_Comp.clear();

    m_strlistBatchReport.clear();
    m_strlistPath_Output_Gammamap.clear();
    m_strlistPath_Output_Failure.clear();
    m_strlistPath_Output_Report.clear();
    m_vRefDose.clear();

    QString dirPathFirstFileDir; //for saving workspace
    QString dirPathFirstFileBase; //for saving workspace

    for (int i = 0; i < cntRef; i++)
    {
        QString strPathRef = m_strlistPath_RD_Original_Ref.at(i);
        QString strPathComp = m_strlistPath_RD_Original_Comp.at(i);

        QFileInfo fInfoRef = QFileInfo(strPathRef);
        QFileInfo fInfoComp = QFileInfo(strPathComp);        

        QString baseNameRef = fInfoRef.completeBaseName();
        QString baseNameComp = fInfoComp.completeBaseName();

        if (i == 0) //first image location
        {
            //dirPathFirstFileDir = dirPath;
            dirPathFirstFileBase = baseNameComp;
        }
        QString strPathBkupRef = strPathDirReadRef + "/" + baseNameRef + ".mha";
        QString strPathBkupComp = strPathDirReadComp + "/" + baseNameComp + ".mha";


        if (strPathRef.length() < 2 || strPathComp.length() < 2)
            continue;//skip this pair

        Gamma_parms parms;
        //Gamma param: should come from the UI
        parms.b_ref_only_threshold = false;
        parms.mask_image_fn = "";
        //parms->reference_dose;
        parms.gamma_max = 2.0;
        parms.b_compute_full_region = false;
        parms.b_resample_nn = false; //default: false

        //From File List
        parms.ref_image_fn = strPathRef.toLocal8Bit().constData();
        parms.cmp_image_fn = strPathComp.toLocal8Bit().constData();

        //From GUI
        if (ui.checkBox_inhereResample->isChecked())
            parms.f_inherent_resample_mm = ui.lineEdit_inhereResample->text().toDouble();
        else
            parms.f_inherent_resample_mm = -1.0;
        
        parms.b_interp_search = ui.checkBox_Interp_search->isChecked();
        
        if (ui.radioButton_localGamma->isChecked())
        {
            parms.b_local_gamma = true;
        }
        else
        {
            parms.b_local_gamma = false;

        }

        float inputRefDose = ui.lineEdit_refDoseInGy->text().toFloat();

        if (inputRefDose <= 0) //blank
        {
            parms.have_reference_dose = false;
            parms.reference_dose = 0.0;
        }
        else
        {
            parms.have_reference_dose = true;
            parms.reference_dose = inputRefDose;
        }

        parms.dta_tolerance = ui.lineEdit_dta_tol->text().toDouble();
        parms.dose_tolerance = ui.lineEdit_dose_tol->text().toDouble() / 100.0;//gui input: 3% --> param: 0.03
        parms.f_analysis_threshold = ui.lineEdit_cutoff_dose->text().toDouble() / 100.0;
        
        //Saving folder: comp folder. FileName Should Include dta, dose, local/global      

        QString strLocGlob;

        if (parms.b_local_gamma)
            strLocGlob = "loc";        
        else
            strLocGlob = "glb";

        QString strSettingAbs = QString::number(parms.dta_tolerance, 'f', 0) + "mm_" + ""
            + QString::number(parms.dose_tolerance*100.0, 'f', 0) + "%_" + strLocGlob;


        QString outputPath = strPathDirAnalysis + "/" + baseNameComp + "_gammamap" + ".mha";
        parms.out_image_fn = outputPath.toLocal8Bit().constData();
        m_strlistPath_Output_Gammamap.push_back(outputPath);
      

        //if (ui.checkBox_failuremap_output->isChecked())
        //{
            //QString outputPath = dirPath + "/" + baseName + "_failmap_" + strSettingAbs + ".mha";
        outputPath = strPathDirAnalysis + "/" + baseNameComp + "_failmap" + ".mha";
        parms.out_failmap_fn = outputPath.toLocal8Bit().constData();
        m_strlistPath_Output_Failure.push_back(outputPath);
        //}           

        //QString outputPath = dirPath + "/" + baseName + "_report_" + strSettingAbs + ".txt";
        outputPath = strPathDirAnalysis + "/" + baseNameComp + "_report" + ".txt";
        parms.out_report_fn = outputPath.toLocal8Bit().constData();
        m_strlistPath_Output_Report.push_back(outputPath);

        float refDoseGy;
        QString overallReport = GammaMain(&parms, refDoseGy, strPathBkupRef, strPathBkupComp);        
        m_strlistBatchReport.push_back(overallReport);

        m_strlistPath_RD_Read_Ref.push_back(strPathBkupRef);
        m_strlistPath_RD_Read_Comp.push_back(strPathBkupComp);

        m_vRefDose.push_back(refDoseGy);        
    }
    //Save WorkSpace File for future loading

    
    //QString strPathGammaWorkSpace = m_strPathDirWorkDir + "/" + dirPathFirstFileBase + "_" + strParamSet + "_" + QString("%1").arg(cntRef) + "cases.gws"; //gamma work space

    QString strPathGammaWorkSpace = m_strPathDirWorkDir + "/" + strSubAnalysis + ".gws"; //gamma work space
    QString strFilePathReport = m_strPathDirWorkDir + "/" + strSubAnalysis + "BatchReport.txt"; //gamma work space

    SaveCurrentGammaWorkSpace(strPathGammaWorkSpace);

    cout << cntRef << " analysis were successfully done!" << endl;

    SLT_LoadResults();

    //After the batch mode analysis, export the simpe report.    
    //Only when the number of files is > 1
//    if (cntRef == 1)
  //      return;    

    SaveBatchGamma3DSimpleReport(strFilePathReport);
        
        /*QString fileName = QFileDialog::getSaveFileName(this, "Save batch report file", "", "report (*.txt)", 0, 0);

        if (fileName.length() < 1)
        return;

        ofstream fout;
        fout.open(fileName.toLocal8Bit().constData());
        fout << "Reference_File\t"
        << "Compare_File\t"
        << "dta_tolerance[mm]\t"
        << "dose_tolerance[%]\t"
        << "doseCutoff[%]\t"
        << "Local/Global\t"
        << "Ref_dose[Gy]\t"
        << "VoxNumAnalyzed\t"
        << "VoxNumPassed\t"
        << "GammaPassRate[%]" << endl;

        for (int i = 0; i < cntRef; i++)
        {
        fout << m_strlistBatchReport.at(i).toLocal8Bit().constData() << endl;
        }

        fout.close();*/
}

void gamma_gui::SaveBatchGamma3DSimpleReport(QString& strFilePath)
{
    //QString fileName = QFileDialog::getSaveFileName(this, "Save batch report file", "", "report (*.txt)", 0, 0);

    /*if (fileName.length() < 1)
        return;*/
    if (m_strlistBatchReport.count() < 1)
    {
        cout << "Error! No report is available" << endl;
        return;
    }        

    ofstream fout;
    fout.open(strFilePath.toLocal8Bit().constData());
    fout << "Reference_File\t"
        << "Compare_File\t"
        << "dta_tolerance[mm]\t"
        << "dose_tolerance[%]\t"
        << "doseCutoff[%]\t"
        << "Local/Global\t"
        << "Ref_dose[Gy]\t"
        << "VoxNumAnalyzed\t"
        << "VoxNumPassed\t"
        << "GammaPassRate[%]" << endl;

    int cnt = m_strlistBatchReport.count();

    for (int i = 0; i < cnt; i++)
    {
        fout << m_strlistBatchReport.at(i).toLocal8Bit().constData() << endl;
    }

    fout.close();
}


void gamma_gui::SLTM_SaveBatchModeSimpleReport()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save batch report file", m_strPathDirWorkDir, "report (*.txt)", 0, 0);

    if (filePath.length() < 1)
        return;

    QFileInfo fInfo(filePath);

    if (fInfo.suffix() != "txt" && fInfo.suffix() != "TXT")
    {
        cout << "Saving filter didn't work. Saving this file as txt." << endl;
        filePath = filePath + ".txt";
    }

    SaveBatchGamma3DSimpleReport(filePath);
}


QString gamma_gui::GammaMain(Gamma_parms* parms, float& refDoseGy, const QString& strPathBkupRef, const QString& strPathBkupComp)
{
    QString reportResult;
    Gamma_dose_comparison gdc;

    //DICOM_RD compatible (added by YK, Feb 2015)
    //In the prev version, RD couldn't be read directly due to the scale factor inside of the DICOM file.
    //work-around was to use (plastimatch convert ...)
    //here, that feature has been integrated into plastimatch gamma
    Plm_file_format file_type_ref, file_type_comp;
    Rt_study rt_study_ref, rt_study_comp;

    file_type_ref = plm_file_format_deduce(parms->ref_image_fn.c_str());
    file_type_comp = plm_file_format_deduce(parms->cmp_image_fn.c_str());

    if (file_type_ref == PLM_FILE_FMT_DICOM_DOSE) {
        rt_study_ref.load(parms->ref_image_fn.c_str(), file_type_ref);
        if (rt_study_ref.has_dose()){
            gdc.set_reference_image(rt_study_ref.get_dose()->clone());
        }
        else{
            gdc.set_reference_image(parms->ref_image_fn.c_str());
        }
    }
    else {
        gdc.set_reference_image(parms->ref_image_fn.c_str());
    }

    if (file_type_comp == PLM_FILE_FMT_DICOM_DOSE) {
        rt_study_comp.load(parms->cmp_image_fn.c_str(), file_type_comp);
        if (rt_study_comp.has_dose()) {
            gdc.set_compare_image(rt_study_comp.get_dose()->clone());
        }
        else {
            gdc.set_compare_image(parms->cmp_image_fn.c_str());
        }

    }
    else {
        gdc.set_compare_image(parms->cmp_image_fn.c_str());
    }
    //End DICOM-RD    

    if (parms->mask_image_fn != "") {
        gdc.set_mask_image(parms->mask_image_fn);
    }

    gdc.set_spatial_tolerance(parms->dta_tolerance);
    gdc.set_dose_difference_tolerance(parms->dose_tolerance);
    if (parms->have_reference_dose) {
        gdc.set_reference_dose(parms->reference_dose);
    }
    gdc.set_gamma_max(parms->gamma_max);

    /*Extended by YK*/
    gdc.set_interp_search(parms->b_interp_search);//default: false
    gdc.set_local_gamma(parms->b_local_gamma);//default: false
    gdc.set_compute_full_region(parms->b_compute_full_region);//default: false
    gdc.set_resample_nn(parms->b_resample_nn); //default: false
    gdc.set_ref_only_threshold(parms->b_ref_only_threshold);

    if (parms->f_inherent_resample_mm > 0.0) {
        gdc.set_inherent_resample_mm(parms->f_inherent_resample_mm);
    }

    if (parms->f_analysis_threshold > 0) {
        gdc.set_analysis_threshold(parms->f_analysis_threshold);//0.1 = 10%
    }

    gdc.run();

    if (parms->out_image_fn != "") {
        Plm_image::Pointer gamma_image = gdc.get_gamma_image();
        gamma_image->save_image(parms->out_image_fn);
    }

    if (parms->out_failmap_fn != "") {
        gdc.get_fail_image()->save_image(parms->out_failmap_fn);
    }

    if (parms->out_report_fn != "") {
        //Export utput text using gdc.get_report_string();
        std::ofstream fout;
        fout.open(parms->out_report_fn.c_str());
        if (!fout.fail()){
            fout << gdc.get_report_string();
            fout << "Reference_file_name\t" << parms->ref_image_fn.c_str() << std::endl;
            fout << "Compare_file_name\t" << parms->cmp_image_fn.c_str() << std::endl;            
            fout.close();
        }
    }
    printf ("Pass rate = %2.6f %%\n", gdc.get_pass_fraction() * 100.0);
    
    //Composite a result string
    //FileName
    QFileInfo info_ref = QFileInfo(parms->ref_image_fn.c_str());
    QFileInfo info_comp = QFileInfo(parms->cmp_image_fn.c_str());   
    
    QString fileName_ref = info_ref.fileName();
    QString fileName_comp = info_comp.fileName();

    QString strTol_dta = QString::number(parms->dta_tolerance,'f',2);
    QString strTol_dose = QString::number(parms->dose_tolerance*100.0, 'f', 0);
    QString strDoseCutoff = QString::number(parms->f_analysis_threshold*100, 'f', 0);

    QString strLocalOrGlobal;
    if (parms->b_local_gamma)
        strLocalOrGlobal = "local";
    else
        strLocalOrGlobal = "global";

    //if this is dcm, save the mha files
    //if (info_ref.suffix() == "dcm" || info_ref.suffix() == "DCM")
    //{
        //QString newPath = info_ref.absolutePath() + "/" + info_ref.completeBaseName() + ".mha";        

    if (!strPathBkupRef.isEmpty())
    {
        Plm_image* pImg = gdc.get_ref_image();
        pImg->save_image(strPathBkupRef.toLocal8Bit().constData());
    }
        
    //}

    //if (info_comp.suffix() == "dcm" || info_comp.suffix() == "DCM")
    //{
        //QString newPath = info_comp.absolutePath() + "/" + info_comp.completeBaseName() + ".mha";
        //QString newPath = info_comp.absolutePath() + "/" + info_comp.completeBaseName() + ".mha";

    if (!strPathBkupComp.isEmpty())
    {
        Plm_image* pImg = gdc.get_comp_image();
        pImg->save_image(strPathBkupComp.toLocal8Bit().constData());
    }    
    QString strRef_dose = QString::number(gdc.get_reference_dose(), 'f', 2);//Gy
    QString strVoxNumAnalyzed = QString::number(gdc.get_analysis_num_vox(), 'f', 2);
    QString strVoxNumPassed = QString::number(gdc.get_passed_num_vox(), 'f', 2);
    QString strPassRate = QString::number(gdc.get_pass_fraction()*100.0, 'f', 2);

    reportResult = fileName_ref + "\t"
        + fileName_comp + "\t"
        + strTol_dta + "\t"
        + strTol_dose + "\t"
        + strDoseCutoff + "\t"
        + strLocalOrGlobal + "\t"
        + strRef_dose + "\t"
        + strVoxNumAnalyzed + "\t"
        + strVoxNumPassed + "\t"
        + strPassRate;
    
    refDoseGy = gdc.get_reference_dose();
    return reportResult;
}

void gamma_gui::SLT_ProfileView()
{
    //m_pView->show();

}

void gamma_gui::SLT_DrawDoseImages()
{
    // refer to probe positions, selected 3D file (spPointer), plane direction

}

void gamma_gui::SLT_DrawGammaMap3D()
{

}

void gamma_gui::SLT_DrawGammaMap2D()
{

}

void gamma_gui::Load_FilesToMem()
{
    /*int cntRef = m_strlistPath_RD_Original_Ref.count();
    int cntComp = m_strlistPath_RD_Original_Comp.count();*/

    int cntRef = m_strlistPath_RD_Read_Ref.count();
    int cntComp = m_strlistPath_RD_Read_Comp.count();

    int cntGamma = m_strlistPath_Output_Gammamap.count();

    if (cntRef*cntComp == 0 || cntRef != cntComp || cntRef != cntGamma)
    {
        cout << "Error: number should be matched" << endl;
        return;
    }

    m_vRefDoseImages.clear();
    m_vCompDoseImages.clear();
    m_vGammaMapImages.clear();

    m_iLastLoadedIndex = -1;
    
    QString strPath_ref, strPath_comp, strPath_gamma;

    for (int i = 0; i < cntRef; i++)
    {   
        strPath_ref = m_strlistPath_RD_Read_Ref.at(i); //always readable mha format
        strPath_comp = m_strlistPath_RD_Read_Comp.at(i);
        strPath_gamma = m_strlistPath_Output_Gammamap.at(i);

        QFileInfo info_ref = QFileInfo(strPath_ref);
        QFileInfo info_comp = QFileInfo(strPath_comp);
    
        //if (info_ref.suffix() == "dcm" || info_ref.suffix() == "DCM")
        //{
        //    //strPath_ref = info_ref.absolutePath() + "/" + info_ref.completeBaseName() + ".mha";            
        //    strPath_ref = info_ref.absolutePath() + "/" + info_ref.completeBaseName() + ".mha";
        //}

        //if (info_comp.suffix() == "dcm" || info_comp.suffix() == "DCM")
        //{
        //    //strPath_comp = info_comp.absolutePath() + "/" + info_comp.completeBaseName() + ".mha";
        //    strPath_comp = info_comp.absolutePath() + "/" + info_comp.completeBaseName() + ".mha";
        //}

        bool bLowMem = ui.checkBox_low_mem->isChecked();       


        FloatImageType::Pointer spImgRef = FloatImageType::New();
        FloatImageType::Pointer spImgComp = FloatImageType::New();
        FloatImageType::Pointer spImgGamma = FloatImageType::New();



        if (bLowMem)
        {
            spImgRef = m_spDummyLowMemImg;
            spImgComp = m_spDummyLowMemImg;
            spImgGamma = m_spDummyLowMemImg;
        }         
        else
        {
            QUTIL::LoadFloatImage3D(strPath_ref.toLocal8Bit().constData(), spImgRef);
            QUTIL::LoadFloatImage3D(strPath_comp.toLocal8Bit().constData(), spImgComp);
            QUTIL::LoadFloatImage3D(strPath_gamma.toLocal8Bit().constData(), spImgGamma);
        }

        m_vRefDoseImages.push_back(spImgRef);        
        m_vCompDoseImages.push_back(spImgComp);
        m_vGammaMapImages.push_back(spImgGamma);        
    }    
}

void gamma_gui::SLT_LoadResults()
{    
    cout << "Loading image data... please wait" << endl;        
    Load_FilesToMem();    
    cout << "Dose and Gamma data are successfully loaded" << endl;

    disconnect(ui.comboBoxCompareFile, SIGNAL(currentIndexChanged(int)), this, SLOT(SLT_WhenSelectCombo()));
    SLT_UpdateComboContents();

    connect(ui.comboBoxCompareFile, SIGNAL(currentIndexChanged(int)), this, SLOT(SLT_WhenSelectCombo()));        

    SLT_WhenSelectCombo(); //initialization

    SLT_GoCenterPosRef();
    SLT_GoCenterPosComp();
    //SLT_DrawAll();
}

void gamma_gui::SLT_UpdateComboContents() //compare image based..
{    
    QComboBox* crntCombo = ui.comboBoxCompareFile;
    crntCombo->clear();    

    int cntComp = m_strlistFileBaseName_Comp.count();

    for (int i = 0; i < cntComp; i++)
    {
        //SLT_WHenComboSelect should be disconnected here
        crntCombo->addItem(m_strlistFileBaseName_Comp.at(i));        
    }    
}
void gamma_gui::SLT_DrawAll()
{
    //Get combo box selection
    QComboBox* crntCombo = ui.comboBoxCompareFile;
    //QString curStr = crntCombo->currentText(); //this should be basename
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    int iCnt = crntCombo->count();

    if (iCnt < 1)
        return;

    if ((int)(m_vRefDoseImages.size()) != iCnt ||
        (int)(m_vCompDoseImages.size()) != iCnt)
    {
        cout << "Error! iCnt not matching in DrawAll" << endl;
        return;
    }

    //ui.labelReferDose->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelReferDose->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    //ui.labelCompDose->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelCompDose->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    //ui.labelGammaMap3D->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelGammaMap3D->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    //ui.labelGammaMap2D->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelGammaMap2D->setFixedHeight(DEFAULT_LABEL_HEIGHT);    


    //Convert3DTo2D (float2D, get radio, probe pos from GUI)
    //Get3DData (ref, comp, gamma)
    FloatImageType::Pointer spCurRef = m_vRefDoseImages.at(curIdx);
    FloatImageType::Pointer spCurComp = m_vCompDoseImages.at(curIdx);
    //FloatImageType::Pointer spCurRef = m_spRefDoseImages;
    //FloatImageType::Pointer spCurComp = m_spCompDoseImages;

    FloatImageType::Pointer spCurGamma;
    
      if ((int)(m_vGammaMapImages.size()) == iCnt)
          spCurGamma = m_vGammaMapImages.at(curIdx);
    /*if (m_pCurImageGamma3D)
        spCurGamma = m_spGammaMapImages;*/

    //DICOM
    float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
    float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
    float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();

    //Move them to Global due to 2D gamma export
    /*FloatImage2DType::Pointer m_spCurRef2D;
    FloatImage2DType::Pointer m_spCurComp2D;*/
    //Move them to Global due to 2D gamma export

    FloatImage2DType::Pointer spCurGamma2DFrom3D; 
    double finalPos1, finalPos2, finalPos3;

    enPLANE curPlane = PLANE_AXIAL;
    float fixedPos = 0.0;

    float probePos2D_X =0.0;
    float probePos2D_Y = 0.0;

    bool bYFlip = false;
    
    if (ui.radioButtonAxial->isChecked())
    {
        curPlane = PLANE_AXIAL;
        fixedPos = probePosZ;

        probePos2D_X = probePosX;
        probePos2D_Y = probePosY;
    }
    else if (ui.radioButtonSagittal->isChecked())
    {
        curPlane = PLANE_SAGITTAL;
        fixedPos = probePosX;

        probePos2D_X = probePosY;
        probePos2D_Y = probePosZ;  //YKDebug: may be reversed     
        bYFlip = true;
    }
    else if (ui.radioButtonFrontal->isChecked())
    {
        curPlane = PLANE_FRONTAL;
        fixedPos = probePosY;        

        probePos2D_X = probePosX;
        probePos2D_Y = probePosZ;//YKDebug: may be reversed   
        bYFlip = true;
    }
    
    QUTIL::Get2DFrom3DByPosition(spCurRef, m_spCurRef2D, curPlane, fixedPos, finalPos1);
    QUTIL::Get2DFrom3DByPosition(spCurComp, m_spCurComp2D, curPlane, fixedPos, finalPos2);

  //  if (curPlane == PLANE_FRONTAL)
    //    QUTIL::SaveFloatImage2D("D:\\testFrontal.mha", m_spCurRef2D);//it is flipped!


    if (spCurGamma)
    {
        QUTIL::Get2DFrom3DByPosition(spCurGamma, spCurGamma2DFrom3D, curPlane, fixedPos, finalPos3);
    }    

    //Actually, frontal and sagittal image should be flipped for display purpose (in axial, Y is small to large, SAG and FRONTAL, Large to Small (head to toe direction)
    //Let's not change original data itself due to massing up the origin. Only change the display image
    //Point probe and profiles, other things works fine.


    //YKImage receives 2D float
    m_pCurImageRef->UpdateFromItkImageFloat(m_spCurRef2D, GY2YKIMG_MAG, NON_NEG_SHIFT, bYFlip); //flip Y for display only
    m_pCurImageComp->UpdateFromItkImageFloat(m_spCurComp2D, GY2YKIMG_MAG, NON_NEG_SHIFT, bYFlip);

    //cout << "GAMMA2YKIMG_MAG= " << GAMMA2YKIMG_MAG << endl;

    if (spCurGamma2DFrom3D)
    {
        m_pCurImageGamma3D->UpdateFromItkImageFloat(spCurGamma2DFrom3D, GAMMA2YKIMG_MAG, NON_NEG_SHIFT, bYFlip);
    }

    
    if (m_spGamma2DResult)
    {
        m_pCurImageGamma2D->UpdateFromItkImageFloat(m_spGamma2DResult, GAMMA2YKIMG_MAG, NON_NEG_SHIFT, bYFlip);
    }

    float doseGyNormRef = 0.01 * (ui.sliderNormRef->value());
    float doseGyNormComp = 0.01 *(ui.sliderNormComp->value());

    //cout << "doseGyNormComp= " << doseGyNormRef << endl;
    //PixMap

    m_pCurImageRef->SetNormValueOriginal(doseGyNormRef);
    m_pCurImageComp->SetNormValueOriginal(doseGyNormComp);

    m_pCurImageRef->FillPixMapDose();
    m_pCurImageComp->FillPixMapDose();
    //m_pCurImageRef->FillPixMapDose(doseGyNormRef);
    //m_pCurImageComp->FillPixMapDose(doseGyNormComp);
    m_pCurImageGamma3D->FillPixMapGamma();
    m_pCurImageGamma2D->FillPixMapGamma();
    
    m_pCurImageRef->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);
    m_pCurImageComp->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);
    m_pCurImageGamma3D->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);
    m_pCurImageGamma2D->SetCrosshairPosPhys(probePos2D_X, probePos2D_Y, curPlane);

    m_pCurImageRef->m_bDrawCrosshair = true;
    m_pCurImageComp->m_bDrawCrosshair = true;
    m_pCurImageGamma3D->m_bDrawCrosshair = true;
    m_pCurImageGamma2D->m_bDrawCrosshair = true;

    m_pCurImageRef->m_bDrawOverlayText = true;
    m_pCurImageComp->m_bDrawOverlayText = true;
    m_pCurImageGamma3D->m_bDrawOverlayText = true;
    m_pCurImageGamma2D->m_bDrawOverlayText = true;

    QString strValRef = QString::number(m_pCurImageRef->GetCrosshairOriginalData() * 100, 'f', 1) + " cGy";
    QString strValComp = QString::number(m_pCurImageComp->GetCrosshairOriginalData() * 100, 'f', 1) + " cGy";
    QString strValGamma3D = QString::number(m_pCurImageGamma3D->GetCrosshairOriginalData(), 'f', 2);
    QString strValGamma2D = QString::number(m_pCurImageGamma2D->GetCrosshairOriginalData(), 'f', 2);

    float fPercRef = m_pCurImageRef->GetCrosshairPercData();
    float fPercComp = m_pCurImageComp->GetCrosshairPercData();
    QString strPercRef = QString::number(fPercRef, 'f', 1) + "%";
    QString strPercComp = QString::number(fPercComp, 'f', 1) + "%";
    QString strDelta = QString::number(fPercComp - fPercRef, 'f', 1) + "%";   

    m_pCurImageRef->m_strOverlayText = strValRef + " [" + strPercRef + "]";
    m_pCurImageComp->m_strOverlayText = strValComp + " [" + strPercComp + "]" + ",          Delta= " + strDelta;
    m_pCurImageGamma3D->m_strOverlayText = strValGamma3D;
    m_pCurImageGamma2D->m_strOverlayText = strValGamma2D;

    ui.labelReferDose->SetBaseImage(m_pCurImageRef);
    ui.labelCompDose->SetBaseImage(m_pCurImageComp);
    ui.labelGammaMap3D->SetBaseImage(m_pCurImageGamma3D);
    ui.labelGammaMap2D->SetBaseImage(m_pCurImageGamma2D);
    //ui.labelRefDose
    //Set probe point in YKImage and crosshair display on
    //Update Table and Plot    
    //Display
    //label.update()
    ui.labelReferDose->update();
    ui.labelCompDose->update();
    ui.labelGammaMap3D->update();
    ui.labelGammaMap2D->update();

    //Update Table and Chart

    //1) prepare vector float
    vector<QPointF> vProfileRef, vProfileComp, vProfileGamma3D, vProfileGamma2D;
    enPROFILE_DIRECTON enDirection = PRIFLE_HOR;
    float fixedPosProfile;

    if (ui.radioButtonHor->isChecked())
    {
        enDirection = PRIFLE_HOR;
    }
    else if (ui.radioButtonVert->isChecked())
    {
        enDirection = PRIFLE_VER;
    }

    if (curPlane == PLANE_AXIAL)
    {
        if (enDirection == PRIFLE_HOR)        
            fixedPosProfile = probePosY;
        else        
            fixedPosProfile = probePosX;
        
    }
    else if (curPlane == PLANE_SAGITTAL)
    {
        if (enDirection == PRIFLE_HOR)
            fixedPosProfile = probePosZ;
        else
            fixedPosProfile = probePosY;
        
    }
    else if (curPlane == PLANE_FRONTAL)
    {
        if (enDirection == PRIFLE_HOR)
            fixedPosProfile = probePosZ;
        else
            fixedPosProfile = probePosX;
    }
   
    QUTIL::GetProfile1DByPosition(m_spCurRef2D, vProfileRef, fixedPosProfile, enDirection);
    QUTIL::GetProfile1DByPosition(m_spCurComp2D, vProfileComp, fixedPosProfile, enDirection);
    QUTIL::GetProfile1DByPosition(spCurGamma2DFrom3D, vProfileGamma3D, fixedPosProfile, enDirection);
    //QUTIL::GetProfile1DByPosition(spCurRef2D, vProfileGamma2D, fixedPosProfile, enDirection);
    //cout << "vectorSize= " << vProfileRef.size() << endl;
    
    //fNorm: Gy
    //float doseGyNormRef = 0.01 * (ui.sliderNormRef->value());
    //float doseGyNormComp = 0.01 *(ui.sliderNormComp->value());
    UpdateTable(vProfileRef, vProfileComp, vProfileGamma3D, doseGyNormRef, doseGyNormComp, 1.0, 100.0, 100.0, 1.0);

    SLT_DrawGraph(ui.checkBoxAutoAdjust->isChecked());

    if (m_pTableModel == NULL)
    {

        //cout << "TableModel is NULL" << endl;
    }

    if (m_pTableModel != NULL)
    {
        //cout << "table column " << m_pTableModel->columnCount() << endl;
        //cout << "table row " << m_pTableModel->rowCount() << endl;
        ui.tableViewProfile->setModel(m_pTableModel);
        ui.tableViewProfile->resizeColumnsToContents();
    }
    

    //Update LineEdit using rounded fixed value
    if (curPlane == PLANE_AXIAL)
    {
        //ui.lineEdit_ProbePosZ->setText(QString("%1").arg(finalPos1));//finalPos1 is for all images
        ui.lineEdit_ProbePosZ->setText(QString::number(finalPos1,'f', 1));
    }
    else if (curPlane == PLANE_SAGITTAL)
    {
        ui.lineEdit_ProbePosX->setText(QString::number(finalPos1, 'f', 1));
    }
    else if (curPlane == PLANE_FRONTAL)
    {
        ui.lineEdit_ProbePosY->setText(QString::number(finalPos1, 'f', 1));
    }

    ui.tableViewProfile->update();

    SLT_UpdateReportTxt();
 
}

void gamma_gui::SLT_UpdateReportTxt()
{
    QComboBox* crntCombo = ui.comboBoxCompareFile;
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    int iCnt = crntCombo->count();

    if (iCnt < 1)
        return;

    //Update plainTextEditGammaResult    
    ui.plainTextEditGammaResult->clear();
    if (m_strlistPath_Output_Report.count() == iCnt)
    {
        QString curPath = m_strlistPath_Output_Report.at(curIdx);
        QStringList strList = QUTIL::LoadTextFile(curPath.toLocal8Bit().constData());

        for (int i = 0; i < strList.count(); i++)
            ui.plainTextEditGammaResult->appendPlainText(strList.at(i));
    }

    QTextCursor txtCursor = ui.plainTextEditGammaResult->textCursor();
    //position where you want it
    txtCursor.setPosition(0);
    ui.plainTextEditGammaResult->setTextCursor(txtCursor);
}


void gamma_gui::SLT_UpdateProbePosRef()
{
    UpdateProbePos(ui.labelReferDose);
}

void gamma_gui::SLT_UpdateProbePosComp()
{
    UpdateProbePos(ui.labelCompDose);

}

void gamma_gui::SLT_UpdateProbePosGamma2D()
{
    UpdateProbePos(ui.labelGammaMap2D);
}

void gamma_gui::SLT_UpdateProbePosGamma3D()
{
    UpdateProbePos(ui.labelGammaMap3D);
}

void gamma_gui::UpdateProbePos(qyklabel* qlabel)
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


    //connect(ui.labelReferDose, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosRef())); //added
    //connect(ui.labelCompDose, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosComp())); //added
    //connect(ui.labelGammaMap2D, SIGNAL(Mouse_Pressed_Left()), this, SLOT(SLT_UpdateProbePosGamma2D())); //added
    //connect(ui.labelGammaMap3D, SIGNAL

    //Update Probe position
    // ui.lineEdit_ProbePosX

    enPLANE curPlane = PLANE_AXIAL;

    if (ui.radioButtonAxial->isChecked())
    {
        curPlane = PLANE_AXIAL;

        physPosX = crntDataPt.x()*spacingX + originX;
        physPosY = crntDataPt.y()*spacingY + originY;


        ui.lineEdit_ProbePosX->setText(QString::number(physPosX, 'f', 1));
        ui.lineEdit_ProbePosY->setText(QString::number(physPosY, 'f', 1));
                //ui.lineEdit_ProbePosZ
    }
    else if (ui.radioButtonSagittal->isChecked())
    {
        curPlane = PLANE_SAGITTAL;

        physPosX = crntDataPt.x()*spacingX + originX;
        physPosY = (iHeight - crntDataPt.y() - 1)*spacingY + originY;        

        ui.lineEdit_ProbePosY->setText(QString::number(physPosX, 'f', 1));
        ui.lineEdit_ProbePosZ->setText(QString::number(physPosY, 'f', 1));


    }
    else if (ui.radioButtonFrontal->isChecked())
    {
        curPlane = PLANE_FRONTAL;

        physPosX = crntDataPt.x()*spacingX + originX;
        physPosY = (iHeight - crntDataPt.y() - 1)*spacingY + originY;

        //ui.lineEdit_ProbePosX->setText(QString("%1").arg(physPosX));
        //ui.lineEdit_ProbePosZ->setText(QString("%1").arg(physPosY));

        ui.lineEdit_ProbePosX->setText(QString::number(physPosX, 'f', 1));
        ui.lineEdit_ProbePosZ->setText(QString::number(physPosY, 'f', 1));
    }   
  
    
    SLT_DrawAll();
}

void gamma_gui::SLT_DrawTable()
{
    

}

void gamma_gui::SLT_DrawChart()
{

}

void gamma_gui::UpdateTable(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3,
    float fNorm1, float fNorm2, float fNorm3, float fMag1, float fMag2, float fMag3)
{
    int numOfData = 3;

    if (m_pTableModel != NULL)
    {
        delete m_pTableModel;
        m_pTableModel = NULL;
    }

    int columnSize = 1;
    int rowSize1, rowSize2, rowSize3 = 0;

    columnSize = numOfData * 2;

    rowSize1 = vData1.size();
    rowSize2 = vData2.size();
    rowSize3 = vData3.size();

    int maxRowSize = 0;
    if (rowSize1 > rowSize2)
    {
        if (rowSize1 < rowSize3)
            maxRowSize = rowSize3;
        else
            maxRowSize = rowSize1;

    }
    else
    {
        if (rowSize2 < rowSize3)
            maxRowSize = rowSize3;
        else
            maxRowSize = rowSize2;
    }

    if (maxRowSize == 0)
    {
        //cout << "MaxRowSize is 0" << endl;
        return;
    }

    if (fNorm1 <= 0 || fNorm2 <= 0 || fNorm3 <= 0)
        return;

    m_pTableModel = new QStandardItemModel(maxRowSize, columnSize, this); //2 Rows and 3 Columns
    m_pTableModel->setHorizontalHeaderItem(0, new QStandardItem(QString("mm")));
    m_pTableModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Ref_cGy")));    
    m_pTableModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Com_cGy")));
    m_pTableModel->setHorizontalHeaderItem(3, new QStandardItem(QString("Ref_%")));
    m_pTableModel->setHorizontalHeaderItem(4, new QStandardItem(QString("Com_%")));    
    m_pTableModel->setHorizontalHeaderItem(5, new QStandardItem(QString("Gamma")));


    bool bData2Exists = false;
    bool bData3Exists = false;

    for (int i = 0; i < maxRowSize; i++)
    {
        if (i < rowSize2)
            bData2Exists = true;
        if (i < rowSize3)
            bData3Exists = true;

        qreal tmpValX1 = vData1.at(i).x();
        qreal tmpValY1 = vData1.at(i).y()*fMag1; //Gy --> cGy
        qreal tmpValY1_Perc = vData1.at(i).y() / fNorm1 * 100.0;

        QString strValX, strValY1, strValY2, strValY1_Perc, strValY2_Perc, strValY3;



        strValX = QString::number(tmpValX1, 'f', 1); //cGy
        strValY1 = QString::number(tmpValY1, 'f', 1); //cGy
        strValY1_Perc = QString::number(tmpValY1_Perc, 'f', 1); //%

        if (bData2Exists)
        {
            //qreal tmpValX2 = vData2.at(i).x();
            qreal tmpValY2 = vData2.at(i).y()*fMag2;
            qreal tmpValY2_Perc = vData2.at(i).y() / fNorm2 * 100.0; //fNorm : Gy not cGy            

            strValY2 = QString::number(tmpValY2, 'f', 1); //cGy        
            strValY2_Perc = QString::number(tmpValY2_Perc, 'f', 1); //%            
        }

        if (bData3Exists)
        {
            //qreal tmpValX3 = vData3.at(i).x();
            qreal tmpValY3 = vData3.at(i).y()*fMag3;
            //qreal tmpValY3_Perc = vData3.at(i).y() / fNorm3 * 100.0; //fNorm : Gy not cGy

            strValY3 = QString::number(tmpValY3, 'f', 2); //gamma
        }       

        m_pTableModel->setItem(i, 0, new QStandardItem(strValX));
        m_pTableModel->setItem(i, 1, new QStandardItem(strValY1));
        m_pTableModel->setItem(i, 2, new QStandardItem(strValY2));
        m_pTableModel->setItem(i, 3, new QStandardItem(strValY1_Perc));
        m_pTableModel->setItem(i, 4, new QStandardItem(strValY2_Perc));
        m_pTableModel->setItem(i, 5, new QStandardItem(strValY3));
    }

    ui.tableViewProfile->setModel(m_pTableModel);
    ui.tableViewProfile->resizeColumnsToContents();
}

void gamma_gui::SLT_CopyTableToClipboard()
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
    list << "Ref_cGy";
    list << "Com_cGy";
    list << "Ref_%";
    list << "Com_%";
    list << "Gamma";
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

void gamma_gui::SLT_DrawGraph(bool bInitMinMax)
{
    if (m_pTableModel == NULL)
        return;


    bool bNorm = ui.checkBoxNormalized->isChecked();// show percentage chart

    //Draw only horizontal, center

    QVector<double> vAxisX1; //can be rows or columns
    QVector<double> vAxisY1;

    QVector<double> vAxisX2; //can be rows or columns
    QVector<double> vAxisY2;

    QVector<double> vAxisX3; //can be rows or columns
    QVector<double> vAxisY3;

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
        QStandardItem* tableItem1 = m_pTableModel->item(i, 0);
        QStandardItem* tableItem2 = m_pTableModel->item(i, 1);
        QStandardItem* tableItem3 = m_pTableModel->item(i, 2);
        QStandardItem* tableItem4 = m_pTableModel->item(i, 3);
        QStandardItem* tableItem5 = m_pTableModel->item(i, 4);
        QStandardItem* tableItem6 = m_pTableModel->item(i, 5);

        double tableVal1 = tableItem1->text().toDouble();
        double tableVal2 = tableItem2->text().toDouble();
        double tableVal3 = tableItem3->text().toDouble();
        double tableVal4 = tableItem4->text().toDouble();
        double tableVal5 = tableItem5->text().toDouble();
        double tableVal6 = tableItem6->text().toDouble();

        if (minX > tableVal1)
            minX = tableVal1;
        if (maxX < tableVal1)
            maxX = tableVal1;

        if (minY > tableVal2)
            minY = tableVal2;
        if (maxY < tableVal2)
            maxY = tableVal2;

        

        if (bNorm) //%
        {            
            vAxisX1.push_back(tableVal1);
            vAxisY1.push_back(tableVal4);

            vAxisX2.push_back(tableVal1);
            vAxisY2.push_back(tableVal5);

            vAxisX3.push_back(tableVal1);
            vAxisY3.push_back(tableVal6);
        }
        else
        {
            vAxisX1.push_back(tableVal1);
            vAxisY1.push_back(tableVal2);

            vAxisX2.push_back(tableVal1);
            vAxisY2.push_back(tableVal3);

            vAxisX3.push_back(tableVal1);
            vAxisY3.push_back(tableVal6);
        }        
    }

    ui.customPlotProfile->addGraph();
    ui.customPlotProfile->graph(0)->setData(vAxisX1, vAxisY1);
    ui.customPlotProfile->graph(0)->setPen(QPen(Qt::blue));
    ui.customPlotProfile->graph(0)->setName("Ref. dose");

    ui.customPlotProfile->addGraph();
    ui.customPlotProfile->graph(1)->setData(vAxisX2, vAxisY2);
    ui.customPlotProfile->graph(1)->setPen(QPen(Qt::red));
    ui.customPlotProfile->graph(1)->setName("Compared dose");

    /* ui.customPlotProfile->addGraph();
     ui.customPlotProfile->graph(2)->setData(vAxisX3, vAxisY3);
     ui.customPlotProfile->graph(2)->setPen(QPen(Qt::green));
     ui.customPlotProfile->graph(2)->setName("Gamma");*/

    if (bInitMinMax)
    {
        //float marginX = 10;
        //float marginY = 100;
        ui.lineEditXMin->setText(QString("%1").arg(minX));
        ui.lineEditXMax->setText(QString("%1").arg(maxX));
        //ui.lineEditYMin->setText(QString("%1").arg(minY));
        //ui.lineEditYMax->setText(QString("%1").arg(maxY + marginY));
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

void gamma_gui::SLT_RunGamma2D()
{
    if (!m_spCurRef2D)
        return;
    if (!m_spCurComp2D)
        return;
    //Find export folder.
    if (m_strlistPath_RD_Read_Comp.empty())
        return;

    if (m_strlistFileBaseName_Comp.empty())
        return;

    QComboBox* crntCombo = ui.comboBoxCompareFile;
    int iCnt = crntCombo->count();
    if (iCnt < 1 || m_strlistFileBaseName_Comp.count() != iCnt)
        return;

    int curIdx = crntCombo->currentIndex(); //this should be basename       

    if (curIdx < 0)
        return;

    //QString strPathOutputRoot = m_strlistPath_RD_Comp.at(curIdx);
    ////QString strPathOutputRoot = m_strPathDirWorkDir;
    //QFileInfo fInfo(strPathOutputRoot);
    //QDir crntDir = fInfo.absolutePath();  

    QDir crntDir = QDir(m_strPathDirWorkDir);

    if (!crntDir.exists())
    {
        QUTIL::ShowErrorMessage("Error! working space is not ready");
    }

    //Get Current plane
    
    float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
    float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
    float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();    

    QString curBaseNameRef = m_strlistFileBaseName_Ref.at(curIdx);
    QString curBaseNameComp = m_strlistFileBaseName_Comp.at(curIdx);

    QString strPlane;
    if (ui.radioButtonAxial->isChecked())
    {
        strPlane = "AXL_Z" + QString::number(probePosZ, 'f', 1) + "mm";
        //fixedPos = probePosZ;
    }
    else if (ui.radioButtonSagittal->isChecked())
    {
        strPlane = "SAG_X" + QString::number(probePosX, 'f', 1) + "mm";
        //fixedPos = probePosX;
    }
    else if (ui.radioButtonFrontal->isChecked())
    {
        strPlane = "FRN_Y" + QString::number(probePosY, 'f', 1) + "mm";
        //fixedPos = probePosY;
    }

    QString subDirName = curBaseNameComp + "_" + strPlane;
    bool tmpResult = crntDir.mkdir(subDirName); //what if the directory exists?	

    if (!tmpResult)
        cout << "Warning! Directory for 2D gamma already exists. files will be overwritten." << endl;

    //QString strSavingFolder = crntDir.absolutePath() + "/" + subDirName;
    QString strSavingFolder = crntDir.absolutePath() + "/" + subDirName;

    QDir dirSaving(strSavingFolder);
    if (!dirSaving.exists())
    {
        cout << "Dir is not found:" << strSavingFolder.toLocal8Bit().constData() << endl;
        return;
    }

    /* From now on, the target folder is ready */

    QString tmpFilePathRef = strSavingFolder + "/" + "Gamma2DRef.mha";
    QString tmpFilePathComp = strSavingFolder + "/" + "Gamma2DComp.mha";    

    //Save current 2D
    m_spCurRef2D;
    m_spCurComp2D;

    QUTIL::SaveFloatImage2D(tmpFilePathRef.toLocal8Bit().constData(), m_spCurRef2D);
    QUTIL::SaveFloatImage2D(tmpFilePathComp.toLocal8Bit().constData(), m_spCurComp2D);        

    Gamma_parms parms;
    //Gamma param: should come from the UI

    parms.mask_image_fn = "";
    //parms->reference_dose;
    parms.gamma_max = 2.0;
    parms.b_compute_full_region = false;
    parms.b_resample_nn = false; //default: false
    parms.b_ref_only_threshold = false;

    //From File List
    parms.ref_image_fn = tmpFilePathRef.toLocal8Bit().constData();
    parms.cmp_image_fn = tmpFilePathComp.toLocal8Bit().constData();

    //From GUI
    if (ui.checkBox_inhereResample->isChecked())
        parms.f_inherent_resample_mm = ui.lineEdit_inhereResample->text().toDouble();
    else
        parms.f_inherent_resample_mm = -1.0;

    parms.b_interp_search = ui.checkBox_Interp_search->isChecked();

    if (ui.radioButton_localGamma->isChecked())
    {

        parms.b_local_gamma = true;        
    }
    else
    {

        parms.b_local_gamma = false;        
    }

    float inputRefDose = ui.lineEdit_refDoseInGy->text().toFloat();

    if (inputRefDose <= 0) //blank
    {
        parms.have_reference_dose = false;
    }
    else
    {
        parms.have_reference_dose = true;
        parms.reference_dose = inputRefDose;
    }

    parms.dta_tolerance = ui.lineEdit_dta_tol->text().toDouble();
    parms.dose_tolerance = ui.lineEdit_dose_tol->text().toDouble() / 100.0;//gui input: 3% --> param: 0.03
    parms.f_analysis_threshold = ui.lineEdit_cutoff_dose->text().toDouble() / 100.0;

    //Saving folder: comp folder. FileName Should Include dta, dose, local/global

    //QFileInfo fInfo = QFileInfo(tmpFilePathComp);
    //QString dirPath = fInfo.absolutePath();
    //QString baseName = fInfo.completeBaseName();

    QString strLocGlob;

    if (parms.b_local_gamma)
        strLocGlob = "loc";
    else
        strLocGlob = "glb";

    QString strSettingAbs = QString::number(parms.dta_tolerance, 'f', 0) + "mm_" + ""
        + QString::number(parms.dose_tolerance*100.0, 'f', 0) + "%_" + strLocGlob;

    //QString tmpFilePathComp = strSavingFolder + "/" + "Gamma2DComp.mha";

    QString strPathGamma2D = strSavingFolder + "/" + "gamma2D" + ".mha";
 //   if (ui.checkBox_gammamap_output->isChecked())
   // {        
        parms.out_image_fn = strPathGamma2D.toLocal8Bit().constData();     
   // }

    QString strPathFailmap2D = strSavingFolder + "/" + "fail2D" + ".mha";
    //if (ui.checkBox_failuremap_output->isChecked())
   // {
        parms.out_failmap_fn = strPathFailmap2D.toLocal8Bit().constData();
   // }

    QString strPathReport = strSavingFolder + "/" + "text_report" + ".txt";
    cout << "strPathReport= " << strPathReport.toLocal8Bit().constData() << endl;

    parms.out_report_fn = strPathReport.toLocal8Bit().constData();

    float refDoseGy;
    QString overallReport = GammaMain(&parms, refDoseGy); //report for a single case
    //Update GUI

    //Read gammap2D

    //m_spGamma2DResult;
    QUTIL::LoadFloatImage2D(strPathGamma2D.toLocal8Bit().constData(), m_spGamma2DResult);    

    //Update plainTextEditGammaResult
    QFileInfo reportInfo = QFileInfo(strPathReport);

    if (!reportInfo.exists())
    {
        cout << "Error! output text doesn't exist." << endl;
        return;
    }    


    //Update Report txt here
    ui.plainTextEditGammaResult2D->clear();
    QStringList strList = QUTIL::LoadTextFile(strPathReport.toLocal8Bit().constData());

    for (int i = 0; i < strList.count(); i++)
        ui.plainTextEditGammaResult2D->appendPlainText(strList.at(i));

    QTextCursor txtCursor = ui.plainTextEditGammaResult2D->textCursor();
    //position where you want it
    txtCursor.setPosition(0);
    ui.plainTextEditGammaResult2D->setTextCursor(txtCursor);


    //SaveIBA Image format

    
    QString IBAFilePathRef = strSavingFolder + "/" + strPlane + "_" + curBaseNameRef + ".OPG";
    QString IBAFilePathComp = strSavingFolder + "/" + strPlane + "_" + curBaseNameComp + ".OPG";
    
    SaveDoseIBAGenericTXTFromItk(IBAFilePathRef.toLocal8Bit().constData(), m_spCurRef2D);
    SaveDoseIBAGenericTXTFromItk(IBAFilePathComp.toLocal8Bit().constData(), m_spCurComp2D);
    
    SLT_RestoreZoomPan();
    SLT_DrawAll();
}

void gamma_gui::SLT_GoCenterPosRef()
{
    QComboBox* crntCombo = ui.comboBoxCompareFile;    
    int curIdx = crntCombo->currentIndex(); //this should be basename       

    if (curIdx < 0)
        return;

    if (m_vRefDoseImages.empty())
        return;

    FloatImageType::Pointer spCurFloat3D = m_vRefDoseImages.at(curIdx);

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

    SLT_DrawAll(); //triggered when Go Position button
}

void gamma_gui::SLT_GoCenterPosComp()
{
    QComboBox* crntCombo = ui.comboBoxCompareFile;
    int curIdx = crntCombo->currentIndex(); //this should be basename       

    if (curIdx < 0)
        return;

    if (m_vCompDoseImages.empty())
        return;

    FloatImageType::Pointer spCurFloat3D = m_vCompDoseImages.at(curIdx);

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

    SLT_DrawAll(); //triggered when Go Position button
}

void gamma_gui::SLT_NormCompFromRefNorm() //button
{
    QString crntNormComp = ui.lineEditNormComp->text();
    float crntNormF = crntNormComp.toFloat();

    if (crntNormF <= 0)
        return;

    //Get RefValue
    int iCurrRefNormVal = ui.sliderNormRef->value(); //cGy
    int iCurrCompNormVal = qRound(iCurrRefNormVal*crntNormF);

    ui.sliderNormComp->setValue(iCurrCompNormVal);
}


void gamma_gui::SaveDoseIBAGenericTXTFromItk(QString strFilePath, FloatImage2DType::Pointer& spFloatDose)
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

void gamma_gui::SLT_WhenSelectCombo()
{
    QComboBox* crntCombo = ui.comboBoxCompareFile;    
    int curIdx = crntCombo->currentIndex(); //this should be basename    
    int iCnt = crntCombo->count();
    if (iCnt < 1)
        return;


    if ((int)m_vRefDose.size() != iCnt ||
        (int)m_strlistPath_RD_Read_Ref.size() != iCnt ||
        (int)m_strlistPath_RD_Read_Comp.size() != iCnt ||
        (int)m_strlistPath_Output_Gammamap.size() != iCnt)
    {
        cout << "Error! SLT_WhenSelectCombo file count doesn't match!" << endl;

        cout << "crntComboCnt " << iCnt << endl;
        cout << "m_vRefDose " << m_vRefDose.size() << endl;
        cout << "m_strlistPath_RD_Read_Ref " << m_strlistPath_RD_Read_Ref.size() << endl;
        cout << "m_strlistPath_RD_Read_Comp " << m_strlistPath_RD_Read_Comp.size() << endl;
        cout << "m_strlistPath_Output_Gammamap " << m_strlistPath_Output_Gammamap.size() << endl;

        return;
    }
        
    if ((int)(m_vRefDoseImages.size()) != iCnt ||
        (int)(m_vCompDoseImages.size()) != iCnt ||
        (int)(m_vGammaMapImages.size()) != iCnt)
    {
        cout << "Error! ItkImage Pointer count doesn't match!" << endl;
        return;
    }


    

    disconnect(ui.sliderNormRef, SIGNAL(valueChanged(int)), this, SLOT(SLT_DrawAll()));
    disconnect(ui.sliderNormComp, SIGNAL(valueChanged(int)), this, SLOT(SLT_DrawAll()));

    ui.sliderNormRef->setValue(qRound(m_vRefDose.at(curIdx) * 100)); //Gy to cGy
    ui.sliderNormComp->setValue(qRound(m_vRefDose.at(curIdx) * 100)); //Gy to cGy

    connect(ui.sliderNormRef, SIGNAL(valueChanged(int)), this, SLOT(SLT_DrawAll()));
    connect(ui.sliderNormComp, SIGNAL(valueChanged(int)), this, SLOT(SLT_DrawAll()));


    QString strPath_ref = m_strlistPath_RD_Read_Ref.at(curIdx); //always readable mha format
    QString strPath_comp = m_strlistPath_RD_Read_Comp.at(curIdx);
    QString strPath_gamma = m_strlistPath_Output_Gammamap.at(curIdx);

    QFileInfo info_ref = QFileInfo(strPath_ref);
    QFileInfo info_comp = QFileInfo(strPath_comp); 

    bool bLowMem = ui.checkBox_low_mem->isChecked();

    if (bLowMem)
    {
        /*FloatImageType::Pointer spImgRef = m_vRefDoseImages.at(curIdx);
        FloatImageType::Pointer spImgComp = m_vCompDoseImages.at(curIdx);
        FloatImageType::Pointer spImgGamma = m_vGammaMapImages.at(curIdx);       */
        cout << "Loading image from file. Index= " << curIdx << endl;
        
        QUTIL::LoadFloatImage3D(strPath_ref.toLocal8Bit().constData(), m_vRefDoseImages.at(curIdx));
        QUTIL::LoadFloatImage3D(strPath_comp.toLocal8Bit().constData(), m_vCompDoseImages.at(curIdx));
        QUTIL::LoadFloatImage3D(strPath_gamma.toLocal8Bit().constData(), m_vGammaMapImages.at(curIdx));


        if (m_iLastLoadedIndex >= 0)
        {
            m_vRefDoseImages.at(m_iLastLoadedIndex) = m_spDummyLowMemImg;
            m_vCompDoseImages.at(m_iLastLoadedIndex) = m_spDummyLowMemImg;
            m_vGammaMapImages.at(m_iLastLoadedIndex) = m_spDummyLowMemImg;
        }

        m_iLastLoadedIndex = curIdx;
    }
    else
    {
        //do nothing
    }


  

    SLT_WhenChangePlane(); //RestorePanZoom + DrawAll
    //SLT_DrawAll();
}




void gamma_gui::SLT_MouseWheelUpdateRef()
{
    if (ui.labelReferDose->m_pYK16Image == NULL||
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
    {
        return;
    }

    QComboBox* crntCombo = ui.comboBoxCompareFile;
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    if (curIdx < 0)
        return;

    if (curIdx >= (int)(m_vRefDoseImages.size()))
        return;

    FloatImageType::Pointer spCurImg = m_vRefDoseImages.at(curIdx);

    VEC3D ptLimitStart = { 0.0, 0.0, 0.0 };
    VEC3D ptLimitEnd = { 0.0, 0.0, 0.0 };

    QUTIL::GetGeometricLimitFloatImg(spCurImg, ptLimitStart, ptLimitEnd);

    if (ui.checkBox_ScrollZoom->isChecked())
    {
        double oldZoom = ui.labelReferDose->m_pYK16Image->m_fZoom;
        double fWeighting = 0.2;
        float vZoomVal = oldZoom + ui.labelReferDose->m_iMouseWheelDelta * fWeighting;
        ui.labelReferDose->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelCompDose->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelGammaMap3D->m_pYK16Image->SetZoom(vZoomVal);

        if (ui.labelGammaMap2D->m_pYK16Image != NULL)
            ui.labelGammaMap2D->m_pYK16Image->SetZoom(vZoomVal);
    }
    else //change slice
    {
        double fWeighting = 1.0;
        enPLANE curPlane = PLANE_AXIAL;
        float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
        float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
        float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();

        if (ui.radioButtonAxial->isChecked())
        {
            curPlane = PLANE_AXIAL;
            probePosZ = probePosZ + ui.labelReferDose->m_iMouseWheelDelta*fWeighting;

            if (probePosZ <= ptLimitStart.z)
                probePosZ = ptLimitStart.z;

            if (probePosZ >= ptLimitEnd.z)
                probePosZ = ptLimitEnd.z;

            
            ui.lineEdit_ProbePosZ->setText(QString::number(probePosZ, 'f', 1));
        }
        else if (ui.radioButtonSagittal->isChecked())
        {
            curPlane = PLANE_SAGITTAL;
            probePosX = probePosX + ui.labelReferDose->m_iMouseWheelDelta*fWeighting;
            if (probePosX <= ptLimitStart.x)
                probePosX = ptLimitStart.x;

            if (probePosX >= ptLimitEnd.x)
                probePosX = ptLimitEnd.x;            

            ui.lineEdit_ProbePosX->setText(QString::number(probePosX, 'f', 1));          
        }
        else if (ui.radioButtonFrontal->isChecked())
        {
            curPlane = PLANE_FRONTAL;
            probePosY = probePosY + ui.labelReferDose->m_iMouseWheelDelta*fWeighting;
            if (probePosY <= ptLimitStart.y)
                probePosY = ptLimitStart.y;
            if (probePosY >= ptLimitEnd.y)
                probePosY = ptLimitEnd.y;       

            ui.lineEdit_ProbePosY->setText(QString::number(probePosY, 'f', 1));
        }        
    }

    SLT_DrawAll();
}

void gamma_gui::SLT_MouseWheelUpdateComp()
{
    if (ui.labelReferDose->m_pYK16Image == NULL ||
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
    {
        return;
    }

    QComboBox* crntCombo = ui.comboBoxCompareFile;
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    if (curIdx < 0)
        return;

    if (curIdx >= (int)(m_vRefDoseImages.size()))
        return;

    FloatImageType::Pointer spCurImg = m_vCompDoseImages.at(curIdx);

    VEC3D ptLimitStart = { 0.0, 0.0, 0.0 };
    VEC3D ptLimitEnd = { 0.0, 0.0, 0.0 };

    QUTIL::GetGeometricLimitFloatImg(spCurImg, ptLimitStart, ptLimitEnd);




    if (ui.checkBox_ScrollZoom->isChecked())
    {
        double oldZoom = ui.labelCompDose->m_pYK16Image->m_fZoom;
        double fWeighting = 0.2;
        float vZoomVal = oldZoom + ui.labelCompDose->m_iMouseWheelDelta * fWeighting;

        ui.labelReferDose->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelCompDose->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelGammaMap3D->m_pYK16Image->SetZoom(vZoomVal);

        if (ui.labelGammaMap2D->m_pYK16Image != NULL)
            ui.labelGammaMap2D->m_pYK16Image->SetZoom(vZoomVal);

    }
    else //change slice
    {
        double fWeighting = 1.0;
        enPLANE curPlane = PLANE_AXIAL;
        float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
        float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
        float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();

        if (ui.radioButtonAxial->isChecked())
        {
            curPlane = PLANE_AXIAL;
            probePosZ = probePosZ + ui.labelCompDose->m_iMouseWheelDelta*fWeighting;


            if (probePosZ <= ptLimitStart.z)
                probePosZ = ptLimitStart.z;

            if (probePosZ >= ptLimitEnd.z)
                probePosZ = ptLimitEnd.z;



            ui.lineEdit_ProbePosZ->setText(QString::number(probePosZ, 'f', 1));
        }
        else if (ui.radioButtonSagittal->isChecked())
        {
            curPlane = PLANE_SAGITTAL;
            probePosX = probePosX + ui.labelCompDose->m_iMouseWheelDelta*fWeighting;

            if (probePosX <= ptLimitStart.x)
                probePosX = ptLimitStart.x;

            if (probePosX >= ptLimitEnd.x)
                probePosX = ptLimitEnd.x;

            ui.lineEdit_ProbePosX->setText(QString::number(probePosX, 'f', 1));
        }
        else if (ui.radioButtonFrontal->isChecked())
        {
            curPlane = PLANE_FRONTAL;
            probePosY = probePosY + ui.labelCompDose->m_iMouseWheelDelta*fWeighting;

            if (probePosY <= ptLimitStart.y)
                probePosY = ptLimitStart.y;
            if (probePosY >= ptLimitEnd.y)
                probePosY = ptLimitEnd.y;

            ui.lineEdit_ProbePosY->setText(QString::number(probePosY, 'f', 1));
        }
    }

    SLT_DrawAll();

}

void gamma_gui::SLT_MouseWheelUpdateGamma2D()
{

}

void gamma_gui::SLT_MouseWheelUpdateGamma3D()
{
    if (ui.labelReferDose->m_pYK16Image == NULL ||
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
    {
        return;
    }

    QComboBox* crntCombo = ui.comboBoxCompareFile;
    int curIdx = crntCombo->currentIndex(); //this should be basename    

    if (curIdx < 0)
        return;

    if (curIdx >= (int)(m_vRefDoseImages.size()))
        return;

    FloatImageType::Pointer spCurImg = m_vGammaMapImages.at(curIdx);

    VEC3D ptLimitStart = { 0.0, 0.0, 0.0 };
    VEC3D ptLimitEnd = { 0.0, 0.0, 0.0 };

    QUTIL::GetGeometricLimitFloatImg(spCurImg, ptLimitStart, ptLimitEnd);

    if (ui.checkBox_ScrollZoom->isChecked())
    {
        double oldZoom = ui.labelGammaMap3D->m_pYK16Image->m_fZoom;
        double fWeighting = 0.2;
        float vZoomVal = oldZoom + ui.labelGammaMap3D->m_iMouseWheelDelta * fWeighting;

        ui.labelReferDose->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelCompDose->m_pYK16Image->SetZoom(vZoomVal);
        ui.labelGammaMap3D->m_pYK16Image->SetZoom(vZoomVal);

        if (ui.labelGammaMap2D->m_pYK16Image != NULL)
            ui.labelGammaMap2D->m_pYK16Image->SetZoom(vZoomVal);

    }
    else //change slice
    {
        double fWeighting = 1.0;
        enPLANE curPlane = PLANE_AXIAL;
        float probePosX = ui.lineEdit_ProbePosX->text().toFloat();
        float probePosY = ui.lineEdit_ProbePosY->text().toFloat();
        float probePosZ = ui.lineEdit_ProbePosZ->text().toFloat();

        if (ui.radioButtonAxial->isChecked())
        {
            curPlane = PLANE_AXIAL;
            probePosZ = probePosZ + ui.labelGammaMap3D->m_iMouseWheelDelta*fWeighting;

            if (probePosZ <= ptLimitStart.z)
                probePosZ = ptLimitStart.z;
            if (probePosZ >= ptLimitEnd.z)
                probePosZ = ptLimitEnd.z;

            ui.lineEdit_ProbePosZ->setText(QString::number(probePosZ, 'f', 1));
        }
        else if (ui.radioButtonSagittal->isChecked())
        {
            curPlane = PLANE_SAGITTAL;
            probePosX = probePosX + ui.labelGammaMap3D->m_iMouseWheelDelta*fWeighting;

            if (probePosX <= ptLimitStart.x)
                probePosX = ptLimitStart.x;
            if (probePosX >= ptLimitEnd.x)
                probePosX = ptLimitEnd.x;
            ui.lineEdit_ProbePosX->setText(QString::number(probePosX, 'f', 1));
        }
        else if (ui.radioButtonFrontal->isChecked())
        {
            curPlane = PLANE_FRONTAL;
            probePosY = probePosY + ui.labelGammaMap3D->m_iMouseWheelDelta*fWeighting;

            if (probePosY <= ptLimitStart.y)
                probePosY = ptLimitStart.y;
            if (probePosY >= ptLimitEnd.y)
                probePosY = ptLimitEnd.y;


            ui.lineEdit_ProbePosY->setText(QString::number(probePosY, 'f', 1));
        }
    }

    SLT_DrawAll();
}

void gamma_gui::SLT_MouseMoveUpdateRef()
{ 

}

void gamma_gui::SLT_MouseMoveUpdateComp()
{

}

void gamma_gui::SLT_MouseMoveUpdateGamma2D()
{

}

void gamma_gui::SLT_MouseMoveUpdateGamma3D()
{

}

void gamma_gui::SLT_RestoreZoomPan()
{
  

    if (ui.labelReferDose->m_pYK16Image == NULL ||
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
    {
        return;
    }

    ui.labelReferDose->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelReferDose->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    ui.labelCompDose->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelCompDose->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    ui.labelGammaMap3D->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelGammaMap3D->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    ui.labelGammaMap2D->setFixedWidth(DEFAULT_LABEL_WIDTH);
    ui.labelGammaMap2D->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    ui.labelReferDose->m_pYK16Image->SetZoom(1.0);
    ui.labelCompDose->m_pYK16Image->SetZoom(1.0);
    ui.labelGammaMap3D->m_pYK16Image->SetZoom(1.0);

    if (ui.labelGammaMap2D->m_pYK16Image != NULL)
        ui.labelGammaMap2D->m_pYK16Image->SetZoom(1.0);


    ui.labelReferDose->m_pYK16Image->SetOffset(0,0);
    ui.labelCompDose->m_pYK16Image->SetOffset(0, 0);
    ui.labelGammaMap3D->m_pYK16Image->SetOffset(0, 0);

    if (ui.labelGammaMap2D->m_pYK16Image != NULL)
        ui.labelGammaMap2D->m_pYK16Image->SetOffset(0, 0);
}

void gamma_gui::SLT_WhenChangePlane()
{
    ///*ui.labelReferDose->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelReferDose->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    //ui.labelCompDose->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelCompDose->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    //ui.labelGammaMap3D->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelGammaMap3D->setFixedHeight(DEFAULT_LABEL_HEIGHT);

    //ui.labelGammaMap2D->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //ui.labelGammaMap2D->setFixedHeight(DEFAULT_LABEL_HEIGHT);   */ 

    SLT_RestoreZoomPan();
    SLT_DrawAll();
}

void gamma_gui::SLT_MousePressedRightRef()
{    
    m_bMousePressedRightRef = true;
    WhenMousePressedRight(ui.labelReferDose);
}

void gamma_gui::WhenMousePressedRight(qyklabel* pWnd)
{
    if (pWnd->m_pYK16Image == NULL)
        return;

    m_ptPanStart.setX(pWnd->x);
    m_ptPanStart.setY(pWnd->y);

    m_ptOriginalDataOffset.setX(pWnd->m_pYK16Image->m_iOffsetX);
    m_ptOriginalDataOffset.setY(pWnd->m_pYK16Image->m_iOffsetY);
}


void gamma_gui::SLT_MousePressedRightComp()
{
    m_bMousePressedRightComp = true;
    WhenMousePressedRight(ui.labelCompDose);
}

void gamma_gui::SLT_MousePressedRightGamma3D()
{
    m_bMousePressedRightGamma3D = true;
    WhenMousePressedRight(ui.labelGammaMap3D);
}

void gamma_gui::SLT_MousePressedRightGamma2D()
{
    m_bMousePressedRightGamma2D = true;
    WhenMousePressedRight(ui.labelGammaMap2D);
}

void gamma_gui::SLT_MouseReleasedRightRef()
{
    m_bMousePressedRightRef = false;
}

void gamma_gui::SLT_MouseReleasedRightComp()
{
    m_bMousePressedRightComp = false;
}

void gamma_gui::SLT_MouseReleasedRightGamma3D()
{
    m_bMousePressedRightGamma3D = false;
}

void gamma_gui::SLT_MouseReleasedRightGamma2D()
{
    m_bMousePressedRightGamma2D = false;
}


void gamma_gui::UpdatePanCommon(qyklabel* qWnd)
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

void gamma_gui::SLT_UpdatePanSettingRef() //Mouse Move
{
    if (!m_bMousePressedRightRef)
        return;

    if (ui.labelReferDose->m_pYK16Image == NULL || 
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
        return;

    UpdatePanCommon(ui.labelReferDose);
    //Sync offset
    int offsetX = ui.labelReferDose->m_pYK16Image->m_iOffsetX;
    int offsetY = ui.labelReferDose->m_pYK16Image->m_iOffsetY;

    ui.labelCompDose->m_pYK16Image->SetOffset(offsetX, offsetY);
    ui.labelGammaMap3D->m_pYK16Image->SetOffset(offsetX, offsetY);

    if (ui.labelGammaMap2D->m_pYK16Image != NULL)
        ui.labelGammaMap2D->m_pYK16Image->SetOffset(offsetX, offsetY);    

    SLT_DrawAll();
}


void gamma_gui::SLT_UpdatePanSettingComp()
{
    if (!m_bMousePressedRightComp)
        return;

    if (ui.labelReferDose->m_pYK16Image == NULL ||
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
        return;

    UpdatePanCommon(ui.labelCompDose);
    //Sync offset
    int offsetX = ui.labelCompDose->m_pYK16Image->m_iOffsetX;
    int offsetY = ui.labelCompDose->m_pYK16Image->m_iOffsetY;

    ui.labelReferDose->m_pYK16Image->SetOffset(offsetX, offsetY);
    ui.labelGammaMap3D->m_pYK16Image->SetOffset(offsetX, offsetY);

    if (ui.labelGammaMap2D->m_pYK16Image != NULL)
        ui.labelGammaMap2D->m_pYK16Image->SetOffset(offsetX, offsetY);

    SLT_DrawAll();
}

void gamma_gui::SLT_UpdatePanSettingGamma3D()
{
    if (!m_bMousePressedRightGamma3D)
        return;

    if (ui.labelReferDose->m_pYK16Image == NULL ||
        ui.labelCompDose->m_pYK16Image == NULL ||
        ui.labelGammaMap3D->m_pYK16Image == NULL)
        return;

    UpdatePanCommon(ui.labelGammaMap3D);
    //Sync offset
    int offsetX = ui.labelGammaMap3D->m_pYK16Image->m_iOffsetX;
    int offsetY = ui.labelGammaMap3D->m_pYK16Image->m_iOffsetY;

    ui.labelReferDose->m_pYK16Image->SetOffset(offsetX, offsetY);
    ui.labelCompDose->m_pYK16Image->SetOffset(offsetX, offsetY);

    if (ui.labelGammaMap2D->m_pYK16Image != NULL)
        ui.labelGammaMap2D->m_pYK16Image->SetOffset(offsetX, offsetY);

    SLT_DrawAll();

}

void gamma_gui::SLT_UpdatePanSettingGamma2D()
{
    return;
}

void gamma_gui::SLTM_RenameRDFilesByDICOMInfo()
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


void gamma_gui::RenameFileByDCMInfo(QStringList& filenameList)
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

void gamma_gui::SaveCurrentGammaWorkSpace(QString& strPathGammaWorkSpace)
{
    ofstream fout;
    fout.open(strPathGammaWorkSpace.toLocal8Bit().constData());

    fout << "#GAMMA_WORK_SPACE_FILE V1.0#" << endl;

    int cntItem1 = m_strlistPath_RD_Read_Ref.size();
    int cntItem2 = m_strlistPath_RD_Read_Comp.size();
    int cntItem3 = m_strlistFileBaseName_Ref.size();
    int cntItem4 = m_strlistFileBaseName_Comp.size();

    //int cntItem5 = m_strlistBatchReport.size();

    int cntItem5 = m_strlistPath_Output_Gammamap.size();
    int cntItem6 = m_strlistPath_Output_Failure.size();
    int cntItem7 = m_strlistPath_Output_Report.size();
    int cntItem8 = m_vRefDose.size();

    if (cntItem1 < 1 ||
        cntItem1 != cntItem2 ||
        cntItem1 != cntItem3 ||
        cntItem1 != cntItem4 ||
        cntItem1 != cntItem5 ||
        cntItem1 != cntItem6 ||
        cntItem1 != cntItem7 ||
        cntItem1 != cntItem8
        )
    {
        cout << "Error! number of item doesn't match" << endl;
        return;
    }
    
    for (int i = 0; i < cntItem1; i++)
    {
        fout << endl;
        fout << "ITEM_ID" << "\t" << i << endl;
        fout << "RD_REF" << "\t" << m_strlistPath_RD_Read_Ref.at(i).toLocal8Bit().constData() << endl;
        fout << "RD_COMP" << "\t" << m_strlistPath_RD_Read_Comp.at(i).toLocal8Bit().constData() << endl;
        fout << "GAMMA_MAP" << "\t" << m_strlistPath_Output_Gammamap.at(i).toLocal8Bit().constData() << endl;
        fout << "BASE_REF" << "\t" << m_strlistFileBaseName_Ref.at(i).toLocal8Bit().constData() << endl;
        fout << "BASE_COMP" << "\t" << m_strlistFileBaseName_Comp.at(i).toLocal8Bit().constData() << endl;
        fout << "GAMMA_FAILURE" << "\t" << m_strlistPath_Output_Failure.at(i).toLocal8Bit().constData() << endl;
        fout << "GAMMA_REPORT" << "\t" << m_strlistPath_Output_Report.at(i).toLocal8Bit().constData() << endl;
        fout << "REF_DOSE[Gy]" << "\t" << m_vRefDose.at(i) << endl;
        fout << endl;
    }

    int numOfLinesInSimpleReport = m_strlistBatchReport.count();

    fout << "#BATCH_SIMPLE_REPORT" << endl;
    for (int i = 0; i < numOfLinesInSimpleReport; i++)
    {
        fout << m_strlistBatchReport.at(i).toLocal8Bit().constData() << endl;
    }

    fout.close();
}


bool gamma_gui::LoadGammaWorkSpace(QString& strPathGammaWorkSpace)
{
    ifstream fin;
    fin.open(strPathGammaWorkSpace.toLocal8Bit().constData());

    if (fin.fail())
    {
        cout << "Error occurred in file reading!" << endl;
        return false;
    }

    //clear all the data
    m_strlistPath_RD_Read_Ref.clear();
    m_strlistPath_RD_Read_Comp.clear();

    m_strlistPath_Output_Gammamap.clear();

    m_strlistFileBaseName_Ref.clear();
    m_strlistFileBaseName_Comp.clear();
    
    m_strlistPath_Output_Failure.clear();
    m_strlistPath_Output_Report.clear();
    m_vRefDose.clear();

    m_strlistBatchReport.clear();

    ////This will be also done in loading to mem
    //m_vRefDoseImages.clear();
    //m_vCompDoseImages.clear();
    //m_vGammaMapImages.clear();

    char str[MAX_LINE_LENGTH];

    while (!fin.eof())
    {
        memset(str, 0, MAX_LINE_LENGTH);
        fin.getline(str, MAX_LINE_LENGTH);
        QString tmpStr = QString(str);
        QString trimmed;
        if (tmpStr.contains("ITEM_ID")) //go into subloop
        {
            while (!fin.eof())
            {
                memset(str, 0, MAX_LINE_LENGTH);
                fin.getline(str, MAX_LINE_LENGTH);
                tmpStr = QString(str);

                trimmed = tmpStr.trimmed(); //if any blank line found
                if (trimmed.length() < 1)
                    break;
                
                //parsing
                QStringList strListTmp = tmpStr.split("\t");

                if (tmpStr.contains("RD_REF") && strListTmp.count() == 2) //go into subloop                
                    m_strlistPath_RD_Read_Ref.push_back(strListTmp.at(1));                
                else if (tmpStr.contains("RD_COMP") && strListTmp.count() == 2) //go into subloop                
                    m_strlistPath_RD_Read_Comp.push_back(strListTmp.at(1));
                else if (tmpStr.contains("GAMMA_MAP") && strListTmp.count() == 2) //go into subloop                
                    m_strlistPath_Output_Gammamap.push_back(strListTmp.at(1));
                else if (tmpStr.contains("BASE_REF") && strListTmp.count() == 2) //go into subloop                
                    m_strlistFileBaseName_Ref.push_back(strListTmp.at(1));
                else if (tmpStr.contains("BASE_COMP") && strListTmp.count() == 2) //go into subloop                
                    m_strlistFileBaseName_Comp.push_back(strListTmp.at(1));
                else if (tmpStr.contains("GAMMA_FAILURE") && strListTmp.count() == 2) //go into subloop                
                    m_strlistPath_Output_Failure.push_back(strListTmp.at(1));
                else if (tmpStr.contains("GAMMA_REPORT") && strListTmp.count() == 2) //go into subloop                
                    m_strlistPath_Output_Report.push_back(strListTmp.at(1));
                else if (tmpStr.contains("REF_DOSE[Gy]") && strListTmp.count() == 2) //go into subloop                
                    m_vRefDose.push_back(strListTmp.at(1).toFloat());
            }
        }//end of ITEM

        if (tmpStr.contains("BATCH_SIMPLE_REPORT"))
        {
            while (!fin.eof())
            {
                memset(str, 0, MAX_LINE_LENGTH);
                fin.getline(str, MAX_LINE_LENGTH);
                tmpStr = QString(str);

                trimmed = tmpStr.trimmed(); //if any blank line found
                if (trimmed.length() < 1)
                    continue;

                m_strlistBatchReport.push_back(tmpStr);
            }
        }
    }
  

    //ui.plainTextEdit_RD_Comp->clear();
    //m_strlistPath_RD_Comp.clear();
    //m_strlistFileBaseName_Comp.clear();

    //m_strlistPath_RD_Comp = tmpList;

    //for (int i = 0; i < iFileCnt; i++)
    //{
    //    ui.plainTextEdit_RD_Comp->appendPlainText(m_strlistPath_RD_Comp.at(i)); //just for display

    //    QFileInfo tmpInfo = QFileInfo(m_strlistPath_RD_Comp.at(i));

    //    m_strlistFileBaseName_Comp.push_back(tmpInfo.completeBaseName());
    //}


    //cout << "File names are successfully loaded" << endl;
    fin.close();

    return true;
}

void gamma_gui::SLTM_LoadSavedWorkSpace()
{
    QString strPath = QFileDialog::getOpenFileName(this, "Select a gamma workspace file to open", m_strPathDirWorkDir, "AccuGamma workspace file (*.gws)");

    if (strPath.length() <= 1)
        return;

    if (!LoadGammaWorkSpace(strPath)) //Only reading text files to fill up the vectors, without any auditing
    {
        QUTIL::ShowErrorMessage("Error! failed in loading workspace file");
        return;
    }       

    QFileInfo fileInfo(strPath);
    QDir dir = fileInfo.absoluteDir();
    SetWorkDir(dir.absolutePath());

    bool bOK = true;

    QString strWorkDirPath = dir.absolutePath();

    //1) Update the previous fileList and convert them according to current wo
    
    //rkspace
    //Get current workspace directoroy

    //QString ReplaceUpperDirOnly(QString& strOriginalPath, QString& strCurrDirPath, QString& strDelim);

    int cntItem0 = m_strlistPath_RD_Read_Ref.count();
    int cntItem1 = m_strlistPath_RD_Read_Comp.count();
    int cntItem2 = m_strlistPath_Output_Gammamap.count();
    int cntItem3 = m_strlistPath_Output_Report.count();

    int cntItem4 = m_strlistPath_Output_Failure.count(); //not mandatory


    if (cntItem0 < 1 ||
        cntItem0 != cntItem1 ||
        cntItem0 != cntItem2 ||
        cntItem0 != cntItem3
        )
    {
        QUTIL::ShowErrorMessage("Error! Number of read files are not matching each other!");
        cout << "RD_ref= " << cntItem0 << endl;
        cout << "RD_comp= " << cntItem1 << endl;
        cout << "Gamma= " << cntItem2 << endl;
        cout << "TxtReport= " << cntItem3 << endl;        
        cout << "Failure= " << cntItem4 << endl;
        bOK = false;
    }

    int cnt = cntItem0;

    if (!bOK)
    {
        SetWorkDir(QString(""));//roll back
        return;
    }

    QString strDelimRef = "DoseRef_";
    QString strDelimComp = "DoseComp_";
    QString strDelimGamma = "Analysis_";
    QString strDelimText = "Analysis_";
    QString strDelimFail = "Analysis_";

    QString originalPath;
    QString newPath;
    //Update file Path
    for (int i = 0; i < cnt; i++)
    {
        originalPath = m_strlistPath_RD_Read_Ref.at(i);
        newPath = ReplaceUpperDirOnly(originalPath, strWorkDirPath, strDelimRef);
        m_strlistPath_RD_Read_Ref.replace(i, newPath);

        originalPath = m_strlistPath_RD_Read_Comp.at(i);
        newPath = ReplaceUpperDirOnly(originalPath, strWorkDirPath, strDelimComp);
        m_strlistPath_RD_Read_Comp.replace(i, newPath);

        originalPath = m_strlistPath_Output_Gammamap.at(i);
        newPath = ReplaceUpperDirOnly(originalPath, strWorkDirPath, strDelimGamma);
        m_strlistPath_Output_Gammamap.replace(i, newPath);

        originalPath = m_strlistPath_Output_Report.at(i);
        newPath = ReplaceUpperDirOnly(originalPath, strWorkDirPath, strDelimText);
        m_strlistPath_Output_Report.replace(i, newPath);

        originalPath = m_strlistPath_Output_Failure.at(i);
        newPath = ReplaceUpperDirOnly(originalPath, strWorkDirPath, strDelimFail);
        m_strlistPath_Output_Failure.replace(i, newPath);
    }

    SLT_LoadResults();
}

QString gamma_gui::ReplaceUpperDirOnly(QString& strOriginalPath, QString& strCurrDirPath, QString& strDelim)
{
    QStringList strListPath = strOriginalPath.split("/");
    //if (strListPath.count() < 1)
      //  strListPath = strOriginalPath.split("/");
    int cnt = strListPath.count();
    
    int idx = 0;

    int cntSecondary = 0;
    for (int i = 0; i < cnt; i++)
    {
        if (strListPath.at(i).contains(strDelim))
        {
            idx = i;
            cntSecondary++;
        }            
    }

    if (cntSecondary > 1)
    {
        cout << "ERROR! strDelim should be shown once" << endl;
        return strOriginalPath;
    }     

    QString strLower;
    for (int i = idx; i < cnt; i++)
    {
        strLower = strLower + "/" + strListPath.at(i);
    }

    QString newPath = strCurrDirPath + strLower;
    return newPath;
}

void gamma_gui::SLT_SetWorkDir()
{
    //default: current one m_strPathDirWorkDir
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Open Gamma Work Directory"),
        m_strPathDirWorkDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    QDir dirIMAGES = QDir(dirPath);
    if (!dirIMAGES.exists())
        return;

    SetWorkDir(dirPath);
}

void gamma_gui::SetWorkDir(const QString& strPath)
{
    m_strPathDirWorkDir = strPath;
    ui.lineEdit_WorkDir->setText(m_strPathDirWorkDir);
}

void gamma_gui::SLTM_ExportBatchReport()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save batch report file", m_strPathDirWorkDir, "report (*.txt)", 0, 0);

    if (filePath.length() < 1)
        return;


    QFileInfo fInfo(filePath);

    if (fInfo.suffix() != "txt" && fInfo.suffix() != "TXT")
    {
        cout << "Saving filter didn't work. Saving this file as txt." << endl;
        filePath = filePath + ".txt";
    }

    SaveBatchGamma3DSimpleReport(filePath);
}

void gamma_gui::SLTM_LoadProtonDoseSetFile()
{   
    QStringList tmpList = QFileDialog::getOpenFileNames(this, "Select proton dose set file to open", m_strPathDirWorkDir, "Proton dose set file (*.set)");

    int iFileCnt = tmpList.size();
    if (iFileCnt < 1)
        return;

    vector<ProtonSetFileMGH> vProtonSetHeader;

    for (int i = 0; i < iFileCnt; i++)
    {
        QString curTextSetFilePath = tmpList.at(i);
        
        ProtonSetFileMGH tmpProtonSetHeader;
        //Read text and fill the class with as it is.
        if (!ReadProtonDoseSet(curTextSetFilePath, tmpProtonSetHeader)) //Only reading text files to fill up the vectors, without any auditing
        {
            //do nothing            
        }

        cout << "FilePathRef= " << tmpProtonSetHeader.strPathRefDose.toLocal8Bit().constData() << endl;
        cout << "FilePathComp= " << tmpProtonSetHeader.strPathCompDose.toLocal8Bit().constData() << endl;

        QFileInfo fInfoRef(tmpProtonSetHeader.strPathRefDose);
        QFileInfo fInfoComp(tmpProtonSetHeader.strPathCompDose);

        if (fInfoRef.exists() && fInfoComp.exists()) //both files should exists.
        {
            vProtonSetHeader.push_back(tmpProtonSetHeader);
        }        
    }

    if (vProtonSetHeader.empty())
    {
        cout << "Error. No file was found" << endl;
        return;
    }


    //Ref clear
    ui.plainTextEdit_RD_Ref->clear();
    m_strlistPath_RD_Original_Ref.clear();
    m_strlistFileBaseName_Ref.clear();

    //Comp Clear
    ui.plainTextEdit_RD_Comp->clear();    
    m_strlistPath_RD_Original_Comp.clear();    
    m_strlistFileBaseName_Comp.clear();

    //output clear
    m_strlistPath_Output_Gammamap.clear();    
    m_strlistPath_Output_Failure.clear();
    m_strlistPath_Output_Report.clear();
    m_strlistBatchReport.clear();

    //Ref dose clear;
    m_vRefDose.clear();   

    //Conversion
    vector<ProtonSetFileMGH>::iterator it;
    for (it = vProtonSetHeader.begin(); it != vProtonSetHeader.end(); ++it)
    {
        ProtonSetFileMGH tempDoseSet = (*it);

        QString strPathMhaRef = ConvertMGHProtonDoseToMha(tempDoseSet.strPathRefDose, tempDoseSet.fDim, tempDoseSet.fOrigin, tempDoseSet.fSpacing);
        QString strPathMhaComp = ConvertMGHProtonDoseToMha(tempDoseSet.strPathCompDose, tempDoseSet.fDim, tempDoseSet.fOrigin, tempDoseSet.fSpacing);

        QFileInfo fInfoMhaRef(strPathMhaRef);
        QFileInfo fInfoMhaComp(strPathMhaComp);

        if (fInfoMhaRef.exists())
        {            
            m_strlistPath_RD_Original_Ref.push_back(strPathMhaRef);            
            m_strlistFileBaseName_Ref.push_back(fInfoMhaRef.completeBaseName());
            ui.plainTextEdit_RD_Ref->appendPlainText(strPathMhaRef); //just for display            
        }        

        if (fInfoMhaComp.exists())
        {
            m_strlistPath_RD_Original_Comp.push_back(strPathMhaComp);            
            m_strlistFileBaseName_Comp.push_back(fInfoMhaComp.completeBaseName());
            ui.plainTextEdit_RD_Comp->appendPlainText(strPathMhaComp); //just for display            
        }
    }          

    if (m_strlistPath_RD_Original_Ref.size() > 0)
    {
        QFileInfo finfo(m_strlistPath_RD_Original_Ref.at(0));
        QDir crntDir = finfo.absoluteDir();
        m_strPathInputDir = crntDir.absolutePath();

        SetWorkDir(m_strPathInputDir); //optional

        //Change the plane to Frontal
        ui.radioButtonFrontal->setChecked(true);
    }     
}

bool gamma_gui::ReadProtonDoseSet(QString& strPathProtonDoseSet, ProtonSetFileMGH& protonSetInfo) //strPathProtonDoseSet text file
{
    //protonSetInfo    

    /*protonSetInfo.fDim = { 1.0, 1.0, 1.0 };
    protonSetInfo.fOrigin = { 0.0, 0.0, 0.0 };
    protonSetInfo.fSpacing = { 1.0, 1.0, 1.0 };*/

    protonSetInfo.fDim.x = 1.0;
    protonSetInfo.fDim.y = 1.0;
    protonSetInfo.fDim.z = 1.0;

    protonSetInfo.fOrigin.x = 0.0;
    protonSetInfo.fOrigin.y = 0.0;
    protonSetInfo.fOrigin.z = 0.0;

    protonSetInfo.fSpacing.x = 1.0;
    protonSetInfo.fSpacing.y = 1.0;
    protonSetInfo.fSpacing.z = 1.0;

    protonSetInfo.strCTDir = "";
    protonSetInfo.strPathRefDose = "";
    protonSetInfo.strPathCompDose = "";


    QFileInfo fInfoBase(strPathProtonDoseSet);

    ifstream fin;
    fin.open(strPathProtonDoseSet.toLocal8Bit().constData());

    if (fin.fail())
    {
        cout << "Error occurred in file reading!" << endl;
        return false;
    } 

    char str[MAX_LINE_LENGTH];

    while (!fin.eof())
    {
        memset(str, 0, MAX_LINE_LENGTH);
        fin.getline(str, MAX_LINE_LENGTH);

        QString tmpStrLine = QString(str);
        QStringList strListData = tmpStrLine.split(" \"");

        QString strHeader, strData;
        if (strListData.count() == 2)
        {
            strHeader = strListData.at(0);
            strData = strListData.at(1);
            strData = strData.remove("\"");
        }        
        strData = strData.trimmed(); //very important for Linux system!!!!!



        if (strHeader.contains("calc-vol"))
        {           

            QStringList strlistVolInfo = strData.split(",");
            if (strlistVolInfo.count() == 9) //should be
            {
                //First 3 digits: size in mm. X (Lat), Y(SI), Z(AntPost), IEC
                VEC3D fLength, fOrigin, fDim, fSpacing;

                fLength.x = strlistVolInfo.at(0).toDouble();
                fLength.y = strlistVolInfo.at(1).toDouble();
                fLength.z = strlistVolInfo.at(2).toDouble();

                fOrigin.x = strlistVolInfo.at(3).toDouble();
                fOrigin.y = strlistVolInfo.at(4).toDouble();
                fOrigin.z = strlistVolInfo.at(5).toDouble();

                fDim.x = strlistVolInfo.at(6).toDouble();
                fDim.y = strlistVolInfo.at(7).toDouble();
                fDim.z = strlistVolInfo.at(8).toDouble();

                if (fDim.x != 0 && fDim.y != 0 && fDim.z != 0)
                {
                    fSpacing.x = fLength.x / fDim.x;
                    fSpacing.y = fLength.y / fDim.y;
                    fSpacing.z = fLength.z / fDim.z;
                }
                else
                {
                    fSpacing.x = 1.0;
                    fSpacing.y = 1.0;
                    fSpacing.z = 1.0;                    
                }

                protonSetInfo.fDim = fDim;
                protonSetInfo.fOrigin = fOrigin;
                protonSetInfo.fSpacing = fSpacing;
            }
        }
        else if (strHeader.contains("ct-dir"))
        {
            protonSetInfo.strCTDir = strData;
        }
        else if (strHeader.trimmed() == "dose")
        {            
            strData = strData + ".MC";                        
            protonSetInfo.strPathCompDose = fInfoBase.absolutePath() + "/" +strData;
        }
        else if (strHeader.trimmed() == "ref-dose")
        {
            strData = strData + ".orig";            
            protonSetInfo.strPathRefDose = fInfoBase.absolutePath() + "/" + strData;
        }
    }
    fin.close();

    return true;
}

QString gamma_gui::ConvertMGHProtonDoseToMha(QString& strPathBinary, VEC3D& fDim, VEC3D& fOrigin, VEC3D& fSpacing)
{
    QString strResult = "";
    ///Create a mha file

    QFileInfo fInfo(strPathBinary);

    if (!fInfo.exists())
    {
        cout << "Cannot find the binary file: " << strPathBinary.toLocal8Bit().constData() << endl;
        return strResult;
    }

    QFile binFile(strPathBinary);
    binFile.open(QIODevice::ReadOnly);
    QDataStream in(&binFile);    
    in.setByteOrder(QDataStream::BigEndian);

    //qint32 voxData;
    quint32 voxData;

    //Coordinate conversion: IEC --> DICOM --> not possible!
    //fDim = IEC
    int width_DCM = (int)fDim.x; //
    int height_DCM = (int)fDim.z; //
    int length_DCM = (int)fDim.y; //

    //Create a float itk image 3D
    FloatImageType::Pointer spItkFloat = FloatImageType::New();

    FloatImageType::SizeType size;
    size[0] = width_DCM;
    size[1] = height_DCM;
    size[2] = length_DCM;

    FloatImageType::IndexType idxStart;
    idxStart[0] = 0;
    idxStart[1] = 0;
    idxStart[2] = 0;

    FloatImageType::SpacingType spacing;    
    spacing[0] = fSpacing.x;
    spacing[1] = fSpacing.z;
    spacing[2] = fSpacing.y;    

    FloatImageType::PointType origin;
    origin[0] = fOrigin.x;
    origin[1] = -fOrigin.z;
    origin[2] = fOrigin.y;

    FloatImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(idxStart);

    spItkFloat->SetRegions(region);
    spItkFloat->SetSpacing(spacing);
    spItkFloat->SetOrigin(origin);
    spItkFloat->Allocate();

    /*ofstream fout;
    fout.open("D:/testDoseOut.txt");*/

    //itk::ImageRegionIterator<FloatImageType> it(spItkFloat, spItkFloat->GetLargestPossibleRegion());
    itk::ImageSliceIteratorWithIndex<FloatImageType> it(spItkFloat, spItkFloat->GetLargestPossibleRegion());
    it.SetFirstDirection(0); //dicom x
    it.SetSecondDirection(2); //dicom z. -2 doesn't work

    it.GoToBegin();          
    
    //for (int k = 3; k < 4; k++)
    float doseGyVal = 0.0;
    for (int k = 0; k < height_DCM && !it.IsAtEnd(); k++)
    {
        for (int i = 0; i < length_DCM && !it.IsAtEndOfSlice(); i++) //coronal plane
        {
            for (int j = 0; j < width_DCM && !it.IsAtEndOfLine(); j++)
            {
                //converted mha file: Gy, float //current file: cGy, uint
                in >> voxData;
                doseGyVal = voxData / 100.0; //cGy to Gy
                it.Set(doseGyVal);
                ++it;                                            
                //    fout << voxData << ",";
            }
            it.NextLine();
          //  fout << endl;
        }
        it.NextSlice();
        //fout << "Plane=" << k << endl;
    }
    binFile.close();

    typedef itk::FlipImageFilter< FloatImageType >  FilterType;
    FilterType::Pointer flipFilter = FilterType::New();
    typedef FilterType::FlipAxesArrayType FlipAxesArrayType;

    FlipAxesArrayType arrFlipAxes;
    arrFlipAxes[0] = 0;
    arrFlipAxes[1] = 0;
    arrFlipAxes[2] = 1;//SI flip

    flipFilter->SetFlipAxes(arrFlipAxes);
    flipFilter->SetInput(spItkFloat); //plan CT, USHORT image
    flipFilter->Update();

    spItkFloat = flipFilter->GetOutput();
    spItkFloat->SetOrigin(origin); //origin was reverted after the flipping
    

    QString strDirPath = fInfo.absolutePath();
    QString strFileName = fInfo.fileName() + ".mha";
    strResult = strDirPath + "/" + strFileName;

    typedef itk::ImageFileWriter<FloatImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(strResult.toLocal8Bit().constData());
    writer->SetUseCompression(true); //not exist in original code (rtkfdk)	
    writer->SetInput(spItkFloat);
    writer->Update();

    //cout << "Writing image file was succeeded: " << strPath.toLocal8Bit().constData() << endl;
    //fout.close();

    QFileInfo fInfoOut(strResult);

    if (!fInfoOut.exists())
    {
        strResult = "";
        cout << "Error!File conversion failed" << endl;
    }
    else
        cout << "Successfully converted to a mha file:" << strResult.toLocal8Bit().constData() <<endl;
     
    
    
    return strResult;
}

void gamma_gui::SLTM_ConvertIBAOPG_Files()
{
	QStringList files = QFileDialog::getOpenFileNames(this, "Select one or more files to open",
		m_strPathInputDir, "IBA OPG files (*.opg)");

	int cnt = files.size();
	if (cnt <= 0)
		return;

	QString strPathOPG;
	QString strPathMHA;
	int filePathLen;
	for (int i = 0; i < cnt; i++)
	{
		strPathOPG = files.at(i);
		filePathLen = strPathOPG.length();
		//replace extention
		strPathMHA = strPathOPG.left(filePathLen - 4);
		strPathMHA = strPathMHA + ".mha";
		ConvertOPG2FloatMHA(strPathOPG, strPathMHA);
	}
}


//From IBA generic Txt (OPG) to float mha
bool gamma_gui::ConvertOPG2FloatMHA(QString& strFilePathOPG, QString& strFilePathMHA)
{
	//FloatImage2DType::Pointer spFloatDose;       


	//Data Type:          Abs. Dose
	//Data Factor : 1.814
	//Data Unit : Should be mGy
	// Length unit: should be cm
	//No.of Columns : 32
	//  No.of Rows : 32
	//Number of Bodies:   1
	//Operators Note : Corr. : Unif.Calib., Bkgnd, Calib Output, Temp., Press.,

	bool bAbsDose = false;
	double fDataFactor = 0.0;
	QString strDataUnit;
	QString strLengthUnit;
	int iColumns = 0;
	int iRows = 0;
	int iBody = 0;
	QString strNote;

	double fPlanePos = 0.0;

	ifstream fin;
	fin.open(strFilePathOPG.toLocal8Bit().constData());

	if (fin.fail())
	{
		cout << "Error occurred in file reading!" << endl;
		return false;
	}
	char str[MAX_LINE_LENGTH];

	vector<double> vXVal;
	vector<double> vYVal;
	float* pFloatImg = NULL;
	int imgSize = 0;

	while (!fin.eof())
	{
		memset(str, 0, MAX_LINE_LENGTH);
		fin.getline(str, MAX_LINE_LENGTH);
		QString tmpStr = QString(str);
		QString trimmed;
		QStringList strListLine;
		QString strHeader;
		QString strData;

		if (tmpStr.contains("<asciiheader>")) //go into subloop
		{
			while (!fin.eof())
			{
				strHeader = "";
				strData = "";

				memset(str, 0, MAX_LINE_LENGTH);
				fin.getline(str, MAX_LINE_LENGTH);
				tmpStr = QString(str);

				strListLine = tmpStr.split("  ");// at least two consec. spaces

				if (strListLine.count() >= 2)
				{
					strHeader = strListLine.at(0);
					strData = tmpStr.right(tmpStr.length() - strHeader.length());
					strData = strData.trimmed();
				}

				//trimmed = tmpStr.trimmed(); //if any blank line found
				//if (trimmed.length() < 1)
				//    break;

				if (strHeader.contains("Data Type:") && strData.length() > 0) //go into subloop                
				{
					if (strData.contains("Abs. Dose"))
						bAbsDose = true;
					else
						bAbsDose = false;
				}
				else if (strHeader.contains("Data Factor:") && strData.length() > 0) //go into subloop                
				{
					fDataFactor = strData.toDouble();
				}
				else if (strHeader.contains("Data Unit:") && strData.length() > 0) //go into subloop                
				{
					strDataUnit = strData;
					if (!strDataUnit.contains("mGy"))
					{
						cout << "Error! unit should be mGy. Current = " << strDataUnit.toLocal8Bit().constData() << endl;
					}
				}
				else if (strHeader.contains("Length Unit:") && strData.length() > 0) //go into subloop                
				{
					strLengthUnit = strData;

					if (!strLengthUnit.contains("cm"))
					{
						cout << "Error! unit should be cm. Current = " << strLengthUnit.toLocal8Bit().constData() << endl;
					}
				}
				else if (strHeader.contains("No. of Columns:") && strData.length() > 0) //go into subloop                
				{
					iColumns = strData.toInt();
				}
				else if (strHeader.contains("No. of Rows:") && strData.length() > 0) //go into subloop                
				{
					iRows = strData.toInt();
				}
				else if (strHeader.contains("Number of Bodies:") && strData.length() > 0) //go into subloop                
				{
					iBody = strData.toInt();
				}
				else if (strHeader.contains("Operators Note:") && strData.length() > 0) //go into subloop                
				{
					strNote = strData;
				}
				//else if (strHeader.contains("</asciiheader>")) //go into subloop                          
				else if (tmpStr.contains("</asciiheader>")) //go into subloop                                            
				{
					break;
				}
			}// end of while
		}//end of if tmpStr.contains("<asciiheader>"     

		if (tmpStr.contains("<asciibody>")) //go into subloop
		{
			imgSize = iColumns*iRows;

			if (imgSize > 1)
				pFloatImg = new float[imgSize];

			while (!fin.eof())
			{
				memset(str, 0, MAX_LINE_LENGTH);
				fin.getline(str, MAX_LINE_LENGTH);
				tmpStr = QString(str);

				if (tmpStr.contains("X[cm]"))
				{
					strListLine = tmpStr.split(" ");
					int iCntItem = strListLine.count();
					vXVal.clear();

					QStringList strListValidX;
					QString strTestX;

					for (int i = 1; i<iCntItem; i++)
					{
						strTestX = strListLine.at(i);
						cout << strTestX.toLocal8Bit().constData() << endl;

						strTestX = strTestX.trimmed();
						if (strTestX.length() > 0)
						{
							strListValidX.push_back(strTestX);
						}
					}

					int iValidXCnt = strListValidX.count();

					cout << "Valid only" << endl;
					for (int i = 0; i < iValidXCnt; i++)
					{
						vXVal.push_back(strListValidX.at(i).toDouble());
						cout << strListValidX.at(i).toDouble() << endl;
					}

					/*if ((int)(vXVal.size()) != iColumns)
					{
					cout << "Error! X[cm] data count = " << vXVal.size() << ", X header info = " << iColumns << endl;
					return false;
					}    */
				}
				else if (tmpStr.contains("Y[cm]"))
				{
					vYVal.clear();
					int iYLineCnt = 0;

					while (!fin.eof())
					{
						memset(str, 0, MAX_LINE_LENGTH);
						fin.getline(str, MAX_LINE_LENGTH);
						tmpStr = QString(str);

						if (tmpStr.contains("</asciibody>"))
							break;

						strListLine = tmpStr.split("\t");// at least two consec. spaces

						int iDataCnt = strListLine.count();

						if (iDataCnt > 1)
						{
							vYVal.push_back(strListLine.at(0).toDouble());
						}

						QStringList strListValid;
						QString strTest;
						for (int i = 1; i<iDataCnt; i++)
						{
							strTest = strListLine.at(i);
							strTest = strTest.trimmed();
							if (strTest.length() > 0)
							{
								strListValid.push_back(strTest);
							}
						}

						if (strListValid.count() != iColumns)
						{
							cout << "Error! X data count = " << strListValid.count() << " vs " << iColumns << endl;
							return false;
						}

						int curIdx = 0;
						for (int i = 0; i < iColumns; i++)
						{
							curIdx = iColumns*iYLineCnt + i;
							if (curIdx < imgSize)
								pFloatImg[curIdx] = (strListValid.at(i).toDouble())*fDataFactor / 1000.0; //Unit: Gy
						}
						iYLineCnt++;
					}//end of sub while

					if (iYLineCnt != iRows)
					{
						cout << "Error! Y data count = " << iYLineCnt << " vs " << iRows << endl;
						return false;
					}
				}// end of Y cm
			}//end of asciibody while
		}//if asciibody       
	} // main while
	fin.close();

	//img array to itk image

	if (vXVal.size() < 2 || vYVal.size() < 2)
		return false;

	FloatImage2DType::PointType ptOriginItk;
	FloatImage2DType::SizeType imgSizeItk;
	FloatImage2DType::SpacingType imgSpacingItk;

	ptOriginItk[0] = vXVal.at(0)*10.0;
	ptOriginItk[1] = vYVal.at(0)*10.0;

	imgSizeItk[0] = iColumns;
	imgSizeItk[1] = iRows;

	imgSpacingItk[0] = fabs(vXVal.at(1) - vXVal.at(0))*10.0; // cm to mm
	imgSpacingItk[1] = fabs(vYVal.at(1) - vYVal.at(0))*10.0;

	FloatImage2DType::IndexType idxStart;
	idxStart[0] = 0;
	idxStart[1] = 0;

	FloatImage2DType::RegionType region;
	region.SetSize(imgSizeItk);
	region.SetIndex(idxStart);

	FloatImage2DType::Pointer spFloatDoseImg = FloatImage2DType::New();
	spFloatDoseImg->SetRegions(region);
	spFloatDoseImg->SetSpacing(imgSpacingItk);
	spFloatDoseImg->SetOrigin(ptOriginItk);

	spFloatDoseImg->Allocate();
	spFloatDoseImg->FillBuffer(0.0);

	itk::ImageRegionIterator<FloatImage2DType> it(spFloatDoseImg, spFloatDoseImg->GetLargestPossibleRegion());

	int idx = 0;
	for (it.GoToBegin(); !it.IsAtEnd(); ++it)
	{
		if (idx < imgSize)
		{
			float curVal = pFloatImg[idx];
			it.Set(curVal);
		}
		idx++;
	}

	if (pFloatImg != NULL)
		delete[] pFloatImg;

	QUTIL::SaveFloatImage2D(strFilePathMHA.toLocal8Bit().constData(), spFloatDoseImg);

	return true;
}
