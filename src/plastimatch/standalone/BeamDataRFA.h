#pragma once

//#include "yk_config.h"

#include <vector>
#include <QPointF>
#include <QString>

using namespace std;

enum enScanType{
    ST_UNKNOWN = 0,
    ST_PDD,
    ST_PROFILE_CR,
    ST_PROFILE_IN,    
};

enum enBeamType{
    BT_UNKNOWN = 0,
    BT_PHOTON,
    BT_ELECTRON,
    BT_PROTON,
};

class CBeamDataRFA {
public:
    enScanType m_ScanType; //file name with extention
    enBeamType m_BeamType;
    float m_fBeamEnergy;
    float m_fFieldSizeX_cm;
    float m_fFieldSizeY_cm;
    float m_fSSD_cm;
    float m_fSAD_cm;
    vector<QPointF> m_vDoseProfile;
    int m_iWedgeType;

    float m_fFixedPosX_mm; //DICOM X, mm for PDD
    float m_fFixedPosY_mm; //DICOM Z, mm for PDD
    float m_fFixedPosDepth_mm; //Depth for PDD, vertical down is +

public:
    CBeamDataRFA();
    ~CBeamDataRFA();
    QString GetBeamName();
        
};