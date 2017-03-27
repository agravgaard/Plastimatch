#include "BeamDataRFA.h"

CBeamDataRFA::CBeamDataRFA()
{
    m_ScanType = ST_UNKNOWN;
    m_BeamType = BT_UNKNOWN;
    m_fBeamEnergy = 6.0;
    m_fFieldSizeX_cm = 10.0;
    m_fFieldSizeY_cm = 10.0;
    m_fSSD_cm = 100.0;
    m_iWedgeType = 0; //Open Field

    m_fFixedPosX_mm = 0.0; //DICOM X, mm for PDD
    m_fFixedPosY_mm = 0.0; //DICOM Z, mm for PDD
    m_fFixedPosDepth_mm = 0.0; //Depth for PDD, vertical down is +

    m_fSAD_cm = 100.0;//fixed
}

CBeamDataRFA::~CBeamDataRFA()
{
    m_vDoseProfile.clear();

}

QString CBeamDataRFA::GetBeamName()
{
    //enScanType scanType; //file name with extention
    //enBeamType beamType;
    //float m_fBeamEnergy;
    //float m_fFieldSizeX_cm;
    //float m_fFieldSizeY_cm;
    //float m_fSSD_cm;

    QString strScan, strBeamType, strEnergy, strFS, strDepth;

    switch (m_ScanType)
    {
    case ST_PDD:
        strScan = "PDD";
        break;
    case ST_PROFILE_CR:
        strScan = "ProfileCr";
        break;
    case ST_PROFILE_IN:
        strScan = "ProfileIn";
        break;

    default:
        strScan = "Unknown";
        break;
    }    

    switch (m_BeamType)
    {
    case BT_PHOTON:
        strBeamType = "PHO";
        break;
    case BT_ELECTRON:
        strBeamType = "ELE";
        break;
    case BT_PROTON:
        strBeamType = "PRO";
        break;
    default:
        strBeamType = "Unknown";
        break;
    }

    strEnergy = QString::number(m_fBeamEnergy, 'f', 1);
    strFS.sprintf("%3.1fx%3.1f_cm2", m_fFieldSizeX_cm, m_fFieldSizeY_cm);
        
    strDepth.sprintf("d%3.1fcm", m_fFixedPosDepth_mm/10.0);

    QString finalStr = strBeamType + strEnergy + "_" + strScan + "_" + strFS;

    if (m_ScanType == ST_PROFILE_CR || m_ScanType == ST_PROFILE_IN)
        finalStr = finalStr + "_" + strDepth;

    return finalStr;
}
