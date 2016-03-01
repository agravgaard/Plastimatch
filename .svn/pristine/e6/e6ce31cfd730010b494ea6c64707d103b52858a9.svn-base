/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _hnd_io_p_h_
#define _hnd_io_p_h_

#include "plmbase_config.h"

typedef struct hnd_header Hnd_header;
struct hnd_header {
    char sFileType[32];
    uint32_t FileLength;
    char sChecksumSpec[4];
    uint32_t nCheckSum;
    char sCreationDate[8];
    char sCreationTime[8];
    char sPatientID[16];
    uint32_t nPatientSer;
    char sSeriesID[16];
    uint32_t nSeriesSer;
    char sSliceID[16];
    uint32_t nSliceSer;
    uint32_t SizeX;
    uint32_t SizeY;
    double dSliceZPos;
    char sModality[16];
    uint32_t nWindow;
    uint32_t nLevel;
    uint32_t nPixelOffset;
    char sImageType[4];
    double dGantryRtn;
    double dSAD;
    double dSFD;
    double dCollX1;
    double dCollX2;
    double dCollY1;
    double dCollY2;
    double dCollRtn;
    double dFieldX;
    double dFieldY;
    double dBladeX1;
    double dBladeX2;
    double dBladeY1;
    double dBladeY2;
    double dIDUPosLng;
    double dIDUPosLat;
    double dIDUPosVrt;
    double dIDUPosRtn;
    double dPatientSupportAngle;
    double dTableTopEccentricAngle;
    double dCouchVrt;
    double dCouchLng;
    double dCouchLat;
    double dIDUResolutionX;
    double dIDUResolutionY;
    double dImageResolutionX;
    double dImageResolutionY;
    double dEnergy;
    double dDoseRate;
    double dXRayKV;
    double dXRayMA;
    double dMetersetExposure;
    double dAcqAdjustment;
    double dCTProjectionAngle;
    double dCTNormChamber;
    double dGatingTimeTag;
    double dGating4DInfoX;
    double dGating4DInfoY;
    double dGating4DInfoZ;
    double dGating4DInfoTime;
};

#endif
