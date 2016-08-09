#include "nki2mha_converter.h"
#include <QString>
#include <QFileDialog>
#include <QListView>
#include <QMessageBox>
//#include "YK16GrayImage.h"
#include <fstream>

#include "mha_io.h"
#include "nki_io.h"
//#include "volume.h"
#include "plm_image.h"
#include "rt_study_metadata.h"

nki2mha_converter::nki2mha_converter(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
    ui.setupUi(this);
    //m_pImgOffset = NULL;
    //m_pImgGain = NULL;
    ////Badpixmap;
    //m_pImgOffset = new YK16GrayImage(IMG_WIDTH, IMG_HEIGHT);
    //m_pImgGain = new YK16GrayImage(IMG_WIDTH, IMG_HEIGHT);

    //	const char* inFileName = "C:\\test.scan";
    //	const char* outFileName = "C:\\test.mha";


    /*Volume *v = nki_load (inFileName);
    if (!v)
    {
    printf("file reading error\n");		
    }
    write_mha(outFileName, v);*/


}

nki2mha_converter::~nki2mha_converter()
{
    //delete m_pImgOffset;
    //delete m_pImgGain;

    //m_vPixelReplMap.clear(); //not necessary

}

void nki2mha_converter::SLT_OpenMultipleRaw()
{
    QStringList tmpList = QFileDialog::getOpenFileNames(this,"Select one or more files to open","/home","3D Image file (*.scan *.mha)");

    int iFileCnt = tmpList.size();

    if (iFileCnt < 1)
        return;

    m_strlistPath = tmpList;

    ui.plainTextEdit_Raw->clear();
    ui.plainTextEdit_Corrected->clear();

    for (int i = 0 ; i<iFileCnt ; i++)
    {
        ui.plainTextEdit_Raw->appendPlainText(m_strlistPath.at(i)); //just for display
    }

    //ui.listView_Raw->
    //ui.plainTextEdit_Raw
    //QPlainTextEdit* text;
}
//
QString nki2mha_converter::CorrectSingle_NKI2MHA(const char* filePath)
{
    Volume *v = nki_load (filePath);
    if (!v)
    {
        printf("file reading error\n");	
        return "";
    }	

    //Load raw file		
    //filePath
    //QString exportName = filePath;
    //corrImg.SaveDataAsRaw();
    QString endFix = "_conv";

    QFileInfo srcFileInfo = QFileInfo(filePath);
    QDir dir = srcFileInfo.absoluteDir();
    QString baseName = srcFileInfo.completeBaseName();
    QString extName = "mha";

    QString newFileName = baseName.append(endFix).append(".").append(extName);
    QString newPath = dir.absolutePath() + "/" + newFileName;

    write_mha(newPath.toLocal8Bit().constData(), v);	

    return newPath;
    //corrImg.ReleaseBuffer();
}
//
void nki2mha_converter::SLT_Correct_NKI2MHA()
{
    //1) Load files from m_strlistPath
    //2) offset or gain correction
    //3) bad pixel correction if available
    int listSize = m_strlistPath.size();

    if (listSize < 1)
        return;

    int cnt = 0;
    for (int i = 0 ; i<listSize ; i++)
    {
        QString filePath = m_strlistPath.at(i);
        QString corrFilePath = CorrectSingle_NKI2MHA(filePath.toLocal8Bit().constData());

        if (corrFilePath.length() > 0 )
        {
            ui.plainTextEdit_Corrected->appendPlainText(corrFilePath);
            cnt++;
        }
    }

    QString msgStr = QString("%1 files were converted").arg(cnt);
    QMessageBox::information(this, "Procedure Done",msgStr);

    cout << "MHA Conversion completed" << endl;
}

void nki2mha_converter::SLT_Correct_NKI2DCM()
{	
    int listSize = m_strlistPath.size();

    if (listSize < 1)
        return;

	QString strPatientID = ui.lineEditPatientID->text();
	QString strPatientName = ui.lineEditPatientName->text();

	bool bPatientIDExist = false;

	if (strPatientID.length() > 1 && strPatientName.length() > 1)
	{
	  bPatientIDExist = true;  	  
	}


    int cnt = 0;
    for (int i = 0 ; i<listSize ; i++)
    {
	  if (listSize > 1)
	  {
		int index = i+1;
		strPatientID = strPatientID + QString("%1").arg(index);
	  }

        QString filePath = m_strlistPath.at(i);

        QFileInfo fileInfo  = QFileInfo(filePath);
        QString extName = fileInfo.completeSuffix();
        QString corrFilePath;

        if (extName == "scan" || extName == "SCAN")
        {		 
		  if (bPatientIDExist)
		  	corrFilePath = CorrectSingle_NKI2DCM(filePath.toLocal8Bit().constData(), strPatientID,strPatientName);		  
		  else
			corrFilePath = CorrectSingle_NKI2DCM(filePath.toLocal8Bit().constData());
        }
        else if (extName == "mha" || extName == "MHA")
        {
		  if (bPatientIDExist)
			corrFilePath = CorrectSingle_MHA2DCM(filePath.toLocal8Bit().constData(), strPatientID,strPatientName);
		  else
            corrFilePath = CorrectSingle_MHA2DCM(filePath.toLocal8Bit().constData());			
        }

        if (corrFilePath.length() > 0 )
        {
            ui.plainTextEdit_Corrected->appendPlainText(corrFilePath);
            cnt++;
        }		
    }

    QString msgStr = QString("%1 files were converted").arg(cnt);
    QMessageBox::information(this, "Procedure Done",msgStr);

    cout << "DCM Conversion completed" << endl;
}

void nki2mha_converter::SLT_Correct_NKI2RAW()
{
    int listSize = m_strlistPath.size();

    if (listSize < 1)
        return;


    int cnt = 0;
    for (int i = 0 ; i<listSize ; i++)
    {
        QString filePath = m_strlistPath.at(i);

        //look into extension name:
        QFileInfo fileInfo  = QFileInfo(filePath);
        QString extName = fileInfo.completeSuffix();

        QString corrFilePath;
        if (extName == "scan" || extName == "SCAN")
        {
            corrFilePath = CorrectSingle_NKI2RAW(filePath.toLocal8Bit().constData());			
        }

        else if (extName == "mha" || extName == "MHA")
        {
            //corrFilePath = CorrectSingle_MHA2RAW(filePath.toLocal8Bit().constData());			
        }


        if (corrFilePath.length() > 0 )
        {
            ui.plainTextEdit_Corrected->appendPlainText(corrFilePath);
            cnt++;
        }
    }

    QString msgStr = QString("%1 files were converted").arg(cnt);
    QMessageBox::information(this, "Procedure Done",msgStr);

    cout << "RAW Conversion completed" << endl;
}

QString nki2mha_converter::CorrectSingle_NKI2DCM( const char* filePath )
{

    Volume *v = nki_load (filePath);
    if (!v)
    {
        printf("file reading error\n");	
        return "";
    }	

    Plm_image plm_img(v);

    QString endFix = "_DCM";

    QFileInfo srcFileInfo = QFileInfo(filePath);
    QDir dir = srcFileInfo.absoluteDir();
    QString baseName = srcFileInfo.completeBaseName();	

    baseName.append(endFix);	
    QString newDirPath = dir.absolutePath() + "/" + baseName;	

    QDir dirNew(newDirPath);
    if (!dirNew.exists()){
        dirNew.mkdir(".");
    }		

    Rt_study_metadata rsm;
    rsm.set_patient_id(baseName.toLocal8Bit().constData());
    rsm.set_patient_name(baseName.toLocal8Bit().constData());

    plm_img.save_short_dicom(newDirPath.toLocal8Bit().constData(), &rsm);	

    return newDirPath;
}

QString nki2mha_converter::CorrectSingle_NKI2DCM( const char* filePath, QString patientID, QString patientName )
{
  Volume *v = nki_load (filePath);
  if (!v)
  {
	printf("file reading error\n");	
	return "";
  }	

  Plm_image plm_img(v);

  //QString endFix = "_DCM";

  QFileInfo srcFileInfo = QFileInfo(filePath);
  QDir dir = srcFileInfo.absoluteDir();
  QString baseName = srcFileInfo.completeBaseName();	

  //baseName.append(endFix);	
  


  //baseName.append(endFix);	
  
  QString dirName;

  if (patientID.length() > 1)
	dirName = patientID;
  else
	dirName = baseName;    

  QString newDirPath = dir.absolutePath() + "/" + dirName;

  QDir dirNew(newDirPath);
  if (!dirNew.exists()){
	dirNew.mkdir(".");
  }		

  Rt_study_metadata rsm;
  rsm.set_patient_id(patientID.toLocal8Bit().constData());
  rsm.set_patient_name(patientName.toLocal8Bit().constData());

  plm_img.save_short_dicom(newDirPath.toLocal8Bit().constData(), &rsm);	

  return newDirPath;

}


QString nki2mha_converter::CorrectSingle_MHA2DCM( const char* filePath)
{
    //Volume *v = nki_load (filePath);
    Volume *v  = read_mha(filePath);

    if (!v)
    {
        printf("file reading error\n");	
        return "";
    }	

    Plm_image plm_img(v);

    QString endFix = "_DCM";

    QFileInfo srcFileInfo = QFileInfo(filePath);
    QDir dir = srcFileInfo.absoluteDir();
    QString baseName = srcFileInfo.completeBaseName();	

    baseName.append(endFix);	
    QString newDirPath = dir.absolutePath() + "/" + baseName;	

    QDir dirNew(newDirPath);
    if (!dirNew.exists()){
        dirNew.mkdir(".");
    }
    //ID: baseName.toLocal8Bit().constData();
    //Name: baseName.toLocal8Bit().constData();
    Rt_study_metadata rsm;
    rsm.set_patient_id(baseName.toLocal8Bit().constData());
    rsm.set_patient_name(baseName.toLocal8Bit().constData());

    plm_img.save_short_dicom(newDirPath.toLocal8Bit().constData(), &rsm);	
    //plm_img.save_short_dicom(newDirPath.toLocal8Bit().constData(), 0, baseName.toLocal8Bit().constData(),baseName.toLocal8Bit().constData());	

    return newDirPath;
}

QString nki2mha_converter::CorrectSingle_MHA2DCM( const char* filePath, QString patientID, QString patientName)
{
  //Volume *v = nki_load (filePath);
  Volume *v  = read_mha(filePath);

  if (!v)
  {
	printf("file reading error\n");	
	return "";
  }	

  Plm_image plm_img(v);

  //QString endFix = "_DCM";

  QFileInfo srcFileInfo = QFileInfo(filePath);
  QDir dir = srcFileInfo.absoluteDir();
  QString baseName = srcFileInfo.completeBaseName();	

  //baseName.append(endFix);	
  //QString newDirPath = dir.absolutePath() + "/" + baseName;
  QString dirName;

  if (patientID.length() > 1)
	dirName = patientID;
  else
	dirName = baseName;    
  
  QString newDirPath = dir.absolutePath() + "/" + dirName;


  QDir dirNew(newDirPath);
  if (!dirNew.exists()){
	dirNew.mkdir(".");
  }
  //ID: baseName.toLocal8Bit().constData();
  //Name: baseName.toLocal8Bit().constData();
  Rt_study_metadata rsm;
  rsm.set_patient_id(patientID.toLocal8Bit().constData());
  rsm.set_patient_name(patientName.toLocal8Bit().constData());

  plm_img.save_short_dicom(newDirPath.toLocal8Bit().constData(), &rsm);	
  //plm_img.save_short_dicom(newDirPath.toLocal8Bit().constData(), 0, baseName.toLocal8Bit().constData(),baseName.toLocal8Bit().constData());	

  return newDirPath;
}


QString nki2mha_converter::CorrectSingle_NKI2RAW( const char* filePath )
{
    Volume *v = nki_load (filePath);
    if (!v)
    {
        printf("file reading error\n");	
        return "";
    }	

    Plm_image plm_img(v);

    QString endFix = "_RAW";

    QFileInfo srcFileInfo = QFileInfo(filePath);
    QDir dir = srcFileInfo.absoluteDir();
    QString baseName = srcFileInfo.completeBaseName();	

    baseName.append(endFix);	
    QString newDirPath = dir.absolutePath() + "/" + baseName;	

    QDir dirNew(newDirPath);
    if (!dirNew.exists()){
        dirNew.mkdir(".");
    }

    //cout << v->npix << endl;//44378400 = 410 * 410 * 264
    //cout << v->vox_planes << endl;//0
    //cout <<  v->pix_size << endl;	//2
    //cout << v->dim[0] << v->dim[1] << v->dim[2] << endl; //410 410 264

    int imgWidth = v->dim[0];
    int imgHeight = v->dim[1];
    int imgSliceNum = v->dim[2];

    if ( v->pix_size != 2)//USHORT or short only
    {		
        cout << "not supported file format. only USHORT 16 bit is compatible" << endl;
        return "";
    }

    int img2DSize = imgWidth*imgHeight;	

    for (int k = 0 ; k< imgSliceNum ; k++)
    {
        FILE* fd = NULL;
        QString filePath;
        filePath.sprintf("%s\\image%03d_w%d_h%d.raw", newDirPath.toLocal8Bit().constData(), k, imgWidth, imgHeight);	


        fd = fopen(filePath.toLocal8Bit().constData(), "wb");
        for (int i = 0 ; i<img2DSize ; i++)
        {				
            //fwrite((unsigned short*)(v->img) + ((k*img2DSize+i)*2), 2, 1, fd);		--> Error occurrs
            fwrite((signed short*)(v->img) + (k*img2DSize+i), 2, 1, fd);		
        }
        fclose(fd);
    }

    //Export raw info file in same folder
    QString rawInfoPath;
    rawInfoPath.sprintf("%s\\00RawInfo.txt", newDirPath.toLocal8Bit().constData());

    std::ofstream fout;
    fout.open (rawInfoPath.toLocal8Bit().constData());

    fout << "Raw File Info" << endl;
    fout << "Original_NKI(*.SCAN)_FileName" << "	" << filePath << endl;
    fout << "Pixel_Type" << "	" << "Signed Short" << endl;
    fout << "Image_Width[px]" << "	" << imgWidth << endl;
    fout << "Image_Height[px]" << "	" << imgHeight << endl;
    fout << "Number_of_Slice" << "	" << imgSliceNum << endl;
    fout << "Spacing_X_Y_Z[mm]" << "	" << v->spacing[0]<< "	" << v->spacing[1] << "	" << v->spacing[2] << endl;
    fout << "Bytes_per_Pixel" << "	" << v->pix_size << endl;		

    fout.close();

    return newDirPath;
}
