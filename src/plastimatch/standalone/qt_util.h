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

    void LoadColorTableFromFile(const char* filePath, vector<VEC3D>& vRGBTable);    
    void LoadColorTableInternal(vector<VEC3D>& vRGBTable, enCOLOR_TABLE col_table);


    VEC3D GetRGBValueFromTable(vector<VEC3D>& vRGBTable, float fMinGray, float fMaxGray, float fLookupGray);

    
    QString GetTimeStampDirPath(const QString& curDirPath, const QString& preFix = QString(""), const QString& endFix = QString(""));

    QString GetTimeStampDirName(const QString& preFix = QString(""), const QString& endFix = QString(""));

    void ShowErrorMessage(QString str);

    void CreateItkDummyImg(FloatImageType::Pointer& spTarget, int sizeX, int sizeY, int sizeZ, float fillVal);//spacing: 1, origin: 0;

    void PrintStrList(QStringList& strList);

    QString GetPathWithEndFix(const QString& curFilePath, const QString& strEndFix);

    void GenDefaultCommandFile(QString strPathCommandFile, enRegisterOption regiOption);

    void GetGeometricLimitFloatImg (FloatImageType::Pointer& spFloatImg, VEC3D& limitStart, VEC3D& limitEnd);

    void Get1DProfileFromTable(QStandardItemModel* pTable, int iCol_X, int iCol_Y, vector<QPointF>& vOutDoseProfile);

    //void UpdateTable3(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3, QTableModel* pTableModel, QTableModel* pTableView);
    //void UpdateFloatTable3(vector<QPointF>& vData1, vector<QPointF>& vData2, vector<QPointF>& vData3,
        //QStandardItemModel* pTableModel, gamma_gui* pParent);


//    typedef itk::ImageFileReader<FloatImageType> ReaderTypeYK;
    bool QPointF_Compare(QPointF& ptData1, QPointF& ptData2);

    void ResampleFloatImg(FloatImageType::Pointer& spFloatInput, FloatImageType::Pointer& spFloatOutput, VEC3D& newSpacing);

    const char *c_str (const QString& s);
};

#define NUM_OF_TBL_ITEM_JET 64
#define NUM_OF_TBL_ITEM_GAMMA 100

//Item: 64
//extern const double colormap_jet[][3] = {
const double colormap_jet[][3] = {
    0, 0, 0.5625,
    0, 0, 0.625,
    0, 0, 0.6875,
    0, 0, 0.75,
    0, 0, 0.8125,
    0, 0, 0.875,
    0, 0, 0.9375,
    0, 0, 1,
    0, 0.0625, 1,
    0, 0.125, 1,
    0, 0.1875, 1,
    0, 0.25, 1,
    0, 0.3125, 1,
    0, 0.375, 1,
    0, 0.4375, 1,
    0, 0.5, 1,
    0, 0.5625, 1,
    0, 0.625, 1,
    0, 0.6875, 1,
    0, 0.75, 1,
    0, 0.8125, 1,
    0, 0.875, 1,
    0, 0.9375, 1,
    0, 1, 1,
    0.0625, 1, 0.9375,
    0.125, 1, 0.875,
    0.1875, 1, 0.8125,
    0.25, 1, 0.75,
    0.3125, 1, 0.6875,
    0.375, 1, 0.625,
    0.4375, 1, 0.5625,
    0.5, 1, 0.5,
    0.5625, 1, 0.4375,
    0.625, 1, 0.375,
    0.6875, 1, 0.3125,
    0.75, 1, 0.25,
    0.8125, 1, 0.1875,
    0.875, 1, 0.125,
    0.9375, 1, 0.0625,
    1, 1, 0,
    1, 0.9375, 0,
    1, 0.875, 0,
    1, 0.8125, 0,
    1, 0.75, 0,
    1, 0.6875, 0,
    1, 0.625, 0,
    1, 0.5625, 0,
    1, 0.5, 0,
    1, 0.4375, 0,
    1, 0.375, 0,
    1, 0.3125, 0,
    1, 0.25, 0,
    1, 0.1875, 0,
    1, 0.125, 0,
    1, 0.0625, 0,
    1, 0, 0,
    0.9375, 0, 0,
    0.875, 0, 0,
    0.8125, 0, 0,
    0.75, 0, 0,
    0.6875, 0, 0,
    0.625, 0, 0,
    0.5625, 0, 0,
    0.5, 0, 0,
};

//Num of item = 100
const double colormap_customgamma[][3] = {
//extern const double colormap_customgamma[][3] = {
    0, 0, 1,
    0, 0.015873016, 0.992063492,
    0, 0.031746032, 0.984126984,
    0, 0.047619048, 0.976190476,
    0, 0.063492063, 0.968253968,
    0, 0.079365079, 0.96031746,
    0, 0.095238095, 0.952380952,
    0, 0.111111111, 0.944444444,
    0, 0.126984127, 0.936507937,
    0, 0.142857143, 0.928571429,
    0, 0.158730159, 0.920634921,
    0, 0.174603175, 0.912698413,
    0, 0.19047619, 0.904761905,
    0, 0.206349206, 0.896825397,
    0, 0.222222222, 0.888888889,
    0, 0.238095238, 0.880952381,
    0, 0.253968254, 0.873015873,
    0, 0.26984127, 0.865079365,
    0, 0.285714286, 0.857142857,
    0, 0.301587302, 0.849206349,
    0, 0.317460317, 0.841269841,
    0, 0.333333333, 0.833333333,
    0, 0.349206349, 0.825396825,
    0, 0.365079365, 0.817460317,
    0, 0.380952381, 0.80952381,
    0, 0.396825397, 0.801587302,
    0, 0.412698413, 0.793650794,
    0, 0.428571429, 0.785714286,
    0, 0.444444444, 0.777777778,
    0, 0.46031746, 0.76984127,
    0, 0.476190476, 0.761904762,
    0, 0.492063492, 0.753968254,
    0, 0.507936508, 0.746031746,
    0, 0.523809524, 0.738095238,
    0, 0.53968254, 0.73015873,
    0, 0.555555556, 0.722222222,
    0, 0.571428571, 0.714285714,
    0, 0.587301587, 0.706349206,
    0, 0.603174603, 0.698412698,
    0, 0.619047619, 0.69047619,
    0, 0.634920635, 0.682539683,
    0, 0.650793651, 0.674603175,
    0, 0.666666667, 0.666666667,
    0, 0.682539683, 0.658730159,
    0, 0.698412698, 0.650793651,
    0, 0.714285714, 0.642857143,
    0, 0.73015873, 0.634920635,
    0, 0.746031746, 0.626984127,
    0, 0.761904762, 0.619047619,
    0, 0.777777778, 0.611111111,
    1, 0.708333333, 0,
    1, 0.6875, 0,
    1, 0.666666667, 0,
    1, 0.645833333, 0,
    1, 0.625, 0,
    1, 0.604166667, 0,
    1, 0.583333333, 0,
    1, 0.5625, 0,
    1, 0.541666667, 0,
    1, 0.520833333, 0,
    1, 0.5, 0,
    1, 0.479166667, 0,
    1, 0.458333333, 0,
    1, 0.4375, 0,
    1, 0.416666667, 0,
    1, 0.395833333, 0,
    1, 0.375, 0,
    1, 0.354166667, 0,
    1, 0.333333333, 0,
    1, 0.3125, 0,
    1, 0.291666667, 0,
    1, 0.270833333, 0,
    1, 0.25, 0,
    1, 0.229166667, 0,
    1, 0.208333333, 0,
    1, 0.1875, 0,
    1, 0.166666667, 0,
    1, 0.145833333, 0,
    1, 0.125, 0,
    1, 0.083333333, 0,
    1, 0.041666667, 0,
    1, 0, 0,
    0.958333333, 0, 0,
    0.916666667, 0, 0,
    0.875, 0, 0,
    0.833333333, 0, 0,
    0.791666667, 0, 0,
    0.75, 0, 0,
    0.708333333, 0, 0,
    0.666666667, 0, 0,
    0.625, 0, 0,
    0.583333333, 0, 0,
    0.541666667, 0, 0,
    0.5, 0, 0,
    0.458333333, 0, 0,
    0.416666667, 0, 0,
    0.375, 0, 0,
    0.333333333, 0, 0,
    0.291666667, 0, 0,
    0.25, 0, 0,
};


#endif // QT_UTIL_H



