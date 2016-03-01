#ifndef QT_UTIL_H
#define QT_UTIL_H

#include "yk_config.h"
#include "itk_image_type.h"
//#include "itkImage.h"


#include <QPointF>
#include <QString>
#include <QStringList>

using namespace std;

class QStandardItemModel;
class gamma_gui;

namespace QUTIL{    
    void Set2DTo3D(FloatImage2DType::Pointer& spSrcImg2D, UShortImageType::Pointer& spTargetImg3D, int idx, enPLANE iDirection);    

    void Get2DFrom3DByIndex(UShortImageType::Pointer& spSrcImg3D, UShortImage2DType::Pointer& spTargetImg2D, int idx, enPLANE iDirection);
    void Get2DFrom3DByIndex(FloatImageType::Pointer& spSrcImg3D, FloatImage2DType::Pointer& spTargetImg2D, int idx, enPLANE iDirection);

    void Get2DFrom3DByPosition(UShortImageType::Pointer& spSrcImg3D, UShortImage2DType::Pointer& spTargImg2D, enPLANE iDirection, double pos, double& finalPos);
    void Get2DFrom3DByPosition(FloatImageType::Pointer& spSrcImg3D, FloatImage2DType::Pointer& spTargImg2D, enPLANE iDirection, double pos, double& finalPos);
        
    bool GetProfile1DByPosition(UShortImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, float fixedPos, enPROFILE_DIRECTON enDirection);
    bool GetProfile1DByPosition(FloatImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, float fixedPos, enPROFILE_DIRECTON enDirection);

    bool GetProfile1DByIndex(UShortImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, int fixedIndex, enPROFILE_DIRECTON enDirection);
    bool GetProfile1DByIndex(FloatImage2DType::Pointer& spSrcImg2D, vector<QPointF>& vProfile, int fixedIndex, enPROFILE_DIRECTON enDirection);

    void LoadFloatImage2D(const char* filePath, FloatImage2DType::Pointer& spTargImg2D);
    void LoadFloatImage3D(const char* filePath, FloatImageType::Pointer& spTargImg3D);

    void SaveFloatImage2D(const char* filePath, FloatImage2DType::Pointer& spSrcImg2D);
    void SaveFloatImage3D(const char* filePath, FloatImageType::Pointer& spSrcImg3D);

    QStringList LoadTextFile(const char* txtFilePath);

    void LoadColorTable(const char* filePath, vector<VEC3D>& vRGBTable);
    VEC3D GetRGBValueFromTable(vector<VEC3D>& vRGBTable, float fMinGray, float fMaxGray, float fLookupGray);

    
    QString GetTimeStampDirPath(const QString& curDirPath, const QString& preFix = QString(""), const QString& endFix = QString(""));

    QString GetTimeStampDirName(const QString& preFix = QString(""), const QString& endFix = QString(""));

    void ShowErrorMessage(QString str);

    void CreateItkDummyImg(FloatImageType::Pointer& spTarget, int sizeX, int sizeY, int sizeZ, float fillVal);//spacing: 1, origin: 0;

    void PrintStrList(QStringList& strList);

    QString GetPathWithEndFix(const QString& curFilePath, const QString& strEndFix);

    void GenSampleCommandFile(QString strPathCommandFile, enRegisterOption regiOption);



    //void UpdateTable3(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3, QTableModel* pTableModel, QTableModel* pTableView);
    //void UpdateFloatTable3(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3,
        //QStandardItemModel* pTableModel, gamma_gui* pParent);


//    typedef itk::ImageFileReader<FloatImageType> ReaderTypeYK;


};

#endif // QT_UTIL_H



