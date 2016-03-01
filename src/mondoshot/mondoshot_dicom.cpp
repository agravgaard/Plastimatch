/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <string.h>
#include "dcmtk_config.h"
#include "dcmtk/dcmdata/dcvrda.h"
#include "dcmtk/dcmdata/dcvrtm.h"
#include "dcmtk/dcmdata/dctypes.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dctag.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk_uid.h"
#include "plm_uid_prefix.h"

/* DCMTK changed some defines between 3.5.4 and 3.6.0 */
#ifndef DCM_PatientName
#define DCM_ReferringPhysicianName               DcmTagKey(0x0008, 0x0090)
#define DCM_PatientName                          DcmTagKey(0x0010, 0x0010)
#define DCM_PatientBirthDate                     DcmTagKey(0x0010, 0x0030)
#define DCM_PatientSex                           DcmTagKey(0x0010, 0x0040)
#endif

bool 
mondoshot_dicom_create_file (
    int height,
    int width,
    unsigned char* bytes,
    bool use_rgb,
    bool use_rtimage,
    const char *patient_id,
    const char *patient_name,
    const char *filename
)
{
    char uid[100];
    DcmFileFormat fileformat;
    DcmDataset *dataset = fileformat.getDataset();
    const char *uid_root = MONDOSHOT_UID_PREFIX;

    OFString date_string, time_string;
    DcmDate::getCurrentDate (date_string);
    DcmTime::getCurrentTime (time_string);

    /* Is this needed? */
    if (use_rtimage) {
	dataset->putAndInsertString (DCM_ImageType, "ORIGINAL\\PRIMARY\\PORTAL");
    }

    /* At least we can set an instance creation Date/Time.  Should we set Study Date/Time too? */
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, date_string);
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, time_string);

    if (use_rtimage) {
	dataset->putAndInsertString (DCM_SOPClassUID, UID_RTImageStorage);
    } else {
	dataset->putAndInsertString (DCM_SOPClassUID, UID_SecondaryCaptureImageStorage);
    }
    dataset->putAndInsertString (DCM_SOPInstanceUID, dcmtk_uid (uid, uid_root));

    if (use_rtimage) {
	dataset->putAndInsertString (DCM_Modality, "RTIMAGE");
    }
    dataset->putAndInsertString (DCM_ConversionType, "WSD");

    /* Mosaiq seems to like this */
    if (use_rtimage) {
	dataset->putAndInsertString (DCM_Manufacturer, "Varian Medical Systems");
    }

    dataset->putAndInsertString (DCM_ReferringPhysicianName, "");
    dataset->putAndInsertString (DCM_PatientName, patient_name);
    dataset->putAndInsertString (DCM_PatientID, patient_id);
    dataset->putAndInsertString (DCM_PatientBirthDate, "");
    dataset->putAndInsertString (DCM_PatientSex, "");

    /* These should be global for a session?? */
    dataset->putAndInsertString (DCM_StudyInstanceUID, dcmtk_uid (uid, uid_root));
    dataset->putAndInsertString (DCM_SeriesInstanceUID, dcmtk_uid (uid, uid_root));

    dataset->putAndInsertString (DCM_StudyID, "");
    dataset->putAndInsertString (DCM_SeriesNumber, "");
    dataset->putAndInsertString (DCM_InstanceNumber, "");
    dataset->putAndInsertString (DCM_PatientOrientation, "");

    if (use_rgb) {
	dataset->putAndInsertString (DCM_SamplesPerPixel, "3");
	dataset->putAndInsertString (DCM_PhotometricInterpretation, "RGB");
	dataset->putAndInsertString (DCM_PlanarConfiguration, "0");
    } else {
	dataset->putAndInsertString (DCM_SamplesPerPixel, "1");
	dataset->putAndInsertString (DCM_PhotometricInterpretation, "MONOCHROME2");
	dataset->putAndInsertString (DCM_PlanarConfiguration, "0");
    }
    dataset->putAndInsertUint16 (DCM_Rows, (Uint16) height);
    dataset->putAndInsertUint16 (DCM_Columns, (Uint16) width);
    dataset->putAndInsertString (DCM_BitsAllocated, "8");
    dataset->putAndInsertString (DCM_BitsStored, "8");
    dataset->putAndInsertString (DCM_HighBit, "7");
    dataset->putAndInsertString (DCM_PixelRepresentation, "0");

    if (use_rgb) {
	dataset->putAndInsertUint8Array (DCM_PixelData, bytes, height * width * 3);
    } else {
	dataset->putAndInsertUint8Array (DCM_PixelData, bytes, height * width);
    }

    OFCondition status = fileformat.saveFile (filename, EXS_LittleEndianExplicit);
    if (status.bad()) {
	// cerr << "Error: cannot write DICOM file (" << status.text() << ")" << endl;
	return false;
    }
    return true;
}
