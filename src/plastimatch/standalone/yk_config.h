#ifndef YK_CONFIG_H
#define YK_CONFIG_H


//#include "itkImage.h"
//#include "itkImageFileReader.h"
//#include "itkImageFileWriter.h"

#include <QString>
#include <QFileInfo>

#define DEFAULT_LABEL_SIZE1 512
#define DEFAULT_LABEL_SIZE2 256
#define DEFAULT_LABEL_SIZE3 256

#define MAX_LINE_LENGTH 1024


#define GY2YKIMG_MAG 700.0 //if 1000: 70Gy --> 70000 --> overflow,  if 500: 70Gy --> 35000 cGy, if 700: 100Gy --> 70000 cGy ->OK
#define GAMMA2YKIMG_MAG 1000.0 //2 --> 512: 0~1: 256, 1-2: 256
#define GY2CGY 100.0;

#define NON_NEG_SHIFT 0.0 //2 --> 512: 0~1: 256, 1-2: 256

#define DEFAULT_LABEL_WIDTH 256
#define DEFAULT_LABEL_HEIGHT 256

struct VEC3D{
    double x;
    double y;
    double z;
};

// 1mm spacing, unit = cGy
class ProtonSetFileMGH{
public:
    VEC3D fDim;
    VEC3D fOrigin;
    VEC3D fSpacing;
    QString strCTDir;
    QString strPathCompDose;
    QString strPathRefDose;
};



enum enPLANE{
    PLANE_AXIAL = 0,
    PLANE_FRONTAL,
    PLANE_SAGITTAL,
};


enum enCOLOR_TABLE{
    COL_TABLE_GAMMA = 0,
    COL_TABLE_JET,
};


enum enPROFILE_DIRECTON{
    PRIFLE_HOR = 0,
    PRIFLE_VER,
};


enum enViewArrange{
    AXIAL_FRONTAL_SAGITTAL = 0,
    FRONTAL_SAGITTAL_AXIAL,
    SAGITTAL_AXIAL_FRONTAL,
};

enum enRegisterOption{
    PLAST_RIGID = 0,
    PLAST_GRADIENT,
    PLAST_AFFINE,
    PLAST_BSPLINE,
};


enum enUpdateDirection{
    GUI2DATA = 0,
    DATA2GUI,    
};


#define DEFAULT_NUM_COLUMN_MAIN 3
#define DEFAULT_MAXNUM_MAIN 50
#define DEFAULT_NUM_COLUMN_QUE 7
#define DEFAULT_MAXNUM_QUE 500


enum enStatus{
    ST_NOT_STARTED = 0,
    ST_PENDING,
    ST_ERROR,
    ST_DONE,
};


enum enPlmCommandInfo{
    PLM_OUTPUT_DIR_PATH = 0,    
    PLM_TEMP1,
    PLM_TEMP2,
};






class CRegiQueString {
public:
    QString m_quePathFixed; //file name with extention
    QString m_quePathMoving;
    QString m_quePathCommand;
    int m_iStatus; //0: not started, 1: pending, 2: done
    double m_fProcessingTime; //sec
    double m_fScore;

public:
    CRegiQueString(){ m_iStatus = ST_NOT_STARTED; m_fScore = 0.0; m_fProcessingTime = 0.0; }
    ~CRegiQueString(){ ; }
    QString GetStrFixed(){
        QFileInfo fInfo(m_quePathFixed);
        return fInfo.fileName();
    }
    QString GetStrMoving(){
        QFileInfo fInfo(m_quePathMoving);
        return fInfo.fileName();
    }
    QString GetStrCommand(){
        QFileInfo fInfo(m_quePathCommand);
        return fInfo.fileName();
    }

    QString GetStrStatus(){
        if (m_iStatus == ST_NOT_STARTED)
            return QString("Wait");
        else if (m_iStatus == ST_PENDING)
            return QString("Pending");
        else if (m_iStatus == ST_DONE)
            return QString("Done");
        else return QString("");
    }

    QString GetStrScore(){
        return QString::number(m_fScore, 'f', 2);
    }

    QString GetStrTime(){
        return QString::number(m_fProcessingTime, 'f', 2);
    }
};


#endif // YK_CONFIG_H



